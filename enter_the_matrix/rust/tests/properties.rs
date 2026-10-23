//! Algebraic laws checked on random matrices and vectors, over the real and
//! the complex numbers. A wrong formula would almost surely break one of them.

use enter_the_matrix::{angle_cos, cross_product, lerp, linear_combination, Complex, Matrix, Scalar, Vector};

struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u32 {
        self.0 ^= self.0 << 13;
        self.0 ^= self.0 >> 7;
        self.0 ^= self.0 << 17;
        (self.0 >> 11) as u32
    }
    /// Uniform in [-1, 1].
    fn unit(&mut self) -> f32 {
        (self.next() % 20001) as f32 / 10000.0 - 1.0
    }
    fn int(&mut self, bound: u32) -> f32 {
        (self.next() % (2 * bound + 1)) as f32 - bound as f32
    }
    fn complex(&mut self) -> Complex {
        Complex::new(self.unit(), self.unit())
    }
}

/// n x n, strictly diagonally dominant: invertible and well conditioned.
fn dominant<K: Scalar>(n: usize, mut gen: impl FnMut() -> K, boost: K) -> Matrix<K> {
    let mut data: Vec<K> = (0..n * n).map(|_| gen()).collect();
    for i in 0..n {
        data[i * n + i] = data[i * n + i] + boost * K::from_f32(n as f32 + 1.0);
    }
    Matrix::new(n, n, data).unwrap()
}

fn max_abs_diff<K: Scalar>(a: &Matrix<K>, b: &Matrix<K>) -> f32 {
    a.as_slice().iter().zip(b.as_slice()).fold(0.0, |m, (x, y)| {
        let d = (*x - *y).modulus();
        if d > m {
            d
        } else {
            m
        }
    })
}

fn identity<K: Scalar>(n: usize) -> Matrix<K> {
    Matrix::new(n, n, (0..n * n).map(|k| if k % (n + 1) == 0 { K::one() } else { K::zero() }).collect()).unwrap()
}

#[test]
fn inverse_times_matrix_is_identity_real() {
    let mut rng = Rng(1);
    for n in 1..=12 {
        for _ in 0..20 {
            let a = dominant(n, || rng.unit(), 1.0);
            let inv = a.inverse().unwrap();
            assert!(max_abs_diff(&a.mul_mat(&inv), &identity(n)) < 1e-4, "n = {n}");
            assert!(max_abs_diff(&inv.mul_mat(&a), &identity(n)) < 1e-4, "n = {n}");
            assert_eq!(a.rank(), n);
        }
    }
}

#[test]
fn inverse_times_matrix_is_identity_complex() {
    let mut rng = Rng(2);
    for n in 1..=8 {
        for _ in 0..20 {
            let a = dominant(n, || rng.complex(), Complex::new(1.0, 0.0));
            let inv = a.inverse().unwrap();
            assert!(max_abs_diff(&a.mul_mat(&inv), &identity(n)) < 1e-4, "n = {n}");
            assert_eq!(a.rank(), n);
        }
    }
}

#[test]
fn determinant_laws() {
    let mut rng = Rng(3);
    for n in 1..=4 {
        for _ in 0..50 {
            let a = Matrix::new(n, n, (0..n * n).map(|_| rng.int(3)).collect()).unwrap();
            let b = Matrix::new(n, n, (0..n * n).map(|_| rng.int(3)).collect()).unwrap();
            // small integers: everything is exact in f32
            assert_eq!(a.mul_mat(&b).determinant(), a.determinant() * b.determinant());
            assert_eq!(a.transpose().determinant(), a.determinant());
            assert_eq!(a.determinant() == 0.0, a.rank() < n);
        }
    }
}

#[test]
fn determinant_of_5x5_and_6x6_matches_a_laplace_expansion() {
    fn laplace(m: &[f32], n: usize) -> f32 {
        if n == 1 {
            return m[0];
        }
        (0..n)
            .map(|j| {
                let minor: Vec<f32> = (1..n).flat_map(|i| (0..n).filter(move |c| *c != j).map(move |c| (i, c))).map(|(i, c)| m[i * n + c]).collect();
                let s = if j % 2 == 0 { 1.0 } else { -1.0 };
                s * m[j] * laplace(&minor, n - 1)
            })
            .sum()
    }
    let mut rng = Rng(4);
    for n in [5usize, 6] {
        for _ in 0..20 {
            let data: Vec<f32> = (0..n * n).map(|_| rng.int(2)).collect();
            let m = Matrix::new(n, n, data.clone()).unwrap();
            let (got, want) = (m.determinant(), laplace(&data, n));
            assert!((got - want).abs() <= 1e-3 * want.abs().max(1.0), "{n}: {got} vs {want}");
        }
    }
}

#[test]
fn rank_of_a_product_of_thin_matrices() {
    let mut rng = Rng(5);
    for _ in 0..50 {
        let (n, r, m) = (6usize, 2 + (rng.next() % 3) as usize, 7usize);
        let b = Matrix::new(n, r, (0..n * r).map(|_| rng.int(3)).collect()).unwrap();
        let c = Matrix::new(r, m, (0..r * m).map(|_| rng.int(3)).collect()).unwrap();
        let a = b.mul_mat(&c);
        assert!(a.rank() <= r);
        assert_eq!(a.rank(), a.transpose().rank());
        assert_eq!(a.row_echelon().rank(), a.rank());
        let rref = a.row_echelon();
        assert_eq!(rref.row_echelon(), rref); // idempotent
    }
}

#[test]
fn row_echelon_solves_random_systems() {
    let mut rng = Rng(6);
    for n in 1..=8 {
        let a = dominant(n, || rng.unit(), 1.0);
        let x: Vec<f32> = (0..n).map(|_| rng.unit()).collect();
        let b = a.mul_vec(&Vector::new(x.clone()));
        let mut aug = Vec::new();
        for i in 0..n {
            aug.extend_from_slice(a.row(i));
            aug.push(b.as_slice()[i]);
        }
        let r = Matrix::new(n, n + 1, aug).unwrap().row_echelon();
        for i in 0..n {
            assert!((r.row(i)[n] - x[i]).abs() < 1e-3, "n = {n}, x[{i}]");
        }
    }
}

#[test]
fn transpose_and_conjugate_transpose_laws() {
    let mut rng = Rng(7);
    let a = Matrix::new(3, 4, (0..12).map(|_| rng.complex()).collect()).unwrap();
    assert_eq!(a.conjugate_transpose().conjugate_transpose(), a);
    assert_eq!(a.transpose().transpose(), a);
    let b = Matrix::new(4, 2, (0..8).map(|_| rng.complex()).collect()).unwrap();
    // (AB)^H = B^H A^H
    let lhs = a.mul_mat(&b).conjugate_transpose();
    let rhs = b.conjugate_transpose().mul_mat(&a.conjugate_transpose());
    assert!(max_abs_diff(&lhs, &rhs) < 1e-5);
}

#[test]
fn vector_space_laws() {
    let mut rng = Rng(8);
    for _ in 0..200 {
        let u = Vector::new((0..5).map(|_| rng.int(9)).collect());
        let v = Vector::new((0..5).map(|_| rng.int(9)).collect());
        let w = Vector::new((0..5).map(|_| rng.int(9)).collect());
        // u + v = v + u
        let (mut uv, mut vu) = (u.clone(), v.clone());
        uv.add(&v);
        vu.add(&u);
        assert_eq!(uv, vu);
        // (u + v) - v = u
        uv.sub(&v);
        assert_eq!(uv, u);
        // linear_combination agrees with scl + add
        let (a, b) = (rng.int(5), rng.int(5));
        let lc = linear_combination(&[u.clone(), v.clone()], &[a, b]);
        let (mut au, mut bv) = (u.clone(), v.clone());
        au.scl(a);
        bv.scl(b);
        au.add(&bv);
        assert_eq!(lc, au);
        // dot is bilinear and symmetric
        assert_eq!(u.dot(&v), v.dot(&u));
        let mut vw = v.clone();
        vw.add(&w);
        assert_eq!(u.dot(&vw), u.dot(&v) + u.dot(&w));
        // lerp at 0 and 1
        assert_eq!(lerp(u.clone(), v.clone(), 0.0), u);
        assert_eq!(lerp(u.clone(), v.clone(), 1.0), v);
    }
}

#[test]
fn norms_and_cosine_inequalities() {
    let mut rng = Rng(9);
    for _ in 0..500 {
        let u = Vector::new((0..6).map(|_| rng.unit() * 100.0).collect());
        let v = Vector::new((0..6).map(|_| rng.unit() * 100.0).collect());
        // |u.v| <= |u| |v|  (Cauchy-Schwarz)
        assert!(u.dot(&v).abs() <= u.norm() * v.norm() * 1.0001);
        // |u + v| <= |u| + |v|  (triangle)
        let mut s = u.clone();
        s.add(&v);
        assert!(s.norm() <= (u.norm() + v.norm()) * 1.0001);
        // norm_inf <= norm <= norm_1
        assert!(u.norm_inf() <= u.norm() * 1.0001 && u.norm() <= u.norm_1() * 1.0001);
        // |u|^2 = u.u
        assert!((u.norm() * u.norm() - u.dot(&u)).abs() <= 1e-3 * u.dot(&u));
        let c = angle_cos(&u, &v);
        assert!((-1.0..=1.0).contains(&c));
    }
}

#[test]
fn cross_product_laws() {
    let mut rng = Rng(10);
    for _ in 0..500 {
        let u = Vector::new((0..3).map(|_| rng.int(20)).collect());
        let v = Vector::new((0..3).map(|_| rng.int(20)).collect());
        let w = Vector::new((0..3).map(|_| rng.int(20)).collect());
        let uv = cross_product(&u, &v);
        assert_eq!(uv.dot(&u), 0.0);
        assert_eq!(uv.dot(&v), 0.0);
        // Lagrange: |u x v|^2 = |u|^2 |v|^2 - (u.v)^2  (exact on small integers)
        assert_eq!(uv.dot(&uv), u.dot(&u) * v.dot(&v) - u.dot(&v) * u.dot(&v));
        // the triple product is the 3x3 determinant
        let m = Matrix::new(3, 3, [u.as_slice(), v.as_slice(), w.as_slice()].concat()).unwrap();
        assert_eq!(w.dot(&uv), m.determinant());
    }
}

#[test]
fn complex_inner_product_laws() {
    let mut rng = Rng(11);
    let u = Vector::new((0..5).map(|_| rng.complex()).collect());
    let v = Vector::new((0..5).map(|_| rng.complex()).collect());
    let (uv, vu) = (u.dot(&v), v.dot(&u));
    assert!((uv - vu.conj()).modulus() < 1e-5); // <u|v> = conj(<v|u>)
    let uu = u.dot(&u);
    assert!(uu.im == 0.0 && uu.re > 0.0); // <u|u> is real and positive
    assert!((u.norm() * u.norm() - uu.re).abs() < 1e-4);
    assert!((-1.0..=1.0).contains(&angle_cos(&u, &v)));
}

#[test]
fn matrix_product_laws() {
    let mut rng = Rng(12);
    let a = Matrix::new(4, 5, (0..20).map(|_| rng.int(4)).collect()).unwrap();
    let b = Matrix::new(5, 3, (0..15).map(|_| rng.int(4)).collect()).unwrap();
    let c = Matrix::new(3, 6, (0..18).map(|_| rng.int(4)).collect()).unwrap();
    assert_eq!(a.mul_mat(&b).mul_mat(&c), a.mul_mat(&b.mul_mat(&c)));
    assert_eq!(a.mul_mat(&b).transpose(), b.transpose().mul_mat(&a.transpose()));
    assert_eq!(identity::<f32>(4).mul_mat(&a), a);
    assert_eq!(a.mul_mat(&identity::<f32>(5)), a);
    // A*x as a linear combination of the columns of A
    let x = Vector::new((0..5).map(|_| rng.int(4)).collect());
    let cols: Vec<Vector<f32>> = (0..5).map(|j| Vector::new((0..4).map(|i| a.row(i)[j]).collect())).collect();
    assert_eq!(a.mul_vec(&x), linear_combination(&cols, x.as_slice()));
    // Tr(AB) = Tr(BA) even for rectangular A, B
    assert_eq!(a.mul_mat(&b.mul_mat(&c).mul_mat(&Matrix::new(6, 4, vec![1.; 24]).unwrap())).trace(), {
        let ab = a.mul_mat(&b);
        let cd = c.mul_mat(&Matrix::new(6, 4, vec![1.; 24]).unwrap());
        cd.mul_mat(&ab).trace()
    });
}

#[test]
fn big_matrices_are_fast_enough() {
    let mut rng = Rng(13);
    let n = 150;
    let a = dominant(n, || rng.unit(), 1.0);
    let b = dominant(n, || rng.unit(), 1.0);
    let p = a.mul_mat(&b);
    assert_eq!(p.size(), (n, n));
    let inv = a.inverse().unwrap();
    assert!(max_abs_diff(&a.mul_mat(&inv), &identity(n)) < 1e-3);
    assert_eq!(p.rank(), n);
    assert!(p.determinant().is_finite() || p.determinant().is_infinite()); // overflow to inf is fine, no panic
}

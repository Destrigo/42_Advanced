//! Exercise 12 - Inverse
//!
//! The inverse of a square matrix `A` is the matrix `A^-1` that undoes it:
//! `A^-1 A = A A^-1 = I`. It exists exactly when `det(A) != 0` (the map loses
//! no information); a matrix without inverse is *singular* and the function
//! returns an error. With it, `Ab = c` is solved by `b = A^-1 c`.
//!
//! Method: Gauss-Jordan elimination on the augmented matrix `[A | I]`. The row
//! operations that turn `A` into `I` turn `I` into `A^-1` (they are exactly
//! the multiplication by `A^-1`). Pivots are chosen by scaled partial
//! pivoting (see `elim.rs`), which keeps the method stable, and a matrix is
//! declared singular when no usable pivot is left: its best entry in a column
//! is below `n * epsilon` times the size of its row, i.e. it is rounding noise.
//! Because the test is relative, `1e-20 * I` is (correctly) invertible and
//! `[[1,2],[2,4]]` or `[[1,2,3],[4,5,6],[7,8,9]]` are (correctly) not.
//!
//! The row updates use the fused multiply-add (the allowed mathematical
//! function). Complexity: O(n^3) time, O(n^2) memory.
//!
//! A non-square matrix is undefined: `inverse` returns an error for it too
//! (the subject only requires an error for singular matrices; so does a
//! matrix containing NaN or infinity). As the subject warns, the result of a
//! nearly singular or very large matrix is only approximate.

use crate::error::MatrixError;
use crate::matrix::Matrix;
use crate::scalar::Scalar;

impl<K: Scalar> Matrix<K> {
    pub fn inverse(&self) -> Result<Matrix<K>, MatrixError> {
        if !self.is_square() {
            return Err(MatrixError::NotSquare { rows: self.rows, cols: self.cols });
        }
        if self.data.iter().any(|x| !x.is_finite()) {
            return Err(MatrixError::Invalid("the matrix contains NaN or infinity".to_string()));
        }
        let n = self.rows;
        let w = 2 * n;
        let (zero, one) = (K::zero(), K::one());
        // [A | I], row by row
        let mut a = vec![zero; n * w];
        for i in 0..n {
            a[i * w..i * w + n].copy_from_slice(&self.data[i * n..(i + 1) * n]);
            a[i * w + n + i] = one;
        }
        let tol = n as f32 * f32::EPSILON;
        let mut scale: Vec<f32> = (0..n)
            .map(|i| self.data[i * n..(i + 1) * n].iter().fold(0.0f32, |m, x| if x.modulus() > m { x.modulus() } else { m }))
            .collect();
        for c in 0..n {
            let (mut p, mut best) = (c, -1.0f32);
            for i in c..n {
                let ratio = if scale[i] > 0.0 { a[i * w + c].modulus() / scale[i] } else { 0.0 };
                if ratio > best {
                    best = ratio;
                    p = i;
                }
            }
            if !(best > tol) {
                return Err(MatrixError::Singular);
            }
            if p != c {
                for j in 0..w {
                    a.swap(p * w + j, c * w + j);
                }
                scale.swap(p, c);
            }
            let piv = a[c * w + c];
            for j in 0..w {
                a[c * w + j] = a[c * w + j] / piv;
            }
            a[c * w + c] = one;
            scale[c] /= piv.modulus();
            for i in 0..n {
                if i == c {
                    continue;
                }
                let f = a[i * w + c];
                if f == zero {
                    continue;
                }
                for j in 0..w {
                    a[i * w + j] = (-f).fma(a[c * w + j], a[i * w + j]);
                }
                a[i * w + c] = zero;
                scale[i] += f.modulus() * scale[c];
            }
        }
        let mut data = Vec::with_capacity(n * n);
        for i in 0..n {
            data.extend_from_slice(&a[i * w + n..(i + 1) * w]);
        }
        Ok(Matrix { rows: n, cols: n, data })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn assert_close(m: &Matrix<f32>, expected: &[&[f32]]) {
        for (i, row) in expected.iter().enumerate() {
            for (j, e) in row.iter().enumerate() {
                let g = m.row(i)[j];
                assert!((g - e).modulus() <= 1e-5 * e.modulus().max(1.0), "[{i}][{j}] = {g}, expected {e}\n{m}");
            }
        }
    }

    #[test]
    fn matches_subject() {
        let id = Matrix::from([[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]]);
        assert_eq!(id.inverse().unwrap(), id);
        let two = Matrix::from([[2., 0., 0.], [0., 2., 0.], [0., 0., 2.]]);
        assert_eq!(two.inverse().unwrap().to_string(), "[0.5, 0.0, 0.0]\n[0.0, 0.5, 0.0]\n[0.0, 0.0, 0.5]");
        let u = Matrix::from([[8., 5., -2.], [4., 7., 20.], [7., 6., 1.]]);
        assert_close(
            &u.inverse().unwrap(),
            &[
                &[0.649425287, 0.097701149, -0.655172414],
                &[-0.781609195, -0.126436782, 0.965517241],
                &[0.143678161, 0.074712644, -0.206896552],
            ],
        );
    }

    #[test]
    fn product_with_the_inverse_is_the_identity() {
        let u = Matrix::from([[8., 5., -2.], [4., 7., 20.], [7., 6., 1.]]);
        let p = u.mul_mat(&u.inverse().unwrap());
        assert_close(&p, &[&[1., 0., 0.], &[0., 1., 0.], &[0., 0., 1.]]);
        let q = u.inverse().unwrap().mul_mat(&u);
        assert_close(&q, &[&[1., 0., 0.], &[0., 1., 0.], &[0., 0., 1.]]);
    }

    #[test]
    fn needs_a_row_swap() {
        let m = Matrix::from([[0., 1.], [1., 0.]]);
        assert_eq!(m.inverse().unwrap(), m);
        let m = Matrix::from([[0., 2., 0.], [3., 0., 0.], [0., 0., 4.]]);
        assert_close(&m.inverse().unwrap(), &[&[0., 1. / 3., 0.], &[0.5, 0., 0.], &[0., 0., 0.25]]);
    }

    #[test]
    fn singular_matrices_are_errors() {
        assert_eq!(Matrix::from([[1., 2.], [2., 4.]]).inverse(), Err(MatrixError::Singular));
        assert_eq!(Matrix::from([[0., 0.], [0., 0.]]).inverse(), Err(MatrixError::Singular));
        assert_eq!(Matrix::from([[1., 2., 3.], [4., 5., 6.], [7., 8., 9.]]).inverse(), Err(MatrixError::Singular));
        assert_eq!(Matrix::from([[1., 1.], [1., 1.]]).inverse(), Err(MatrixError::Singular));
        assert_eq!(Matrix::from([[0.]]).inverse(), Err(MatrixError::Singular));
    }

    #[test]
    fn scale_independent() {
        assert_close(&Matrix::from([[1e-20, 0.], [0., 1e-20]]).inverse().unwrap(), &[&[1e20, 0.], &[0., 1e20]]);
        assert_close(&Matrix::from([[1e8, 0.], [0., 1.]]).inverse().unwrap(), &[&[1e-8, 0.], &[0., 1.]]);
    }

    #[test]
    fn degenerate_and_invalid_inputs() {
        assert_eq!(Matrix::from([[4.]]).inverse().unwrap(), Matrix::from([[0.25]]));
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().inverse().unwrap().size(), (0, 0));
        assert!(matches!(Matrix::from([[1., 2., 3.], [4., 5., 6.]]).inverse(), Err(MatrixError::NotSquare { .. })));
        assert!(matches!(Matrix::from([[f32::NAN, 0.], [0., 1.]]).inverse(), Err(MatrixError::Invalid(_))));
        assert!(matches!(Matrix::from([[f32::INFINITY, 0.], [0., 1.]]).inverse(), Err(MatrixError::Invalid(_))));
    }
}

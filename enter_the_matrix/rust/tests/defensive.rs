//! Hostile-input tests: every function must either give the right answer or
//! fail in the documented way (a clear panic / `Err` for nonsensical shapes),
//! and must never hang, overflow an index or return garbage silently.

use enter_the_matrix::{
    angle_cos, column_major_text, cross_product, lerp, linear_combination, projection, try_angle_cos, try_cross_product,
    try_lerp, try_linear_combination, try_projection, Complex, Matrix, MatrixError, Vector,
};
use std::panic::catch_unwind;

#[test]
fn constructors_reject_bad_shapes() {
    assert!(Matrix::new(2, 3, vec![1.; 5]).is_err());
    assert!(Matrix::new(2, 3, vec![1.; 7]).is_err());
    assert!(Matrix::<f32>::new(usize::MAX, 2, vec![]).is_err()); // multiplication overflow
    assert!(Matrix::<f32>::new(usize::MAX, usize::MAX, vec![]).is_err());
    assert!(Matrix::from_rows(vec![vec![1., 2.], vec![3.]]).is_err()); // ragged
    assert_eq!(Matrix::<f32>::from_rows(vec![]).unwrap().size(), (0, 0));
    assert!(Vector::from([1., 2., 3.]).reshape(2, 2).is_err());
    assert_eq!(Vector::from([1., 2., 3., 4.]).reshape(2, 2).unwrap(), Matrix::from([[1., 2.], [3., 4.]]));
    assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).to_vector(), Vector::from([1., 2., 3., 4.]));
    assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).to_column_major(), vec![1., 3., 2., 4.]);
    assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).get(1, 0), Some(&3.));
    assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).get(2, 0), None);
    assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).get(0, 9), None);
}

#[test]
fn every_shape_mismatch_is_an_error_not_a_crash() {
    let v2 = Vector::from([1., 2.]);
    let v3 = Vector::from([1., 2., 3.]);
    let m22 = Matrix::from([[1., 2.], [3., 4.]]);
    let m23 = Matrix::from([[1., 2., 3.], [4., 5., 6.]]);
    assert!(v2.clone().try_add(&v3).is_err() && v2.clone().try_sub(&v3).is_err());
    assert!(m22.clone().try_add(&m23).is_err() && m22.clone().try_sub(&m23).is_err());
    assert!(v2.try_dot(&v3).is_err());
    assert!(try_linear_combination(&[v2.clone(), v3.clone()], &[1., 1.]).is_err());
    assert!(try_lerp(v2.clone(), v3.clone(), 0.5).is_err());
    assert!(try_lerp(m22.clone(), m23.clone(), 0.5).is_err());
    assert!(try_angle_cos(&v2, &v3).is_err());
    assert!(try_cross_product(&v2, &v2).is_err());
    assert!(m22.try_mul_vec(&v3).is_err());
    assert!(m22.try_mul_mat(&Matrix::from([[1., 2., 3.]])).is_err());
    assert_eq!(m23.try_trace(), Err(MatrixError::NotSquare { rows: 2, cols: 3 }));
    assert_eq!(m23.try_determinant(), Err(MatrixError::NotSquare { rows: 2, cols: 3 }));
    assert_eq!(m23.inverse(), Err(MatrixError::NotSquare { rows: 2, cols: 3 }));
    assert_eq!(m22.try_trace(), Ok(5.0));
}

#[test]
fn the_subject_named_functions_panic_with_a_message_on_nonsense() {
    let panics = |f: fn()| catch_unwind(f).is_err();
    assert!(panics(|| Vector::from([1.]).add(&Vector::from([1., 2.]))));
    assert!(panics(|| Vector::from([1.]).sub(&Vector::from([1., 2.]))));
    assert!(panics(|| Matrix::from([[1.]]).add(&Matrix::from([[1., 2.]]))));
    assert!(panics(|| { let _ = Vector::from([1.]).dot(&Vector::from([1., 2.])); }));
    assert!(panics(|| { let _ = angle_cos(&Vector::from([0., 0.]), &Vector::from([1., 1.])); }));
    assert!(panics(|| { let _ = cross_product(&Vector::from([1., 2.]), &Vector::from([1., 2.])); }));
    assert!(panics(|| { let _ = lerp(Vector::from([1.]), Vector::from([1., 2.]), 0.5); }));
    assert!(panics(|| { let _ = linear_combination::<f32, Vector<f32>>(&[], &[]); }));
    assert!(panics(|| { let _ = Matrix::from([[1., 2.]]).trace(); }));
    assert!(panics(|| { let _ = Matrix::from([[1., 2.]]).determinant(); }));
    assert!(panics(|| { let _ = projection(0., 1., 1., 2.); }));
}

#[test]
fn empty_inputs_never_panic() {
    let ev = Vector::<f32>::from([]);
    let em = Matrix::<f32>::new(0, 0, vec![]).unwrap();
    assert_eq!(ev.norm_1() + ev.norm() + ev.norm_inf(), 0.0);
    assert_eq!(ev.dot(&ev), 0.0);
    assert_eq!(lerp(ev.clone(), ev.clone(), 0.5), ev);
    assert_eq!(em.trace(), 0.0);
    assert_eq!(em.determinant(), 1.0);
    assert_eq!(em.rank(), 0);
    assert_eq!(em.transpose(), em);
    assert_eq!(em.row_echelon(), em);
    assert_eq!(em.inverse().unwrap(), em);
    assert_eq!(ev.to_string(), "");
    assert_eq!(em.to_string(), "");
    let tall = Matrix::<f32>::new(3, 0, vec![]).unwrap();
    assert_eq!(tall.transpose().size(), (0, 3));
    assert_eq!(tall.rank(), 0);
    assert_eq!(tall.to_string(), "[]\n[]\n[]");
}

#[test]
fn nan_and_infinity_do_not_crash_or_hang() {
    let nan = f32::NAN;
    let inf = f32::INFINITY;
    let m = Matrix::from([[nan, 1.], [2., inf]]);
    let _ = m.determinant();
    let _ = m.rank();
    let _ = m.row_echelon();
    let _ = m.transpose();
    let _ = m.trace();
    assert!(matches!(m.inverse(), Err(MatrixError::Invalid(_))));
    let v = Vector::from([nan, inf, -inf, 1.]);
    let _ = (v.norm_1(), v.norm(), v.norm_inf(), v.dot(&v));
    let _ = lerp(v.clone(), v.clone(), nan);
    let _ = try_angle_cos(&v, &v);
    assert!(Vector::from([inf, inf]).norm().is_infinite());
    assert!(Vector::from([nan]).norm().is_nan());
    let big = Matrix::from([[f32::MAX, f32::MAX], [f32::MAX, f32::MAX]]);
    let _ = (big.determinant(), big.rank(), big.row_echelon(), big.mul_mat(&big));
}

#[test]
fn extreme_magnitudes_stay_correct() {
    // squares overflow / underflow, the norm must not
    assert!((Vector::from([1e-25, 1e-25]).norm() / 1.4142135e-25 - 1.0).abs() < 1e-5);
    assert!((Vector::from([1e25, 1e25]).norm() / 1.4142135e25 - 1.0).abs() < 1e-5);
    // the cosine of huge parallel vectors is still 1
    assert!((angle_cos(&Vector::from([1e30, 1e30]), &Vector::from([2e30, 2e30])) - 1.0).abs() < 1e-5);
    // tiny and huge scales invert correctly
    let tiny = Matrix::from([[1e-30, 0.], [0., 1e-30]]).inverse().unwrap();
    assert!((tiny.row(0)[0] / 1e30 - 1.0).abs() < 1e-5);
    // rank does not depend on the scale of the data
    assert_eq!(Matrix::from([[1e-30, 2e-30], [3e-30, 4e-30]]).rank(), 2);
    assert_eq!(Matrix::from([[1e30, 2e30], [2e30, 4e30]]).rank(), 1);
}

#[test]
fn zero_vectors_and_degenerate_geometry() {
    assert_eq!(cross_product(&Vector::from([0., 0., 0.]), &Vector::from([1., 2., 3.])).as_slice(), &[0., 0., 0.]);
    assert_eq!(Vector::from([0., 0., 0.]).norm_inf(), 0.0);
    assert_eq!(Matrix::from([[0., 0.], [0., 0.]]).determinant(), 0.0);
    assert_eq!(Matrix::from([[0., 0.], [0., 0.]]).trace(), 0.0);
    // negative zero never leaks into the output
    let z = cross_product(&Vector::from([1., 0., 0.]), &Vector::from([1., 0., 0.]));
    assert_eq!(z.to_string(), "[0.0]\n[0.0]\n[0.0]");
    let mut neg = Vector::from([0.0f32]);
    neg.scl(-1.0);
    assert_eq!(neg.to_string(), "[0.0]");
}

#[test]
fn linear_combination_edge_cases() {
    let v = Vector::from([1., 2.]);
    assert_eq!(linear_combination(&[v.clone()], &[0.]), Vector::from([0., 0.]));
    assert_eq!(linear_combination(&[v.clone(), v.clone(), v.clone()], &[1., 1., 1.]), Vector::from([3., 6.]));
    assert_eq!(try_linear_combination::<f32, Vector<f32>>(&[], &[]), Err(MatrixError::Empty));
}

#[test]
fn projection_edge_cases() {
    // extreme but valid parameters must give finite matrices
    for (fov, ratio, near, far) in [(0.001, 1., 1e-3, 1e6), (3.14, 100., 1., 2.), (1., 1e-3, 0.1, 100.)] {
        let p = projection(fov, ratio, near, far);
        assert!(p.as_slice().iter().all(|x| x.is_finite()), "{fov} {ratio} {near} {far}");
    }
    assert!(try_projection(1., 1., 1., f32::MAX).is_ok());
    assert_eq!(column_major_text(&projection(1., 1., 1., 10.)).lines().count(), 4);
}

#[test]
fn complex_numbers_behave_as_a_field() {
    let a = Complex::new(3., 4.);
    let b = Complex::new(1., -2.);
    assert_eq!((a * b) / b, Complex::new(3., 4.));
    assert_eq!(format!("{a} {b}"), "3.0+4.0i 1.0-2.0i");
    let tiny_den = Complex::new(1e-30, 1e-30);
    let q = Complex::new(1., 1.) / tiny_den; // would overflow with the naive formula
    assert!(q.is_finite_for_test());
    assert_eq!(Vector::from([Complex::new(3., 4.)]).norm(), 5.0);
    assert!(Matrix::from([[Complex::new(0., 0.)]]).inverse().is_err());
}

trait FiniteForTest {
    fn is_finite_for_test(&self) -> bool;
}
impl FiniteForTest for Complex {
    fn is_finite_for_test(&self) -> bool {
        self.re.is_finite() && self.im.is_finite()
    }
}

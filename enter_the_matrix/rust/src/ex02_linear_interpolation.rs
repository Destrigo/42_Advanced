//! Exercise 02 - Linear interpolation
//!
//! `lerp(u, v, t)` slides from `u` (at `t = 0`) to `v` (at `t = 1`) along the
//! straight line between them; `t = 0.5` is the exact middle (the
//! isobarycentre). It is the weighted average `(1 - t) * u + t * v`, which is
//! a linear combination with weights summing to 1. Any real `t` is accepted:
//! outside `[0, 1]` it extrapolates along the same infinite line.
//!
//! The formula is evaluated as one fused multiply-add, `fma(1 - t, u, t * v)`
//! per coordinate. Written this way (and not as `u + t * (v - u)`) the
//! endpoints are exact: `t = 0` gives exactly `u` and `t = 1` exactly `v`,
//! which `u + t * (v - u)` does not guarantee in floating point.
//!
//! `lerp` works on scalars, vectors and matrices (any `V: Lerp`).
//!
//! Complexity: O(n) time and memory (n = number of coordinates). Vectors or
//! matrices of different shapes are undefined: `lerp` panics with a message,
//! `try_lerp` returns an error.

use crate::complex::Complex;
use crate::error::{or_panic, MatrixError};
use crate::matrix::Matrix;
use crate::scalar::Scalar;
use crate::vector::Vector;

pub trait Lerp: Sized {
    fn try_lerp(u: Self, v: Self, t: f32) -> Result<Self, MatrixError>;
}

/// `(1 - t) * a + t * b` for one coordinate.
pub fn lerp_scalar<K: Scalar>(a: K, b: K, t: f32) -> K {
    K::from_f32(1.0 - t).fma(a, K::from_f32(t) * b)
}

impl Lerp for f32 {
    fn try_lerp(u: f32, v: f32, t: f32) -> Result<f32, MatrixError> {
        Ok(lerp_scalar(u, v, t))
    }
}

impl Lerp for Complex {
    fn try_lerp(u: Complex, v: Complex, t: f32) -> Result<Complex, MatrixError> {
        Ok(lerp_scalar(u, v, t))
    }
}

impl<K: Scalar> Lerp for Vector<K> {
    fn try_lerp(u: Vector<K>, v: Vector<K>, t: f32) -> Result<Vector<K>, MatrixError> {
        if u.size() != v.size() {
            return Err(MatrixError::Shape(format!("vectors of size {} and {}", u.size(), v.size())));
        }
        Ok(Vector::new(u.data.iter().zip(&v.data).map(|(a, b)| lerp_scalar(*a, *b, t)).collect()))
    }
}

impl<K: Scalar> Lerp for Matrix<K> {
    fn try_lerp(u: Matrix<K>, v: Matrix<K>, t: f32) -> Result<Matrix<K>, MatrixError> {
        if u.size() != v.size() {
            return Err(MatrixError::Shape(format!(
                "matrices of size {}x{} and {}x{}",
                u.rows, u.cols, v.rows, v.cols
            )));
        }
        let data = u.data.iter().zip(&v.data).map(|(a, b)| lerp_scalar(*a, *b, t)).collect();
        Ok(Matrix { rows: u.rows, cols: u.cols, data })
    }
}

pub fn try_lerp<V: Lerp>(u: V, v: V, t: f32) -> Result<V, MatrixError> {
    V::try_lerp(u, v, t)
}

pub fn lerp<V: Lerp>(u: V, v: V, t: f32) -> V {
    or_panic(try_lerp(u, v, t))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(lerp(0., 1., 0.).to_string(), "0");
        assert_eq!(format!("{:?} {:?} {:?} {:?}", lerp(0., 1., 0.), lerp(0., 1., 1.), lerp(0., 1., 0.5), lerp(21., 42., 0.3)), "0.0 1.0 0.5 27.3");
        assert_eq!(lerp(Vector::from([2., 1.]), Vector::from([4., 2.]), 0.3).to_string(), "[2.6]\n[1.3]");
        assert_eq!(
            lerp(Matrix::from([[2., 1.], [3., 4.]]), Matrix::from([[20., 10.], [30., 40.]]), 0.5).to_string(),
            "[11.0, 5.5]\n[16.5, 22.0]"
        );
    }

    #[test]
    fn endpoints_are_exact() {
        for (u, v) in [(0.1f32, 0.3f32), (1e-20, 7.0), (-5.5, 123456.7), (3.0, 3.0)] {
            assert_eq!(lerp(u, v, 0.0), u);
            assert_eq!(lerp(u, v, 1.0), v);
        }
    }

    #[test]
    fn extrapolates_on_the_same_line() {
        assert_eq!(lerp(0., 1., 2.), 2.0);
        assert_eq!(lerp(0., 1., -1.), -1.0);
    }

    #[test]
    fn mismatched_shapes_are_errors() {
        assert!(try_lerp(Vector::from([1.]), Vector::from([1., 2.]), 0.5).is_err());
        assert!(try_lerp(Matrix::from([[1.]]), Matrix::from([[1., 2.]]), 0.5).is_err());
    }
}

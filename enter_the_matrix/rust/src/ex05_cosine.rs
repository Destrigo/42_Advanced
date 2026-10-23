//! Exercise 05 - Cosine
//!
//! The cosine of the angle between two vectors, from the dot product alone
//! (the standard `cos` is forbidden, and not needed):
//!
//! `cos(u, v) = <u|v> / (|u| * |v|)`
//!
//! It is 1 for vectors pointing the same way, 0 for perpendicular ones, -1 for
//! opposite ones, and does not depend on the lengths. In statistics the same
//! formula with covariance as the dot product is the Pearson correlation
//! coefficient.
//!
//! It uses `dot` (exercise 03) and `norm` (exercise 04), as the subject
//! suggests. Rounding can push a mathematically exact 1 slightly outside
//! `[-1, 1]` (e.g. `1.0000001`), so the result is clamped to that range.
//!
//! Over complex vectors the result is the real part of the normalised inner
//! product.
//!
//! Complexity: O(n) time, O(1) extra memory. A zero vector (no direction) or
//! different dimensions are undefined: `angle_cos` panics with a message,
//! `try_angle_cos` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::scalar::Scalar;
use crate::vector::Vector;

pub fn try_angle_cos<K: Scalar>(u: &Vector<K>, v: &Vector<K>) -> Result<f32, MatrixError> {
    let dot = u.try_dot(v)?.re();
    let (nu, nv) = (u.norm(), v.norm());
    if nu == 0.0 || nv == 0.0 {
        return Err(MatrixError::Invalid("the angle with a zero vector is undefined".to_string()));
    }
    let c = dot / nu / nv;
    Ok(if c > 1.0 {
        1.0
    } else if c < -1.0 {
        -1.0
    } else {
        c
    })
}

pub fn angle_cos<K: Scalar>(u: &Vector<K>, v: &Vector<K>) -> f32 {
    or_panic(try_angle_cos(u, v))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(angle_cos(&Vector::from([1., 0.]), &Vector::from([1., 0.])), 1.0);
        assert_eq!(angle_cos(&Vector::from([1., 0.]), &Vector::from([0., 1.])), 0.0);
        assert_eq!(angle_cos(&Vector::from([-1., 1.]), &Vector::from([1., -1.])), -1.0);
        assert_eq!(angle_cos(&Vector::from([2., 1.]), &Vector::from([4., 2.])), 1.0);
        let c = angle_cos(&Vector::from([1., 2., 3.]), &Vector::from([4., 5., 6.]));
        assert!((c - 0.974631846).modulus() < 1e-6, "{c}");
    }

    #[test]
    fn always_within_bounds() {
        // parallel vectors with awkward magnitudes must never give |cos| > 1
        for k in [0.1f32, 0.3, 3.0, 7.7, 1e10, 1e-10] {
            let u = Vector::from([1.1, 2.2, 3.3]);
            let mut v = u.clone();
            v.scl(k);
            let c = angle_cos(&u, &v);
            assert!((-1.0..=1.0).contains(&c) && c > 0.9999, "{k}: {c}");
        }
    }

    #[test]
    fn undefined_cases_are_errors() {
        assert!(try_angle_cos(&Vector::from([0., 0.]), &Vector::from([1., 1.])).is_err());
        assert!(try_angle_cos(&Vector::from([1., 1.]), &Vector::from([0., 0.])).is_err());
        assert!(try_angle_cos(&Vector::from([1.]), &Vector::from([1., 1.])).is_err());
        assert!(try_angle_cos(&Vector::<f32>::from([]), &Vector::from([])).is_err());
    }
}

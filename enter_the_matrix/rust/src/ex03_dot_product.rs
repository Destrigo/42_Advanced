//! Exercise 03 - Dot product
//!
//! The dot (inner) product of two vectors of the same dimension:
//! `<u|v> = u1*v1 + ... + un*vn`. It turns two vectors into a scalar and
//! carries the geometry of the space: it gives lengths (`<v|v> = |v|^2`) and
//! angles (`<u|v> = |u| |v| cos(angle)`, exercise 05), and two vectors are
//! perpendicular exactly when it is zero. Matrix products are just many dot
//! products (one per row of the left matrix and column of the right one).
//!
//! The sum is accumulated with one fused multiply-add per coordinate
//! (`acc = fma(u_i, v_i, acc)`), the allowed mathematical function.
//!
//! Over the complex numbers the first argument is conjugated (sesquilinear
//! inner product), see `complex.rs`; over the reals this changes nothing.
//!
//! Complexity: O(n) time, O(1) extra memory. Different dimensions are
//! undefined: `dot` panics with a message, `try_dot` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::scalar::Scalar;
use crate::vector::Vector;

impl<K: Scalar> Vector<K> {
    pub fn try_dot(&self, v: &Vector<K>) -> Result<K, MatrixError> {
        if self.size() != v.size() {
            return Err(MatrixError::Shape(format!("vectors of size {} and {}", self.size(), v.size())));
        }
        Ok(self.data.iter().zip(&v.data).fold(K::zero(), |acc, (a, b)| a.conj().fma(*b, acc)))
    }

    pub fn dot(&self, v: &Vector<K>) -> K {
        or_panic(self.try_dot(v))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(Vector::from([0., 0.]).dot(&Vector::from([1., 1.])), 0.0);
        assert_eq!(Vector::from([1., 1.]).dot(&Vector::from([1., 1.])), 2.0);
        assert_eq!(Vector::from([-1., 6.]).dot(&Vector::from([3., 2.])), 9.0);
    }

    #[test]
    fn properties() {
        let u = Vector::from([1., -2., 3.5]);
        let v = Vector::from([4., 0.5, -1.]);
        assert_eq!(u.dot(&v), v.dot(&u)); // symmetric
        assert_eq!(Vector::from([1., 0.]).dot(&Vector::from([0., 1.])), 0.0); // perpendicular
        assert_eq!(Vector::<f32>::from([]).dot(&Vector::from([])), 0.0); // empty sum
    }

    #[test]
    fn mismatched_dimensions_are_errors() {
        assert!(Vector::from([1.]).try_dot(&Vector::from([1., 2.])).is_err());
    }
}

//! Exercise 00 - Add, Subtract and Scale
//!
//! The three basic operations of a vector space, for vectors and for matrices
//! (matrices of a given shape are a vector space too):
//! - addition `u + v`, coordinate by coordinate (same size required);
//! - subtraction `u - v`;
//! - scaling `a * u`: every coordinate is multiplied by the scalar `a`.
//!
//! Like the prototypes of the subject, the functions work in place: `self` is
//! the accumulator. No mathematical function is used, only `+ - *`.
//!
//! Complexity: one pass over the coordinates, O(n) time and no extra memory.
//!
//! Sizes that do not match make no sense: `add`, `sub` and `scl` panic with a
//! clear message (undefined behaviour in the subject); `try_add` and
//! `try_sub` return an error instead and leave `self` untouched.

use crate::error::{or_panic, MatrixError};
use crate::matrix::Matrix;
use crate::scalar::Scalar;
use crate::vector::Vector;

impl<K: Scalar> Vector<K> {
    pub fn try_add(&mut self, v: &Vector<K>) -> Result<(), MatrixError> {
        if self.size() != v.size() {
            return Err(MatrixError::Shape(format!("vectors of size {} and {}", self.size(), v.size())));
        }
        for (a, b) in self.data.iter_mut().zip(&v.data) {
            *a = *a + *b;
        }
        Ok(())
    }

    pub fn try_sub(&mut self, v: &Vector<K>) -> Result<(), MatrixError> {
        if self.size() != v.size() {
            return Err(MatrixError::Shape(format!("vectors of size {} and {}", self.size(), v.size())));
        }
        for (a, b) in self.data.iter_mut().zip(&v.data) {
            *a = *a - *b;
        }
        Ok(())
    }

    pub fn add(&mut self, v: &Vector<K>) {
        or_panic(self.try_add(v))
    }

    pub fn sub(&mut self, v: &Vector<K>) {
        or_panic(self.try_sub(v))
    }

    pub fn scl(&mut self, a: K) {
        for x in self.data.iter_mut() {
            *x = *x * a;
        }
    }
}

impl<K: Scalar> Matrix<K> {
    fn check_same_shape(&self, v: &Matrix<K>) -> Result<(), MatrixError> {
        if self.size() != v.size() {
            return Err(MatrixError::Shape(format!(
                "matrices of size {}x{} and {}x{}",
                self.rows, self.cols, v.rows, v.cols
            )));
        }
        Ok(())
    }

    pub fn try_add(&mut self, v: &Matrix<K>) -> Result<(), MatrixError> {
        self.check_same_shape(v)?;
        for (a, b) in self.data.iter_mut().zip(&v.data) {
            *a = *a + *b;
        }
        Ok(())
    }

    pub fn try_sub(&mut self, v: &Matrix<K>) -> Result<(), MatrixError> {
        self.check_same_shape(v)?;
        for (a, b) in self.data.iter_mut().zip(&v.data) {
            *a = *a - *b;
        }
        Ok(())
    }

    pub fn add(&mut self, v: &Matrix<K>) {
        or_panic(self.try_add(v))
    }

    pub fn sub(&mut self, v: &Matrix<K>) {
        or_panic(self.try_sub(v))
    }

    pub fn scl(&mut self, a: K) {
        for x in self.data.iter_mut() {
            *x = *x * a;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn vectors_match_subject() {
        let mut u = Vector::from([2., 3.]);
        u.add(&Vector::from([5., 7.]));
        assert_eq!(u.to_string(), "[7.0]\n[10.0]");
        let mut u = Vector::from([2., 3.]);
        u.sub(&Vector::from([5., 7.]));
        assert_eq!(u.to_string(), "[-3.0]\n[-4.0]");
        let mut u = Vector::from([2., 3.]);
        u.scl(2.);
        assert_eq!(u.to_string(), "[4.0]\n[6.0]");
    }

    #[test]
    fn matrices_match_subject() {
        let v = Matrix::from([[7., 4.], [-2., 2.]]);
        let mut u = Matrix::from([[1., 2.], [3., 4.]]);
        u.add(&v);
        assert_eq!(u.to_string(), "[8.0, 6.0]\n[1.0, 6.0]");
        let mut u = Matrix::from([[1., 2.], [3., 4.]]);
        u.sub(&v);
        assert_eq!(u.to_string(), "[-6.0, -2.0]\n[5.0, 2.0]");
        let mut u = Matrix::from([[1., 2.], [3., 4.]]);
        u.scl(2.);
        assert_eq!(u.to_string(), "[2.0, 4.0]\n[6.0, 8.0]");
    }

    #[test]
    fn mismatched_sizes_are_errors_and_leave_self_untouched() {
        let mut u = Vector::from([1., 2.]);
        assert!(u.try_add(&Vector::from([1., 2., 3.])).is_err());
        assert!(u.try_sub(&Vector::from([1.])).is_err());
        assert_eq!(u, Vector::from([1., 2.]));
        let mut m = Matrix::from([[1., 2.], [3., 4.]]);
        assert!(m.try_add(&Matrix::from([[1., 2., 3.], [4., 5., 6.]])).is_err());
        assert!(m.try_sub(&Matrix::from([[1., 2.]])).is_err());
        assert_eq!(m, Matrix::from([[1., 2.], [3., 4.]]));
    }

    #[test]
    #[should_panic(expected = "incompatible shapes")]
    fn mismatched_sizes_panic_with_a_message() {
        Vector::from([1.]).add(&Vector::from([1., 2.]));
    }

    #[test]
    fn empty_and_extreme_values() {
        let mut e: Vector<f32> = Vector::from([]);
        e.add(&Vector::from([]));
        e.scl(3.);
        assert!(e.is_empty());
        let mut u = Vector::from([f32::MAX, f32::NAN, -0.0]);
        u.scl(2.);
        assert_eq!(u.as_slice()[0], f32::INFINITY);
        assert!(u.as_slice()[1].is_nan());
    }
}

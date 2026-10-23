//! Exercise 08 - Trace
//!
//! The trace of a square matrix is the sum of its diagonal:
//! `Tr(A) = A_11 + A_22 + ... + A_nn`. It is linear (`Tr(A+B) = Tr(A)+Tr(B)`),
//! satisfies `Tr(AB) = Tr(BA)`, does not change when the basis changes, and
//! equals the sum of the eigenvalues.
//!
//! No mathematical function is allowed here, just additions.
//!
//! Complexity: O(n) time, O(1) extra memory. A matrix that is not square is
//! undefined: `trace` panics with a message, `try_trace` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::matrix::Matrix;
use crate::scalar::Scalar;

impl<K: Scalar> Matrix<K> {
    pub fn try_trace(&self) -> Result<K, MatrixError> {
        if !self.is_square() {
            return Err(MatrixError::NotSquare { rows: self.rows, cols: self.cols });
        }
        let mut sum = K::zero();
        for i in 0..self.rows {
            sum = sum + self.data[i * self.cols + i];
        }
        Ok(sum)
    }

    pub fn trace(&self) -> K {
        or_panic(self.try_trace())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(Matrix::from([[1., 0.], [0., 1.]]).trace(), 2.0);
        assert_eq!(Matrix::from([[2., -5., 0.], [4., 3., 7.], [-2., 3., 4.]]).trace(), 9.0);
        assert_eq!(Matrix::from([[-2., -8., 4.], [1., -23., 4.], [0., 6., 4.]]).trace(), -21.0);
    }

    #[test]
    fn properties() {
        let a = Matrix::from([[1., 2.], [3., 4.]]);
        let b = Matrix::from([[0., 5.], [-1., 2.]]);
        assert_eq!(a.mul_mat(&b).trace(), b.mul_mat(&a).trace()); // Tr(AB) = Tr(BA)
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().trace(), 0.0);
        assert_eq!(Matrix::from([[7.]]).trace(), 7.0);
    }

    #[test]
    fn non_square_is_an_error() {
        assert_eq!(
            Matrix::from([[1., 2., 3.], [4., 5., 6.]]).try_trace(),
            Err(MatrixError::NotSquare { rows: 2, cols: 3 })
        );
    }
}

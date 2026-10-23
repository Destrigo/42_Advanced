//! Exercise 13 - Rank
//!
//! The rank of a matrix is the dimension of the image of the linear map it
//! represents: how many independent directions the output actually spans (the
//! number of linearly independent columns, which is also the number of
//! linearly independent rows). A 3x3 matrix of rank 3 maps space onto space; of
//! rank 2 it flattens it onto a plane; of rank 1 onto a line; of rank 0 (only
//! the zero matrix) onto the origin. A square matrix is invertible exactly
//! when its rank is full.
//!
//! The rank-nullity theorem ties it to the kernel (the vectors squashed onto
//! the origin): `rank + nullity = number of columns`.
//!
//! Method: Gaussian elimination (see `elim.rs`); the rank is the number of
//! pivots found. No mathematical function is allowed. Complexity: O(n^3) time
//! for an n x n matrix; one working copy of the matrix is used.

use crate::elim::rref;
use crate::matrix::Matrix;
use crate::scalar::Scalar;

impl<K: Scalar> Matrix<K> {
    pub fn rank(&self) -> usize {
        let mut work = self.data.clone();
        rref(&mut work, self.rows, self.cols)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(Matrix::from([[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]]).rank(), 3);
        assert_eq!(Matrix::from([[1., 2., 0., 0.], [2., 4., 0., 0.], [-1., 2., 1., 1.]]).rank(), 2);
        assert_eq!(Matrix::from([[8., 5., -2.], [4., 7., 20.], [7., 6., 1.], [21., 18., 7.]]).rank(), 3);
    }

    #[test]
    fn edge_cases() {
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().rank(), 0);
        assert_eq!(Matrix::<f32>::new(3, 0, vec![]).unwrap().rank(), 0);
        assert_eq!(Matrix::from([[0., 0.], [0., 0.]]).rank(), 0);
        assert_eq!(Matrix::from([[0.]]).rank(), 0);
        assert_eq!(Matrix::from([[7.]]).rank(), 1);
        assert_eq!(Matrix::from([[1., 2., 3.]]).rank(), 1); // one row
        assert_eq!(Matrix::from([[1.], [2.], [3.]]).rank(), 1); // one column
    }

    #[test]
    fn rounding_noise_is_not_rank() {
        assert_eq!(Matrix::from([[1., 2., 3.], [4., 5., 6.], [7., 8., 9.]]).rank(), 2);
        assert_eq!(Matrix::from([[0.1, 0.2, 0.3], [0.4, 0.5, 0.6], [0.7, 0.8, 0.9]]).rank(), 2);
        assert_eq!(Matrix::from([[1., 1.], [1., 1.]]).rank(), 1);
        assert_eq!(Matrix::from([[1e8, 0.], [0., 1.]]).rank(), 2); // different scales, still full rank
    }

    #[test]
    fn rank_laws() {
        let a = Matrix::from([[1., 2., 3.], [4., 5., 6.]]);
        assert_eq!(a.rank(), a.transpose().rank()); // row rank = column rank
        assert!(a.rank() <= 2);
        let b = Matrix::from([[1., 0.], [0., 1.], [1., 1.]]);
        assert!(a.mul_mat(&b).rank() <= a.rank().min(b.rank())); // rank(AB) <= min
        let rows = a.row_echelon();
        assert_eq!(rows.rank(), a.rank()); // elimination does not change it
    }
}

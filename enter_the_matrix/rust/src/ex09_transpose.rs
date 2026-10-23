//! Exercise 09 - Transpose
//!
//! The transpose of an m x n matrix `A` is the n x m matrix `B` with
//! `B_ji = A_ij`: rows become columns. It satisfies `(AB)^T = B^T A^T`,
//! `(A^T)^T = A`, and swaps the roles of the two dot-product sides
//! (`<Au|v> = <u|A^T v>` over the reals).
//!
//! `transpose` is the plain transpose, valid over any field. Over the complex
//! numbers the matching operation is the conjugate transpose (every entry is
//! also conjugated), provided as `conjugate_transpose`; on real matrices both
//! are identical.
//!
//! No mathematical function is allowed here. The time complexity counts value
//! assignments: one per entry, O(nm), and O(nm) memory for the new matrix.

use crate::matrix::Matrix;
use crate::scalar::Scalar;

impl<K: Scalar> Matrix<K> {
    pub fn transpose(&self) -> Matrix<K> {
        let mut data = Vec::with_capacity(self.data.len());
        for j in 0..self.cols {
            for i in 0..self.rows {
                data.push(self.data[i * self.cols + j]);
            }
        }
        Matrix { rows: self.cols, cols: self.rows, data }
    }

    pub fn conjugate_transpose(&self) -> Matrix<K> {
        let mut t = self.transpose();
        for x in t.data.iter_mut() {
            *x = x.conj();
        }
        t
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn square_and_rectangular() {
        assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).transpose(), Matrix::from([[1., 3.], [2., 4.]]));
        let a = Matrix::from([[1., 2., 3.], [4., 5., 6.]]);
        assert_eq!(a.transpose(), Matrix::from([[1., 4.], [2., 5.], [3., 6.]]));
        assert_eq!(a.transpose().transpose(), a);
        assert_eq!(a.transpose().to_string(), "[1.0, 4.0]\n[2.0, 5.0]\n[3.0, 6.0]");
    }

    #[test]
    fn degenerate_shapes() {
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().transpose().size(), (0, 0));
        assert_eq!(Matrix::<f32>::new(2, 0, vec![]).unwrap().transpose().size(), (0, 2));
        assert_eq!(Matrix::from([[1., 2., 3.]]).transpose().size(), (3, 1));
    }

    #[test]
    fn product_rule() {
        let a = Matrix::from([[1., 2., 3.], [4., 5., 6.]]);
        let b = Matrix::from([[1., 0.], [2., 1.], [0., 3.]]);
        assert_eq!(a.mul_mat(&b).transpose(), b.transpose().mul_mat(&a.transpose()));
    }
}

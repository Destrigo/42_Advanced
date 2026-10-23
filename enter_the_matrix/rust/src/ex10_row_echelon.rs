//! Exercise 10 - Row-echelon form
//!
//! Gauss-Jordan elimination: the matrix is transformed with the three row
//! operations that do not change the solutions of the linear system it
//! represents (swap two rows, scale a row by a non-zero number, add a multiple
//! of a row to another) until it is in *reduced* row echelon form, the one
//! shown by the examples of the subject:
//! - the first non-zero entry of each non-zero row (its pivot) is 1;
//! - each pivot is the only non-zero entry of its column;
//! - pivots go down and to the right, and zero rows are at the bottom.
//!
//! Algorithm, column by column: choose the pivot (scaled partial pivoting, see
//! `elim.rs`), swap it up, divide its row by the pivot, and subtract multiples
//! of that row from all the others to clear the column. Applied to the
//! augmented matrix `[A | c]` of a system `Ab = c`, the last column of the
//! result is the solution (see the interlude of the subject).
//!
//! No mathematical function is allowed. Complexity: O(n^3) time (for an n x n
//! matrix, O(mn * min(m, n)) in general), O(n^2) memory (one copy of the
//! matrix). The results are approximate in the usual floating point sense.

use crate::elim::rref;
use crate::matrix::Matrix;
use crate::scalar::Scalar;

impl<K: Scalar> Matrix<K> {
    pub fn row_echelon(&self) -> Matrix<K> {
        let mut out = self.clone();
        rref(&mut out.data, out.rows, out.cols);
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn assert_close(m: &Matrix<f32>, expected: &[&[f32]]) {
        assert_eq!(m.size(), (expected.len(), expected.first().map_or(0, |r| r.len())));
        for (i, row) in expected.iter().enumerate() {
            for (j, e) in row.iter().enumerate() {
                let g = m.row(i)[j];
                assert!((g - e).modulus() <= 1e-4 * e.modulus().max(1.0), "[{i}][{j}] = {g}, expected {e}\n{m}");
            }
        }
    }

    #[test]
    fn matches_subject() {
        let id = Matrix::from([[1., 0., 0.], [0., 1., 0.], [0., 0., 1.]]);
        assert_eq!(id.row_echelon(), id);
        assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).row_echelon().to_string(), "[1.0, 0.0]\n[0.0, 1.0]");
        assert_eq!(Matrix::from([[1., 2.], [2., 4.]]).row_echelon().to_string(), "[1.0, 2.0]\n[0.0, 0.0]");
        let u = Matrix::from([[8., 5., -2., 4., 28.], [4., 2.5, 20., 4., -4.], [8., 5., 1., 4., 17.]]);
        assert_close(
            &u.row_echelon(),
            &[&[1., 0.625, 0., 0., -12.1666667], &[0., 0., 1., 0., -3.6666667], &[0., 0., 0., 1., 29.5]],
        );
    }

    #[test]
    fn noise_is_not_mistaken_for_a_pivot() {
        let m = Matrix::from([[1., 2., 3.], [4., 5., 6.], [7., 8., 9.]]);
        assert_close(&m.row_echelon(), &[&[1., 0., -1.], &[0., 1., 2.], &[0., 0., 0.]]);
    }

    #[test]
    fn scale_independent() {
        assert_close(&Matrix::from([[1e8, 0.], [0., 1.]]).row_echelon(), &[&[1., 0.], &[0., 1.]]);
        assert_close(&Matrix::from([[1e-20, 0.], [0., 1e-20]]).row_echelon(), &[&[1., 0.], &[0., 1.]]);
    }

    #[test]
    fn zero_and_degenerate_matrices() {
        assert_eq!(Matrix::from([[0., 0.], [0., 0.]]).row_echelon(), Matrix::from([[0., 0.], [0., 0.]]));
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().row_echelon().size(), (0, 0));
        assert_eq!(Matrix::<f32>::new(3, 0, vec![]).unwrap().row_echelon().size(), (3, 0));
        assert_eq!(Matrix::from([[5.]]).row_echelon(), Matrix::from([[1.]]));
        assert_eq!(Matrix::from([[0., 3.]]).row_echelon(), Matrix::from([[0., 1.]]));
    }

    #[test]
    fn row_swaps_and_idempotence() {
        let m = Matrix::from([[0., 1., 2.], [3., 4., 5.], [6., 7., 9.]]);
        let r = m.row_echelon();
        assert_close(&r, &[&[1., 0., 0.], &[0., 1., 0.], &[0., 0., 1.]]);
        assert_eq!(r.row_echelon(), r);
    }

    #[test]
    fn solves_a_system() {
        // 2x + 3y = 8, -7x + 4y = 2  ->  x = 26/29, y = 60/29 (interlude of the subject)
        let aug = Matrix::from([[2., 3., 8.], [-7., 4., 2.]]);
        assert_close(&aug.row_echelon(), &[&[1., 0., 26. / 29.], &[0., 1., 60. / 29.]]);
    }
}

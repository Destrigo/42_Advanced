//! Exercise 11 - Determinant
//!
//! The determinant of a square matrix is the signed factor by which the
//! linear map it represents scales volumes (areas in 2D): the unit square /
//! cube is mapped to a parallelogram / parallelepiped of that (signed) volume,
//! and a negative sign means the orientation was flipped (a reflection).
//!
//! - `det(A) = 0` means the map squashes space into a lower dimension (a plane
//!   into a line, a volume into a plane...): the columns are linearly
//!   dependent, information is lost, so the matrix has no inverse (it is
//!   *singular*) and the system `Ax = b` has no unique solution; it is also
//!   the matrix whose rank is below n.
//! - `det(AB) = det(A) det(B)` (scalings compose), `det(A^T) = det(A)`, and
//!   `det(A^-1) = 1 / det(A)`.
//! - algebraically it is the (magnitude of the) wedge product of the columns.
//!
//! Method, as the subject suggests, by dimension:
//! - n <= 4 (the required cases): cofactor expansion along the first row,
//!   `det = sum_j (-1)^j * a_0j * det(minor_0j)`, with 2x2 determinants
//!   `ad - bc` computed with the compensated difference of products of
//!   exercise 06. Exact for small integers, and no division, so singular
//!   matrices give exactly 0.
//! - n > 4 (a bonus, since cofactors grow like n!): LU decomposition with
//!   partial pivoting, the product of the pivots with the sign of the row
//!   swaps, in O(n^3).
//!
//! The sums use the fused multiply-add (the allowed mathematical function).
//! Complexity: O(n^3) time, O(n^2) memory. A non-square matrix is undefined:
//! `determinant` panics with a message, `try_determinant` returns an error.
//! The determinant of the 0x0 matrix is 1 (the empty product).

use crate::error::{or_panic, MatrixError};
use crate::ex06_cross_product::diff_of_products;
use crate::matrix::Matrix;
use crate::scalar::Scalar;

/// Cofactor expansion of the n x n matrix `m` (row by row), for n <= 4.
fn cofactor_det<K: Scalar>(m: &[K], n: usize) -> K {
    match n {
        0 => K::one(),
        1 => m[0],
        2 => diff_of_products(m[0], m[3], m[1], m[2]),
        _ => {
            let mut acc = K::zero();
            let mut minor = vec![K::zero(); (n - 1) * (n - 1)];
            for j in 0..n {
                let mut k = 0;
                for i in 1..n {
                    for c in (0..n).filter(|c| *c != j) {
                        minor[k] = m[i * n + c];
                        k += 1;
                    }
                }
                let d = cofactor_det(&minor, n - 1);
                let a = if j % 2 == 0 { m[j] } else { -m[j] };
                acc = a.fma(d, acc);
            }
            acc
        }
    }
}

/// LU decomposition with partial pivoting, for any n.
fn lu_det<K: Scalar>(mut a: Vec<K>, n: usize) -> K {
    let mut det = K::one();
    for c in 0..n {
        let mut p = c;
        for i in c + 1..n {
            if a[i * n + c].modulus() > a[p * n + c].modulus() {
                p = i;
            }
        }
        if a[p * n + c] == K::zero() {
            return K::zero();
        }
        if p != c {
            for j in 0..n {
                a.swap(p * n + j, c * n + j);
            }
            det = -det;
        }
        let piv = a[c * n + c];
        det = det * piv;
        for i in c + 1..n {
            let f = a[i * n + c] / piv;
            for j in c + 1..n {
                a[i * n + j] = (-f).fma(a[c * n + j], a[i * n + j]);
            }
        }
    }
    det
}

impl<K: Scalar> Matrix<K> {
    pub fn try_determinant(&self) -> Result<K, MatrixError> {
        if !self.is_square() {
            return Err(MatrixError::NotSquare { rows: self.rows, cols: self.cols });
        }
        Ok(if self.rows <= 4 { cofactor_det(&self.data, self.rows) } else { lu_det(self.data.clone(), self.rows) })
    }

    pub fn determinant(&self) -> K {
        or_panic(self.try_determinant())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(Matrix::from([[1., -1.], [-1., 1.]]).determinant(), 0.0);
        assert_eq!(Matrix::from([[2., 0., 0.], [0., 2., 0.], [0., 0., 2.]]).determinant(), 8.0);
        assert_eq!(Matrix::from([[8., 5., -2.], [4., 7., 20.], [7., 6., 1.]]).determinant(), -174.0);
        let m = Matrix::from([[8., 5., -2., 4.], [4., 2.5, 20., 4.], [8., 5., 1., 4.], [28., -4., 17., 1.]]);
        assert_eq!(m.determinant(), 1032.0);
    }

    #[test]
    fn small_sizes() {
        assert_eq!(Matrix::<f32>::new(0, 0, vec![]).unwrap().determinant(), 1.0);
        assert_eq!(Matrix::from([[-3.5]]).determinant(), -3.5);
        assert_eq!(Matrix::from([[1., 2.], [3., 4.]]).determinant(), -2.0);
        assert_eq!(Matrix::from([[1., 2., 3.], [4., 5., 6.], [7., 8., 9.]]).determinant(), 0.0);
    }

    #[test]
    fn five_by_five_and_up_use_lu() {
        let mut data = vec![0.0f32; 25];
        for i in 0..5 {
            data[i * 5 + i] = (i + 1) as f32;
        }
        assert_eq!(Matrix::new(5, 5, data.clone()).unwrap().determinant(), 120.0);
        for j in 0..5 {
            data.swap(j, 5 + j); // swap rows 0 and 1: the sign flips
        }
        assert_eq!(Matrix::new(5, 5, data).unwrap().determinant(), -120.0);
        let id6 = Matrix::new(6, 6, (0..36).map(|k| if k % 7 == 0 { 1. } else { 0. }).collect()).unwrap();
        assert_eq!(id6.determinant(), 1.0);
    }

    #[test]
    fn row_swap_flips_the_sign_and_product_rule() {
        let a = Matrix::from([[1., 2., 3.], [0., 1., 4.], [5., 6., 0.]]);
        let swapped = Matrix::from([[0., 1., 4.], [1., 2., 3.], [5., 6., 0.]]);
        assert_eq!(a.determinant(), 1.0);
        assert_eq!(swapped.determinant(), -1.0);
        let b = Matrix::from([[2., 1., 0.], [1., 3., 1.], [0., 1., 4.]]);
        assert_eq!(a.mul_mat(&b).determinant(), a.determinant() * b.determinant());
        assert_eq!(a.transpose().determinant(), a.determinant());
    }

    #[test]
    fn non_square_is_an_error() {
        assert!(Matrix::from([[1., 2., 3.], [4., 5., 6.]]).try_determinant().is_err());
    }
}

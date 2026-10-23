//! Exercise 07 - Linear map, Matrix multiplication
//!
//! A matrix `A` (m x n) is a linear map from `K^n` to `K^m`; applying it to a
//! vector `u` is the matrix-vector product `A*u`, and composing two maps is the
//! matrix-matrix product `A*B` (m x n times n x p gives m x p): the number of
//! columns of the left matrix must equal the number of rows of the right one.
//!
//! `(A*u)_i = sum_j A_ij * u_j`: each coordinate is the dot product of a row of
//! `A` with `u`, accumulated with one fused multiply-add per term (the allowed
//! mathematical function). Equivalently `A*u` is the linear combination of the
//! columns of `A` weighted by the coordinates of `u` (exercise 01).
//!
//! `(A*B)_ij = sum_k A_ik * B_kj`. The loops run in the order i, k, j so that
//! both matrices and the result are read row by row (cache friendly) while
//! the arithmetic is exactly the same as the textbook formula.
//!
//! Complexity: `mul_vec` O(nm) time and space; `mul_mat` O(nmp) time and
//! O(nm + mp + np) space (the two operands and the result).
//!
//! Incompatible shapes are undefined: `mul_vec` / `mul_mat` panic with a
//! message, `try_mul_vec` / `try_mul_mat` return an error.

use crate::error::{or_panic, MatrixError};
use crate::matrix::Matrix;
use crate::scalar::Scalar;
use crate::vector::Vector;

impl<K: Scalar> Matrix<K> {
    pub fn try_mul_vec(&self, vec: &Vector<K>) -> Result<Vector<K>, MatrixError> {
        if self.cols != vec.size() {
            return Err(MatrixError::Shape(format!(
                "a {}x{} matrix cannot multiply a vector of size {}",
                self.rows,
                self.cols,
                vec.size()
            )));
        }
        let mut out = Vec::with_capacity(self.rows);
        for i in 0..self.rows {
            let row = &self.data[i * self.cols..(i + 1) * self.cols];
            out.push(row.iter().zip(&vec.data).fold(K::zero(), |acc, (a, x)| a.fma(*x, acc)));
        }
        Ok(Vector::new(out))
    }

    pub fn try_mul_mat(&self, mat: &Matrix<K>) -> Result<Matrix<K>, MatrixError> {
        if self.cols != mat.rows {
            return Err(MatrixError::Shape(format!(
                "a {}x{} matrix cannot multiply a {}x{} matrix",
                self.rows, self.cols, mat.rows, mat.cols
            )));
        }
        let (m, n, p) = (self.rows, self.cols, mat.cols);
        let mut out = vec![K::zero(); m * p];
        for i in 0..m {
            for k in 0..n {
                let a = self.data[i * n + k];
                let b_row = &mat.data[k * p..(k + 1) * p];
                let c_row = &mut out[i * p..(i + 1) * p];
                for (c, b) in c_row.iter_mut().zip(b_row) {
                    *c = a.fma(*b, *c);
                }
            }
        }
        Ok(Matrix { rows: m, cols: p, data: out })
    }

    pub fn mul_vec(&self, vec: &Vector<K>) -> Vector<K> {
        or_panic(self.try_mul_vec(vec))
    }

    pub fn mul_mat(&self, mat: &Matrix<K>) -> Matrix<K> {
        or_panic(self.try_mul_mat(mat))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn mul_vec_matches_subject() {
        let v = Vector::from([4., 2.]);
        assert_eq!(Matrix::from([[1., 0.], [0., 1.]]).mul_vec(&v).to_string(), "[4.0]\n[2.0]");
        assert_eq!(Matrix::from([[2., 0.], [0., 2.]]).mul_vec(&v).to_string(), "[8.0]\n[4.0]");
        assert_eq!(Matrix::from([[2., -2.], [-2., 2.]]).mul_vec(&v).to_string(), "[4.0]\n[-4.0]");
    }

    #[test]
    fn mul_mat_matches_subject() {
        let id = Matrix::from([[1., 0.], [0., 1.]]);
        assert_eq!(id.mul_mat(&id).to_string(), "[1.0, 0.0]\n[0.0, 1.0]");
        let v = Matrix::from([[2., 1.], [4., 2.]]);
        assert_eq!(id.mul_mat(&v).to_string(), "[2.0, 1.0]\n[4.0, 2.0]");
        let u = Matrix::from([[3., -5.], [6., 8.]]);
        assert_eq!(u.mul_mat(&v).to_string(), "[-14.0, -7.0]\n[44.0, 22.0]");
    }

    #[test]
    fn rectangular_shapes() {
        let a = Matrix::from([[1., 2., 3.], [4., 5., 6.]]); // 2x3
        let b = Matrix::from([[1., 0.], [0., 1.], [1., 1.]]); // 3x2
        assert_eq!(a.mul_mat(&b), Matrix::from([[4., 5.], [10., 11.]]));
        assert_eq!(b.mul_mat(&a).size(), (3, 3));
        assert_eq!(a.mul_vec(&Vector::from([1., 1., 1.])), Vector::from([6., 15.]));
    }

    #[test]
    fn algebra() {
        let a = Matrix::from([[1., 2.], [3., 4.]]);
        let b = Matrix::from([[0., 1.], [1., 0.]]);
        let c = Matrix::from([[2., 0.], [1., 3.]]);
        assert_eq!(a.mul_mat(&b).mul_mat(&c), a.mul_mat(&b.mul_mat(&c))); // associative
        assert_ne!(a.mul_mat(&b), b.mul_mat(&a)); // not commutative
        let id = Matrix::from([[1., 0.], [0., 1.]]);
        assert_eq!(a.mul_mat(&id), a);
    }

    #[test]
    fn degenerate_shapes() {
        let empty = Matrix::<f32>::new(0, 0, vec![]).unwrap();
        assert_eq!(empty.mul_mat(&empty).size(), (0, 0));
        let a = Matrix::<f32>::new(2, 0, vec![]).unwrap(); // 2x0 times 0x3 = 2x3 of zeros
        let b = Matrix::<f32>::new(0, 3, vec![]).unwrap();
        assert_eq!(a.mul_mat(&b), Matrix::new(2, 3, vec![0.; 6]).unwrap());
        assert_eq!(a.mul_vec(&Vector::from([])), Vector::from([0., 0.]));
    }

    #[test]
    fn incompatible_shapes_are_errors() {
        let a = Matrix::from([[1., 2.], [3., 4.]]);
        assert!(a.try_mul_vec(&Vector::from([1., 2., 3.])).is_err());
        assert!(a.try_mul_mat(&Matrix::from([[1., 2., 3.]])).is_err());
    }
}

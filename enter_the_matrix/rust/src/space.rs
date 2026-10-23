//! What "being a vector space over `K`" means for the generic functions of the
//! subject (`linear_combination`, `lerp`): vectors and matrices are both vector
//! spaces, and the exercises are written once for any of them.

use crate::matrix::Matrix;
use crate::scalar::Scalar;
use crate::vector::Vector;

pub trait VectorSpace<K: Scalar>: Clone {
    /// The zero element, with the same shape as `self`.
    fn zero_like(&self) -> Self;
    fn same_shape(&self, other: &Self) -> bool;
    /// Human readable shape, for error messages.
    fn shape_string(&self) -> String;
    /// `self += a * x`, with one fused multiply-add per coordinate.
    /// The shapes must be equal (checked by the callers).
    fn axpy(&mut self, a: K, x: &Self);
}

impl<K: Scalar> VectorSpace<K> for Vector<K> {
    fn zero_like(&self) -> Self {
        Vector::new(vec![K::zero(); self.size()])
    }
    fn same_shape(&self, other: &Self) -> bool {
        self.size() == other.size()
    }
    fn shape_string(&self) -> String {
        format!("vector of size {}", self.size())
    }
    fn axpy(&mut self, a: K, x: &Self) {
        for (s, v) in self.data.iter_mut().zip(&x.data) {
            *s = a.fma(*v, *s);
        }
    }
}

impl<K: Scalar> VectorSpace<K> for Matrix<K> {
    fn zero_like(&self) -> Self {
        Matrix { rows: self.rows, cols: self.cols, data: vec![K::zero(); self.data.len()] }
    }
    fn same_shape(&self, other: &Self) -> bool {
        self.size() == other.size()
    }
    fn shape_string(&self) -> String {
        format!("{}x{} matrix", self.rows, self.cols)
    }
    fn axpy(&mut self, a: K, x: &Self) {
        for (s, v) in self.data.iter_mut().zip(&x.data) {
            *s = a.fma(*v, *s);
        }
    }
}

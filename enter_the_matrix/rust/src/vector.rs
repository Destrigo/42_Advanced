//! The `Vector<K>` type: a store of data plus a few utility functions
//! (size, reshape into a matrix, printing). The mathematical operations live
//! in the module of the exercise that introduces them.

use crate::error::MatrixError;
use crate::matrix::Matrix;
use crate::scalar::Scalar;
use std::fmt;

#[derive(Clone, PartialEq, Debug, Default)]
pub struct Vector<K> {
    pub(crate) data: Vec<K>,
}

impl<K> Vector<K> {
    pub fn new(data: Vec<K>) -> Vector<K> {
        Vector { data }
    }

    /// Number of coordinates (the dimension).
    pub fn size(&self) -> usize {
        self.data.len()
    }

    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    pub fn as_slice(&self) -> &[K] {
        &self.data
    }

    pub fn into_vec(self) -> Vec<K> {
        self.data
    }
}

impl<K: Clone> Vector<K> {
    /// Reshapes the vector into a `rows`x`cols` matrix (filled row by row).
    pub fn reshape(&self, rows: usize, cols: usize) -> Result<Matrix<K>, MatrixError> {
        Matrix::new(rows, cols, self.data.clone())
    }

    /// The vector as an n x 1 column matrix.
    pub fn to_matrix(&self) -> Matrix<K> {
        Matrix { rows: self.data.len(), cols: 1, data: self.data.clone() }
    }
}

impl<K, const N: usize> From<[K; N]> for Vector<K> {
    fn from(a: [K; N]) -> Vector<K> {
        Vector { data: Vec::from(a) }
    }
}

impl<K> From<Vec<K>> for Vector<K> {
    fn from(data: Vec<K>) -> Vector<K> {
        Vector { data }
    }
}

/// One coordinate per line, like in the subject: `[7.0]`.
impl<K: Scalar> fmt::Display for Vector<K> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for (i, x) in self.data.iter().enumerate() {
            if i > 0 {
                writeln!(f)?;
            }
            write!(f, "[{:?}]", x.clean())?;
        }
        Ok(())
    }
}

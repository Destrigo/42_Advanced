//! The `Matrix<K>` type: a store of data plus a few utility functions
//! (shape, squareness, reshape, printing). The mathematical operations live in
//! the module of the exercise that introduces them.
//!
//! The examples of the subject write a matrix row by row: in
//! `Matrix::from([[1., 2.], [3., 4.]])` the inner arrays are the rows (this is
//! what makes the matrix products of exercise 07 come out as printed). The
//! data is stored row by row; `to_column_major` gives the column-by-column
//! layout that GPUs / OpenGL expect.

use crate::error::MatrixError;
use crate::scalar::Scalar;
use crate::vector::Vector;
use std::fmt;

#[derive(Clone, PartialEq, Debug, Default)]
pub struct Matrix<K> {
    pub(crate) rows: usize,
    pub(crate) cols: usize,
    pub(crate) data: Vec<K>,
}

impl<K> Matrix<K> {
    /// `data` is the content row by row; its length must be `rows * cols`.
    pub fn new(rows: usize, cols: usize, data: Vec<K>) -> Result<Matrix<K>, MatrixError> {
        match rows.checked_mul(cols) {
            Some(n) if n == data.len() => Ok(Matrix { rows, cols, data }),
            _ => Err(MatrixError::Shape(format!(
                "{rows}x{cols} matrix needs {} values, got {}",
                rows.saturating_mul(cols),
                data.len()
            ))),
        }
    }

    /// Builds a matrix from a list of rows; they must all have the same length.
    pub fn from_rows(rows: Vec<Vec<K>>) -> Result<Matrix<K>, MatrixError> {
        let cols = rows.first().map_or(0, Vec::len);
        let n = rows.len();
        if rows.iter().any(|r| r.len() != cols) {
            return Err(MatrixError::Shape("rows of different lengths".to_string()));
        }
        Ok(Matrix { rows: n, cols, data: rows.into_iter().flatten().collect() })
    }

    /// `(rows, columns)`.
    pub fn size(&self) -> (usize, usize) {
        (self.rows, self.cols)
    }

    pub fn rows(&self) -> usize {
        self.rows
    }

    pub fn cols(&self) -> usize {
        self.cols
    }

    pub fn is_square(&self) -> bool {
        self.rows == self.cols
    }

    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    pub fn get(&self, i: usize, j: usize) -> Option<&K> {
        if i < self.rows && j < self.cols {
            self.data.get(i * self.cols + j)
        } else {
            None
        }
    }

    /// Row `i` as a slice (panics if out of range, like slice indexing).
    pub fn row(&self, i: usize) -> &[K] {
        &self.data[i * self.cols..(i + 1) * self.cols]
    }

    /// The content row by row.
    pub fn as_slice(&self) -> &[K] {
        &self.data
    }

    pub(crate) fn at(&self, i: usize, j: usize) -> &K {
        &self.data[i * self.cols + j]
    }
}

impl<K: Clone> Matrix<K> {
    /// Reshapes (row by row) into `rows`x`cols`; the element count must match.
    pub fn reshape(&self, rows: usize, cols: usize) -> Result<Matrix<K>, MatrixError> {
        Matrix::new(rows, cols, self.data.clone())
    }

    /// Flattens the matrix row by row into a vector.
    pub fn to_vector(&self) -> Vector<K> {
        Vector::new(self.data.clone())
    }

    /// The content column by column (the layout of OpenGL / the display program).
    pub fn to_column_major(&self) -> Vec<K> {
        let mut out = Vec::with_capacity(self.data.len());
        for j in 0..self.cols {
            for i in 0..self.rows {
                out.push(self.data[i * self.cols + j].clone());
            }
        }
        out
    }
}

impl<K, const R: usize, const C: usize> From<[[K; C]; R]> for Matrix<K> {
    fn from(a: [[K; C]; R]) -> Matrix<K> {
        Matrix { rows: R, cols: C, data: a.into_iter().flatten().collect() }
    }
}

/// One row per line, like in the subject: `[8.0, 6.0]`.
impl<K: Scalar> fmt::Display for Matrix<K> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for i in 0..self.rows {
            if i > 0 {
                writeln!(f)?;
            }
            write!(f, "[")?;
            for j in 0..self.cols {
                if j > 0 {
                    write!(f, ", ")?;
                }
                write!(f, "{:?}", self.at(i, j).clean())?;
            }
            write!(f, "]")?;
        }
        Ok(())
    }
}

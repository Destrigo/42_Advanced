//! Errors returned by the `try_*` functions.
//!
//! The functions named like in the subject (`add`, `dot`, `inverse`...) keep
//! the signatures of the subject. When the subject says the behaviour is
//! "undefined" for a nonsensical input (different sizes, non-square matrix...)
//! they panic with a clear message; the `try_*` twin of each of them returns
//! one of these errors instead, so callers (and evaluators) can pick.
//! `Matrix::inverse` is the exception: the subject asks for an error on a
//! singular matrix, so it returns a `Result` directly.

use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum MatrixError {
    /// Operands have incompatible sizes / shapes.
    Shape(String),
    /// The operation needs a square matrix.
    NotSquare { rows: usize, cols: usize },
    /// The operation needs at least one element / vector.
    Empty,
    /// The matrix has no inverse.
    Singular,
    /// Anything else that makes no mathematical sense (bad argument...).
    Invalid(String),
}

impl fmt::Display for MatrixError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MatrixError::Shape(s) => write!(f, "incompatible shapes: {s}"),
            MatrixError::NotSquare { rows, cols } => {
                write!(f, "the matrix must be square, but it is {rows}x{cols}")
            }
            MatrixError::Empty => write!(f, "empty input"),
            MatrixError::Singular => write!(f, "the matrix is singular (it has no inverse)"),
            MatrixError::Invalid(s) => write!(f, "invalid argument: {s}"),
        }
    }
}

impl std::error::Error for MatrixError {}

/// Used by the subject-named wrappers: undefined behaviour = clear panic.
pub(crate) fn or_panic<T>(r: Result<T, MatrixError>) -> T {
    r.unwrap_or_else(|e| panic!("{e}"))
}

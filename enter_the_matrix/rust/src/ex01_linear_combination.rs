//! Exercise 01 - Linear combination
//!
//! Given vectors `u1..uk` and scalars `l1..lk`, compute `l1*u1 + ... + lk*uk`.
//! It is the most general thing one can do in a vector space (every other
//! operation of exercise 00 is a special case) and, with the matrices of the
//! next exercises, the way to read a matrix-vector product: `A*x` is the
//! combination of the columns of `A` weighted by the coordinates of `x`.
//!
//! The function is generic over the vector space `V` (vectors or matrices).
//! It starts from the zero element and accumulates `acc += l_i * u_i`, one
//! fused multiply-add per coordinate (the allowed mathematical function): a
//! single rounding per step instead of two, so the result is more accurate.
//!
//! Complexity: O(n) per vector (n = number of coordinates), O(n) extra memory
//! (the accumulator).
//!
//! `u` and `coefs` of different lengths, an empty list (the dimension of the
//! zero vector would be unknown) or vectors of different sizes are undefined
//! in the subject: `linear_combination` panics with a message,
//! `try_linear_combination` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::scalar::Scalar;
use crate::space::VectorSpace;

pub fn try_linear_combination<K: Scalar, V: VectorSpace<K>>(u: &[V], coefs: &[K]) -> Result<V, MatrixError> {
    if u.len() != coefs.len() {
        return Err(MatrixError::Shape(format!("{} vectors but {} coefficients", u.len(), coefs.len())));
    }
    let first = u.first().ok_or(MatrixError::Empty)?;
    if let Some(bad) = u.iter().find(|v| !v.same_shape(first)) {
        return Err(MatrixError::Shape(format!("{} and {}", first.shape_string(), bad.shape_string())));
    }
    let mut acc = first.zero_like();
    for (v, c) in u.iter().zip(coefs) {
        acc.axpy(*c, v);
    }
    Ok(acc)
}

pub fn linear_combination<K: Scalar, V: VectorSpace<K>>(u: &[V], coefs: &[K]) -> V {
    or_panic(try_linear_combination(u, coefs))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::matrix::Matrix;
    use crate::vector::Vector;

    #[test]
    fn matches_subject() {
        let e1 = Vector::from([1., 0., 0.]);
        let e2 = Vector::from([0., 1., 0.]);
        let e3 = Vector::from([0., 0., 1.]);
        let v1 = Vector::from([1., 2., 3.]);
        let v2 = Vector::from([0., 10., -100.]);
        assert_eq!(linear_combination(&[e1, e2, e3], &[10., -2., 0.5]).to_string(), "[10.0]\n[-2.0]\n[0.5]");
        assert_eq!(linear_combination(&[v1, v2], &[10., -2.]).to_string(), "[10.0]\n[0.0]\n[230.0]");
    }

    #[test]
    fn works_on_matrices_too() {
        let a = Matrix::from([[1., 2.], [3., 4.]]);
        let b = Matrix::from([[1., 0.], [0., 1.]]);
        assert_eq!(linear_combination(&[a, b], &[2., -1.]), Matrix::from([[1., 4.], [6., 7.]]));
    }

    #[test]
    fn bad_input_is_an_error() {
        let v = Vector::from([1., 2.]);
        assert!(try_linear_combination(&[v.clone()], &[1., 2.]).is_err()); // lengths differ
        assert!(try_linear_combination::<f32, Vector<f32>>(&[], &[]).is_err()); // empty
        assert!(try_linear_combination(&[v.clone(), Vector::from([1.])], &[1., 1.]).is_err()); // sizes differ
    }
}

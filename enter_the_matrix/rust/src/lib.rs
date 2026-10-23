//! Enter the Matrix - an introduction to linear algebra.
//!
//! `Vector<K>` and `Matrix<K>` are generic over the field of scalars `K`
//! ([`Scalar`]): `f32` for the real numbers and [`Complex`] for the bonus.
//! Each exercise lives in its own module, which adds its functions to those
//! two types (or defines a free function), with an explanation of the maths at
//! the top of the file.

pub mod complex;
mod elim;
pub mod error;
pub mod ex00_add_sub_scale;
pub mod ex01_linear_combination;
pub mod ex02_linear_interpolation;
pub mod ex03_dot_product;
pub mod ex04_norm;
pub mod ex05_cosine;
pub mod ex06_cross_product;
pub mod ex07_matrix_multiplication;
pub mod ex08_trace;
pub mod ex09_transpose;
pub mod ex10_row_echelon;
pub mod ex11_determinant;
pub mod ex12_inverse;
pub mod ex13_rank;
pub mod ex14_projection;
pub mod matrix;
pub mod scalar;
pub mod space;
pub mod vector;

pub use complex::Complex;
pub use error::MatrixError;
pub use ex01_linear_combination::{linear_combination, try_linear_combination};
pub use ex02_linear_interpolation::{lerp, try_lerp, Lerp};
pub use ex05_cosine::{angle_cos, try_angle_cos};
pub use ex06_cross_product::{cross_product, try_cross_product};
pub use ex14_projection::{column_major_text, projection, try_projection};
pub use matrix::Matrix;
pub use scalar::Scalar;
pub use space::VectorSpace;
pub use vector::Vector;

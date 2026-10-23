//! The scalar field `K` of the vector spaces.
//!
//! Everything in the library is generic over [`Scalar`], implemented here for
//! `f32` (the real numbers of the subject) and in `complex.rs` for the bonus
//! complex numbers.
//!
//! The subject forbids the mathematical functions of the standard library
//! except, exercise by exercise, the fused multiply-add, `pow`, `max` and
//! `tan`. So this trait exposes only what is needed and the `f32`
//! implementation takes care to use nothing else: the absolute value is a
//! comparison, the square root is `pow(x, 0.5)`.

use std::fmt::Debug;
use std::ops::{Add, Div, Mul, Neg, Sub};

pub trait Scalar:
    Copy
    + Debug
    + PartialEq
    + Add<Output = Self>
    + Sub<Output = Self>
    + Mul<Output = Self>
    + Div<Output = Self>
    + Neg<Output = Self>
{
    fn zero() -> Self;
    fn one() -> Self;
    fn from_f32(x: f32) -> Self;
    /// Fused multiply-add: `self * a + b` with a single rounding.
    fn fma(self, a: Self, b: Self) -> Self;
    /// Complex conjugate (the identity on real numbers).
    fn conj(self) -> Self;
    /// Absolute value / modulus, always a real number.
    fn modulus(self) -> f32;
    /// Real part.
    fn re(self) -> f32;
    fn is_finite(self) -> bool;
    /// Same value with `-0.0` replaced by `0.0` (cosmetic, for printing).
    fn clean(self) -> Self {
        self
    }
}

impl Scalar for f32 {
    fn zero() -> f32 {
        0.0
    }
    fn one() -> f32 {
        1.0
    }
    fn from_f32(x: f32) -> f32 {
        x
    }
    fn fma(self, a: f32, b: f32) -> f32 {
        self.mul_add(a, b)
    }
    fn conj(self) -> f32 {
        self
    }
    fn modulus(self) -> f32 {
        if self < 0.0 {
            -self
        } else {
            self
        }
    }
    fn re(self) -> f32 {
        self
    }
    fn is_finite(self) -> bool {
        f32::is_finite(self)
    }
    fn clean(self) -> f32 {
        if self == 0.0 {
            0.0
        } else {
            self
        }
    }
}

/// Square root using only `pow`, the function the subject allows (NaN for x < 0).
pub fn sqrt(x: f32) -> f32 {
    if x < 0.0 {
        f32::NAN
    } else {
        x.powf(0.5)
    }
}

/// Euclidean length of a list of reals, without overflow or underflow:
/// the values are first divided by the largest one.
pub fn scaled_norm(values: impl Iterator<Item = f32> + Clone) -> f32 {
    let mut max = 0.0f32;
    for x in values.clone() {
        if x != x {
            return f32::NAN;
        }
        if x > max {
            max = x;
        }
    }
    if max == 0.0 || !f32::is_finite(max) {
        return max;
    }
    let mut acc = 0.0f32;
    for x in values {
        let r = x / max;
        acc = r.mul_add(r, acc);
    }
    max * sqrt(acc)
}

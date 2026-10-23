//! Exercise 15 (bonus) - complex numbers as the field of scalars.
//!
//! `Complex` is a pair of `f32` with the usual field operations, and
//! implements [`Scalar`], so every function of the library (vectors, matrices,
//! determinant, inverse, rank...) works over the complex numbers without a
//! single change: that is the point of having written everything generically.
//!
//! Notes on the complex versions of the exercises:
//! - `dot` is the sesquilinear inner product `<u|v> = sum(conj(u_i) * v_i)`
//!   (conjugate-linear in the first argument), the one that makes
//!   `<v|v> = |v|^2` real and positive.
//! - norms and `angle_cos` return real numbers (`angle_cos` is the real part
//!   of the normalised inner product).
//! - `Matrix::conjugate_transpose` is the transpose that goes with complex
//!   numbers (`transpose` is the plain one).

use crate::scalar::{sqrt, Scalar};
use std::fmt;
use std::ops::{Add, Div, Mul, Neg, Sub};

#[derive(Clone, Copy, PartialEq, Default)]
pub struct Complex {
    pub re: f32,
    pub im: f32,
}

impl Complex {
    pub const fn new(re: f32, im: f32) -> Complex {
        Complex { re, im }
    }
}

impl From<f32> for Complex {
    fn from(re: f32) -> Complex {
        Complex { re, im: 0.0 }
    }
}

impl Add for Complex {
    type Output = Complex;
    fn add(self, o: Complex) -> Complex {
        Complex::new(self.re + o.re, self.im + o.im)
    }
}

impl Sub for Complex {
    type Output = Complex;
    fn sub(self, o: Complex) -> Complex {
        Complex::new(self.re - o.re, self.im - o.im)
    }
}

impl Mul for Complex {
    type Output = Complex;
    fn mul(self, o: Complex) -> Complex {
        Complex::new(self.re * o.re - self.im * o.im, self.re * o.im + self.im * o.re)
    }
}

impl Div for Complex {
    type Output = Complex;
    /// Smith's algorithm: no overflow when the squared modulus would overflow.
    fn div(self, o: Complex) -> Complex {
        let (a, b, c, d) = (self.re, self.im, o.re, o.im);
        if d.modulus() <= c.modulus() {
            let r = d / c;
            let den = c + d * r;
            Complex::new((a + b * r) / den, (b - a * r) / den)
        } else {
            let r = c / d;
            let den = c * r + d;
            Complex::new((a * r + b) / den, (b * r - a) / den)
        }
    }
}

impl Neg for Complex {
    type Output = Complex;
    fn neg(self) -> Complex {
        Complex::new(-self.re, -self.im)
    }
}

impl Scalar for Complex {
    fn zero() -> Complex {
        Complex::new(0.0, 0.0)
    }
    fn one() -> Complex {
        Complex::new(1.0, 0.0)
    }
    fn from_f32(x: f32) -> Complex {
        Complex::new(x, 0.0)
    }
    /// A complex product is four real products, so there is no single fused
    /// operation for it. Plain `self * a + b` is used on purpose (rather than
    /// chaining real fused operations): the two products that must cancel in
    /// `conj(z) * z` are then rounded identically, so the imaginary part of
    /// `<v|v>` is exactly 0.
    fn fma(self, a: Complex, b: Complex) -> Complex {
        self * a + b
    }
    fn conj(self) -> Complex {
        Complex::new(self.re, -self.im)
    }
    fn modulus(self) -> f32 {
        let (a, b) = (self.re.modulus(), self.im.modulus());
        if a != a || b != b {
            return f32::NAN;
        }
        let m = if a > b { a } else { b };
        if m == 0.0 || !f32::is_finite(m) {
            return m;
        }
        let (x, y) = (a / m, b / m);
        m * sqrt(x.mul_add(x, y * y))
    }
    fn re(self) -> f32 {
        self.re
    }
    fn is_finite(self) -> bool {
        f32::is_finite(self.re) && f32::is_finite(self.im)
    }
    fn clean(self) -> Complex {
        Complex::new(self.re.clean(), self.im.clean())
    }
}

impl fmt::Debug for Complex {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let sign = if self.im < 0.0 || (self.im == 0.0 && self.im.is_sign_negative()) { '-' } else { '+' };
        write!(f, "{:?}{}{:?}i", self.re, sign, self.im.modulus())
    }
}

impl fmt::Display for Complex {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        fmt::Debug::fmt(self, f)
    }
}

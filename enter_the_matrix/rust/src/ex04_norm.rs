//! Exercise 04 - Norm
//!
//! A norm measures the length of a vector. Three classic ones:
//! - 1-norm (taxicab / Manhattan): `|v1| + ... + |vn|`, the distance walked
//!   along the axes;
//! - 2-norm (Euclidean): `sqrt(v1^2 + ... + vn^2) = sqrt(<v|v>)`, the length
//!   of the arrow;
//! - infinity-norm (supremum): `max |vi|`, the largest coordinate.
//!
//! They always return a real number, also for complex vectors (the modulus of
//! each coordinate is used).
//!
//! Allowed mathematical functions: fused multiply-add, `pow`, `max`. The
//! square root is `pow(x, 0.5)` and the absolute value / maximum are plain
//! comparisons. The 2-norm divides the coordinates by the largest one first,
//! so that squaring them neither overflows (`[1e30, 1e30]` has norm
//! `1.4142e30`, not infinity) nor underflows, then multiplies back.
//!
//! Complexity: O(n) time, O(1) extra memory. A NaN coordinate gives NaN.

use crate::scalar::{scaled_norm, Scalar};
use crate::vector::Vector;

impl<K: Scalar> Vector<K> {
    pub fn norm_1(&self) -> f32 {
        let mut sum = 0.0f32;
        for x in &self.data {
            sum += x.modulus();
        }
        sum
    }

    pub fn norm(&self) -> f32 {
        scaled_norm(self.data.iter().map(|x| x.modulus()))
    }

    pub fn norm_inf(&self) -> f32 {
        let mut max = 0.0f32;
        for x in &self.data {
            let m = x.modulus();
            if m != m {
                return f32::NAN;
            }
            if m > max {
                max = m;
            }
        }
        max
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn close(a: f32, b: f32) -> bool {
        (a - b).modulus() <= 1e-6 * b.modulus().max(1.0)
    }

    #[test]
    fn matches_subject() {
        let u = Vector::from([0., 0., 0.]);
        assert_eq!((u.norm_1(), u.norm(), u.norm_inf()), (0.0, 0.0, 0.0));
        let u = Vector::from([1., 2., 3.]);
        assert_eq!(u.norm_1(), 6.0);
        assert!(close(u.norm(), 3.74165738));
        assert_eq!(u.norm_inf(), 3.0);
        let u = Vector::from([-1., -2.]);
        assert_eq!(u.norm_1(), 3.0);
        assert!(close(u.norm(), 2.236067977));
        assert_eq!(u.norm_inf(), 2.0);
    }

    #[test]
    fn exact_cases() {
        assert_eq!(Vector::from([3., 4.]).norm(), 5.0);
        assert_eq!(Vector::from([-3., 4.]).norm(), 5.0);
        assert_eq!(Vector::from([5.]).norm(), 5.0);
        assert_eq!(Vector::<f32>::from([]).norm(), 0.0);
        assert_eq!(Vector::<f32>::from([]).norm_inf(), 0.0);
    }

    #[test]
    fn no_overflow_or_underflow() {
        assert!(close(Vector::from([1e30, 1e30]).norm(), 1.4142135e30));
        assert!(close(Vector::from([1e-30, 1e-30]).norm(), 1.4142135e-30));
        // squares overflow f32 (1.8e38^2 = 3.2e76) but the norm itself, 3e38, does not
        assert!(close(Vector::from([1.8e38, 2.4e38]).norm(), 3e38));
        assert_eq!(Vector::from([f32::INFINITY, 1.]).norm(), f32::INFINITY);
    }

    #[test]
    fn nan_propagates() {
        let u = Vector::from([1., f32::NAN]);
        assert!(u.norm().is_nan() && u.norm_1().is_nan() && u.norm_inf().is_nan());
    }

    #[test]
    fn norm_axioms_on_samples() {
        let u = Vector::from([1., -2., 0.5]);
        let v = Vector::from([3., 1., -4.]);
        let mut w = u.clone();
        w.add(&v);
        assert!(w.norm() <= u.norm() + v.norm() + 1e-6); // triangle inequality
        let mut s = u.clone();
        s.scl(-3.);
        assert!(close(s.norm(), 3.0 * u.norm())); // homogeneity
    }
}

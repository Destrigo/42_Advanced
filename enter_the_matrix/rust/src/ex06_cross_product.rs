//! Exercise 06 - Cross product
//!
//! The cross product `u x v` of two 3-dimensional vectors is the vector
//! perpendicular to both, whose length is the area of the parallelogram they
//! span (`|u| |v| sin(angle)`) and whose direction follows the right-hand
//! rule. It is zero when the vectors are parallel, and anticommutative:
//! `u x v = -(v x u)`.
//!
//! `(u1,u2,u3) x (v1,v2,v3) = (u2 v3 - u3 v2,  u3 v1 - u1 v3,  u1 v2 - u2 v1)`
//!
//! Each coordinate is a difference of two products, the classic place where
//! rounding errors blow up through cancellation. With a fused multiply-add
//! (the allowed mathematical function) the error of one product is recovered
//! exactly and added back (Kahan's trick): `w = c*d`, `e = fma(-c, d, w)` is
//! exactly the rounding error of `w`, `f = fma(a, b, -w)`, and `a*b - c*d`
//! is `f + e`.
//!
//! Complexity: O(1). Anything that is not a pair of 3D vectors is undefined:
//! `cross_product` panics with a message, `try_cross_product` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::scalar::Scalar;
use crate::vector::Vector;

/// `a*b - c*d` with a single final rounding error compensation.
pub(crate) fn diff_of_products<K: Scalar>(a: K, b: K, c: K, d: K) -> K {
    let w = c * d;
    let e = (-c).fma(d, w);
    let f = a.fma(b, -w);
    f + e
}

pub fn try_cross_product<K: Scalar>(u: &Vector<K>, v: &Vector<K>) -> Result<Vector<K>, MatrixError> {
    if u.size() != 3 || v.size() != 3 {
        return Err(MatrixError::Shape(format!(
            "the cross product needs two 3D vectors, got sizes {} and {}",
            u.size(),
            v.size()
        )));
    }
    let (a, b) = (&u.data, &v.data);
    Ok(Vector::new(vec![
        diff_of_products(a[1], b[2], a[2], b[1]),
        diff_of_products(a[2], b[0], a[0], b[2]),
        diff_of_products(a[0], b[1], a[1], b[0]),
    ]))
}

pub fn cross_product<K: Scalar>(u: &Vector<K>, v: &Vector<K>) -> Vector<K> {
    or_panic(try_cross_product(u, v))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        let c = cross_product(&Vector::from([0., 0., 1.]), &Vector::from([1., 0., 0.]));
        assert_eq!(c.to_string(), "[0.0]\n[1.0]\n[0.0]");
        let c = cross_product(&Vector::from([1., 2., 3.]), &Vector::from([4., 5., 6.]));
        assert_eq!(c.to_string(), "[-3.0]\n[6.0]\n[-3.0]");
        let c = cross_product(&Vector::from([4., 2., -3.]), &Vector::from([-2., -5., 16.]));
        assert_eq!(c.to_string(), "[17.0]\n[-58.0]\n[-16.0]");
    }

    #[test]
    fn algebraic_properties() {
        let u = Vector::from([1.5, -2., 0.25]);
        let v = Vector::from([3., 0.5, -4.]);
        let uv = cross_product(&u, &v);
        let mut vu = cross_product(&v, &u);
        vu.scl(-1.);
        assert_eq!(uv, vu); // anticommutative
        assert_eq!(uv.dot(&u), 0.0); // perpendicular to both
        assert_eq!(uv.dot(&v), 0.0);
        assert_eq!(cross_product(&u, &u).as_slice(), &[0., 0., 0.]); // parallel
    }

    #[test]
    fn cancellation_is_compensated() {
        // a*b - c*d where the plain formula loses everything
        // a*a = 1 + 2^-12 + 2^-26 exactly; plain f32 rounds the 2^-26 away and
        // the difference with c = 1 + 2^-12 comes out as 0.
        let a = 1.0f32 + 1.0 / 8192.0;
        let c = 1.0f32 + 1.0 / 4096.0;
        assert_eq!(a * a - c * 1.0, 0.0);
        assert_eq!(diff_of_products(a, a, c, 1.0), 1.0 / 67108864.0);
    }

    #[test]
    fn not_3d_is_an_error() {
        assert!(try_cross_product(&Vector::from([1., 2.]), &Vector::from([1., 2.])).is_err());
        assert!(try_cross_product(&Vector::from([1., 2., 3.]), &Vector::from([1., 2., 3., 4.])).is_err());
        assert!(try_cross_product(&Vector::<f32>::from([]), &Vector::from([])).is_err());
    }
}

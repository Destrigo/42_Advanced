//! Exercise 14 (bonus) - Projection matrix
//!
//! The matrix that turns the 3D scene seen by the camera into the cube the
//! GPU draws: `s = P * V * M * p`, where `M` places the object in the world,
//! `V` moves the world so that the camera sits at the origin looking down -Z,
//! and `P` (built here) squeezes the viewing frustum, the truncated pyramid
//! between the near and far planes, into the unit cube. The 4th dimension is
//! the homogeneous coordinate `w`: this matrix puts `-z` there, and the GPU's
//! division by `w` is what makes distant things smaller (perspective).
//!
//! Conventions (the ones the display program of the subject uses):
//! - right-handed view space, the camera looks down the negative Z axis, so
//!   visible points have `z` between `-near` and `-far`;
//! - normalised device coordinates: X and Y in `[-1, 1]`, Z in `[0, 1]`
//!   (0 on the near plane, 1 on the far plane), Y up;
//! - `fov` is the vertical field of view in **radians**, `ratio` is
//!   `width / height`, and the matrix acts on column vectors `(x, y, z, 1)`.
//!
//! With `f = 1 / tan(fov / 2)`:
//!
//! ```text
//! [ f/ratio  0      0                 0                  ]
//! [ 0        f      0                 0                  ]
//! [ 0        0   far/(near-far)   near*far/(near-far)    ]
//! [ 0        0     -1                 0                  ]
//! ```
//!
//! Checks (also in the tests): a point on the near plane maps to Z = 0, on the
//! far plane to Z = 1, and a point on the edge of the frustum to X or Y = +-1.
//!
//! The display program reads the matrix column by column; use
//! [`column_major_text`] to produce that text. Changing `near` and `far`
//! changes only the third row: it redistributes the depth precision (a very
//! small `near` wastes almost all the Z range on the first few units), and
//! `far / near` too large causes Z-fighting.
//!
//! Allowed mathematical functions: `tan` and the fused multiply-add.
//! Complexity: O(1). Invalid parameters (`fov` outside `(0, pi)`, a
//! non-positive `ratio` or `near`, `far <= near`, NaN...) are undefined:
//! `projection` panics with a message, `try_projection` returns an error.

use crate::error::{or_panic, MatrixError};
use crate::matrix::Matrix;

pub fn try_projection(fov: f32, ratio: f32, near: f32, far: f32) -> Result<Matrix<f32>, MatrixError> {
    let pi = std::f32::consts::PI;
    if !(fov > 0.0 && fov < pi) {
        return Err(MatrixError::Invalid(format!("fov must be in (0, pi) radians, got {fov}")));
    }
    if !(ratio > 0.0 && ratio.is_finite()) {
        return Err(MatrixError::Invalid(format!("ratio must be positive, got {ratio}")));
    }
    if !(near > 0.0 && near.is_finite() && far.is_finite() && far > near) {
        return Err(MatrixError::Invalid(format!("need 0 < near < far, got near = {near}, far = {far}")));
    }
    let f = 1.0 / (fov * 0.5).tan();
    let d = near - far;
    Matrix::new(
        4,
        4,
        vec![
            f / ratio, 0., 0., 0.,
            0., f, 0., 0.,
            0., 0., far / d, (near * far) / d,
            0., 0., -1., 0.,
        ],
    )
}

pub fn projection(fov: f32, ratio: f32, near: f32, far: f32) -> Matrix<f32> {
    or_panic(try_projection(fov, ratio, near, far))
}

/// The matrix as the display program reads it: one line per column, values
/// separated by commas.
pub fn column_major_text(m: &Matrix<f32>) -> String {
    let mut out = String::new();
    for j in 0..m.cols() {
        let col: Vec<String> = (0..m.rows()).map(|i| format!("{:?}", m.row(i)[j])).collect();
        out.push_str(&col.join(", "));
        out.push('\n');
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::scalar::Scalar;
    use crate::vector::Vector;

    const PI: f32 = std::f32::consts::PI;

    /// Applies P to the point (x, y, z), divides by w, returns the NDC.
    fn ndc(p: &Matrix<f32>, x: f32, y: f32, z: f32) -> (f32, f32, f32) {
        let s = p.mul_vec(&Vector::from([x, y, z, 1.]));
        let w = s.as_slice()[3];
        (s.as_slice()[0] / w, s.as_slice()[1] / w, s.as_slice()[2] / w)
    }

    fn close(a: f32, b: f32) -> bool {
        (a - b).modulus() < 1e-5
    }

    #[test]
    fn depth_range_is_zero_to_one() {
        let p = projection(PI / 3., 16. / 9., 0.5, 100.);
        assert!(close(ndc(&p, 0., 0., -0.5).2, 0.0)); // near plane
        assert!(close(ndc(&p, 0., 0., -100.).2, 1.0)); // far plane
        let mid = ndc(&p, 0., 0., -10.).2;
        assert!(mid > 0.0 && mid < 1.0);
        assert!(ndc(&p, 0., 0., -1.).2 < ndc(&p, 0., 0., -2.).2); // monotonic
    }

    #[test]
    fn frustum_edges_map_to_the_border() {
        let (fov, ratio, near, far) = (PI / 2., 2., 1., 50.);
        let p = projection(fov, ratio, near, far);
        // fov = 90 degrees: at distance d the half height is d, the half width ratio * d
        for d in [1.0f32, 5., 50.] {
            let (x, y, _) = ndc(&p, ratio * d, d, -d);
            assert!(close(x, 1.0) && close(y, 1.0), "d = {d}: ({x}, {y})");
            let (x, y, _) = ndc(&p, -ratio * d, -d, -d);
            assert!(close(x, -1.0) && close(y, -1.0));
        }
        let (x, y, _) = ndc(&p, 0., 0., -7.);
        assert!(close(x, 0.0) && close(y, 0.0)); // the axis stays at the centre
    }

    #[test]
    fn farther_means_smaller() {
        let p = projection(PI / 3., 1., 0.1, 100.);
        assert!(ndc(&p, 1., 0., -2.).0 > ndc(&p, 1., 0., -4.).0);
    }

    #[test]
    fn column_major_output() {
        let p = projection(PI / 2., 1., 1., 3.);
        let text = column_major_text(&p);
        assert_eq!(text.lines().count(), 4);
        let cols: Vec<Vec<f32>> = text.lines().map(|l| l.split(", ").map(|v| v.parse().unwrap()).collect()).collect();
        // column 2 holds the third column of P: (0, 0, far/(near-far), -1)
        assert!(close(cols[2][2], -1.5) && cols[2][3] == -1.0);
        // column 3 holds (0, 0, near*far/(near-far), 0)
        assert!(close(cols[3][2], -1.5) && cols[3][3] == 0.0);
    }

    #[test]
    fn invalid_parameters_are_errors() {
        for (fov, ratio, near, far) in [
            (0., 1., 1., 2.),
            (PI, 1., 1., 2.),
            (-1., 1., 1., 2.),
            (1., 0., 1., 2.),
            (1., -1., 1., 2.),
            (1., 1., 0., 2.),
            (1., 1., -1., 2.),
            (1., 1., 2., 1.),
            (1., 1., 2., 2.),
            (f32::NAN, 1., 1., 2.),
            (1., f32::NAN, 1., 2.),
            (1., 1., 1., f32::INFINITY),
        ] {
            assert!(try_projection(fov, ratio, near, far).is_err(), "{fov} {ratio} {near} {far}");
        }
    }
}

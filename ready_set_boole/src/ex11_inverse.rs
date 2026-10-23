//! Exercise 11 - Inverse function (bonus)
//!
//! Decode a real in [0, 1) back into the (x, y) point that `ex10_curve::map`
//! produced, so that `reverse_map(map(x, y)) == (x, y)` and
//! `map(reverse_map(n)) == n`.
//!
//! Idea: undo each step of ex10 in reverse order. Multiply by 2^32 to recover
//! the 32-bit Z-order index (exact, since n is a multiple of 2^-32), then
//! de-interleave it: x is the bits at even positions, y the bits at odd
//! positions (the index shifted right by one). Gathering bits uses the same
//! magic masks as ex10, applied in the opposite direction.

/// Inverse of `ex10_curve::spread`: gathers the bits at even positions.
fn compact(z: u32) -> u16 {
    let mut x = z & 0x5555_5555;
    x = (x | (x >> 1)) & 0x3333_3333;
    x = (x | (x >> 2)) & 0x0F0F_0F0F;
    x = (x | (x >> 4)) & 0x00FF_00FF;
    x = (x | (x >> 8)) & 0x0000_FFFF;
    x as u16
}

/// Inverse of `map`. Out-of-range input prints an error and returns (0, 0).
pub fn reverse_map(n: f64) -> (u16, u16) {
    if !(0.0..1.0).contains(&n) {
        eprintln!("Error: {n} is outside [0, 1)");
        return (0, 0);
    }
    let z = (n * 4294967296.0) as u32;
    (compact(z), compact(z >> 1))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ex10_curve::map;

    #[test]
    fn round_trip() {
        for x in (0..=u16::MAX).step_by(251) {
            for y in (0..=u16::MAX).step_by(257) {
                assert_eq!(reverse_map(map(x, y)), (x, y));
            }
        }
        assert_eq!(reverse_map(map(u16::MAX, u16::MAX)), (u16::MAX, u16::MAX));
    }

    #[test]
    fn inverse_both_ways() {
        for z in (0u32..=u32::MAX).step_by(65_537) {
            let n = z as f64 / 4294967296.0;
            let (x, y) = reverse_map(n);
            assert_eq!(map(x, y), n);
        }
    }
}

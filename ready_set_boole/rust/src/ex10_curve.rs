//! Exercise 10 - Curve (bonus)
//!
//! Map a point (x, y) in [0, 2^16) x [0, 2^16) to a unique real in [0, 1).
//! This is the inverse of a space-filling curve: it stores a 2D space on a
//! line.
//!
//! Idea: Z-order (Lebesgue / Morton) curve. Interleave the bits of x and y:
//! bit i of x goes to bit 2i and bit i of y to bit 2i+1 of a 32-bit index z.
//! Nearby points tend to get nearby indexes, which is why GPUs use it for
//! texture layout. The index is then divided by 2^32 to land in [0, 1). A
//! u32 fits exactly in the 53-bit mantissa of an f64, so no precision is lost
//! and the mapping stays bijective.
//!
//! The bit interleaving is done in O(1) with the classic "magic number"
//! masks (spread bits by 8, 4, 2, 1 positions).

/// Spreads the 16 bits of `v` so that bit i lands on bit 2i.
fn spread(v: u16) -> u32 {
    let mut x = v as u32;
    x = (x | (x << 8)) & 0x00FF_00FF;
    x = (x | (x << 4)) & 0x0F0F_0F0F;
    x = (x | (x << 2)) & 0x3333_3333;
    x = (x | (x << 1)) & 0x5555_5555;
    x
}

/// Z-order (Lebesgue) curve: interleave the bits of x and y into a 32-bit
/// index, then scale it into [0, 1) by dividing by 2^32. The index fits the
/// 53-bit mantissa of an f64, so the mapping is exact and bijective.
pub fn map(x: u16, y: u16) -> f64 {
    let z = spread(x) | (spread(y) << 1);
    z as f64 / 4294967296.0
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn corners() {
        assert_eq!(map(0, 0), 0.0);
        assert_eq!(map(1, 0), 1.0 / 4294967296.0);
        assert_eq!(map(0, 1), 2.0 / 4294967296.0);
        assert!(map(u16::MAX, u16::MAX) < 1.0);
    }

    #[test]
    fn injective_on_a_grid() {
        let mut seen = std::collections::HashSet::new();
        for x in (0..=u16::MAX).step_by(997) {
            for y in (0..=u16::MAX).step_by(991) {
                assert!(seen.insert(map(x, y).to_bits()));
            }
        }
    }
}

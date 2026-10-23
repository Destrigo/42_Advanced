//! Exercise 02 - Gray code
//!
//! Convert an integer to its Gray code.
//!
//! A Gray code is an ordering of binary numbers where two consecutive values
//! differ by exactly one bit (useful to avoid glitches in encoders and in
//! Karnaugh maps). The binary-reflected version is obtained by XOR-ing the
//! number with itself shifted right by one: `g = n ^ (n >> 1)`. Each output
//! bit is the XOR of two adjacent input bits, so going from n to n+1 changes
//! exactly one output bit.
//!
//! Complexity: O(1).

/// Binary-reflected Gray code: consecutive values differ by one bit.
pub fn gray_code(n: u32) -> u32 {
    n ^ (n >> 1)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        let expected = [0, 1, 3, 2, 6, 7, 5, 4, 12];
        for (n, e) in expected.iter().enumerate() {
            assert_eq!(gray_code(n as u32), *e);
        }
    }
}

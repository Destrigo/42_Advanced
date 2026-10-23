//! Exercise 01 - Multiplier
//!
//! Multiply two u32 using only bitwise operators (and the adder from ex00).
//!
//! Idea: schoolbook binary multiplication. Write b as a sum of powers of two;
//! for every bit set in b, add `a << position` to the result. In the loop we
//! look at the lowest bit of b, add a if it is 1, then shift a left and b
//! right so the next bit is examined.
//!
//! Complexity: the loop runs at most 32 times (width of b), each step calls
//! the adder (at most 32 steps), so the cost is bounded by a constant: O(1)
//! time and space for fixed-width integers. Overflow wraps modulo 2^32.

use crate::ex00_adder::adder;

/// a * b by shift-and-add (at most 32 iterations, whatever the values).
pub fn multiplier(a: u32, b: u32) -> u32 {
    let (mut a, mut b) = (a, b);
    let mut result = 0;
    while b != 0 {
        if b & 1 == 1 {
            result = adder(result, a);
        }
        a <<= 1;
        b >>= 1;
    }
    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn multiplies() {
        assert_eq!(multiplier(0, 5), 0);
        assert_eq!(multiplier(1, 5), 5);
        assert_eq!(multiplier(7, 6), 42);
        assert_eq!(multiplier(1000, 1000), 1_000_000);
        assert_eq!(multiplier(65535, 65537), 65535u32.wrapping_mul(65537));
    }
}

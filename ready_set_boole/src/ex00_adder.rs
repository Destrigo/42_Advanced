//! Exercise 00 - Adder
//!
//! Add two u32 using only `& | ^ << >>`, assignment and comparisons.
//!
//! Idea: this is how a hardware adder works. XOR adds two bits without
//! looking at the carry (1^1 = 0, 1^0 = 1), AND finds the positions where a
//! carry is produced (1&1), and shifting that left by one moves each carry to
//! the next bit. Adding the carry is the same problem again, so we repeat
//! "sum = a ^ b, carry = (a & b) << 1" until the carry is zero.
//!
//! Complexity: at most 32 iterations (the carry moves up one bit per step),
//! i.e. O(log n) in the value of the inputs, O(1) space.

/// a + b using only bitwise operators: XOR is the sum without carry,
/// AND shifted left is the carry. Repeat until no carry is left.
pub fn adder(a: u32, b: u32) -> u32 {
    let (mut a, mut b) = (a, b);
    while b != 0 {
        let carry = (a & b) << 1;
        a ^= b;
        b = carry;
    }
    a
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn adds() {
        assert_eq!(adder(0, 0), 0);
        assert_eq!(adder(1, 2), 3);
        assert_eq!(adder(255, 1), 256);
        assert_eq!(adder(123456, 654321), 777777);
        assert_eq!(adder(u32::MAX, 0), u32::MAX);
    }
}

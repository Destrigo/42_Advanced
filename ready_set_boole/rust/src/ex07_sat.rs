//! Exercise 07 - SAT
//!
//! Decide whether a formula is satisfiable, i.e. whether at least one
//! assignment of its variables makes it true.
//!
//! Idea: brute force over the 2^n assignments (SAT is the classic
//! NP-complete problem: no polynomial algorithm is known, and the exercise
//! accepts exponential time). To keep that exponential as cheap as possible
//! 64 assignments are evaluated at once: every variable is a 64-bit word where
//! bit j is its value in assignment j, and `& | ^ !` work on all 64 bits in
//! parallel. The first 6 variables get the classic alternating patterns
//! (0xAAAA..., 0xCCCC..., ...) and the others are constant per word, so 2^n
//! assignments need only 2^(n-6) evaluations of the formula.
//!
//! Examples: `AB|` is satisfiable, `AA!&` (A and not A) and `AA^` (A xor A)
//! are not. Invalid formulas print an error on stderr and return false.
//!
//! Complexity: O(2^n) assignments (O(2^n / 64) evaluations), each evaluation
//! linear in the formula.

use crate::rpn;

const PATTERN: [u64; 6] = [
    0xAAAA_AAAA_AAAA_AAAA,
    0xCCCC_CCCC_CCCC_CCCC,
    0xF0F0_F0F0_F0F0_F0F0,
    0xFF00_FF00_FF00_FF00,
    0xFFFF_0000_FFFF_0000,
    0xFFFF_FFFF_0000_0000,
];

pub fn sat(formula: &str) -> bool {
    if let Err(e) = rpn::check(formula) {
        eprintln!("Error: {e}");
        return false;
    }
    let mask = rpn::vars(formula);
    let present: Vec<usize> = (0..26).filter(|k| mask >> k & 1 == 1).collect();
    let nv = present.len();
    // With fewer than 6 variables only the low 2^nv bits of a word are real.
    let valid = if nv >= 6 { !0u64 } else { (1u64 << (1u32 << nv)) - 1 };
    let mut stack = Vec::new();
    let mut vals = [0u64; 26];
    for chunk in 0u64..(1u64 << nv.saturating_sub(6)) {
        for (v, k) in present.iter().enumerate() {
            vals[*k] = if v < 6 { PATTERN[v] } else { 0u64.wrapping_sub(chunk >> (v - 6) & 1) };
        }
        if rpn::eval_word(formula, &vals, &mut stack) & valid != 0 {
            return true;
        }
    }
    false
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert!(sat("AB|"));
        assert!(sat("AB&"));
        assert!(!sat("AA!&"));
        assert!(!sat("AA^"));
    }

    #[test]
    fn small_cases() {
        assert!(sat("A"));
        assert!(sat("A!"));
        assert!(!sat("AA!&B&"));
        assert!(sat("AB^"));
        assert!(!sat("AB=A!B&"));
        assert!(sat("1"));
        assert!(!sat("0"));
    }

    #[test]
    fn invalid_is_false() {
        for f in ["", "AB", "A&", "a"] {
            assert!(!sat(f), "{f:?}");
        }
    }

    #[test]
    fn matches_brute_force_on_every_assignment() {
        // 7 variables crosses the 6-variable word boundary.
        for f in ["ABCDEFG&&&&&&", "ABCDEFG|||||!|", "ABCDEFG&&&&&&A!&", "AB=CD=^EF^G=|"] {
            let mask = rpn::vars(f);
            let present: Vec<usize> = (0..26).filter(|k| mask >> k & 1 == 1).collect();
            let mut st = Vec::new();
            let brute = (0u32..1 << present.len()).any(|row| {
                let mut vals = [0i8; 26];
                for (b, k) in present.iter().enumerate() {
                    vals[*k] = (row >> b & 1) as i8;
                }
                rpn::eval(f, &vals, &mut st) == 1
            });
            assert_eq!(sat(f), brute, "{f}");
        }
    }
}

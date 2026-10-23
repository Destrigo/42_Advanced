//! Exercise 03 - Boolean evaluation
//!
//! Evaluate a propositional formula written in reverse polish notation (RPN).
//!
//! The formula is checked and evaluated with the explicit-stack machine in
//! `rpn.rs` (see the comment at the top of that file): each character is
//! handled once, so the cost is O(n) in the length of the formula, with no
//! recursion, however long the input is.
//!
//! Operators: `!` not, `&` and, `|` or, `^` xor, `>` implication (`!a | b`),
//! `=` equivalence (`a == b`). The only operands here are `0` and `1`.
//!
//! Invalid formulas (bad symbol, missing or leftover operands, variables)
//! print an error on stderr and return false.

use crate::rpn;

pub fn eval_formula(formula: &str) -> bool {
    if let Err(e) = rpn::check(formula) {
        eprintln!("Error: {e}");
        return false;
    }
    match rpn::eval(formula, &[-1; 26], &mut Vec::new()) {
        -1 => {
            eprintln!("Error: variables are not allowed here");
            false
        }
        r => r == 1,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert!(!eval_formula("10&"));
        assert!(eval_formula("10|"));
        assert!(eval_formula("11>"));
        assert!(!eval_formula("10="));
        assert!(eval_formula("1011||="));
        assert!(eval_formula("10|1&"));
        assert!(eval_formula("101|&"));
        assert!(!eval_formula("1!0^"));
        assert!(eval_formula("1"));
        assert!(!eval_formula("0"));
        assert!(eval_formula("0!!!"));
    }

    #[test]
    fn invalid_is_false() {
        for f in ["", "1&", "!", "11", "1 1&", "A", "1a&", "10é&"] {
            assert!(!eval_formula(f), "{f:?}");
        }
    }
}

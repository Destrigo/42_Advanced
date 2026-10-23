//! Exercise 03 - Boolean evaluation
//!
//! Evaluate a propositional formula written in reverse polish notation (RPN).
//!
//! In RPN the operator comes after its operands (`10&` is `1 AND 0`), so no
//! parentheses are needed. The shared parser in `ast.rs` reads the string left
//! to right with a stack: constants and variables are pushed; `!` pops one
//! node, every binary operator (`& | ^ > =`) pops two (right operand first)
//! and pushes the combined node. A valid formula leaves exactly one node: the
//! root of a binary AST, which is then evaluated recursively.
//!
//! Operators: `!` not, `&` and, `|` or, `^` xor, `>` implication (!a | b),
//! `=` equivalence (a == b).
//!
//! Complexity: O(n) in the length of the formula.

use crate::ast::Node;

/// Evaluates an RPN formula made of `0 1 ! & | ^ > =`.
/// Invalid formulas print an error and return false.
pub fn eval_formula(formula: &str) -> bool {
    match Node::parse(formula).and_then(|n| n.eval(&|_| None)) {
        Ok(b) => b,
        Err(e) => {
            eprintln!("Error: {e}");
            false
        }
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
    }
}

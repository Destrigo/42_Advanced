//! Exercise 07 - SAT
//!
//! Decide whether a formula is satisfiable, i.e. whether at least one
//! assignment of its variables makes it true.
//!
//! Idea: brute force. Try all 2^n assignments (the bits of a counter), return
//! true as soon as one evaluates to true, false if none does. SAT is the
//! classic NP-complete problem, so no polynomial algorithm is known; the
//! exercise accepts exponential time. Examples: `AB|` is satisfiable, `AA!&`
//! (A and not A) and `AA^` (A xor A) are not.
//!
//! Complexity: O(2^n) assignments, each evaluated in linear time.

use crate::ast::Node;

/// True if at least one assignment of the variables makes the formula true.
/// Brute force over the 2^n assignments. Invalid formulas print an error
/// and return false.
pub fn sat(formula: &str) -> bool {
    match is_satisfiable(formula) {
        Ok(b) => b,
        Err(e) => {
            eprintln!("Error: {e}");
            false
        }
    }
}

fn is_satisfiable(formula: &str) -> Result<bool, String> {
    let node = Node::parse(formula)?;
    let vars: Vec<char> = node.vars().into_iter().collect();
    for row in 0u32..(1u32 << vars.len()) {
        let env = |c: char| vars.iter().position(|v| *v == c).map(|k| (row >> k) & 1 == 1);
        if node.eval(&env)? {
            return Ok(true);
        }
    }
    Ok(false)
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
    fn invalid_is_false() {
        assert!(!sat("AB"));
    }
}

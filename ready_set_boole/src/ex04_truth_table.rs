//! Exercise 04 - Truth table
//!
//! Print the truth table of a formula whose variables are the letters A..Z.
//!
//! Idea: collect the distinct variables (sorted alphabetically, one column
//! each), then enumerate all 2^n assignments with a counter whose bits are the
//! variable values: the first variable is the most significant bit, so rows
//! come out in the usual 000, 001, 010, ... order. Each row evaluates the AST
//! with that assignment and prints the result in the last column ("=").
//!
//! Complexity: O(2^n) rows, each evaluated in time linear in the formula.

use crate::ast::Node;

/// Prints the truth table of an RPN formula over `A..Z`.
/// Columns are the variables actually used, in alphabetical order.
pub fn print_truth_table(formula: &str) {
    match truth_table(formula) {
        Ok(t) => print!("{t}"),
        Err(e) => eprintln!("Error: {e}"),
    }
}

pub fn truth_table(formula: &str) -> Result<String, String> {
    let node = Node::parse(formula)?;
    let vars: Vec<char> = node.vars().into_iter().collect();
    let n = vars.len();
    let mut out = String::new();
    for v in &vars {
        out.push_str(&format!("| {v} "));
    }
    out.push_str("| = |\n");
    for _ in 0..=n {
        out.push_str("|---");
    }
    out.push_str("|\n");
    for row in 0u32..(1u32 << n) {
        // First variable is the most significant bit of the row counter.
        let value_of = |c: char| {
            let k = vars.iter().position(|v| *v == c)?;
            Some((row >> (n - 1 - k)) & 1 == 1)
        };
        for k in 0..n {
            out.push_str(&format!("| {} ", (row >> (n - 1 - k)) & 1));
        }
        out.push_str(&format!("| {} |\n", node.eval(&value_of)? as u8));
    }
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        let expected = "\
| A | B | C | = |
|---|---|---|---|
| 0 | 0 | 0 | 0 |
| 0 | 0 | 1 | 1 |
| 0 | 1 | 0 | 0 |
| 0 | 1 | 1 | 1 |
| 1 | 0 | 0 | 0 |
| 1 | 0 | 1 | 1 |
| 1 | 1 | 0 | 1 |
| 1 | 1 | 1 | 1 |
";
        assert_eq!(truth_table("AB&C|").unwrap(), expected);
    }

    #[test]
    fn invalid() {
        assert!(truth_table("AB").is_err());
        assert!(truth_table("A&").is_err());
    }
}

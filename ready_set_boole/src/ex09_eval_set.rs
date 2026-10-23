//! Exercise 09 - Set evaluation
//!
//! Evaluate an RPN formula where every letter stands for a set of integers
//! (A is the first set given, B the second, ...) and return the resulting set.
//!
//! Logic and set theory mirror each other, so each operator has a set
//! counterpart. The universe is the union of all given sets:
//! - `!a`  complement:            universe \ a
//! - `a&b` intersection:          a ∩ b
//! - `a|b` union:                 a ∪ b
//! - `a^b` symmetric difference:  (a ∪ b) \ (a ∩ b)
//! - `a>b` material condition:    !a ∪ b
//! - `a=b` equivalence:           (a ∩ b) ∪ (!a ∩ !b)
//!
//! The formula is parsed with the shared AST and walked recursively. The
//! number of distinct variables must equal the number of sets provided.

use crate::ast::{Node, Op};
use std::collections::BTreeSet;

type Set = BTreeSet<i32>;

/// Evaluates an RPN formula where each letter is a set (A = first set, ...):
/// ! is the complement in the union of all sets, & the intersection,
/// | the union, ^ the symmetric difference.
/// Invalid input prints an error and returns an empty set.
pub fn eval_set(formula: &str, sets: Vec<Vec<i32>>) -> Vec<i32> {
    match evaluate(formula, &sets) {
        Ok(s) => s.into_iter().collect(),
        Err(e) => {
            eprintln!("Error: {e}");
            Vec::new()
        }
    }
}

fn evaluate(formula: &str, sets: &[Vec<i32>]) -> Result<Set, String> {
    let node = Node::parse(formula)?;
    let vars = node.vars();
    if vars.len() != sets.len() {
        return Err(format!(
            "{} variable(s) in the formula but {} set(s) given",
            vars.len(),
            sets.len()
        ));
    }
    if let Some(v) = vars.iter().find(|v| (**v as usize - 'A' as usize) >= sets.len()) {
        return Err(format!("variable '{v}' has no matching set"));
    }
    let sets: Vec<Set> = sets.iter().map(|s| s.iter().copied().collect()).collect();
    let universe: Set = sets.iter().flatten().copied().collect();
    walk(&node, &sets, &universe)
}

fn walk(node: &Node, sets: &[Set], universe: &Set) -> Result<Set, String> {
    match node {
        Node::Const(_) => Err("constants are not allowed in set formulas".to_string()),
        Node::Var(c) => Ok(sets[*c as usize - 'A' as usize].clone()),
        Node::Not(a) => Ok(universe - &walk(a, sets, universe)?),
        Node::Bin(op, a, b) => {
            let (x, y) = (walk(a, sets, universe)?, walk(b, sets, universe)?);
            let not_x = universe - &x;
            Ok(match op {
                Op::And => &x & &y,
                Op::Or => &x | &y,
                Op::Xor => &x ^ &y,
                Op::Imp => &not_x | &y,
                Op::Eq => &(&x & &y) | &(&not_x & &(universe - &y)),
            })
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn sorted(mut v: Vec<i32>) -> Vec<i32> {
        v.sort();
        v
    }

    #[test]
    fn matches_subject() {
        let ab = vec![vec![0, 1, 2], vec![0, 3, 4]];
        assert_eq!(eval_set("AB&", ab.clone()), vec![0]);
        assert_eq!(sorted(eval_set("AB^", ab.clone())), vec![1, 2, 3, 4]);
        assert_eq!(
            sorted(eval_set("AB|", vec![vec![0, 1, 2], vec![3, 4, 5]])),
            vec![0, 1, 2, 3, 4, 5]
        );
        assert!(eval_set("A!", vec![vec![0, 1, 2]]).is_empty());
        assert_eq!(sorted(eval_set("A!B&", ab)), vec![3, 4]);
    }

    #[test]
    fn invalid() {
        assert!(eval_set("AB&", vec![vec![1]]).is_empty());
        assert!(eval_set("AC&", vec![vec![1], vec![2]]).is_empty());
    }
}

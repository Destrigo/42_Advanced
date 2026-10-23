//! Exercise 06 - Conjunctive Normal Form (CNF)
//!
//! Rewrite a formula as an AND of ORs of literals (a literal is a variable or
//! its negation), e.g. `(A | B) & (B | !C)`.
//!
//! Idea: first convert to NNF (ex05), so only `& | !` on variables remain.
//! Then represent the formula as a list of clauses (each clause a list of
//! literals, meaning an OR):
//! - `a & b`: the clauses of a followed by the clauses of b.
//! - `a | b`: distribute OR over AND, i.e. every clause of a is merged with
//!   every clause of b (`A | (B & C) = (A | B) & (A | C)`).
//! Duplicate literals and duplicate clauses are dropped. Finally the clauses
//! are written back in RPN, both ORs and ANDs right-nested (`ABC||`, `ABC&&`).
//!
//! The output can grow exponentially with the input (distribution). Constants
//! `0`/`1` are not supported here: the exercise only uses variables.

use crate::ast::{Node, Op};
use crate::ex05_nnf::nnf;

/// A literal: variable and whether it is negated.
type Lit = (char, bool);
type Clause = Vec<Lit>;

/// Conjunctive Normal Form: a conjunction of disjunctions of literals.
/// Invalid formulas (or formulas with constants) print an error and
/// return an empty string.
pub fn conjunctive_normal_form(formula: &str) -> String {
    match Node::parse(formula).and_then(|n| clauses(&nnf(&n, false))) {
        Ok(c) => emit(&c),
        Err(e) => {
            eprintln!("Error: {e}");
            String::new()
        }
    }
}

/// Clause set of a formula already in NNF, by distributing | over &.
fn clauses(node: &Node) -> Result<Vec<Clause>, String> {
    match node {
        Node::Var(c) => Ok(vec![vec![(*c, false)]]),
        Node::Not(a) => match **a {
            Node::Var(c) => Ok(vec![vec![(c, true)]]),
            _ => Err("not in negation normal form".to_string()),
        },
        Node::Bin(Op::And, a, b) => {
            let mut left = clauses(a)?;
            for c in clauses(b)? {
                push_unique(&mut left, c);
            }
            Ok(left)
        }
        Node::Bin(Op::Or, a, b) => {
            let (left, right) = (clauses(a)?, clauses(b)?);
            let mut out = Vec::new();
            for l in &left {
                for r in &right {
                    let mut merged: Clause = Vec::new();
                    for lit in l.iter().chain(r) {
                        if !merged.contains(lit) {
                            merged.push(*lit);
                        }
                    }
                    push_unique(&mut out, merged);
                }
            }
            Ok(out)
        }
        Node::Const(_) => Err("constants are not supported in CNF".to_string()),
        Node::Bin(..) => Err("not in negation normal form".to_string()),
    }
}

fn push_unique(set: &mut Vec<Clause>, clause: Clause) {
    if !set.contains(&clause) {
        set.push(clause);
    }
}

/// RPN output: literals of a clause joined by `|`, clauses joined by `&`,
/// both right-nested (`ABC||`, `ABC&&`).
fn emit(clauses: &[Clause]) -> String {
    let mut out = String::new();
    for clause in clauses {
        for (c, neg) in clause {
            out.push(*c);
            if *neg {
                out.push('!');
            }
        }
        out.push_str(&"|".repeat(clause.len() - 1));
    }
    out.push_str(&"&".repeat(clauses.len() - 1));
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(conjunctive_normal_form("AB&!"), "A!B!|");
        assert_eq!(conjunctive_normal_form("AB|!"), "A!B!&");
        assert_eq!(conjunctive_normal_form("AB|C&"), "AB|C&");
        assert_eq!(conjunctive_normal_form("AB|C|D|"), "ABCD|||");
        assert_eq!(conjunctive_normal_form("AB&C&D&"), "ABCD&&&");
        assert_eq!(conjunctive_normal_form("AB&!C!|"), "A!B!C!||");
        assert_eq!(conjunctive_normal_form("AB|!C!&"), "A!B!C!&&");
        assert_eq!(conjunctive_normal_form("ABCD&|&"), "ABC|BD|&&");
    }

    #[test]
    fn equivalent() {
        for f in ["AB^", "AB=", "AB>C=", "AB^C|D&!", "ABC^=!"] {
            let n = Node::parse(f).unwrap();
            let r = Node::parse(&conjunctive_normal_form(f)).unwrap();
            let vars: Vec<char> = n.vars().into_iter().collect();
            for row in 0u32..(1 << vars.len()) {
                let env = |c: char| vars.iter().position(|v| *v == c).map(|k| (row >> k) & 1 == 1);
                assert_eq!(n.eval(&env), r.eval(&env), "{f}");
            }
        }
    }
}

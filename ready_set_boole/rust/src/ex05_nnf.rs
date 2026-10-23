//! Exercise 05 - Negation Normal Form (NNF)
//!
//! Rewrite a formula so that it only uses variables, `!`, `&` and `|`, and
//! every `!` sits directly on a variable.
//!
//! Idea: one recursive pass that carries a flag "this subtree is negated".
//! - `!x` just flips the flag (double negation elimination).
//! - De Morgan: `!(a & b) = !a | !b`, `!(a | b) = !a & !b`.
//! - Material condition: `a > b = !a | b`, so `!(a > b) = a & !b`.
//! - Equivalence: `a = b = (a & b) | (!a & !b)`.
//! - Xor is the negation of equivalence: `a ^ b = (a & !b) | (!a & b)`.
//! A negated variable is emitted as `A!`; a plain one as `A`.
//!
//! The result is logically equivalent to the input (checked in the tests by
//! comparing truth tables). Size can grow for nested `=`/`^` since each side
//! is needed in both polarities.

use crate::ast::{Node, Op};

/// Negation Normal Form: only variables, `!`, `&` and `|`,
/// with every `!` directly after a variable.
/// Invalid formulas print an error and return an empty string.
pub fn negation_normal_form(formula: &str) -> String {
    match Node::parse(formula) {
        Ok(n) => nnf(&n, false).to_rpn(),
        Err(e) => {
            eprintln!("Error: {e}");
            String::new()
        }
    }
}

/// NNF of `node` (or of `!node` when `neg`), pushing negations down
/// with De Morgan and the rewrite rules for ^, > and =.
pub fn nnf(node: &Node, neg: bool) -> Node {
    use Op::*;
    match node {
        Node::Const(b) => Node::Const(*b ^ neg),
        Node::Var(_) => {
            if neg {
                Node::not(node.clone())
            } else {
                node.clone()
            }
        }
        Node::Not(a) => nnf(a, !neg),
        Node::Bin(op, a, b) => {
            let (p, np) = (nnf(a, false), nnf(a, true));
            let (q, nq) = (nnf(b, false), nnf(b, true));
            match (op, neg) {
                (And, false) => Node::bin(And, p, q),
                (Or, false) => Node::bin(Or, p, q),
                // De Morgan
                (And, true) => Node::bin(Or, np, nq),
                (Or, true) => Node::bin(And, np, nq),
                // a>b = !a|b ; !(a>b) = a&!b
                (Imp, false) => Node::bin(Or, np, q),
                (Imp, true) => Node::bin(And, p, nq),
                // a=b = (a&b)|(!a&!b) ; a^b = !(a=b) = (a&!b)|(!a&b)
                (Eq, false) | (Xor, true) => {
                    Node::bin(Or, Node::bin(And, p, q), Node::bin(And, np, nq))
                }
                (Xor, false) | (Eq, true) => {
                    Node::bin(Or, Node::bin(And, p, nq), Node::bin(And, np, q))
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        assert_eq!(negation_normal_form("AB&!"), "A!B!|");
        assert_eq!(negation_normal_form("AB|!"), "A!B!&");
        assert_eq!(negation_normal_form("AB>"), "A!B|");
        assert_eq!(negation_normal_form("AB="), "AB&A!B!&|");
        assert_eq!(negation_normal_form("AB|C&!"), "A!B!&C!|");
    }

    #[test]
    fn equivalent_and_normal() {
        for f in ["AB^", "AB^!", "AB=!", "AB>!", "A!!", "AB^C=!", "ABC>=C^"] {
            let n = Node::parse(f).unwrap();
            let r = Node::parse(&negation_normal_form(f)).unwrap();
            assert!(!r.to_rpn().contains(['^', '>', '=']));
            let vars: Vec<char> = n.vars().into_iter().collect();
            for row in 0u32..(1 << vars.len()) {
                let env = |c: char| vars.iter().position(|v| *v == c).map(|k| (row >> k) & 1 == 1);
                assert_eq!(n.eval(&env), r.eval(&env), "{f}");
            }
        }
    }
}

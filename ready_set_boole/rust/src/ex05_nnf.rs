//! Exercise 05 - Negation Normal Form (NNF)
//!
//! Rewrite a formula so that it only uses variables, `!`, `&` and `|`, and
//! every `!` sits directly on a variable.
//!
//! Idea: process the RPN left to right with a stack, and keep for every
//! sub-formula x BOTH its NNF and the NNF of `!x` (called "pos" and "neg").
//! Combining two sub-formulas a and b then only needs the rewrite rules:
//! - `!x`:    swap pos and neg (double negation elimination).
//! - `a & b`: pos = `a & b`          neg = `!a | !b`        (De Morgan)
//! - `a | b`: pos = `a | b`          neg = `!a & !b`
//! - `a > b`: pos = `!a | b`         neg = `a & !b`         (material condition)
//! - `a = b`: pos = `(a & b) | (!a & !b)`                   (equivalence)
//!            neg = `(a & !b) | (!a & b)`
//! - `a ^ b`: the same as `=`, with pos and neg exchanged.
//!
//! A negated variable is emitted as `A!`; a plain one as `A`.
//!
//! The pieces are stored in an arena as a DAG (a node may be shared by
//! several parents), so building the result is linear in the input and uses no
//! recursion. Only writing it out expands the sharing, and that is where the
//! result can become exponentially large for nested `=` / `^` (each side is
//! needed in both polarities): the output is capped at [`MAX_OUT`] bytes and
//! an error is reported beyond that instead of exhausting memory.

use crate::rpn;

/// Longest result produced before giving up.
pub const MAX_OUT: usize = 1 << 26;

const NONE: u32 = u32::MAX;

#[derive(Clone, Copy)]
struct Dn {
    sym: u8,
    l: u32,
    r: u32,
}

/// NNF of a formula, or an error (invalid formula / result too large).
pub fn try_nnf(formula: &str) -> Result<String, String> {
    rpn::check(formula)?;
    let mut arena: Vec<Dn> = Vec::new();
    let mut mk = |sym: u8, l: u32, r: u32| {
        arena.push(Dn { sym, l, r });
        (arena.len() - 1) as u32
    };
    let mut st: Vec<(u32, u32)> = Vec::new(); // (pos, neg)
    for c in formula.bytes() {
        match c {
            b'0' | b'1' => {
                let pos = mk(c, NONE, NONE);
                let neg = mk(if c == b'0' { b'1' } else { b'0' }, NONE, NONE);
                st.push((pos, neg));
            }
            b'A'..=b'Z' => {
                let pos = mk(c, NONE, NONE);
                let neg = mk(b'!', pos, NONE);
                st.push((pos, neg));
            }
            b'!' => {
                if let Some(top) = st.last_mut() {
                    *top = (top.1, top.0);
                }
            }
            _ => {
                let (yp, yn) = st.pop().ok_or("stack underflow")?;
                let (xp, xn) = st.pop().ok_or("stack underflow")?;
                let r = match c {
                    b'&' | b'|' => (mk(c, xp, yp), mk(if c == b'&' { b'|' } else { b'&' }, xn, yn)),
                    b'>' => (mk(b'|', xn, yp), mk(b'&', xp, yn)),
                    _ => {
                        let eq_l = mk(b'&', xp, yp);
                        let eq_r = mk(b'&', xn, yn);
                        let eq = mk(b'|', eq_l, eq_r);
                        let xo_l = mk(b'&', xp, yn);
                        let xo_r = mk(b'&', xn, yp);
                        let xo = mk(b'|', xo_l, xo_r);
                        if c == b'=' { (eq, xo) } else { (xo, eq) }
                    }
                };
                st.push(r);
            }
        }
    }
    let root = st.pop().ok_or("empty formula")?.0;
    emit(&arena, root)
}

/// Writes the DAG rooted at `root` as RPN (post-order, explicit stack).
fn emit(arena: &[Dn], root: u32) -> Result<String, String> {
    let mut out: Vec<u8> = Vec::new();
    let mut stack: Vec<(u32, bool)> = vec![(root, false)];
    while let Some((n, seen)) = stack.pop() {
        let d = arena[n as usize];
        if d.l != NONE && !seen {
            stack.push((n, true));
            if d.r != NONE {
                stack.push((d.r, false));
            }
            stack.push((d.l, false));
        } else {
            if out.len() >= MAX_OUT {
                return Err(format!("result too large (over {MAX_OUT} bytes)"));
            }
            out.push(d.sym);
        }
    }
    String::from_utf8(out).map_err(|e| e.to_string())
}

/// Negation Normal Form of `formula`. Invalid or oversized input prints an
/// error on stderr and returns an empty string.
pub fn negation_normal_form(formula: &str) -> String {
    try_nnf(formula).unwrap_or_else(|e| {
        eprintln!("Error: {e}");
        String::new()
    })
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
    fn edge_cases() {
        assert_eq!(negation_normal_form("A"), "A");
        assert_eq!(negation_normal_form("A!"), "A!");
        assert_eq!(negation_normal_form("A!!"), "A");
        assert_eq!(negation_normal_form("A!!!"), "A!");
        for f in ["", "AB", "A&", "a", "A B&"] {
            assert_eq!(negation_normal_form(f), "", "{f:?}");
        }
    }

    #[test]
    fn equivalent_and_normal() {
        for f in ["AB^", "AB^!", "AB=!", "AB>!", "A!!", "AB^C=!", "ABC>=C^", "A", "A!"] {
            let r = negation_normal_form(f);
            assert!(!r.contains(['^', '>', '=']));
            assert!(rpn::equivalent(f, &r), "{f}");
        }
    }
}

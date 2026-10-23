//! Exercise 06 - Conjunctive Normal Form (CNF)
//!
//! Rewrite a formula as an AND of ORs of literals (a literal is a variable or
//! its negation), e.g. `(A | B) & (B | !C)`.
//!
//! Idea: represent every sub-formula as a list of clauses (each clause a list
//! of literals, meaning an OR; the list means an AND), and process the RPN
//! with a stack, as in ex05, keeping for every sub-formula x BOTH the CNF of
//! x ("pos") and the CNF of `!x` ("neg"). With "union" = concatenating the
//! clauses of two CNFs (their AND) and "or" = distributing OR over AND (every
//! clause of one is merged with every clause of the other, because
//! `A | (B & C) = (A | B) & (A | C)`):
//! - `!x`:    swap pos and neg.
//! - `a & b`: pos = union(a, b)         neg = or(!a, !b)
//! - `a | b`: pos = or(a, b)            neg = union(!a, !b)
//! - `a > b`: pos = or(!a, b)           neg = union(a, !b)
//! - `a = b`: pos = or(union(a, b), union(!a, !b))
//!            neg = or(union(a, !b), union(!a, b))
//! - `a ^ b`: the same as `=`, with pos and neg exchanged.
//!
//! Duplicate literals and duplicate clauses are dropped. Finally the clauses
//! are written back in RPN, both ORs and ANDs right-nested (`ABC||`, `ABC&&`).
//!
//! There is no recursion, so any length of input is fine. The result can grow
//! exponentially with the input (distribution); beyond [`MAX_CLAUSES`]
//! clauses an error is reported instead of exhausting time and memory.
//! Constants `0`/`1` are not supported here: the exercise only uses variables.

use crate::rpn;
use std::collections::HashSet;

pub const MAX_CLAUSES: usize = 20_000;
/// Max clause pairs merged by one `|`.
const MAX_WORK: usize = 4_000_000;

/// A literal is `(letter index << 1) | negated`.
type Clause = Vec<u8>;

#[derive(Default, Clone)]
struct Cnf {
    clauses: Vec<Clause>,
    seen: HashSet<Clause>,
}

type Res<T> = Result<T, String>;

fn too_large() -> String {
    format!("CNF too large (over {MAX_CLAUSES} clauses)")
}

impl Cnf {
    fn add(&mut self, cl: Clause) -> Res<()> {
        if self.seen.contains(&cl) {
            return Ok(());
        }
        if self.clauses.len() >= MAX_CLAUSES {
            return Err(too_large());
        }
        self.seen.insert(cl.clone());
        self.clauses.push(cl);
        Ok(())
    }

    fn lit(l: u8) -> Cnf {
        let mut c = Cnf::default();
        c.clauses.push(vec![l]);
        c.seen.insert(vec![l]);
        c
    }

    /// a AND b
    fn union(&self, other: &Cnf) -> Res<Cnf> {
        let mut out = self.clone();
        for cl in &other.clauses {
            out.add(cl.clone())?;
        }
        Ok(out)
    }

    /// a OR b, distributed.
    fn or(&self, other: &Cnf) -> Res<Cnf> {
        if self.clauses.len().saturating_mul(other.clauses.len()) > MAX_WORK {
            return Err(too_large());
        }
        let mut out = Cnf::default();
        for a in &self.clauses {
            for b in &other.clauses {
                let mut m: Clause = Vec::with_capacity(a.len() + b.len());
                for l in a.iter().chain(b) {
                    if !m.contains(l) {
                        m.push(*l);
                    }
                }
                out.add(m)?;
            }
        }
        Ok(out)
    }
}

/// (CNF of x, CNF of !x)
type Item = (Cnf, Cnf);

fn combine(c: u8, (ap, an): &Item, (bp, bn): &Item) -> Res<Item> {
    Ok(match c {
        b'&' => (ap.union(bp)?, an.or(bn)?),
        b'|' => (ap.or(bp)?, an.union(bn)?),
        b'>' => (an.or(bp)?, ap.union(bn)?),
        _ => {
            let eq = ap.union(bp)?.or(&an.union(bn)?)?;
            let xo = ap.union(bn)?.or(&an.union(bp)?)?;
            if c == b'=' { (eq, xo) } else { (xo, eq) }
        }
    })
}

fn emit(cnf: &Cnf) -> String {
    let mut out = String::new();
    for cl in &cnf.clauses {
        for l in cl {
            out.push((b'A' + (l >> 1)) as char);
            if l & 1 == 1 {
                out.push('!');
            }
        }
        out.push_str(&"|".repeat(cl.len() - 1));
    }
    out.push_str(&"&".repeat(cnf.clauses.len() - 1));
    out
}

/// CNF of a formula, or an error (invalid formula, constants, too large).
pub fn try_cnf(formula: &str) -> Res<String> {
    rpn::check(formula)?;
    let mut st: Vec<Item> = Vec::new();
    for c in formula.bytes() {
        match c {
            b'0' | b'1' => return Err("constants are not supported in CNF".to_string()),
            b'A'..=b'Z' => {
                let l = (c - b'A') << 1;
                st.push((Cnf::lit(l), Cnf::lit(l | 1)));
            }
            b'!' => {
                if let Some((p, n)) = st.last_mut() {
                    std::mem::swap(p, n);
                }
            }
            _ => {
                let b = st.pop().ok_or("stack underflow")?;
                let a = st.pop().ok_or("stack underflow")?;
                st.push(combine(c, &a, &b)?);
            }
        }
    }
    Ok(emit(&st.pop().ok_or("empty formula")?.0))
}

/// Conjunctive Normal Form of `formula`. Invalid or oversized input prints an
/// error on stderr and returns an empty string.
pub fn conjunctive_normal_form(formula: &str) -> String {
    try_cnf(formula).unwrap_or_else(|e| {
        eprintln!("Error: {e}");
        String::new()
    })
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
    fn edge_cases() {
        assert_eq!(conjunctive_normal_form("A"), "A");
        assert_eq!(conjunctive_normal_form("AA&"), "A");
        for f in ["", "AB", "A&", "a", "A1&"] {
            assert_eq!(conjunctive_normal_form(f), "", "{f:?}");
        }
    }

    #[test]
    fn equivalent() {
        for f in ["AB^", "AB=", "AB>C=", "AB^C|D&!", "ABC^=!", "A!"] {
            let r = conjunctive_normal_form(f);
            assert!(!r.contains(['^', '>', '=']));
            assert!(rpn::equivalent(f, &r), "{f}");
        }
    }
}

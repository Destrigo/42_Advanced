//! Exercise 09 - Set evaluation
//!
//! Evaluate an RPN formula where every letter stands for a set of integers
//! (A is the first set given, B the second, ...) and return the resulting set.
//!
//! Logic and set theory mirror each other, so each operator has a set
//! counterpart. The universe is the union of all given sets:
//! - `!a`  complement:           universe \ a
//! - `a&b` intersection
//! - `a|b` union
//! - `a^b` symmetric difference
//! - `a>b` material condition:   `!a | b`
//! - `a=b` equivalence:          `(a & b) | (!a & !b)`
//!
//! Idea: these operations are pointwise. An element u of the universe belongs
//! to the result exactly when the boolean formula is true after replacing each
//! letter with "u is in that set". So we build the (sorted, duplicate-free)
//! universe and evaluate the ordinary boolean formula once per element, with
//! the stack machine of `rpn.rs`. Each input set is sorted once, so "u is in
//! set k" is a binary search.
//!
//! The number of distinct variables must equal the number of sets provided,
//! and they must be A, B, C, ... without gaps. Otherwise an error is printed
//! on stderr and an empty set is returned. Duplicates inside a set are ignored.
//!
//! Complexity: O(N log N) to sort the N input numbers, then one formula
//! evaluation (plus one binary search per set) per distinct element.

use crate::rpn;

pub fn eval_set(formula: &str, sets: Vec<Vec<i32>>) -> Vec<i32> {
    try_eval_set(formula, sets).unwrap_or_else(|e| {
        eprintln!("Error: {e}");
        Vec::new()
    })
}

pub fn try_eval_set(formula: &str, sets: Vec<Vec<i32>>) -> Result<Vec<i32>, String> {
    rpn::check(formula)?;
    let n = sets.len();
    if n > 26 || rpn::vars(formula) != (1u32 << n) - 1 {
        return Err(format!(
            "the formula needs exactly the variables A..{} for the {n} set(s) given",
            (b'A' + n.saturating_sub(1) as u8) as char
        ));
    }
    let sorted: Vec<Vec<i32>> = sets
        .into_iter()
        .map(|mut s| {
            s.sort_unstable();
            s.dedup();
            s
        })
        .collect();
    let mut universe: Vec<i32> = sorted.iter().flatten().copied().collect();
    universe.sort_unstable();
    universe.dedup();
    let mut stack = Vec::new();
    let mut vals = [0i8; 26];
    Ok(universe
        .into_iter()
        .filter(|u| {
            for (k, s) in sorted.iter().enumerate() {
                vals[k] = s.binary_search(u).is_ok() as i8;
            }
            rpn::eval(formula, &vals, &mut stack) == 1
        })
        .collect())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        let ab = vec![vec![0, 1, 2], vec![0, 3, 4]];
        assert_eq!(eval_set("AB&", ab.clone()), vec![0]);
        assert_eq!(eval_set("AB^", ab.clone()), vec![1, 2, 3, 4]);
        assert_eq!(eval_set("AB|", vec![vec![0, 1, 2], vec![3, 4, 5]]), vec![0, 1, 2, 3, 4, 5]);
        assert!(eval_set("A!", vec![vec![0, 1, 2]]).is_empty());
        assert_eq!(eval_set("A!B&", ab.clone()), vec![3, 4]);
        assert_eq!(eval_set("AB>", ab), vec![0, 3, 4]);
    }

    #[test]
    fn edge_cases() {
        assert_eq!(eval_set("A", vec![vec![3, 3, 1, i32::MIN, i32::MAX]]), vec![i32::MIN, 1, 3, i32::MAX]);
        assert!(eval_set("A", vec![vec![]]).is_empty());
        assert_eq!(eval_set("A!B&", vec![vec![], vec![1]]), vec![1]);
    }

    #[test]
    fn invalid() {
        assert!(eval_set("AB&", vec![vec![1]]).is_empty()); // too few sets
        assert!(eval_set("A", vec![vec![1], vec![2]]).is_empty()); // too many sets
        assert!(eval_set("AC&", vec![vec![1], vec![2]]).is_empty()); // gap
        assert!(eval_set("AB", vec![vec![1], vec![2]]).is_empty()); // invalid RPN
        assert!(eval_set("", vec![]).is_empty());
        assert!(eval_set("A", vec![]).is_empty());
    }
}

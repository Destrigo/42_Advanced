//! Exercise 08 - Powerset
//!
//! Return the powerset P(A) of a set A: the set of all its subsets, including
//! the empty set and A itself. A set with n elements has 2^n subsets.
//!
//! Idea: build it incrementally. Start with `{ {} }`. For each element x,
//! every subset built so far either contains x or not, so we append a copy of
//! each existing subset with x added. After processing all elements we have
//! every combination exactly once (the input is assumed to have no duplicates).
//!
//! Complexity: O(2^n) space for the result, as the subject allows.

/// All subsets of `set`: start from {{}} and, for each element,
/// add a copy of every existing subset extended with that element.
pub fn powerset(set: Vec<i32>) -> Vec<Vec<i32>> {
    let mut result: Vec<Vec<i32>> = vec![Vec::new()];
    for x in set {
        let extended: Vec<Vec<i32>> = result
            .iter()
            .map(|s| {
                let mut s = s.clone();
                s.push(x);
                s
            })
            .collect();
        result.extend(extended);
    }
    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn subsets() {
        assert_eq!(powerset(vec![]), vec![Vec::<i32>::new()]);
        let mut p = powerset(vec![1, 2, 3]);
        p.sort();
        assert_eq!(
            p,
            vec![
                vec![],
                vec![1],
                vec![1, 2],
                vec![1, 2, 3],
                vec![1, 3],
                vec![2],
                vec![2, 3],
                vec![3]
            ]
        );
        assert_eq!(powerset((0..10).collect()).len(), 1024);
    }
}

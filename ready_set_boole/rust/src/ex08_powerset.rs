//! Exercise 08 - Powerset
//!
//! Return the powerset P(A) of a set A: the set of all its subsets, including
//! the empty set and A itself. A set with n elements has 2^n subsets.
//!
//! Idea: every subset corresponds to an n-bit mask (bit i set = element i is
//! in the subset). Counting from 0 to 2^n - 1 therefore enumerates every
//! combination exactly once (the input is assumed to have no duplicates). The
//! order produced is `{}, {a0}, {a1}, {a0,a1}, {a2}, ...`
//!
//! Complexity: O(2^n) space for the result, as the subject allows. Since the
//! result grows so fast, its size is computed before allocating anything:
//! beyond [`MAX_BYTES`] an error is printed on stderr and an empty vector
//! (not even the empty subset, which a valid result always contains) is
//! returned.

/// Largest result (estimated) that will be built.
pub const MAX_BYTES: usize = 2 << 30;

fn fits(n: usize) -> bool {
    if n > 40 {
        return false;
    }
    let count = 1usize << n;
    let per_vec = std::mem::size_of::<Vec<i32>>();
    count
        .checked_mul(per_vec)
        .and_then(|a| a.checked_add(n * (count / 2) * std::mem::size_of::<i32>()))
        .is_some_and(|total| total <= MAX_BYTES)
}

pub fn powerset(set: Vec<i32>) -> Vec<Vec<i32>> {
    let n = set.len();
    if !fits(n) {
        eprintln!("Error: powerset of {n} elements is too large (over {MAX_BYTES} bytes)");
        return Vec::new();
    }
    (0usize..1 << n)
        .map(|mask| {
            let mut s = Vec::with_capacity(mask.count_ones() as usize);
            s.extend((0..n).filter(|i| mask >> i & 1 == 1).map(|i| set[i]));
            s
        })
        .collect()
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
            vec![vec![], vec![1], vec![1, 2], vec![1, 2, 3], vec![1, 3], vec![2], vec![2, 3], vec![3]]
        );
        assert_eq!(powerset(vec![i32::MIN, i32::MAX])[3], vec![i32::MIN, i32::MAX]);
        let big = powerset((0..20).collect());
        assert_eq!(big.len(), 1 << 20);
        assert_eq!(big[big.len() - 1].len(), 20);
    }

    #[test]
    fn too_large_is_refused_not_attempted() {
        assert!(powerset((0..40).collect()).is_empty());
        assert!(powerset((0..64).collect()).is_empty());
    }
}

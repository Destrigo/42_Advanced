//! Hostile-input tests: nothing may panic or overflow the stack.

use ready_set_boole::ast::{Node, MAX_DEPTH};
use ready_set_boole::ex03_eval_formula::eval_formula;
use ready_set_boole::ex04_truth_table::truth_table;
use ready_set_boole::ex05_nnf::negation_normal_form;
use ready_set_boole::ex06_cnf::conjunctive_normal_form;
use ready_set_boole::ex07_sat::sat;
use ready_set_boole::ex09_eval_set::eval_set;
use ready_set_boole::ex11_inverse::reverse_map;

struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u32 {
        self.0 ^= self.0 << 13;
        self.0 ^= self.0 >> 7;
        self.0 ^= self.0 << 17;
        (self.0 >> 11) as u32
    }
}

#[test]
fn garbage_never_panics() {
    let alpha: Vec<char> = "01ABCZ!&|^>=  xé\u{0}€".chars().collect();
    let mut rng = Rng(88172645463325252);
    for _ in 0..100_000 {
        let len = rng.next() % 24;
        let s: String = (0..len).map(|_| alpha[rng.next() as usize % alpha.len()]).collect();
        eval_formula(&s);
        sat(&s);
        let _ = truth_table(&s);
        negation_normal_form(&s);
        conjunctive_normal_form(&s);
        let _ = eval_set(&s, vec![vec![1], vec![2]]);
    }
}

#[test]
fn deep_formulas_are_rejected() {
    let n = 200_000;
    let deep = format!("{}{}", "A".repeat(n), "&".repeat(n - 1));
    assert!(Node::parse(&deep).is_err());
    assert!(!sat(&deep));
    assert!(negation_normal_form(&deep).is_empty());
    let deep_not = format!("A{}", "!".repeat(n));
    assert!(Node::parse(&deep_not).is_err());

    // Just under the limit still works.
    let ok = format!("{}{}", "A".repeat(MAX_DEPTH - 1), "&".repeat(MAX_DEPTH - 2));
    assert!(sat(&ok));
}

#[test]
fn edge_cases() {
    assert!(!eval_formula(""));
    assert!(!eval_formula("1 1&"));
    assert!(conjunctive_normal_form("A1&").is_empty());
    assert!(eval_set("A", vec![]).is_empty());
    assert_eq!(reverse_map(f64::NAN), (0, 0));
    assert_eq!(reverse_map(f64::INFINITY), (0, 0));
    assert_eq!(reverse_map(-0.0), (0, 0));
    // Many distinct variables (2^20 assignments; 26 would take minutes: SAT is exponential).
    assert!(sat("ABCDEFGHIJKLMNOPQRST&&&&&&&&&&&&&&&&&&&"));
}

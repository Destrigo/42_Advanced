//! Hostile-input tests: nothing may panic, hang, overflow the stack or eat all
//! the memory. Error messages on stderr are expected.

use ready_set_boole::ex00_adder::adder;
use ready_set_boole::ex01_multiplier::multiplier;
use ready_set_boole::ex02_gray_code::gray_code;
use ready_set_boole::ex03_eval_formula::eval_formula;
use ready_set_boole::ex04_truth_table::{truth_table, write_truth_table};
use ready_set_boole::ex05_nnf::{negation_normal_form, try_nnf};
use ready_set_boole::ex06_cnf::{conjunctive_normal_form, try_cnf};
use ready_set_boole::ex07_sat::sat;
use ready_set_boole::ex08_powerset::powerset;
use ready_set_boole::ex09_eval_set::eval_set;
use ready_set_boole::ex10_curve::map;
use ready_set_boole::ex11_inverse::reverse_map;
use ready_set_boole::rpn;

struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u32 {
        self.0 ^= self.0 << 13;
        self.0 ^= self.0 >> 7;
        self.0 ^= self.0 << 17;
        (self.0 >> 11) as u32
    }
}

/// Random valid RPN formula over A..(A+nv-1) with about `ops` operators.
fn gen(rng: &mut Rng, out: &mut String, ops: i32, nv: u32) {
    if ops <= 0 || rng.next() % 5 == 0 {
        out.push((b'A' + (rng.next() % nv) as u8) as char);
    } else if rng.next() % 4 == 0 {
        gen(rng, out, ops - 1, nv);
        out.push('!');
    } else {
        gen(rng, out, (ops - 1) / 2, nv);
        gen(rng, out, ops - 1 - (ops - 1) / 2, nv);
        out.push(['&', '|', '^', '>', '='][(rng.next() % 5) as usize]);
    }
}

fn long_chain(operand: &str, op: &str, n: usize) -> String {
    format!("{}{}", operand.repeat(n), op.repeat(n - 1))
}

#[test]
fn garbage_never_panics() {
    let alpha: Vec<char> = "01ABCZ!&|^>=  xé\u{0}€\n".chars().collect();
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
fn random_formulas_are_equivalent_to_their_normal_forms() {
    let mut rng = Rng(7);
    for _ in 0..5_000 {
        let mut f = String::new();
        let ops = (rng.next() % 9) as i32;
        let nv = 1 + rng.next() % 4;
        gen(&mut rng, &mut f, ops, nv);
        let nnf = try_nnf(&f).expect("small formulas fit");
        assert!(!nnf.contains(['^', '>', '=', '0', '1']), "{f} -> {nnf}");
        assert!(rpn::equivalent(&f, &nnf), "{f} -> {nnf}");
        let cnf = try_cnf(&f).expect("small formulas fit");
        assert!(!cnf.contains(['^', '>', '=', '0', '1']), "{f} -> {cnf}");
        assert!(rpn::equivalent(&f, &cnf), "{f} -> {cnf}");
    }
}

#[test]
fn huge_formulas_work_without_recursion() {
    let n = 1_000_000;
    assert!(eval_formula(&long_chain("1", "&", n))); // right-nested
    assert!(eval_formula(&format!("1{}", "0|".repeat(n)))); // left-nested
    assert!(eval_formula(&format!("1{}", "!".repeat(2 * n)))); // negation chain
    let a = long_chain("A", "&", n);
    assert!(sat(&a));
    assert_eq!(negation_normal_form(&a).len(), 2 * n - 1);
    assert_eq!(conjunctive_normal_form(&a), "A");
    let alt = format!("A{}", "B|C&".repeat(n / 4));
    assert!(sat(&alt));
    assert!(!conjunctive_normal_form(&alt).is_empty());
}

#[test]
fn exponential_blowups_are_refused() {
    let mut f = String::from("AB=");
    for i in 0..60 {
        f.push_str(if i % 2 == 1 { "C=" } else { "D=" });
    }
    assert!(try_nnf(&f).is_err());
    assert!(try_cnf(&f).is_err());
    assert!(sat(&f));
}

#[test]
fn sat_with_many_variables_is_fast() {
    let all = format!("{}{}", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "&".repeat(25));
    assert!(sat(&all)); // 2^26 assignments
    let unsat = format!("AA!&{}", "BCDEFGHIJKLMNOPQRSTUVWXYZ".chars().map(|c| format!("{c}&")).collect::<String>());
    assert!(!sat(&unsat));
}

#[test]
fn arithmetic_matches_the_real_thing() {
    let mut rng = Rng(42);
    for _ in 0..200_000 {
        let a = rng.next() << (rng.next() % 20);
        let b = rng.next() << (rng.next() % 20);
        assert_eq!(adder(a, b), a.wrapping_add(b));
        assert_eq!(multiplier(a, b), a.wrapping_mul(b));
    }
    assert_eq!(adder(u32::MAX, 1), 0);
    assert_eq!(multiplier(u32::MAX, u32::MAX), 1);
    assert_eq!(gray_code(u32::MAX), 0x8000_0000);
    for n in 0..1_000_000u32 {
        assert_eq!((gray_code(n) ^ gray_code(n + 1)).count_ones(), 1);
    }
}

#[test]
fn powerset_limits() {
    assert!(powerset((0..40).collect()).is_empty());
    assert!(powerset((0..64).collect()).is_empty());
    assert_eq!(powerset(vec![]).len(), 1);
}

#[test]
fn big_sets() {
    let n = 200_000i32;
    let x: Vec<i32> = (0..n).map(|i| (i % (n / 2) - 50_000) * 2).chain([i32::MIN]).collect();
    let y: Vec<i32> = (0..n).map(|i| (i - 100_000) * 3).chain([i32::MAX]).collect();
    let both = eval_set("AB&", vec![x.clone(), y.clone()]);
    assert!(both.windows(2).all(|w| w[0] < w[1]) && both.iter().all(|v| v % 6 == 0));
    let any = eval_set("AB|", vec![x, y]);
    assert!(any.len() > n as usize && any.windows(2).all(|w| w[0] < w[1]));
}

#[test]
fn curve_edge_cases() {
    assert_eq!(reverse_map(f64::NAN), (0, 0));
    assert_eq!(reverse_map(f64::INFINITY), (0, 0));
    assert_eq!(reverse_map(f64::NEG_INFINITY), (0, 0));
    assert_eq!(reverse_map(-0.0), (0, 0));
    assert_eq!(reverse_map(1.0), (0, 0));
    assert_eq!(reverse_map(0.999_999_999_999_999_9), (65535, 65535));
    assert!(map(65535, 65535) < 1.0);
    let mut prev = -1.0;
    for x in 0..=u16::MAX {
        let v = map(x, 12345);
        assert!(v != prev);
        prev = v;
    }
}

#[test]
fn truth_table_survives_a_closed_pipe() {
    struct Closed;
    impl std::io::Write for Closed {
        fn write(&mut self, _: &[u8]) -> std::io::Result<usize> {
            Err(std::io::ErrorKind::BrokenPipe.into())
        }
        fn flush(&mut self) -> std::io::Result<()> {
            Ok(())
        }
    }
    assert!(write_truth_table("ABCDEFGHIJKLMNOPQRSTUVWXYZ&&&&&&&&&&&&&&&&&&&&&&&&&", &mut Closed).is_err());
}

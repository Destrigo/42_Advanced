//! Shared helpers for formulas written in reverse polish notation (RPN).
//!
//! In RPN the operator comes after its operands (`10&` is `1 AND 0`), so no
//! parentheses are needed and a formula can be checked and evaluated with a
//! single left-to-right pass and a stack:
//! - `0`, `1` and variables push a value;
//! - `!` replaces the top of the stack by its negation;
//! - a binary operator (`& | ^ > =`) pops two values (right operand first) and
//!   pushes the result.
//!
//! A formula is valid when the stack never underflows and exactly one value is
//! left at the end. Doing it this way (instead of building a recursive tree)
//! means very long or very deep formulas cannot overflow the call stack.

/// Syntax check of an RPN formula (`0 1 A..Z ! & | ^ > =`).
pub fn check(f: &str) -> Result<(), String> {
    let mut depth: usize = 0;
    for (i, c) in f.chars().enumerate() {
        match c {
            '0' | '1' | 'A'..='Z' => depth += 1,
            '!' if depth < 1 => return Err(format!("missing operand for '!' at position {i}")),
            '!' => {}
            '&' | '|' | '^' | '>' | '=' => {
                if depth < 2 {
                    return Err(format!("missing operand for '{c}' at position {i}"));
                }
                depth -= 1;
            }
            _ => return Err(format!("invalid symbol {c:?} at position {i}")),
        }
    }
    match depth {
        0 => Err("empty formula".to_string()),
        1 => Ok(()),
        _ => Err("leftover operands".to_string()),
    }
}

/// Applies a binary operator symbol.
pub fn op(c: u8, a: bool, b: bool) -> bool {
    match c {
        b'&' => a & b,
        b'|' => a | b,
        b'^' => a ^ b,
        b'>' => !a | b,
        _ => a == b,
    }
}

/// Bit i of the result is set if the variable `'A' + i` appears. `f` must be valid.
pub fn vars(f: &str) -> u32 {
    f.bytes()
        .filter(|c| c.is_ascii_uppercase())
        .fold(0, |m, c| m | 1 << (c - b'A'))
}

/// Evaluates a valid formula. `vals[i]` is the value of variable `'A' + i`
/// (0, 1 or -1 = unbound). Returns 0 or 1, or -1 if an unbound variable was used.
pub fn eval(f: &str, vals: &[i8; 26], stack: &mut Vec<i8>) -> i8 {
    stack.clear();
    for c in f.bytes() {
        match c {
            b'0' => stack.push(0),
            b'1' => stack.push(1),
            b'A'..=b'Z' => stack.push(vals[(c - b'A') as usize]),
            b'!' => {
                let a = stack.pop().unwrap_or(-1);
                stack.push(if a < 0 { -1 } else { 1 - a });
            }
            _ => {
                let b = stack.pop().unwrap_or(-1);
                let a = stack.pop().unwrap_or(-1);
                stack.push(if a < 0 || b < 0 { -1 } else { op(c, a == 1, b == 1) as i8 });
            }
        }
    }
    stack.pop().unwrap_or(-1)
}

/// Same as [`eval`] but evaluates 64 assignments at once: bit j of `vals[i]`
/// is the value of variable i in assignment j.
pub fn eval_word(f: &str, vals: &[u64; 26], stack: &mut Vec<u64>) -> u64 {
    stack.clear();
    for c in f.bytes() {
        match c {
            b'0' => stack.push(0),
            b'1' => stack.push(!0),
            b'A'..=b'Z' => stack.push(vals[(c - b'A') as usize]),
            b'!' => {
                let a = stack.pop().unwrap_or(0);
                stack.push(!a);
            }
            _ => {
                let b = stack.pop().unwrap_or(0);
                let a = stack.pop().unwrap_or(0);
                stack.push(match c {
                    b'&' => a & b,
                    b'|' => a | b,
                    b'^' => a ^ b,
                    b'>' => !a | b,
                    _ => !(a ^ b),
                });
            }
        }
    }
    stack.pop().unwrap_or(0)
}

/// Brute-force equivalence check of two valid formulas (up to 20 distinct
/// variables in total). Meant for tests.
pub fn equivalent(f: &str, g: &str) -> bool {
    let mask = vars(f) | vars(g);
    let present: Vec<usize> = (0..26).filter(|k| mask >> k & 1 == 1).collect();
    if present.len() > 20 {
        return false;
    }
    let (mut sf, mut sg) = (Vec::new(), Vec::new());
    (0u32..1 << present.len()).all(|row| {
        let mut vals = [0i8; 26];
        for (bit, k) in present.iter().enumerate() {
            vals[*k] = (row >> bit & 1) as i8;
        }
        eval(f, &vals, &mut sf) == eval(g, &vals, &mut sg)
    })
}

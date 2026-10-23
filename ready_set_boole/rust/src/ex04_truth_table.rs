//! Exercise 04 - Truth table
//!
//! Print the truth table of a formula whose variables are the letters A..Z.
//!
//! Idea: collect the distinct variables (sorted alphabetically, one column
//! each), then enumerate all 2^n assignments with a counter whose bits are the
//! variable values: the first variable is the most significant bit, so rows
//! come out in the usual 000, 001, 010, ... order. Each row evaluates the
//! formula with that assignment (stack machine, see `rpn.rs`) and prints the
//! result in the last column ("="). Rows are streamed through a buffered
//! writer instead of being collected, so even 26 variables use no extra
//! memory, and a closed pipe (`| head`) just stops the output.
//!
//! Complexity: O(2^n) rows, each evaluated in time linear in the formula.
//! Invalid formulas print an error on stderr and nothing on stdout.

use crate::rpn;
use std::io::{self, BufWriter, Write};

pub fn print_truth_table(formula: &str) {
    let stdout = io::stdout();
    let mut out = BufWriter::new(stdout.lock());
    // A write error (closed pipe...) is not worth a panic: just stop.
    if let Err(e) = write_truth_table(formula, &mut out) {
        if e.kind() == io::ErrorKind::InvalidInput {
            eprintln!("Error: {e}");
        }
    }
    let _ = out.flush();
}

/// Same output, returned as a string (for small tables, mainly tests).
pub fn truth_table(formula: &str) -> Result<String, String> {
    let mut buf = Vec::new();
    write_truth_table(formula, &mut buf).map_err(|e| e.to_string())?;
    String::from_utf8(buf).map_err(|e| e.to_string())
}

pub fn write_truth_table<W: Write>(formula: &str, out: &mut W) -> io::Result<()> {
    rpn::check(formula).map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))?;
    let mask = rpn::vars(formula);
    let vars: Vec<u8> = (0..26u8).filter(|k| mask >> k & 1 == 1).collect();
    let n = vars.len();
    let mut header = String::from("|");
    for v in &vars {
        header.push_str(&format!(" {} |", (b'A' + v) as char));
    }
    header.push_str(" = |\n|");
    header.push_str(&"---|".repeat(n + 1));
    writeln!(out, "{header}")?;
    let mut vals = [-1i8; 26];
    let mut stack = Vec::new();
    let mut line = String::new();
    for row in 0u32..(1u32 << n) {
        line.clear();
        line.push('|');
        for (k, v) in vars.iter().enumerate() {
            let bit = (row >> (n - 1 - k) & 1) as i8;
            vals[*v as usize] = bit;
            line.push_str(if bit == 1 { " 1 |" } else { " 0 |" });
        }
        line.push_str(if rpn::eval(formula, &vals, &mut stack) == 1 { " 1 |\n" } else { " 0 |\n" });
        out.write_all(line.as_bytes())?;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matches_subject() {
        let expected = "\
| A | B | C | = |
|---|---|---|---|
| 0 | 0 | 0 | 0 |
| 0 | 0 | 1 | 1 |
| 0 | 1 | 0 | 0 |
| 0 | 1 | 1 | 1 |
| 1 | 0 | 0 | 0 |
| 1 | 0 | 1 | 1 |
| 1 | 1 | 0 | 1 |
| 1 | 1 | 1 | 1 |
";
        assert_eq!(truth_table("AB&C|").unwrap(), expected);
    }

    #[test]
    fn single_variable_and_constants() {
        assert_eq!(truth_table("A!").unwrap(), "| A | = |\n|---|---|\n| 0 | 1 |\n| 1 | 0 |\n");
        assert_eq!(truth_table("1").unwrap(), "| = |\n|---|\n| 1 |\n");
    }

    #[test]
    fn invalid() {
        for f in ["", "AB", "A&", "a", "A B&", "AB&\n"] {
            assert!(truth_table(f).is_err(), "{f:?}");
        }
    }
}

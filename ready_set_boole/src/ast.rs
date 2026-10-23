//! Binary AST for propositional formulas written in reverse polish notation.

use std::collections::BTreeSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Op {
    And,
    Or,
    Xor,
    Imp,
    Eq,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Node {
    Const(bool),
    Var(char),
    Not(Box<Node>),
    Bin(Op, Box<Node>, Box<Node>),
}

impl Op {
    fn from_char(c: char) -> Option<Op> {
        match c {
            '&' => Some(Op::And),
            '|' => Some(Op::Or),
            '^' => Some(Op::Xor),
            '>' => Some(Op::Imp),
            '=' => Some(Op::Eq),
            _ => None,
        }
    }

    fn symbol(self) -> char {
        match self {
            Op::And => '&',
            Op::Or => '|',
            Op::Xor => '^',
            Op::Imp => '>',
            Op::Eq => '=',
        }
    }

    pub fn apply(self, a: bool, b: bool) -> bool {
        match self {
            Op::And => a & b,
            Op::Or => a | b,
            Op::Xor => a ^ b,
            Op::Imp => !a | b,
            Op::Eq => a == b,
        }
    }
}

impl Node {
    pub fn not(n: Node) -> Node {
        Node::Not(Box::new(n))
    }

    pub fn bin(op: Op, a: Node, b: Node) -> Node {
        Node::Bin(op, Box::new(a), Box::new(b))
    }

    /// Parses a formula in RPN (`0`, `1`, `A..Z`, `! & | ^ > =`).
    pub fn parse(formula: &str) -> Result<Node, String> {
        let mut stack: Vec<Node> = Vec::new();
        for (i, c) in formula.chars().enumerate() {
            match c {
                '0' => stack.push(Node::Const(false)),
                '1' => stack.push(Node::Const(true)),
                'A'..='Z' => stack.push(Node::Var(c)),
                '!' => {
                    let a = stack.pop().ok_or(format!("'!' at {i}: missing operand"))?;
                    stack.push(Node::not(a));
                }
                _ => {
                    let op = Op::from_char(c).ok_or(format!("invalid symbol '{c}' at {i}"))?;
                    let b = stack.pop().ok_or(format!("'{c}' at {i}: missing operand"))?;
                    let a = stack.pop().ok_or(format!("'{c}' at {i}: missing operand"))?;
                    stack.push(Node::bin(op, a, b));
                }
            }
        }
        match (stack.pop(), stack.is_empty()) {
            (Some(n), true) => Ok(n),
            (None, _) => Err("empty formula".to_string()),
            _ => Err("malformed formula: leftover operands".to_string()),
        }
    }

    /// Evaluates the tree; `env` gives the value of each variable.
    pub fn eval(&self, env: &dyn Fn(char) -> Option<bool>) -> Result<bool, String> {
        match self {
            Node::Const(b) => Ok(*b),
            Node::Var(c) => env(*c).ok_or(format!("unbound variable '{c}'")),
            Node::Not(a) => Ok(!a.eval(env)?),
            Node::Bin(op, a, b) => Ok(op.apply(a.eval(env)?, b.eval(env)?)),
        }
    }

    pub fn vars(&self) -> BTreeSet<char> {
        let mut set = BTreeSet::new();
        self.collect_vars(&mut set);
        set
    }

    fn collect_vars(&self, set: &mut BTreeSet<char>) {
        match self {
            Node::Const(_) => {}
            Node::Var(c) => {
                set.insert(*c);
            }
            Node::Not(a) => a.collect_vars(set),
            Node::Bin(_, a, b) => {
                a.collect_vars(set);
                b.collect_vars(set);
            }
        }
    }

    pub fn to_rpn(&self) -> String {
        let mut out = String::new();
        self.write_rpn(&mut out);
        out
    }

    fn write_rpn(&self, out: &mut String) {
        match self {
            Node::Const(b) => out.push(if *b { '1' } else { '0' }),
            Node::Var(c) => out.push(*c),
            Node::Not(a) => {
                a.write_rpn(out);
                out.push('!');
            }
            Node::Bin(op, a, b) => {
                a.write_rpn(out);
                b.write_rpn(out);
                out.push(op.symbol());
            }
        }
    }
}

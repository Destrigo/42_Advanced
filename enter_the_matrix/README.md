# Enter the Matrix

Introduction to linear algebra (exercises 00-15, including both bonuses),
implemented twice:

| Directory | Language | Build / test |
|---|---|---|
| [rust](rust/) | Rust | `cargo test`, `cargo run --bin ex00` ... `ex15` |
| [c](c/) | C99 | `make`, `make test`, `bin/ex00` ... `bin/ex15` |

The subject gives its prototypes in Rust and asks for a *generic* library, so
everything is written once over a field of scalars `K`: `trait Scalar` in Rust,
with `f32` for the real numbers and `Complex` (exercise 15) for the bonus.
Vectors and matrices are both vector spaces (`trait VectorSpace`), which is what
lets `linear_combination` and `lerp` work on either.

**The C version** has no generics, so it gets the same effect with a table of
operations, `t_field` (`g_real` = float, `g_complex`): `t_vec` and `t_mat`
carry a pointer to their field, and every function works through it, exactly
like the Rust trait. Functions return a `t_err` code (never abort) and leave
their arguments untouched on error; scalars are passed as `void *` to a `K`
(`float *` for real data, `t_complex *` for complex data). It builds with
`-Wall -Wextra -Werror -std=c99 -pedantic -ffp-contract=off` (no hidden fusing
of operations: the only fused ones are the explicit `fmaf` the subject allows)
and links `-lm` for `fmaf`, `powf` and `tanf` only.

Each `src/exNN_*.rs` file starts with an explanation of the maths and of the
method, the allowed functions and the complexity.

## Choices worth knowing for the defence

- **Matrices in the examples are written row by row** (`[[1, 2], [3, 4]]` has
  the rows `[1, 2]` and `[3, 4]`): it is the only reading that reproduces the
  products and the inverse printed in the subject. `to_column_major()` /
  `column_major_text()` give the column-by-column layout for the GPU / display.
- **Forbidden maths functions:** the only ones used are the fused multiply-add
  (`mul_add`), `powf` (the square root is `pow(x, 0.5)`) and `tan`; the absolute
  value and the maximum are comparisons. Exercises 00, 08, 09, 10 and 13, which
  allow none, use only `+ - * /`.
- **Undefined behaviour is defined as a clear panic** in the functions with the
  subject's names (`add`, `dot`, `mul_mat`...). Each has a `try_*` twin that
  returns a `MatrixError` instead (`inverse` returns a `Result` directly, as
  the subject asks).
- **Numerical care:** the 2-norm and complex modulus do not overflow/underflow;
  `cross_product` compensates the cancellation error with the fused
  multiply-add; `lerp` is exact at `t = 0` and `t = 1`; `angle_cos` is clamped to
  `[-1, 1]`; rank / row echelon / inverse use scaled partial pivoting with a
  tolerance relative to each row, so noise is not mistaken for a pivot and a
  badly scaled matrix such as `diag(1e8, 1)` keeps its rank.
- **Exercise 10** returns the *reduced* row echelon form (what the examples show).
- **Exercise 11** uses cofactor expansion for n <= 4 (exact on small integers)
  and LU decomposition beyond.
- **Exercise 14**: the display program of the subject is not in the repository,
  so the matrix is checked numerically instead (near plane -> z = 0, far plane
  -> z = 1, frustum edges -> x, y = +-1). Conventions (camera looking down -Z,
  z in [0, 1], y up, `fov` vertical and in radians, column vectors) are
  documented at the top of `ex14_projection.rs`; `cargo run --bin ex14 -- 60 1.78 0.1 100`
  prints the text to put in the `proj` file.
- **Complex bonus**: `dot` conjugates its first argument; `transpose` is the
  plain transpose and `conjugate_transpose` the one that goes with complex
  numbers; norms and `angle_cos` return real numbers.

## Tests

`cargo test` runs the subject's examples, unit tests per exercise, algebraic
laws on random real and complex matrices (`A * A^-1 = I`, `det(AB) = det A det B`,
Cauchy-Schwarz, triangle inequality, `(AB)^T = B^T A^T`...) and hostile input
(wrong shapes, empty vectors, NaN / infinity, extreme magnitudes, shape
overflow).

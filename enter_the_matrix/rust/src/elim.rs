//! Gaussian elimination shared by the row-echelon form (exercise 10) and the
//! rank (exercise 13). Plain `+ - * /` only: those exercises allow no
//! mathematical function.
//!
//! Decisions of the algorithm, all numerical:
//! - scaled partial pivoting: in each column the pivot is the entry that is
//!   largest *relative to the size of its row*, which keeps the elimination
//!   stable and, above all, makes "is this entry zero?" independent of the
//!   scale of the row (`diag(1e8, 1)` has rank 2);
//! - an entry counts as zero when it is below `n * epsilon` times the
//!   magnitude of the terms that produced it (the row's original scale, grown
//!   by what was subtracted from it): this is what removes the rounding noise
//!   left by `[[1,2,3],[4,5,6],[7,8,9]]` while exact rank deficiency is
//!   detected exactly.

use crate::scalar::Scalar;

/// Reduces `data` (row by row, `rows` x `cols`) to its reduced row echelon
/// form in place and returns the rank (the number of pivots).
pub(crate) fn rref<K: Scalar>(data: &mut [K], rows: usize, cols: usize) -> usize {
    let dim = if rows > cols { rows } else { cols };
    let tol = dim as f32 * f32::EPSILON;
    let mut scale: Vec<f32> = (0..rows)
        .map(|i| data[i * cols..(i + 1) * cols].iter().fold(0.0f32, |m, x| {
            let a = x.modulus();
            if a > m {
                a
            } else {
                m
            }
        }))
        .collect();
    let zero = K::zero();
    let mut r = 0;
    for c in 0..cols {
        if r == rows {
            break;
        }
        // pivot: the largest entry of the column relative to its row
        let (mut p, mut best) = (r, -1.0f32);
        for i in r..rows {
            let ratio = if scale[i] > 0.0 { data[i * cols + c].modulus() / scale[i] } else { 0.0 };
            if ratio > best {
                best = ratio;
                p = i;
            }
        }
        if !(best > tol) {
            // nothing but noise below r in this column: it holds no pivot
            for i in r..rows {
                data[i * cols + c] = zero;
            }
            continue;
        }
        if p != r {
            for j in 0..cols {
                data.swap(p * cols + j, r * cols + j);
            }
            scale.swap(p, r);
        }
        let piv = data[r * cols + c];
        for j in c + 1..cols {
            data[r * cols + j] = data[r * cols + j] / piv;
        }
        data[r * cols + c] = K::one();
        scale[r] /= piv.modulus();
        for i in 0..rows {
            if i == r {
                continue;
            }
            let f = data[i * cols + c];
            if f == zero {
                continue;
            }
            for j in c + 1..cols {
                data[i * cols + j] = data[i * cols + j] - f * data[r * cols + j];
            }
            data[i * cols + c] = zero;
            scale[i] += f.modulus() * scale[r];
        }
        r += 1;
    }
    r
}

/*
** Exercise 10 - Row-echelon form
**
** Gauss-Jordan elimination: the matrix is transformed with the three row
** operations that do not change the solutions of the linear system it
** represents (swap two rows, scale a row by a non-zero number, add a multiple
** of a row to another) until it is in *reduced* row echelon form, the one
** shown by the examples of the subject:
**  - the first non-zero entry of each non-zero row (its pivot) is 1;
**  - each pivot is the only non-zero entry of its column;
**  - pivots go down and to the right, and zero rows are at the bottom.
**
** Algorithm, column by column: choose the pivot (scaled partial pivoting, see
** elim.c), swap it up, divide its row by the pivot, and subtract multiples of
** that row from all the others to clear the column. Applied to the augmented
** matrix [A | c] of a system Ab = c, the last column of the result is the
** solution (see the interlude of the subject).
**
** No mathematical function is allowed. Complexity: O(n^3) time (for an n x n
** matrix, O(mn * min(m, n)) in general), O(n^2) memory (one copy of the
** matrix). The results are approximate in the usual floating point sense.
*/

#include <string.h>
#include "internal.h"

t_err	mat_row_echelon(t_mat *out, const t_mat *m)
{
	t_err	e;
	size_t	rank;

	e = mat_clone(out, m);
	if (e != E_OK)
		return (e);
	e = rref_inplace(out->f, out->data, out->rows, out->cols, &rank);
	if (e != E_OK)
		mat_free(out);
	return (e);
}

/*
** Exercise 09 - Transpose
**
** The transpose of an m x n matrix A is the n x m matrix B with B_ji = A_ij:
** rows become columns. It satisfies (AB)^T = B^T A^T, (A^T)^T = A, and swaps
** the roles of the two dot-product sides (<Au|v> = <u|A^T v> over the reals).
**
** mat_transpose is the plain transpose, valid over any field. Over the
** complex numbers the matching operation is the conjugate transpose (every
** entry is also conjugated), provided as mat_conjugate_transpose; on real
** matrices both are identical.
**
** No mathematical function is allowed here. The time complexity counts value
** assignments: one per entry, O(nm), and O(nm) memory for the new matrix.
*/

#include <string.h>
#include "internal.h"

t_err	mat_transpose(t_mat *out, const t_mat *m)
{
	t_err	e;
	size_t	i;
	size_t	j;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!m || !m->f)
		return (E_INVALID);
	e = mat_new(out, m->f, m->cols, m->rows);
	for (i = 0; e == E_OK && i < m->rows; i++)
		for (j = 0; j < m->cols; j++)
			memcpy(EL(m->f, out->data, j * m->rows + i),
				CEL(m->f, m->data, i * m->cols + j), m->f->size);
	return (e);
}

t_err	mat_conjugate_transpose(t_mat *out, const t_mat *m)
{
	t_err	e;
	size_t	i;

	e = mat_transpose(out, m);
	for (i = 0; e == E_OK && i < out->rows * out->cols; i++)
		out->f->conj(EL(out->f, out->data, i), EL(out->f, out->data, i));
	return (e);
}

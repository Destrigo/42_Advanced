/*
** Exercise 13 - Rank
**
** The rank of a matrix is the dimension of the image of the linear map it
** represents: how many independent directions the output actually spans (the
** number of linearly independent columns, which is also the number of
** linearly independent rows). A 3x3 matrix of rank 3 maps space onto space; of
** rank 2 it flattens it onto a plane; of rank 1 onto a line; of rank 0 (only
** the zero matrix) onto the origin. A square matrix is invertible exactly when
** its rank is full.
**
** The rank-nullity theorem ties it to the kernel (the vectors squashed onto
** the origin): rank + nullity = number of columns.
**
** Method: Gaussian elimination (see elim.c); the rank is the number of pivots
** found. No mathematical function is allowed. Complexity: O(n^3) time for an
** n x n matrix; one working copy of the matrix is used.
*/

#include "internal.h"

t_err	mat_rank(const t_mat *m, size_t *out)
{
	t_mat	work;
	t_err	e;

	if (!out)
		return (E_INVALID);
	*out = 0;
	e = mat_clone(&work, m);
	if (e != E_OK)
		return (e);
	e = rref_inplace(work.f, work.data, work.rows, work.cols, out);
	mat_free(&work);
	return (e);
}

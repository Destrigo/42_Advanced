/*
** Exercise 07 - Linear map, Matrix multiplication
**
** A matrix A (m x n) is a linear map from K^n to K^m; applying it to a vector
** u is the matrix-vector product A*u, and composing two maps is the
** matrix-matrix product A*B (m x n times n x p gives m x p): the number of
** columns of the left matrix must equal the number of rows of the right one.
**
** (A*u)_i = sum_j A_ij * u_j: each coordinate is the dot product of a row of A
** with u, accumulated with one fused multiply-add per term (the allowed
** mathematical function). Equivalently A*u is the linear combination of the
** columns of A weighted by the coordinates of u (exercise 01).
**
** (A*B)_ij = sum_k A_ik * B_kj. The loops run in the order i, k, j so that
** both matrices and the result are read row by row (cache friendly) while the
** arithmetic is exactly the same as the textbook formula.
**
** Complexity: mul_vec O(nm) time and space; mul_mat O(nmp) time and
** O(nm + mp + np) space (the two operands and the result).
**
** Incompatible shapes are undefined: an error is returned.
*/

#include <string.h>
#include "internal.h"

t_err	mat_mul_vec(t_vec *out, const t_mat *a, const t_vec *vec)
{
	const t_field	*f;
	t_err			e;
	t_k				acc;
	size_t			i;
	size_t			j;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!a || !vec || !a->f || !vec->f)
		return (E_INVALID);
	if (a->f != vec->f || a->cols != vec->n)
		return (E_SHAPE);
	f = a->f;
	e = vec_new(out, f, a->rows);
	for (i = 0; e == E_OK && i < a->rows; i++)
	{
		f->zero(&acc);
		for (j = 0; j < a->cols; j++)
			f->fma(&acc, CEL(f, a->data, i * a->cols + j),
				CEL(f, vec->data, j), &acc);
		memcpy(EL(f, out->data, i), &acc, f->size);
	}
	return (e);
}

t_err	mat_mul_mat(t_mat *out, const t_mat *a, const t_mat *b)
{
	const t_field	*f;
	t_err			e;
	size_t			i;
	size_t			k;
	size_t			j;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!a || !b || !a->f || !b->f)
		return (E_INVALID);
	if (a->f != b->f || a->cols != b->rows)
		return (E_SHAPE);
	f = a->f;
	e = mat_new(out, f, a->rows, b->cols);
	for (i = 0; e == E_OK && i < a->rows; i++)
		for (k = 0; k < a->cols; k++)
			for (j = 0; j < b->cols; j++)
				f->fma(EL(f, out->data, i * b->cols + j),
					CEL(f, a->data, i * a->cols + k),
					CEL(f, b->data, k * b->cols + j),
					EL(f, out->data, i * b->cols + j));
	return (e);
}

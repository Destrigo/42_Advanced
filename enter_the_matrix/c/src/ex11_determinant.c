/*
** Exercise 11 - Determinant
**
** The determinant of a square matrix is the signed factor by which the linear
** map it represents scales volumes (areas in 2D): the unit square / cube is
** mapped to a parallelogram / parallelepiped of that (signed) volume, and a
** negative sign means the orientation was flipped (a reflection).
**
**  - det(A) = 0 means the map squashes space into a lower dimension (a plane
**    into a line, a volume into a plane...): the columns are linearly
**    dependent, information is lost, so the matrix has no inverse (it is
**    *singular*) and the system Ax = b has no unique solution; it is also the
**    matrix whose rank is below n.
**  - det(AB) = det(A) det(B) (scalings compose), det(A^T) = det(A), and
**    det(A^-1) = 1 / det(A).
**  - algebraically it is the (magnitude of the) wedge product of the columns.
**
** Method, as the subject suggests, by dimension:
**  - n <= 4 (the required cases): cofactor expansion along the first row,
**    det = sum_j (-1)^j * a_0j * det(minor_0j), with 2x2 determinants ad - bc
**    computed with the compensated difference of products of exercise 06.
**    Exact for small integers, and no division, so singular matrices give
**    exactly 0.
**  - n > 4 (a bonus, since cofactors grow like n!): LU decomposition with
**    partial pivoting, the product of the pivots with the sign of the row
**    swaps, in O(n^3).
**
** The sums use the fused multiply-add (the allowed mathematical function).
** Complexity: O(n^3) time, O(n^2) memory. A non-square matrix is undefined:
** E_NOT_SQUARE is returned. The determinant of the 0x0 matrix is 1 (the empty
** product). The result is written to *out, a K.
*/

#include <stdlib.h>
#include <string.h>
#include "internal.h"

/* Cofactor expansion of the n x n matrix m (row by row), for n <= 4. */
static void	cofactor_det(const t_field *f, const void *m, size_t n, void *out)
{
	t_k		minor[9];
	t_k		acc;
	t_k		d;
	t_k		a;
	size_t	j;
	size_t	i;
	size_t	c;
	size_t	k;

	if (n == 0)
		f->one(out);
	else if (n == 1)
		memcpy(out, m, f->size);
	else if (n == 2)
		diff_of_products(f, out, CEL(f, m, 0), CEL(f, m, 3), CEL(f, m, 1),
			CEL(f, m, 2));
	if (n < 3)
		return ;
	f->zero(&acc);
	for (j = 0; j < n; j++)
	{
		for (k = 0, i = 1; i < n; i++)
			for (c = 0; c < n; c++)
				if (c != j)
					memcpy(EL(f, minor, k++), CEL(f, m, i * n + c), f->size);
		cofactor_det(f, minor, n - 1, &d);
		memcpy(&a, CEL(f, m, j), f->size);
		if (j % 2 == 1)
			f->neg(&a, &a);
		f->fma(&acc, &a, &d, &acc);
	}
	memcpy(out, &acc, f->size);
}

static void	swap_rows(const t_field *f, void *a, size_t n, size_t p, size_t c)
{
	t_k		tmp;
	size_t	j;

	for (j = 0; j < n; j++)
	{
		memcpy(&tmp, CEL(f, a, p * n + j), f->size);
		memcpy(EL(f, a, p * n + j), CEL(f, a, c * n + j), f->size);
		memcpy(EL(f, a, c * n + j), &tmp, f->size);
	}
}

/* LU decomposition with partial pivoting, for any n. a is destroyed. */
static void	lu_det(const t_field *f, void *a, size_t n, void *out)
{
	t_k		det;
	t_k		fct;
	t_k		nf;
	size_t	c;
	size_t	i;
	size_t	j;
	size_t	p;

	f->one(&det);
	for (c = 0; c < n; c++)
	{
		for (p = c, i = c + 1; i < n; i++)
			if (f->modulus(CEL(f, a, i * n + c)) > f->modulus(CEL(f, a, p * n + c)))
				p = i;
		if (f->is_zero(CEL(f, a, p * n + c)))
		{
			f->zero(out);
			return ;
		}
		if (p != c)
		{
			swap_rows(f, a, n, p, c);
			f->neg(&det, &det);
		}
		f->mul(&det, &det, CEL(f, a, c * n + c));
		for (i = c + 1; i < n; i++)
		{
			f->div(&fct, CEL(f, a, i * n + c), CEL(f, a, c * n + c));
			f->neg(&nf, &fct);
			for (j = c + 1; j < n; j++)
				f->fma(EL(f, a, i * n + j), &nf, CEL(f, a, c * n + j),
					CEL(f, a, i * n + j));
		}
	}
	memcpy(out, &det, f->size);
}

t_err	mat_determinant(const t_mat *m, void *out)
{
	t_mat	copy;
	t_k		res;
	t_err	e;

	if (!m || !m->f || !out)
		return (E_INVALID);
	if (!mat_is_square(m))
		return (E_NOT_SQUARE);
	if (m->rows <= 4)
		cofactor_det(m->f, m->data, m->rows, &res);
	else
	{
		e = mat_clone(&copy, m);
		if (e != E_OK)
			return (e);
		lu_det(m->f, copy.data, copy.rows, &res);
		mat_free(&copy);
	}
	memcpy(out, &res, m->f->size);
	return (E_OK);
}

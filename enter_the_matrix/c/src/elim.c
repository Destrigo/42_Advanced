/*
** Gaussian elimination shared by the row-echelon form (exercise 10) and the
** rank (exercise 13). Plain + - * / only: those exercises allow no
** mathematical function.
**
** Decisions of the algorithm, all numerical:
**  - scaled partial pivoting: in each column the pivot is the entry that is
**    largest *relative to the size of its row*, which keeps the elimination
**    stable and, above all, makes "is this entry zero?" independent of the
**    scale of the row (diag(1e8, 1) has rank 2);
**  - an entry counts as zero when it is below n * epsilon times the magnitude
**    of the terms that produced it (the row's original scale, grown by what
**    was subtracted from it): this is what removes the rounding noise left by
**    [[1,2,3],[4,5,6],[7,8,9]] while exact rank deficiency is detected
**    exactly.
*/

#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

static void	swap_rows(const t_field *f, void *data, size_t cols, size_t a,
				size_t b)
{
	t_k		tmp;
	size_t	j;

	for (j = 0; j < cols; j++)
	{
		memcpy(&tmp, CEL(f, data, a * cols + j), f->size);
		memcpy(EL(f, data, a * cols + j), CEL(f, data, b * cols + j), f->size);
		memcpy(EL(f, data, b * cols + j), &tmp, f->size);
	}
}

/* Reduces the rows below r in column c, choosing the pivot. Returns false if
** the column holds no pivot (it is zeroed from row r down). */
static bool	choose_pivot(const t_field *f, void *data, size_t rows,
				size_t cols, size_t r, size_t c, const float *scale, float tol,
				size_t *p)
{
	float	best;
	float	ratio;
	size_t	i;
	t_k		zero;

	best = -1.0f;
	*p = r;
	for (i = r; i < rows; i++)
	{
		ratio = 0.0f;
		if (scale[i] > 0.0f)
			ratio = f->modulus(CEL(f, data, i * cols + c)) / scale[i];
		if (ratio > best)
		{
			best = ratio;
			*p = i;
		}
	}
	if (best > tol)
		return (true);
	f->zero(&zero);
	for (i = r; i < rows; i++)
		memcpy(EL(f, data, i * cols + c), &zero, f->size);
	return (false);
}

/* Normalises row r on its pivot and clears column c in all the other rows. */
static void	eliminate(const t_field *f, void *data, size_t rows, size_t cols,
				size_t r, size_t c, float *scale)
{
	t_k		piv;
	t_k		fct;
	t_k		prod;
	size_t	i;
	size_t	j;

	memcpy(&piv, CEL(f, data, r * cols + c), f->size);
	for (j = c + 1; j < cols; j++)
		f->div(EL(f, data, r * cols + j), CEL(f, data, r * cols + j), &piv);
	f->one(EL(f, data, r * cols + c));
	scale[r] /= f->modulus(&piv);
	for (i = 0; i < rows; i++)
	{
		memcpy(&fct, CEL(f, data, i * cols + c), f->size);
		if (i == r || f->is_zero(&fct))
			continue ;
		for (j = c + 1; j < cols; j++)
		{
			f->mul(&prod, &fct, CEL(f, data, r * cols + j));
			f->sub(EL(f, data, i * cols + j), CEL(f, data, i * cols + j), &prod);
		}
		f->zero(EL(f, data, i * cols + c));
		scale[i] += f->modulus(&fct) * scale[r];
	}
}

t_err	rref_inplace(const t_field *f, void *data, size_t rows, size_t cols,
			size_t *rank)
{
	float	*scale;
	float	tol;
	float	tol_swap;
	size_t	r;
	size_t	c;
	size_t	i;
	size_t	j;
	size_t	p;

	*rank = 0;
	if (rows == 0 || cols == 0)
		return (E_OK);
	scale = calloc(rows, sizeof(*scale));
	if (!scale)
		return (E_NOMEM);
	tol = (float)(rows > cols ? rows : cols) * FLT_EPSILON;
	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			if (f->modulus(CEL(f, data, i * cols + j)) > scale[i])
				scale[i] = f->modulus(CEL(f, data, i * cols + j));
	for (r = 0, c = 0; c < cols && r < rows; c++)
	{
		if (!choose_pivot(f, data, rows, cols, r, c, scale, tol, &p))
			continue ;
		if (p != r)
		{
			swap_rows(f, data, cols, p, r);
			tol_swap = scale[p];
			scale[p] = scale[r];
			scale[r] = tol_swap;
		}
		eliminate(f, data, rows, cols, r, c, scale);
		r++;
	}
	free(scale);
	*rank = r;
	return (E_OK);
}

/*
** Exercise 12 - Inverse
**
** The inverse of a square matrix A is the matrix A^-1 that undoes it:
** A^-1 A = A A^-1 = I. It exists exactly when det(A) != 0 (the map loses no
** information); a matrix without inverse is *singular* and the function
** returns E_SINGULAR. With it, Ab = c is solved by b = A^-1 c.
**
** Method: Gauss-Jordan elimination on the augmented matrix [A | I]. The row
** operations that turn A into I turn I into A^-1 (they are exactly the
** multiplication by A^-1). Pivots are chosen by scaled partial pivoting (see
** elim.c), which keeps the method stable, and a matrix is declared singular
** when no usable pivot is left: its best entry in a column is below
** n * epsilon times the size of its row, i.e. it is rounding noise. Because
** the test is relative, 1e-20 * I is (correctly) invertible and
** [[1,2],[2,4]] or [[1,2,3],[4,5,6],[7,8,9]] are (correctly) not.
**
** The row updates use the fused multiply-add (the allowed mathematical
** function). Complexity: O(n^3) time, O(n^2) memory.
**
** A non-square matrix is undefined: E_NOT_SQUARE is returned for it too (the
** subject only requires an error for singular matrices; so does a matrix
** containing NaN or infinity, E_INVALID). As the subject warns, the result of
** a nearly singular or very large matrix is only approximate.
*/

#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

typedef struct s_aug
{
	const t_field	*f;
	char			*a;		/* n rows of w = 2n scalars: [A | I] */
	float			*scale;
	size_t			n;
	size_t			w;
}	t_aug;

static t_err	init_aug(t_aug *g, const t_mat *m)
{
	size_t	i;
	size_t	j;
	float	mod;

	g->f = m->f;
	g->n = m->rows;
	g->w = 2 * m->rows;
	g->a = alloc_scalars(m->f, g->n * g->w);
	g->scale = calloc(g->n ? g->n : 1, sizeof(float));
	if (!g->a || !g->scale)
		return (free(g->a), free(g->scale), E_NOMEM);
	for (i = 0; i < g->n; i++)
	{
		for (j = 0; j < g->n; j++)
		{
			memcpy(EL(g->f, g->a, i * g->w + j), CEL(g->f, m->data, i * g->n + j),
				g->f->size);
			mod = g->f->modulus(CEL(g->f, m->data, i * g->n + j));
			if (mod > g->scale[i])
				g->scale[i] = mod;
		}
		g->f->one(EL(g->f, g->a, i * g->w + g->n + i));
	}
	return (E_OK);
}

static void	swap_rows(t_aug *g, size_t p, size_t c)
{
	t_k		tmp;
	float	ts;
	size_t	j;

	for (j = 0; j < g->w; j++)
	{
		memcpy(&tmp, CEL(g->f, g->a, p * g->w + j), g->f->size);
		memcpy(EL(g->f, g->a, p * g->w + j), CEL(g->f, g->a, c * g->w + j),
			g->f->size);
		memcpy(EL(g->f, g->a, c * g->w + j), &tmp, g->f->size);
	}
	ts = g->scale[p];
	g->scale[p] = g->scale[c];
	g->scale[c] = ts;
}

/* Normalises row c on its pivot and clears column c in the other rows. */
static void	clear_column(t_aug *g, size_t c)
{
	const t_field	*f = g->f;
	t_k				piv;
	t_k				fct;
	t_k				nf;
	size_t			i;
	size_t			j;

	memcpy(&piv, CEL(f, g->a, c * g->w + c), f->size);
	for (j = 0; j < g->w; j++)
		f->div(EL(f, g->a, c * g->w + j), CEL(f, g->a, c * g->w + j), &piv);
	f->one(EL(f, g->a, c * g->w + c));
	g->scale[c] /= f->modulus(&piv);
	for (i = 0; i < g->n; i++)
	{
		memcpy(&fct, CEL(f, g->a, i * g->w + c), f->size);
		if (i == c || f->is_zero(&fct))
			continue ;
		f->neg(&nf, &fct);
		for (j = 0; j < g->w; j++)
			f->fma(EL(f, g->a, i * g->w + j), &nf, CEL(f, g->a, c * g->w + j),
				CEL(f, g->a, i * g->w + j));
		f->zero(EL(f, g->a, i * g->w + c));
		g->scale[i] += f->modulus(&fct) * g->scale[c];
	}
}

/* Index of the row (from c down) whose entry in column c is the largest
** relative to its row, or n if it is only noise (the matrix is singular). */
static size_t	find_pivot(const t_aug *g, size_t c)
{
	float	best;
	float	ratio;
	size_t	p;
	size_t	i;

	best = -1.0f;
	p = c;
	for (i = c; i < g->n; i++)
	{
		ratio = 0.0f;
		if (g->scale[i] > 0.0f)
			ratio = g->f->modulus(CEL(g->f, g->a, i * g->w + c)) / g->scale[i];
		if (ratio > best)
		{
			best = ratio;
			p = i;
		}
	}
	if (!(best > (float)g->n * FLT_EPSILON))
		return (g->n);
	return (p);
}

t_err	mat_inverse(t_mat *out, const t_mat *m)
{
	t_aug	g;
	t_err	e;
	size_t	c;
	size_t	p;
	size_t	i;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!m || !m->f || (m->rows * m->cols != 0 && !m->data))
		return (E_INVALID);
	if (!mat_is_square(m))
		return (E_NOT_SQUARE);
	for (i = 0; i < m->rows * m->cols; i++)
		if (!m->f->is_finite(CEL(m->f, m->data, i)))
			return (E_INVALID);
	e = init_aug(&g, m);
	if (e != E_OK)
		return (e);
	for (c = 0; c < g.n && e == E_OK; c++)
	{
		p = find_pivot(&g, c);
		if (p == g.n)
			e = E_SINGULAR;
		else
		{
			if (p != c)
				swap_rows(&g, p, c);
			clear_column(&g, c);
		}
	}
	if (e == E_OK)
		e = mat_new(out, m->f, g.n, g.n);
	for (i = 0; e == E_OK && i < g.n; i++)
		memcpy(EL(m->f, out->data, i * g.n), CEL(m->f, g.a, i * g.w + g.n),
			g.n * m->f->size);
	free(g.a);
	free(g.scale);
	return (e);
}

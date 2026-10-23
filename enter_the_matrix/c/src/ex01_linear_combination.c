/*
** Exercise 01 - Linear combination
**
** Given vectors u1..uk and scalars l1..lk, compute l1*u1 + ... + lk*uk. It is
** the most general thing one can do in a vector space (every other operation
** of exercise 00 is a special case) and, with the matrices of the next
** exercises, the way to read a matrix-vector product: A*x is the combination
** of the columns of A weighted by the coordinates of x.
**
** The same code serves vectors and matrices: both are flat buffers of
** scalars. It starts from the zero element and accumulates acc += l_i * u_i,
** one fused multiply-add per coordinate (the allowed mathematical function,
** fmaf for the reals): a single rounding per step instead of two.
**
** Complexity: O(n) per vector (n = number of coordinates), O(n) extra memory
** (the accumulator).
**
** u is an array of k vectors (or matrices), coefs an array of k scalars of
** the same field. An empty list (the dimension of the zero vector would be
** unknown) or items of different shapes are undefined in the subject: an
** error is returned.
*/

#include <string.h>
#include "internal.h"

/* d += a * s, one fused multiply-add per scalar */
void	buf_axpy(const t_field *f, void *d, const void *a, const void *s,
			size_t n)
{
	size_t	i;

	for (i = 0; i < n; i++)
		f->fma(EL(f, d, i), a, CEL(f, s, i), EL(f, d, i));
}

t_err	vec_linear_combination(t_vec *out, const t_vec *u, size_t k,
			const void *coefs)
{
	size_t	i;
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (k == 0)
		return (E_EMPTY);
	if (!u || !coefs)
		return (E_INVALID);
	for (i = 0; i < k; i++)
		if (!u[i].f || u[i].f != u[0].f || u[i].n != u[0].n)
			return (u[i].f ? E_SHAPE : E_INVALID);
	e = vec_new(out, u[0].f, u[0].n);
	if (e != E_OK)
		return (e);
	for (i = 0; i < k; i++)
		buf_axpy(out->f, out->data, CEL(out->f, coefs, i), u[i].data, out->n);
	return (E_OK);
}

t_err	mat_linear_combination(t_mat *out, const t_mat *u, size_t k,
			const void *coefs)
{
	size_t	i;
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (k == 0)
		return (E_EMPTY);
	if (!u || !coefs)
		return (E_INVALID);
	for (i = 0; i < k; i++)
	{
		if (!u[i].f)
			return (E_INVALID);
		if (u[i].f != u[0].f || u[i].rows != u[0].rows
			|| u[i].cols != u[0].cols)
			return (E_SHAPE);
	}
	e = mat_new(out, u[0].f, u[0].rows, u[0].cols);
	if (e != E_OK)
		return (e);
	for (i = 0; i < k; i++)
		buf_axpy(out->f, out->data, CEL(out->f, coefs, i), u[i].data,
			out->rows * out->cols);
	return (E_OK);
}

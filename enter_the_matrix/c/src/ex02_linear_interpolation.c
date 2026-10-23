/*
** Exercise 02 - Linear interpolation
**
** lerp(u, v, t) slides from u (at t = 0) to v (at t = 1) along the straight
** line between them; t = 0.5 is the exact middle (the isobarycentre). It is
** the weighted average (1 - t) * u + t * v, a linear combination with weights
** summing to 1. Any real t is accepted: outside [0, 1] it extrapolates along
** the same infinite line.
**
** The formula is evaluated as one fused multiply-add, fma(1 - t, u, t * v) per
** coordinate. Written this way (and not as u + t * (v - u)) the endpoints are
** exact: t = 0 gives exactly u and t = 1 exactly v, which u + t * (v - u)
** does not guarantee in floating point.
**
** It works on scalars (scalar_lerp, lerp_f32), vectors and matrices.
**
** Complexity: O(n) time and memory (n = number of coordinates). Vectors or
** matrices of different shapes are undefined: an error is returned.
*/

#include <math.h>
#include <string.h>
#include "internal.h"

float	lerp_f32(float u, float v, float t)
{
	return (fmaf(1.0f - t, u, t * v));
}

/* out = (1 - t) * u + t * v for scalars of the field f (out may alias u, v) */
void	scalar_lerp(const t_field *f, void *out, const void *u, const void *v,
			float t)
{
	t_k	wt;
	t_k	w1;
	t_k	tv;

	f->from_f32(&wt, t);
	f->from_f32(&w1, 1.0f - t);
	f->mul(&tv, &wt, v);
	f->fma(out, &w1, u, &tv);
}

static void	lerp_buf(const t_field *f, void *out, const void *u, const void *v,
				size_t n, float t)
{
	size_t	i;

	for (i = 0; i < n; i++)
		scalar_lerp(f, EL(f, out, i), CEL(f, u, i), CEL(f, v, i), t);
}

t_err	vec_lerp(t_vec *out, const t_vec *u, const t_vec *v, float t)
{
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!u || !v || !u->f || !v->f)
		return (E_INVALID);
	if (u->f != v->f || u->n != v->n)
		return (E_SHAPE);
	e = vec_new(out, u->f, u->n);
	if (e == E_OK)
		lerp_buf(out->f, out->data, u->data, v->data, out->n, t);
	return (e);
}

t_err	mat_lerp(t_mat *out, const t_mat *u, const t_mat *v, float t)
{
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!u || !v || !u->f || !v->f)
		return (E_INVALID);
	if (u->f != v->f || u->rows != v->rows || u->cols != v->cols)
		return (E_SHAPE);
	e = mat_new(out, u->f, u->rows, u->cols);
	if (e == E_OK)
		lerp_buf(out->f, out->data, u->data, v->data, out->rows * out->cols, t);
	return (e);
}

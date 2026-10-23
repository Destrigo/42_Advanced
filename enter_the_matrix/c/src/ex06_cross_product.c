/*
** Exercise 06 - Cross product
**
** The cross product u x v of two 3-dimensional vectors is the vector
** perpendicular to both, whose length is the area of the parallelogram they
** span (|u| |v| sin(angle)) and whose direction follows the right-hand rule.
** It is zero when the vectors are parallel, and anticommutative:
** u x v = -(v x u).
**
**   (u1,u2,u3) x (v1,v2,v3) = (u2 v3 - u3 v2,  u3 v1 - u1 v3,  u1 v2 - u2 v1)
**
** Each coordinate is a difference of two products, the classic place where
** rounding errors blow up through cancellation. With a fused multiply-add (the
** allowed mathematical function) the error of one product is recovered
** exactly and added back (Kahan's trick): w = c*d, e = fma(-c, d, w) is
** exactly the rounding error of w, f = fma(a, b, -w), and a*b - c*d is f + e.
**
** Complexity: O(1). Anything that is not a pair of 3D vectors is undefined:
** an error is returned.
*/

#include <string.h>
#include "internal.h"

/* r = a*b - c*d (r may alias any input) */
void	diff_of_products(const t_field *f, void *r, const void *a, const void *b,
			const void *c, const void *d)
{
	t_k	w;
	t_k	e;
	t_k	nc;
	t_k	nw;
	t_k	g;

	f->mul(&w, c, d);
	f->neg(&nc, c);
	f->fma(&e, &nc, d, &w);
	f->neg(&nw, &w);
	f->fma(&g, a, b, &nw);
	f->add(r, &g, &e);
}

t_err	cross_product(t_vec *out, const t_vec *u, const t_vec *v)
{
	const t_field	*f;
	t_k				r[3];
	t_err			e;
	size_t			i;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!u || !v || !u->f || !v->f)
		return (E_INVALID);
	if (u->f != v->f || u->n != 3 || v->n != 3)
		return (E_SHAPE);
	f = u->f;
	diff_of_products(f, &r[0], CEL(f, u->data, 1), CEL(f, v->data, 2),
		CEL(f, u->data, 2), CEL(f, v->data, 1));
	diff_of_products(f, &r[1], CEL(f, u->data, 2), CEL(f, v->data, 0),
		CEL(f, u->data, 0), CEL(f, v->data, 2));
	diff_of_products(f, &r[2], CEL(f, u->data, 0), CEL(f, v->data, 1),
		CEL(f, u->data, 1), CEL(f, v->data, 0));
	e = vec_new(out, f, 3);
	for (i = 0; e == E_OK && i < 3; i++)
		memcpy(EL(f, out->data, i), &r[i], f->size);
	return (e);
}

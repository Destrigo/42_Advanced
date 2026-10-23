/*
** Exercise 03 - Dot product
**
** The dot (inner) product of two vectors of the same dimension:
** <u|v> = u1*v1 + ... + un*vn. It turns two vectors into a scalar and carries
** the geometry of the space: it gives lengths (<v|v> = |v|^2) and angles
** (<u|v> = |u| |v| cos(angle), exercise 05), and two vectors are
** perpendicular exactly when it is zero. Matrix products are just many dot
** products (one per row of the left matrix and column of the right one).
**
** The sum is accumulated with one fused multiply-add per coordinate
** (acc = fma(u_i, v_i, acc)), the allowed mathematical function.
**
** Over the complex numbers the first argument is conjugated (sesquilinear
** inner product); over the reals this changes nothing.
**
** Complexity: O(n) time, O(1) extra memory. Different dimensions are
** undefined: an error is returned. The result is written to *out, a K.
*/

#include <string.h>
#include "internal.h"

t_err	vec_dot(const t_vec *u, const t_vec *v, void *out)
{
	const t_field	*f;
	t_k				acc;
	t_k				c;
	size_t			i;

	if (!u || !v || !out || !u->f || !v->f)
		return (E_INVALID);
	if (u->f != v->f || u->n != v->n)
		return (E_SHAPE);
	f = u->f;
	f->zero(&acc);
	for (i = 0; i < u->n; i++)
	{
		f->conj(&c, CEL(f, u->data, i));
		f->fma(&acc, &c, CEL(f, v->data, i), &acc);
	}
	memcpy(out, &acc, f->size);
	return (E_OK);
}

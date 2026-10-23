/*
** Exercise 05 - Cosine
**
** The cosine of the angle between two vectors, from the dot product alone
** (the standard cos is forbidden, and not needed):
**
**     cos(u, v) = <u|v> / (|u| * |v|)
**
** It is 1 for vectors pointing the same way, 0 for perpendicular ones, -1 for
** opposite ones, and does not depend on the lengths. In statistics the same
** formula with covariance as the dot product is the Pearson correlation
** coefficient.
**
** It uses vec_dot (exercise 03) and vec_norm (exercise 04), as the subject
** suggests. Rounding can push a mathematically exact 1 slightly outside
** [-1, 1] (e.g. 1.0000001), so the result is clamped to that range.
**
** Over complex vectors the result is the real part of the normalised inner
** product.
**
** Complexity: O(n) time, O(1) extra memory. A zero vector (no direction) or
** different dimensions are undefined: an error is returned.
*/

#include "internal.h"

t_err	angle_cos(const t_vec *u, const t_vec *v, float *out)
{
	t_k		dot;
	t_err	e;
	float	nu;
	float	nv;
	float	c;

	if (!out)
		return (E_INVALID);
	e = vec_dot(u, v, &dot);
	if (e != E_OK)
		return (e);
	nu = vec_norm(u);
	nv = vec_norm(v);
	if (nu == 0.0f || nv == 0.0f)
		return (E_INVALID);
	c = u->f->re(&dot) / nu / nv;
	if (c > 1.0f)
		c = 1.0f;
	else if (c < -1.0f)
		c = -1.0f;
	*out = c;
	return (E_OK);
}

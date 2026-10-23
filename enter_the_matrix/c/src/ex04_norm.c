/*
** Exercise 04 - Norm
**
** A norm measures the length of a vector. Three classic ones:
**  - 1-norm (taxicab / Manhattan): |v1| + ... + |vn|, the distance walked
**    along the axes;
**  - 2-norm (Euclidean): sqrt(v1^2 + ... + vn^2) = sqrt(<v|v>), the length of
**    the arrow;
**  - infinity-norm (supremum): max |vi|, the largest coordinate.
**
** They always return a real number, also for complex vectors (the modulus of
** each coordinate is used).
**
** Allowed mathematical functions: fused multiply-add, pow, max. The square
** root is powf(x, 0.5) and the absolute value / maximum are plain
** comparisons. The 2-norm divides the coordinates by the largest one first,
** so that squaring them neither overflows ([1e30, 1e30] has norm 1.4142e30,
** not infinity) nor underflows, then multiplies back.
**
** Complexity: O(n) time, O(1) extra memory. A NaN coordinate gives NaN (and
** so does a NULL vector).
*/

#include <math.h>
#include "internal.h"

float	vec_norm_1(const t_vec *v)
{
	float	sum;
	size_t	i;

	if (!v || !v->f)
		return (NAN);
	sum = 0.0f;
	for (i = 0; i < v->n; i++)
		sum += v->f->modulus(CEL(v->f, v->data, i));
	return (sum);
}

float	vec_norm_inf(const t_vec *v)
{
	float	max;
	float	m;
	size_t	i;

	if (!v || !v->f)
		return (NAN);
	max = 0.0f;
	for (i = 0; i < v->n; i++)
	{
		m = v->f->modulus(CEL(v->f, v->data, i));
		if (m != m)
			return (NAN);
		if (m > max)
			max = m;
	}
	return (max);
}

float	vec_norm(const t_vec *v)
{
	float	max;
	float	r;
	float	acc;
	size_t	i;

	max = vec_norm_inf(v);
	if (max == 0.0f || !fin_f32(max))
		return (max);
	acc = 0.0f;
	for (i = 0; i < v->n; i++)
	{
		r = v->f->modulus(CEL(v->f, v->data, i)) / max;
		acc = fmaf(r, r, acc);
	}
	return (max * sqrt_f32(acc));
}

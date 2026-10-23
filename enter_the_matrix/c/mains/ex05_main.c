#include <stdio.h>
#include "matrix.h"

static void	cos_of(size_t n, const float *a, const float *b)
{
	t_vec	u;
	t_vec	v;
	float	c;
	char	buf[64];

	vec_from_f32(&u, n, a);
	vec_from_f32(&v, n, b);
	if (angle_cos(&u, &v, &c) == E_OK)
	{
		fmt_f32(buf, sizeof(buf), c);
		puts(buf);
	}
	vec_free(&u);
	vec_free(&v);
}

int	main(void)
{
	cos_of(2, (float[]){1., 0.}, (float[]){1., 0.});
	cos_of(2, (float[]){1., 0.}, (float[]){0., 1.});
	cos_of(2, (float[]){-1., 1.}, (float[]){1., -1.});
	cos_of(2, (float[]){2., 1.}, (float[]){4., 2.});
	cos_of(3, (float[]){1., 2., 3.}, (float[]){4., 5., 6.});
	return (0);
}

#include <stdio.h>
#include "matrix.h"

static void	dot(const float *a, const float *b)
{
	t_vec	u;
	t_vec	v;
	float	r;
	char	buf[64];

	vec_from_f32(&u, 2, a);
	vec_from_f32(&v, 2, b);
	vec_dot(&u, &v, &r);
	fmt_f32(buf, sizeof(buf), r);
	puts(buf);
	vec_free(&u);
	vec_free(&v);
}

int	main(void)
{
	dot((float[]){0., 0.}, (float[]){1., 1.});
	dot((float[]){1., 1.}, (float[]){1., 1.});
	dot((float[]){-1., 6.}, (float[]){3., 2.});
	return (0);
}

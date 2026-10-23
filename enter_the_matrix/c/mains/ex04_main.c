#include <stdio.h>
#include "matrix.h"

static void	show(size_t n, const float *vals)
{
	t_vec	u;
	char	a[64];
	char	b[64];
	char	c[64];

	vec_from_f32(&u, n, vals);
	fmt_f32(a, sizeof(a), vec_norm_1(&u));
	fmt_f32(b, sizeof(b), vec_norm(&u));
	fmt_f32(c, sizeof(c), vec_norm_inf(&u));
	printf("%s, %s, %s\n", a, b, c);
	vec_free(&u);
}

int	main(void)
{
	show(3, (float[]){0., 0., 0.});
	show(3, (float[]){1., 2., 3.});
	show(2, (float[]){-1., -2.});
	return (0);
}

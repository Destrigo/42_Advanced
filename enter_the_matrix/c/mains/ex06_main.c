#include <stdio.h>
#include "matrix.h"

static void	cross(const float *a, const float *b)
{
	t_vec	u;
	t_vec	v;
	t_vec	r;

	vec_from_f32(&u, 3, a);
	vec_from_f32(&v, 3, b);
	if (cross_product(&r, &u, &v) == E_OK)
	{
		vec_print(&r);
		printf("\n");
	}
	vec_free(&u);
	vec_free(&v);
	vec_free(&r);
}

int	main(void)
{
	cross((float[]){0., 0., 1.}, (float[]){1., 0., 0.});
	cross((float[]){1., 2., 3.}, (float[]){4., 5., 6.});
	cross((float[]){4., 2., -3.}, (float[]){-2., -5., 16.});
	return (0);
}

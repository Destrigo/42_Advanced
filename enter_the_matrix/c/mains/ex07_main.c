#include <stdio.h>
#include "matrix.h"

static void	mv(const float *m, const float *v)
{
	t_mat	a;
	t_vec	x;
	t_vec	r;

	mat_from_f32(&a, 2, 2, m);
	vec_from_f32(&x, 2, v);
	if (mat_mul_vec(&r, &a, &x) == E_OK)
	{
		vec_print(&r);
		printf("\n");
	}
	mat_free(&a);
	vec_free(&x);
	vec_free(&r);
}

static void	mm(const float *p, const float *q)
{
	t_mat	a;
	t_mat	b;
	t_mat	r;

	mat_from_f32(&a, 2, 2, p);
	mat_from_f32(&b, 2, 2, q);
	if (mat_mul_mat(&r, &a, &b) == E_OK)
	{
		mat_print(&r);
		printf("\n");
	}
	mat_free(&a);
	mat_free(&b);
	mat_free(&r);
}

int	main(void)
{
	mv((float[]){1., 0., 0., 1.}, (float[]){4., 2.});
	mv((float[]){2., 0., 0., 2.}, (float[]){4., 2.});
	mv((float[]){2., -2., -2., 2.}, (float[]){4., 2.});
	mm((float[]){1., 0., 0., 1.}, (float[]){1., 0., 0., 1.});
	mm((float[]){1., 0., 0., 1.}, (float[]){2., 1., 4., 2.});
	mm((float[]){3., -5., 6., 8.}, (float[]){2., 1., 4., 2.});
	return (0);
}

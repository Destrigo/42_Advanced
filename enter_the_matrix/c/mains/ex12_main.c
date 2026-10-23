#include <stdio.h>
#include "matrix.h"

static void	inv(size_t n, const float *vals)
{
	t_mat	m;
	t_mat	r;
	t_err	e;

	mat_from_f32(&m, n, n, vals);
	e = mat_inverse(&r, &m);
	if (e == E_OK)
		mat_print(&r);
	else
		printf("error: %s\n", err_str(e));
	printf("\n");
	mat_free(&m);
	mat_free(&r);
}

int	main(void)
{
	inv(3, (float[]){1., 0., 0., 0., 1., 0., 0., 0., 1.});
	inv(3, (float[]){2., 0., 0., 0., 2., 0., 0., 0., 2.});
	inv(3, (float[]){8., 5., -2., 4., 7., 20., 7., 6., 1.});
	inv(2, (float[]){1., 2., 2., 4.});
	return (0);
}

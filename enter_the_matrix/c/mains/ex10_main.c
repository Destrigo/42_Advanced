#include <stdio.h>
#include "matrix.h"

static void	show(size_t rows, size_t cols, const float *vals)
{
	t_mat	a;
	t_mat	r;

	mat_from_f32(&a, rows, cols, vals);
	if (mat_row_echelon(&r, &a) == E_OK)
	{
		mat_print(&r);
		printf("\n");
	}
	mat_free(&a);
	mat_free(&r);
}

int	main(void)
{
	show(3, 3, (float[]){1., 0., 0., 0., 1., 0., 0., 0., 1.});
	show(2, 2, (float[]){1., 2., 3., 4.});
	show(2, 2, (float[]){1., 2., 2., 4.});
	show(3, 5, (float[]){8., 5., -2., 4., 28., 4., 2.5, 20., 4., -4., 8., 5., 1.,
		4., 17.});
	return (0);
}

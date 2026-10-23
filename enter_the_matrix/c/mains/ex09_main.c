#include <stdio.h>
#include "matrix.h"

static void	show(size_t rows, size_t cols, const float *vals)
{
	t_mat	a;
	t_mat	t;

	mat_from_f32(&a, rows, cols, vals);
	if (mat_transpose(&t, &a) == E_OK)
	{
		mat_print(&a);
		printf("->\n");
		mat_print(&t);
		printf("\n");
	}
	mat_free(&a);
	mat_free(&t);
}

int	main(void)
{
	show(2, 2, (float[]){1., 2., 3., 4.});
	show(2, 3, (float[]){1., 2., 3., 4., 5., 6.});
	return (0);
}

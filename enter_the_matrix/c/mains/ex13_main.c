#include <stdio.h>
#include "matrix.h"

static void	rank_of(size_t rows, size_t cols, const float *vals)
{
	t_mat	m;
	size_t	r;

	mat_from_f32(&m, rows, cols, vals);
	if (mat_rank(&m, &r) == E_OK)
		printf("%zu\n", r);
	mat_free(&m);
}

int	main(void)
{
	rank_of(3, 3, (float[]){1., 0., 0., 0., 1., 0., 0., 0., 1.});
	rank_of(3, 4, (float[]){1., 2., 0., 0., 2., 4., 0., 0., -1., 2., 1., 1.});
	rank_of(4, 3, (float[]){8., 5., -2., 4., 7., 20., 7., 6., 1., 21., 18., 7.});
	return (0);
}

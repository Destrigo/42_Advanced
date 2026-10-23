#include <stdio.h>
#include "matrix.h"

static void	det_of(size_t n, const float *vals)
{
	t_mat	m;
	float	d;
	char	buf[64];

	mat_from_f32(&m, n, n, vals);
	if (mat_determinant(&m, &d) == E_OK)
	{
		fmt_f32(buf, sizeof(buf), d);
		puts(buf);
	}
	mat_free(&m);
}

int	main(void)
{
	det_of(2, (float[]){1., -1., -1., 1.});
	det_of(3, (float[]){2., 0., 0., 0., 2., 0., 0., 0., 2.});
	det_of(3, (float[]){8., 5., -2., 4., 7., 20., 7., 6., 1.});
	det_of(4, (float[]){8., 5., -2., 4., 4., 2.5, 20., 4., 8., 5., 1., 4., 28.,
		-4., 17., 1.});
	return (0);
}

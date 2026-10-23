#include <stdio.h>
#include "matrix.h"

static void	trace_of(size_t n, const float *vals)
{
	t_mat	m;
	float	t;
	char	buf[64];

	mat_from_f32(&m, n, n, vals);
	if (mat_trace(&m, &t) == E_OK)
	{
		fmt_f32(buf, sizeof(buf), t);
		puts(buf);
	}
	mat_free(&m);
}

int	main(void)
{
	trace_of(2, (float[]){1., 0., 0., 1.});
	trace_of(3, (float[]){2., -5., 0., 4., 3., 7., -2., 3., 4.});
	trace_of(3, (float[]){-2., -8., 4., 1., -23., 4., 0., 6., 4.});
	return (0);
}

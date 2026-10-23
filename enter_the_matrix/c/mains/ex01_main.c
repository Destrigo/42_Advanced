#include <stdio.h>
#include "matrix.h"

int	main(void)
{
	t_vec	u[3];
	t_vec	out;
	float	c1[3] = {10.f, -2.f, 0.5f};
	float	c2[2] = {10.f, -2.f};

	VEC_F(&u[0], 1., 0., 0.);
	VEC_F(&u[1], 0., 1., 0.);
	VEC_F(&u[2], 0., 0., 1.);
	vec_linear_combination(&out, u, 3, c1);
	vec_print(&out);
	printf("\n");
	vec_free(&out);
	vec_free(&u[0]);
	vec_free(&u[1]);
	vec_free(&u[2]);
	VEC_F(&u[0], 1., 2., 3.);
	VEC_F(&u[1], 0., 10., -100.);
	vec_linear_combination(&out, u, 2, c2);
	vec_print(&out);
	vec_free(&out);
	vec_free(&u[0]);
	vec_free(&u[1]);
	return (0);
}

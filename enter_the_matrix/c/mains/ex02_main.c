#include <stdio.h>
#include "matrix.h"

static void	show(float x)
{
	char	buf[64];

	fmt_f32(buf, sizeof(buf), x);
	puts(buf);
}

int	main(void)
{
	t_vec	u;
	t_vec	v;
	t_vec	r;
	t_mat	a;
	t_mat	b;
	t_mat	m;

	show(lerp_f32(0., 1., 0.));
	show(lerp_f32(0., 1., 1.));
	show(lerp_f32(0., 1., 0.5));
	show(lerp_f32(21., 42., 0.3f));
	VEC_F(&u, 2., 1.);
	VEC_F(&v, 4., 2.);
	vec_lerp(&r, &u, &v, 0.3f);
	vec_print(&r);
	printf("\n");
	MAT_F(&a, 2, 2, 2., 1., 3., 4.);
	MAT_F(&b, 2, 2, 20., 10., 30., 40.);
	mat_lerp(&m, &a, &b, 0.5f);
	mat_print(&m);
	vec_free(&u);
	vec_free(&v);
	vec_free(&r);
	mat_free(&a);
	mat_free(&b);
	mat_free(&m);
	return (0);
}

#include <stdio.h>
#include "matrix.h"

static void	show_v(const t_vec *v)
{
	vec_print(v);
	printf("\n");
}

static void	show_m(const t_mat *m)
{
	mat_print(m);
	printf("\n");
}

int	main(void)
{
	t_vec	u;
	t_vec	v;
	t_mat	a;
	t_mat	b;
	float	two = 2.0f;

	VEC_F(&v, 5., 7.);
	VEC_F(&u, 2., 3.);
	vec_add(&u, &v);
	show_v(&u);
	vec_free(&u);
	VEC_F(&u, 2., 3.);
	vec_sub(&u, &v);
	show_v(&u);
	vec_free(&u);
	VEC_F(&u, 2., 3.);
	vec_scl(&u, &two);
	show_v(&u);
	vec_free(&u);
	vec_free(&v);

	MAT_F(&b, 2, 2, 7., 4., -2., 2.);
	MAT_F(&a, 2, 2, 1., 2., 3., 4.);
	mat_add(&a, &b);
	show_m(&a);
	mat_free(&a);
	MAT_F(&a, 2, 2, 1., 2., 3., 4.);
	mat_sub(&a, &b);
	show_m(&a);
	mat_free(&a);
	MAT_F(&a, 2, 2, 1., 2., 3., 4.);
	mat_scl(&a, &two);
	mat_print(&a);
	mat_free(&a);
	mat_free(&b);
	return (0);
}

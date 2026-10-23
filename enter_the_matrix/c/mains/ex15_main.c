/* Bonus: every exercise again, with complex numbers as the scalars. */

#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

static void	say(const char *label, const t_field *f, const void *x)
{
	char	buf[96];

	f->format(buf, sizeof(buf), x);
	printf("%s %s\n", label, buf);
}

static void	say_vec(const char *label, const t_vec *v)
{
	char	*s;
	size_t	i;

	s = vec_to_string(v);
	for (i = 0; s && s[i]; i++)
		if (s[i] == '\n')
			s[i] = ' ';
	printf("%s %s\n", label, s ? s : "(error)");
	free(s);
}

static void	say_mat(const char *label, const t_mat *m)
{
	char	*s;

	s = mat_to_string(m);
	printf("%s\n%s\n", label, s ? s : "(error)");
	free(s);
}

int	main(void)
{
	t_complex	uv[2] = {{1, 2}, {3, -1}};
	t_complex	vv[2] = {{0, 1}, {2, 2}};
	t_complex	mv[4] = {{1, 1}, {0, 2}, {3, 0}, {1, -1}};
	t_complex	i_unit = {0, 1};
	t_complex	coefs[2] = {{1, 0}, {0, 1}};
	t_complex	d;
	t_vec		u;
	t_vec		v;
	t_vec		w;
	t_vec		lin[2];
	t_mat		m;
	t_mat		r;
	t_mat		inv;
	t_mat		prod;
	float		n;

	vec_from_complex(&u, 2, uv);
	vec_from_complex(&v, 2, vv);
	vec_clone(&w, &u);
	vec_add(&w, &v);
	say_vec("00 add         ", &w);
	vec_free(&w);
	vec_clone(&w, &u);
	vec_scl(&w, &i_unit);
	say_vec("00 scl by i    ", &w);
	vec_free(&w);
	lin[0] = u;
	lin[1] = v;
	vec_linear_combination(&w, lin, 2, coefs);
	say_vec("01 lin. comb.  ", &w);
	vec_free(&w);
	vec_lerp(&w, &u, &v, 0.5f);
	say_vec("02 lerp        ", &w);
	vec_free(&w);
	vec_dot(&u, &v, &d);
	say("03 <u|v>      ", &g_complex, &d);
	vec_dot(&u, &u, &d);
	say("03 <u|u>      ", &g_complex, &d);
	printf("04 norms       %g %g %g\n", (double)vec_norm_1(&u),
		(double)vec_norm(&u), (double)vec_norm_inf(&u));
	angle_cos(&u, &v, &n);
	printf("05 cos         %g\n", (double)n);
	mat_from_complex(&m, 2, 2, mv);
	mat_mul_vec(&w, &m, &u);
	say_vec("07 M*u         ", &w);
	vec_free(&w);
	mat_trace(&m, &d);
	say("08 trace      ", &g_complex, &d);
	mat_transpose(&r, &m);
	say_mat("09 transpose", &r);
	mat_free(&r);
	mat_conjugate_transpose(&r, &m);
	say_mat("09 conjugate transpose", &r);
	mat_free(&r);
	mat_determinant(&m, &d);
	say("11 det        ", &g_complex, &d);
	if (mat_inverse(&inv, &m) == E_OK)
	{
		say_mat("12 inverse", &inv);
		mat_mul_mat(&prod, &m, &inv);
		say_mat("12 M*M^-1", &prod);
		mat_free(&prod);
	}
	mat_free(&inv);
	mat_row_echelon(&r, &m);
	say_mat("10 row echelon", &r);
	mat_free(&r);
	vec_free(&u);
	vec_free(&v);
	mat_free(&m);
	return (0);
}

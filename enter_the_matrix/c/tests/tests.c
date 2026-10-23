/* Self-checking tests for every exercise. Run with `make test`.
** Besides the subject examples this checks algebraic laws on random real and
** complex matrices, and tries hard to break each exercise: NULL pointers,
** wrong shapes, empty vectors, NaN / infinity, extreme magnitudes, shape
** overflow... Nothing here may crash; errors are returned as codes. */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

static int	g_fail;
static int	g_total;

#define CHECK(cond) do { g_total++; if (!(cond)) { g_fail++; \
	printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)

/* ---------------------------------------------------------------------------
** Memory bookkeeping: every buffer a test creates is freed at its end.
** ------------------------------------------------------------------------ */

static void	*g_bufs[1 << 16];
static size_t	g_nbufs;

static void	reg(void *p)
{
	if (p && g_nbufs < sizeof(g_bufs) / sizeof(*g_bufs))
		g_bufs[g_nbufs++] = p;
}

static void	free_all(void)
{
	while (g_nbufs)
		free(g_bufs[--g_nbufs]);
}

static t_vec	V(size_t n, const float *vals)
{
	t_vec	v;

	CHECK(vec_from_f32(&v, n, vals) == E_OK);
	reg(v.data);
	return (v);
}

static t_mat	M(size_t rows, size_t cols, const float *vals)
{
	t_mat	m;

	CHECK(mat_from_f32(&m, rows, cols, vals) == E_OK);
	reg(m.data);
	return (m);
}

# define VF(...) V(sizeof((float[]){__VA_ARGS__}) / sizeof(float), \
	(float[]){__VA_ARGS__})
# define MF(r, c, ...) M((r), (c), (float[]){__VA_ARGS__})

static void	check_vstr(const t_vec *v, const char *want)
{
	char	*s;

	s = vec_to_string(v);
	g_total++;
	if (!s || strcmp(s, want) != 0)
	{
		g_fail++;
		printf("FAIL vector: got '%s', want '%s'\n", s ? s : "(null)", want);
	}
	free(s);
}

static void	check_mstr(const t_mat *m, const char *want)
{
	char	*s;

	s = mat_to_string(m);
	g_total++;
	if (!s || strcmp(s, want) != 0)
	{
		g_fail++;
		printf("FAIL matrix: got '%s', want '%s'\n", s ? s : "(null)", want);
	}
	free(s);
}

static void	check_fstr(float x, const char *want)
{
	char	buf[64];

	fmt_f32(buf, sizeof(buf), x);
	g_total++;
	if (strcmp(buf, want) != 0)
	{
		g_fail++;
		printf("FAIL format: got '%s', want '%s'\n", buf, want);
	}
}

static bool	near_f(float a, float b, float tol)
{
	float	scale;

	scale = fabsf(b) > 1.0f ? fabsf(b) : 1.0f;
	return (fabsf(a - b) <= tol * scale);
}

static uint64_t	g_rng = 88172645463325252ull;

static uint32_t	rnd(void)
{
	g_rng ^= g_rng << 13;
	g_rng ^= g_rng >> 7;
	g_rng ^= g_rng << 17;
	return ((uint32_t)(g_rng >> 11));
}

/* uniform in [-1, 1] */
static float	unit(void)
{
	return ((float)(rnd() % 20001) / 10000.0f - 1.0f);
}

/* integer in [-bound, bound] */
static float	rint_(uint32_t bound)
{
	return ((float)(rnd() % (2 * bound + 1)) - (float)bound);
}

static t_mat	rand_int_mat(size_t r, size_t c, uint32_t bound)
{
	t_mat	m;
	size_t	i;

	CHECK(mat_new(&m, &g_real, r, c) == E_OK);
	reg(m.data);
	for (i = 0; i < r * c; i++)
		((float *)m.data)[i] = rint_(bound);
	return (m);
}

static t_vec	rand_int_vec(size_t n, uint32_t bound)
{
	t_vec	v;
	size_t	i;

	CHECK(vec_new(&v, &g_real, n) == E_OK);
	reg(v.data);
	for (i = 0; i < n; i++)
		((float *)v.data)[i] = rint_(bound);
	return (v);
}

/* n x n strictly diagonally dominant matrix: invertible, well conditioned */
static t_mat	dominant_real(size_t n)
{
	t_mat	m;
	size_t	i;

	CHECK(mat_new(&m, &g_real, n, n) == E_OK);
	reg(m.data);
	for (i = 0; i < n * n; i++)
		((float *)m.data)[i] = unit();
	for (i = 0; i < n; i++)
		((float *)m.data)[i * n + i] += (float)n + 1.0f;
	return (m);
}

static t_mat	dominant_complex(size_t n)
{
	t_mat	m;
	size_t	i;

	CHECK(mat_new(&m, &g_complex, n, n) == E_OK);
	reg(m.data);
	for (i = 0; i < n * n; i++)
	{
		((t_complex *)m.data)[i].re = unit();
		((t_complex *)m.data)[i].im = unit();
	}
	for (i = 0; i < n; i++)
		((t_complex *)m.data)[i * n + i].re += (float)n + 1.0f;
	return (m);
}

static t_mat	identity(const t_field *f, size_t n)
{
	t_mat	m;
	size_t	i;

	CHECK(mat_new(&m, f, n, n) == E_OK);
	reg(m.data);
	for (i = 0; i < n; i++)
		f->one(EL(f, m.data, i * n + i));
	return (m);
}

/* largest |a - b| over all entries */
static float	max_diff(const t_mat *a, const t_mat *b)
{
	const t_field	*f = a->f;
	size_t			i;
	float			m;
	float			d;
	t_complex		x;

	m = 0.0f;
	for (i = 0; i < a->rows * a->cols; i++)
	{
		f->sub(&x, (char *)a->data + i * f->size, (char *)b->data + i * f->size);
		d = f->modulus(&x);
		if (d > m || d != d)
			m = d;
	}
	return (m);
}

/* ---------------------------------------------------------------------------
** Number formatting
** ------------------------------------------------------------------------ */

static void	test_format(void)
{
	uint32_t	bits;
	float		x;
	char		buf[64];
	int			i;

	check_fstr(0.0f, "0.0");
	check_fstr(-0.0f, "0.0");
	check_fstr(1.0f, "1.0");
	check_fstr(-1.0f, "-1.0");
	check_fstr(0.5f, "0.5");
	check_fstr(27.3f, "27.3");
	check_fstr(100.0f, "100.0");
	check_fstr(230.0f, "230.0");
	check_fstr(-12.166666f, "-12.166666");
	check_fstr(0.001f, "0.001");
	check_fstr(0.0001f, "0.0001");
	check_fstr(1e-5f, "1e-5");
	check_fstr(1.5e-7f, "1.5e-7");
	check_fstr(1e20f, "1e20");
	check_fstr(1.5e20f, "1.5e20");
	check_fstr(1e15f, "1000000000000000.0");
	check_fstr(1e16f, "1e16");
	check_fstr(3.4028235e38f, "3.4028235e38");
	check_fstr(1.0f / 0.0f, "inf");
	check_fstr(-1.0f / 0.0f, "-inf");
	check_fstr(0.0f / 0.0f, "NaN");
	/* a million random bit patterns: the text must read back as the same float */
	for (i = 0; i < 1000000; i++)
	{
		bits = rnd() ^ (rnd() << 17);
		memcpy(&x, &bits, sizeof(x));
		if (x != x || x - x != 0.0f || x == 0.0f)
			continue ;
		fmt_f32(buf, sizeof(buf), x);
		if (strtof(buf, NULL) != x)
		{
			printf("FAIL round trip: %.9g -> '%s'\n", (double)x, buf);
			g_fail++;
			break ;
		}
	}
	g_total++;
}

/* ---------------------------------------------------------------------------
** The examples of the subject, exercise by exercise
** ------------------------------------------------------------------------ */

static void	test_subject_00_to_03(void)
{
	t_vec	u;
	t_vec	v;
	t_vec	r;
	t_mat	a;
	t_mat	b;
	t_mat	m;
	float	two = 2.0f;
	float	dot;

	u = VF(2., 3.);
	v = VF(5., 7.);
	CHECK(vec_add(&u, &v) == E_OK);
	check_vstr(&u, "[7.0]\n[10.0]");
	u = VF(2., 3.);
	CHECK(vec_sub(&u, &v) == E_OK);
	check_vstr(&u, "[-3.0]\n[-4.0]");
	u = VF(2., 3.);
	CHECK(vec_scl(&u, &two) == E_OK);
	check_vstr(&u, "[4.0]\n[6.0]");
	b = MF(2, 2, 7., 4., -2., 2.);
	a = MF(2, 2, 1., 2., 3., 4.);
	CHECK(mat_add(&a, &b) == E_OK);
	check_mstr(&a, "[8.0, 6.0]\n[1.0, 6.0]");
	a = MF(2, 2, 1., 2., 3., 4.);
	CHECK(mat_sub(&a, &b) == E_OK);
	check_mstr(&a, "[-6.0, -2.0]\n[5.0, 2.0]");
	a = MF(2, 2, 1., 2., 3., 4.);
	CHECK(mat_scl(&a, &two) == E_OK);
	check_mstr(&a, "[2.0, 4.0]\n[6.0, 8.0]");

	{
		t_vec	e[3] = {VF(1., 0., 0.), VF(0., 1., 0.), VF(0., 0., 1.)};
		float	c3[3] = {10.f, -2.f, 0.5f};
		t_vec	w[2] = {VF(1., 2., 3.), VF(0., 10., -100.)};
		float	c2[2] = {10.f, -2.f};

		CHECK(vec_linear_combination(&r, e, 3, c3) == E_OK);
		reg(r.data);
		check_vstr(&r, "[10.0]\n[-2.0]\n[0.5]");
		CHECK(vec_linear_combination(&r, w, 2, c2) == E_OK);
		reg(r.data);
		check_vstr(&r, "[10.0]\n[0.0]\n[230.0]");
	}

	CHECK(lerp_f32(0., 1., 0.) == 0.0f && lerp_f32(0., 1., 1.) == 1.0f);
	CHECK(lerp_f32(0., 1., 0.5f) == 0.5f);
	check_fstr(lerp_f32(21., 42., 0.3f), "27.3");
	u = VF(2., 1.);
	v = VF(4., 2.);
	CHECK(vec_lerp(&r, &u, &v, 0.3f) == E_OK);
	reg(r.data);
	check_vstr(&r, "[2.6]\n[1.3]");
	a = MF(2, 2, 2., 1., 3., 4.);
	b = MF(2, 2, 20., 10., 30., 40.);
	CHECK(mat_lerp(&m, &a, &b, 0.5f) == E_OK);
	reg(m.data);
	check_mstr(&m, "[11.0, 5.5]\n[16.5, 22.0]");

	u = VF(0., 0.);
	v = VF(1., 1.);
	CHECK(vec_dot(&u, &v, &dot) == E_OK && dot == 0.0f);
	u = VF(1., 1.);
	CHECK(vec_dot(&u, &v, &dot) == E_OK && dot == 2.0f);
	u = VF(-1., 6.);
	v = VF(3., 2.);
	CHECK(vec_dot(&u, &v, &dot) == E_OK && dot == 9.0f);
	free_all();
}

static void	test_subject_04_to_09(void)
{
	t_vec	u;
	t_vec	v;
	t_vec	r;
	t_mat	a;
	t_mat	b;
	t_mat	m;
	float	c;
	float	t;

	u = VF(0., 0., 0.);
	CHECK(vec_norm_1(&u) == 0.0f && vec_norm(&u) == 0.0f
		&& vec_norm_inf(&u) == 0.0f);
	u = VF(1., 2., 3.);
	CHECK(vec_norm_1(&u) == 6.0f && vec_norm_inf(&u) == 3.0f);
	CHECK(near_f(vec_norm(&u), 3.74165738f, 1e-6f));
	u = VF(-1., -2.);
	CHECK(vec_norm_1(&u) == 3.0f && vec_norm_inf(&u) == 2.0f);
	CHECK(near_f(vec_norm(&u), 2.236067977f, 1e-6f));
	u = VF(3., 4.);
	CHECK(vec_norm(&u) == 5.0f);

	u = VF(1., 0.);
	v = VF(1., 0.);
	CHECK(angle_cos(&u, &v, &c) == E_OK && c == 1.0f);
	v = VF(0., 1.);
	CHECK(angle_cos(&u, &v, &c) == E_OK && c == 0.0f);
	u = VF(-1., 1.);
	v = VF(1., -1.);
	CHECK(angle_cos(&u, &v, &c) == E_OK && c == -1.0f);
	u = VF(2., 1.);
	v = VF(4., 2.);
	CHECK(angle_cos(&u, &v, &c) == E_OK && c == 1.0f);
	u = VF(1., 2., 3.);
	v = VF(4., 5., 6.);
	CHECK(angle_cos(&u, &v, &c) == E_OK && near_f(c, 0.974631846f, 1e-6f));

	u = VF(0., 0., 1.);
	v = VF(1., 0., 0.);
	CHECK(cross_product(&r, &u, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[0.0]\n[1.0]\n[0.0]");
	u = VF(1., 2., 3.);
	v = VF(4., 5., 6.);
	CHECK(cross_product(&r, &u, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[-3.0]\n[6.0]\n[-3.0]");
	u = VF(4., 2., -3.);
	v = VF(-2., -5., 16.);
	CHECK(cross_product(&r, &u, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[17.0]\n[-58.0]\n[-16.0]");

	v = VF(4., 2.);
	a = MF(2, 2, 1., 0., 0., 1.);
	CHECK(mat_mul_vec(&r, &a, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[4.0]\n[2.0]");
	a = MF(2, 2, 2., 0., 0., 2.);
	CHECK(mat_mul_vec(&r, &a, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[8.0]\n[4.0]");
	a = MF(2, 2, 2., -2., -2., 2.);
	CHECK(mat_mul_vec(&r, &a, &v) == E_OK);
	reg(r.data);
	check_vstr(&r, "[4.0]\n[-4.0]");
	a = MF(2, 2, 1., 0., 0., 1.);
	CHECK(mat_mul_mat(&m, &a, &a) == E_OK);
	reg(m.data);
	check_mstr(&m, "[1.0, 0.0]\n[0.0, 1.0]");
	b = MF(2, 2, 2., 1., 4., 2.);
	CHECK(mat_mul_mat(&m, &a, &b) == E_OK);
	reg(m.data);
	check_mstr(&m, "[2.0, 1.0]\n[4.0, 2.0]");
	a = MF(2, 2, 3., -5., 6., 8.);
	CHECK(mat_mul_mat(&m, &a, &b) == E_OK);
	reg(m.data);
	check_mstr(&m, "[-14.0, -7.0]\n[44.0, 22.0]");

	a = MF(2, 2, 1., 0., 0., 1.);
	CHECK(mat_trace(&a, &t) == E_OK && t == 2.0f);
	a = MF(3, 3, 2., -5., 0., 4., 3., 7., -2., 3., 4.);
	CHECK(mat_trace(&a, &t) == E_OK && t == 9.0f);
	a = MF(3, 3, -2., -8., 4., 1., -23., 4., 0., 6., 4.);
	CHECK(mat_trace(&a, &t) == E_OK && t == -21.0f);

	a = MF(2, 3, 1., 2., 3., 4., 5., 6.);
	CHECK(mat_transpose(&m, &a) == E_OK);
	reg(m.data);
	check_mstr(&m, "[1.0, 4.0]\n[2.0, 5.0]\n[3.0, 6.0]");
	free_all();
}

static void	test_subject_10_to_14(void)
{
	t_mat	a;
	t_mat	m;
	float	d;
	size_t	rank;
	size_t	i;
	float	want[3][5] = {{1., 0.625f, 0., 0., -12.1666667f},
	{0., 0., 1., 0., -3.6666667f}, {0., 0., 0., 1., 29.5f}};
	float	inv[9] = {0.649425287f, 0.097701149f, -0.655172414f, -0.781609195f,
		-0.126436782f, 0.965517241f, 0.143678161f, 0.074712644f, -0.206896552f};

	a = MF(3, 3, 1., 0., 0., 0., 1., 0., 0., 0., 1.);
	CHECK(mat_row_echelon(&m, &a) == E_OK);
	reg(m.data);
	CHECK(mat_equal(&a, &m));
	a = MF(2, 2, 1., 2., 3., 4.);
	CHECK(mat_row_echelon(&m, &a) == E_OK);
	reg(m.data);
	check_mstr(&m, "[1.0, 0.0]\n[0.0, 1.0]");
	a = MF(2, 2, 1., 2., 2., 4.);
	CHECK(mat_row_echelon(&m, &a) == E_OK);
	reg(m.data);
	check_mstr(&m, "[1.0, 2.0]\n[0.0, 0.0]");
	a = MF(3, 5, 8., 5., -2., 4., 28., 4., 2.5, 20., 4., -4., 8., 5., 1., 4., 17.);
	CHECK(mat_row_echelon(&m, &a) == E_OK);
	reg(m.data);
	for (i = 0; i < 15; i++)
		CHECK(near_f(((float *)m.data)[i], want[i / 5][i % 5], 1e-4f));

	a = MF(2, 2, 1., -1., -1., 1.);
	CHECK(mat_determinant(&a, &d) == E_OK && d == 0.0f);
	a = MF(3, 3, 2., 0., 0., 0., 2., 0., 0., 0., 2.);
	CHECK(mat_determinant(&a, &d) == E_OK && d == 8.0f);
	a = MF(3, 3, 8., 5., -2., 4., 7., 20., 7., 6., 1.);
	CHECK(mat_determinant(&a, &d) == E_OK && d == -174.0f);
	a = MF(4, 4, 8., 5., -2., 4., 4., 2.5, 20., 4., 8., 5., 1., 4., 28., -4., 17., 1.);
	CHECK(mat_determinant(&a, &d) == E_OK && d == 1032.0f);

	a = MF(3, 3, 1., 0., 0., 0., 1., 0., 0., 0., 1.);
	CHECK(mat_inverse(&m, &a) == E_OK);
	reg(m.data);
	CHECK(mat_equal(&a, &m));
	a = MF(3, 3, 2., 0., 0., 0., 2., 0., 0., 0., 2.);
	CHECK(mat_inverse(&m, &a) == E_OK);
	reg(m.data);
	check_mstr(&m, "[0.5, 0.0, 0.0]\n[0.0, 0.5, 0.0]\n[0.0, 0.0, 0.5]");
	a = MF(3, 3, 8., 5., -2., 4., 7., 20., 7., 6., 1.);
	CHECK(mat_inverse(&m, &a) == E_OK);
	reg(m.data);
	for (i = 0; i < 9; i++)
		CHECK(near_f(((float *)m.data)[i], inv[i], 1e-5f));

	a = MF(3, 3, 1., 0., 0., 0., 1., 0., 0., 0., 1.);
	CHECK(mat_rank(&a, &rank) == E_OK && rank == 3);
	a = MF(3, 4, 1., 2., 0., 0., 2., 4., 0., 0., -1., 2., 1., 1.);
	CHECK(mat_rank(&a, &rank) == E_OK && rank == 2);
	a = MF(4, 3, 8., 5., -2., 4., 7., 20., 7., 6., 1., 21., 18., 7.);
	CHECK(mat_rank(&a, &rank) == E_OK && rank == 3);
	free_all();
}

/* ---------------------------------------------------------------------------
** Projection (exercise 14)
** ------------------------------------------------------------------------ */

static bool	ndc(const t_mat *p, float x, float y, float z, float out[3])
{
	t_vec	pt;
	t_vec	s;
	float	*q;
	bool	ok;

	pt = VF(x, y, z, 1.);
	ok = mat_mul_vec(&s, p, &pt) == E_OK;
	reg(s.data);
	q = s.data;
	out[0] = q[0] / q[3];
	out[1] = q[1] / q[3];
	out[2] = q[2] / q[3];
	return (ok);
}

static void	test_projection(void)
{
	const float	pi = 3.14159265358979f;
	t_mat		p;
	float		o[3];
	float		d;
	char		*text;
	float		a[3];

	CHECK(projection(&p, pi / 3.0f, 16.0f / 9.0f, 0.5f, 100.0f) == E_OK);
	reg(p.data);
	CHECK(ndc(&p, 0., 0., -0.5f, o) && near_f(o[2], 0.0f, 1e-5f));
	CHECK(ndc(&p, 0., 0., -100.f, o) && near_f(o[2], 1.0f, 1e-5f));
	CHECK(ndc(&p, 0., 0., -10.f, a) && a[2] > 0.0f && a[2] < 1.0f);
	CHECK(ndc(&p, 0., 0., -1.f, o) && ndc(&p, 0., 0., -2.f, a) && o[2] < a[2]);
	CHECK(projection(&p, pi / 2.0f, 2.0f, 1.0f, 50.0f) == E_OK);
	reg(p.data);
	for (d = 1.0f; d <= 50.0f; d *= 7.0f)
	{
		CHECK(ndc(&p, 2.0f * d, d, -d, o) && near_f(o[0], 1.0f, 1e-5f)
			&& near_f(o[1], 1.0f, 1e-5f));
		CHECK(ndc(&p, -2.0f * d, -d, -d, o) && near_f(o[0], -1.0f, 1e-5f)
			&& near_f(o[1], -1.0f, 1e-5f));
	}
	CHECK(ndc(&p, 0., 0., -7.f, o) && near_f(o[0], 0.0f, 1e-6f)
		&& near_f(o[1], 0.0f, 1e-6f));
	CHECK(projection(&p, pi / 2.0f, 1.0f, 1.0f, 3.0f) == E_OK);
	reg(p.data);
	text = column_major_text(&p);
	CHECK(text && strchr(text, '\n') && strlen(text) > 20);
	free(text);

	/* invalid parameters are errors, never a matrix */
	CHECK(projection(&p, 0.0f, 1.f, 1.f, 2.f) == E_INVALID && !p.data);
	CHECK(projection(&p, pi, 1.f, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, -1.f, 1.f, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 0.f, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, -1.f, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 1.f, 0.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 1.f, -1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 1.f, 2.f, 1.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 1.f, 2.f, 2.f) == E_INVALID);
	CHECK(projection(&p, NAN, 1.f, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, NAN, 1.f, 2.f) == E_INVALID);
	CHECK(projection(&p, 1.f, 1.f, 1.f, INFINITY) == E_INVALID);
	CHECK(projection(NULL, 1.f, 1.f, 1.f, 2.f) == E_INVALID);
	/* extreme but valid parameters stay finite */
	CHECK(projection(&p, 0.001f, 1.f, 1e-3f, 1e6f) == E_OK);
	reg(p.data);
	CHECK(projection(&p, 3.14f, 100.f, 1.f, 2.f) == E_OK);
	reg(p.data);
	free_all();
}

/* ---------------------------------------------------------------------------
** Algebraic laws on random data
** ------------------------------------------------------------------------ */

static void	test_inverse_laws(const t_field *f, t_mat (*gen)(size_t), float tol)
{
	t_mat	a;
	t_mat	inv;
	t_mat	prod;
	size_t	n;
	size_t	k;
	size_t	rank;

	for (n = 1; n <= 10; n++)
		for (k = 0; k < 15; k++)
		{
			a = gen(n);
			CHECK(mat_inverse(&inv, &a) == E_OK);
			reg(inv.data);
			CHECK(mat_mul_mat(&prod, &a, &inv) == E_OK);
			reg(prod.data);
			CHECK(max_diff(&prod, &(t_mat){f, n, n, identity(f, n).data}) < tol);
			CHECK(mat_mul_mat(&prod, &inv, &a) == E_OK);
			reg(prod.data);
			CHECK(max_diff(&prod, &(t_mat){f, n, n, identity(f, n).data}) < tol);
			CHECK(mat_rank(&a, &rank) == E_OK && rank == n);
		}
	free_all();
}

static void	test_real_laws(void)
{
	t_mat	a;
	t_mat	b;
	t_mat	ab;
	t_mat	t;
	t_mat	t2;
	t_mat	rr;
	t_mat	rr2;
	float	da;
	float	db;
	float	dab;
	size_t	n;
	size_t	k;
	size_t	rank;
	size_t	rank_t;

	test_inverse_laws(&g_real, dominant_real, 1e-4f);
	for (n = 1; n <= 4; n++)
		for (k = 0; k < 50; k++)
		{
			a = rand_int_mat(n, n, 3);
			b = rand_int_mat(n, n, 3);
			CHECK(mat_mul_mat(&ab, &a, &b) == E_OK);
			reg(ab.data);
			CHECK(mat_determinant(&a, &da) == E_OK);
			CHECK(mat_determinant(&b, &db) == E_OK);
			CHECK(mat_determinant(&ab, &dab) == E_OK);
			CHECK(dab == da * db);	/* small integers: exact in float */
			CHECK(mat_transpose(&t, &a) == E_OK);
			reg(t.data);
			CHECK(mat_determinant(&t, &db) == E_OK && db == da);
			CHECK(mat_rank(&a, &rank) == E_OK);
			CHECK((da == 0.0f) == (rank < n));
		}
	/* rank of a product of thin matrices, rank of the transpose, rref idempotent */
	for (k = 0; k < 50; k++)
	{
		size_t r = 2 + rnd() % 3;
		t_mat  bb = rand_int_mat(6, r, 3);
		t_mat  cc = rand_int_mat(r, 7, 3);

		CHECK(mat_mul_mat(&a, &bb, &cc) == E_OK);
		reg(a.data);
		CHECK(mat_rank(&a, &rank) == E_OK && rank <= r);
		CHECK(mat_transpose(&t, &a) == E_OK);
		reg(t.data);
		CHECK(mat_rank(&t, &rank_t) == E_OK && rank_t == rank);
		CHECK(mat_row_echelon(&rr, &a) == E_OK);
		reg(rr.data);
		CHECK(mat_rank(&rr, &rank_t) == E_OK && rank_t == rank);
		CHECK(mat_row_echelon(&rr2, &rr) == E_OK);
		reg(rr2.data);
		CHECK(mat_equal(&rr, &rr2));
	}
	/* (AB)^T = B^T A^T, associativity, identity */
	a = rand_int_mat(4, 5, 4);
	b = rand_int_mat(5, 3, 4);
	{
		t_mat	c = rand_int_mat(3, 6, 4);
		t_mat	bc;
		t_mat	abc1;
		t_mat	abc2;
		t_mat	bt;
		t_mat	at;
		t_mat	btat;
		t_mat	id;

		CHECK(mat_mul_mat(&ab, &a, &b) == E_OK);
		reg(ab.data);
		CHECK(mat_mul_mat(&abc1, &ab, &c) == E_OK);
		reg(abc1.data);
		CHECK(mat_mul_mat(&bc, &b, &c) == E_OK);
		reg(bc.data);
		CHECK(mat_mul_mat(&abc2, &a, &bc) == E_OK);
		reg(abc2.data);
		CHECK(mat_equal(&abc1, &abc2));
		CHECK(mat_transpose(&t, &ab) == E_OK);
		reg(t.data);
		CHECK(mat_transpose(&bt, &b) == E_OK && mat_transpose(&at, &a) == E_OK);
		reg(bt.data);
		reg(at.data);
		CHECK(mat_mul_mat(&btat, &bt, &at) == E_OK);
		reg(btat.data);
		CHECK(mat_equal(&t, &btat));
		id = identity(&g_real, 5);
		CHECK(mat_mul_mat(&t2, &a, &id) == E_OK);
		reg(t2.data);
		CHECK(mat_equal(&t2, &a));
	}
	free_all();
}

static void	test_vector_laws(void)
{
	t_vec	u;
	t_vec	v;
	t_vec	s;
	t_vec	uv;
	t_vec	vu;
	float	a;
	float	b;
	float	d1;
	float	d2;
	float	c;
	int		k;

	for (k = 0; k < 300; k++)
	{
		u = rand_int_vec(5, 9);
		v = rand_int_vec(5, 9);
		CHECK(vec_clone(&uv, &u) == E_OK && vec_clone(&vu, &v) == E_OK);
		reg(uv.data);
		reg(vu.data);
		vec_add(&uv, &v);
		vec_add(&vu, &u);
		CHECK(vec_equal(&uv, &vu));			/* u + v = v + u */
		vec_sub(&uv, &v);
		CHECK(vec_equal(&uv, &u));			/* (u + v) - v = u */
		CHECK(vec_dot(&u, &v, &d1) == E_OK && vec_dot(&v, &u, &d2) == E_OK
			&& d1 == d2);
		/* linear_combination agrees with scl + add */
		a = rint_(5);
		b = rint_(5);
		{
			t_vec	two[2];
			float	co[2];
			t_vec	lc;
			t_vec	au;
			t_vec	bv;

			two[0] = u;
			two[1] = v;
			co[0] = a;
			co[1] = b;
			CHECK(vec_linear_combination(&lc, two, 2, co) == E_OK);
			reg(lc.data);
			vec_clone(&au, &u);
			vec_clone(&bv, &v);
			reg(au.data);
			reg(bv.data);
			vec_scl(&au, &a);
			vec_scl(&bv, &b);
			vec_add(&au, &bv);
			CHECK(vec_equal(&lc, &au));
		}
		/* lerp at the endpoints is exact */
		CHECK(vec_lerp(&s, &u, &v, 0.0f) == E_OK && vec_equal(&s, &u));
		reg(s.data);
		CHECK(vec_lerp(&s, &u, &v, 1.0f) == E_OK && vec_equal(&s, &v));
		reg(s.data);
	}
	for (k = 0; k < 500; k++)
	{
		t_vec	x;
		t_vec	y;
		t_vec	sum;
		size_t	i;

		vec_new(&x, &g_real, 6);
		vec_new(&y, &g_real, 6);
		reg(x.data);
		reg(y.data);
		for (i = 0; i < 6; i++)
		{
			((float *)x.data)[i] = unit() * 100.0f;
			((float *)y.data)[i] = unit() * 100.0f;
		}
		vec_dot(&x, &y, &d1);
		CHECK(fabsf(d1) <= vec_norm(&x) * vec_norm(&y) * 1.0001f);	/* Cauchy-Schwarz */
		vec_clone(&sum, &x);
		reg(sum.data);
		vec_add(&sum, &y);
		CHECK(vec_norm(&sum) <= (vec_norm(&x) + vec_norm(&y)) * 1.0001f);
		CHECK(vec_norm_inf(&x) <= vec_norm(&x) * 1.0001f
			&& vec_norm(&x) <= vec_norm_1(&x) * 1.0001f);
		vec_dot(&x, &x, &d2);
		CHECK(fabsf(vec_norm(&x) * vec_norm(&x) - d2) <= 1e-3f * d2);
		CHECK(angle_cos(&x, &y, &c) == E_OK && c >= -1.0f && c <= 1.0f);
	}
	free_all();
}

static void	test_cross_laws(void)
{
	t_vec	u;
	t_vec	v;
	t_vec	w;
	t_vec	uv;
	t_mat	m;
	float	d1;
	float	d2;
	float	det;
	int		k;

	for (k = 0; k < 500; k++)
	{
		u = rand_int_vec(3, 20);
		v = rand_int_vec(3, 20);
		w = rand_int_vec(3, 20);
		CHECK(cross_product(&uv, &u, &v) == E_OK);
		reg(uv.data);
		CHECK(vec_dot(&uv, &u, &d1) == E_OK && d1 == 0.0f);
		CHECK(vec_dot(&uv, &v, &d1) == E_OK && d1 == 0.0f);
		vec_dot(&uv, &uv, &d1);
		{
			float	uu;
			float	vv;
			float	uvd;

			vec_dot(&u, &u, &uu);
			vec_dot(&v, &v, &vv);
			vec_dot(&u, &v, &uvd);
			CHECK(d1 == uu * vv - uvd * uvd);	/* Lagrange identity, exact */
		}
		/* the triple product is the 3x3 determinant */
		CHECK(mat_new(&m, &g_real, 3, 3) == E_OK);
		reg(m.data);
		memcpy((float *)m.data, u.data, 3 * sizeof(float));
		memcpy((float *)m.data + 3, v.data, 3 * sizeof(float));
		memcpy((float *)m.data + 6, w.data, 3 * sizeof(float));
		vec_dot(&w, &uv, &d2);
		CHECK(mat_determinant(&m, &det) == E_OK && det == d2);
	}
	free_all();
}

/* the same laws over the complex numbers (exercise 15) */
static void	test_complex(void)
{
	t_mat		a;
	t_mat		b;
	t_mat		ab;
	t_mat		h;
	t_mat		hh;
	t_mat		lhs;
	t_mat		rhs;
	t_mat		bh;
	t_mat		ah;
	t_vec		u;
	t_vec		v;
	t_complex	uv;
	t_complex	vu;
	t_complex	uu;
	t_complex	z;
	float		c;
	size_t		i;

	test_inverse_laws(&g_complex, dominant_complex, 1e-4f);
	a = dominant_complex(3);
	CHECK(mat_conjugate_transpose(&h, &a) == E_OK);
	reg(h.data);
	CHECK(mat_conjugate_transpose(&hh, &h) == E_OK);
	reg(hh.data);
	CHECK(mat_equal(&hh, &a));
	b = dominant_complex(3);
	CHECK(mat_mul_mat(&ab, &a, &b) == E_OK);
	reg(ab.data);
	CHECK(mat_conjugate_transpose(&lhs, &ab) == E_OK);
	reg(lhs.data);
	mat_conjugate_transpose(&bh, &b);
	mat_conjugate_transpose(&ah, &a);
	reg(bh.data);
	reg(ah.data);
	CHECK(mat_mul_mat(&rhs, &bh, &ah) == E_OK);
	reg(rhs.data);
	CHECK(max_diff(&lhs, &rhs) < 1e-4f);	/* (AB)^H = B^H A^H */

	vec_new(&u, &g_complex, 5);
	vec_new(&v, &g_complex, 5);
	reg(u.data);
	reg(v.data);
	for (i = 0; i < 5; i++)
	{
		((t_complex *)u.data)[i] = (t_complex){unit(), unit()};
		((t_complex *)v.data)[i] = (t_complex){unit(), unit()};
	}
	vec_dot(&u, &v, &uv);
	vec_dot(&v, &u, &vu);
	g_complex.conj(&z, &vu);
	CHECK(fabsf(uv.re - z.re) < 1e-5f && fabsf(uv.im - z.im) < 1e-5f);
	vec_dot(&u, &u, &uu);
	CHECK(uu.im == 0.0f && uu.re > 0.0f);	/* <u|u> is exactly real */
	CHECK(fabsf(vec_norm(&u) * vec_norm(&u) - uu.re) < 1e-4f);
	CHECK(angle_cos(&u, &v, &c) == E_OK && c >= -1.0f && c <= 1.0f);

	/* field facts */
	{
		t_complex	x = {3.f, 4.f};
		t_complex	y = {1.f, -2.f};
		t_complex	q;
		char		buf[96];

		g_complex.mul(&q, &x, &y);
		g_complex.div(&q, &q, &y);
		CHECK(fabsf(q.re - 3.f) < 1e-5f && fabsf(q.im - 4.f) < 1e-5f);
		CHECK(g_complex.modulus(&x) == 5.0f);
		g_complex.format(buf, sizeof(buf), &x);
		CHECK(strcmp(buf, "3.0+4.0i") == 0);
		g_complex.format(buf, sizeof(buf), &y);
		CHECK(strcmp(buf, "1.0-2.0i") == 0);
		x = (t_complex){1e-30f, 1e-30f};
		y = (t_complex){1.f, 1.f};
		g_complex.div(&q, &y, &x);	/* would overflow with the naive formula */
		CHECK(fin_f32(q.re) && fin_f32(q.im));
	}
	free_all();
}

/* ---------------------------------------------------------------------------
** Hostile input
** ------------------------------------------------------------------------ */

static void	test_null_pointers(void)
{
	t_vec	v;
	t_mat	m;
	t_vec	out;
	t_mat	mo;
	float	f = 1.0f;
	size_t	rank;

	v = VF(1., 2., 3.);
	m = MF(3, 3, 1., 0., 0., 0., 1., 0., 0., 0., 1.);
	CHECK(vec_add(NULL, &v) == E_INVALID && vec_add(&v, NULL) == E_INVALID);
	CHECK(vec_sub(NULL, NULL) == E_INVALID);
	CHECK(vec_scl(NULL, &f) == E_INVALID && vec_scl(&v, NULL) == E_INVALID);
	CHECK(mat_add(NULL, &m) == E_INVALID && mat_add(&m, NULL) == E_INVALID);
	CHECK(mat_scl(&m, NULL) == E_INVALID);
	CHECK(vec_linear_combination(NULL, &v, 1, &f) == E_INVALID);
	CHECK(vec_linear_combination(&out, NULL, 1, &f) == E_INVALID);
	CHECK(vec_linear_combination(&out, &v, 1, NULL) == E_INVALID);
	CHECK(vec_linear_combination(&out, &v, 0, &f) == E_EMPTY);
	CHECK(mat_linear_combination(&mo, &m, 0, &f) == E_EMPTY);
	CHECK(vec_lerp(&out, NULL, &v, 0.5f) == E_INVALID);
	CHECK(mat_lerp(&mo, &m, NULL, 0.5f) == E_INVALID);
	CHECK(vec_dot(NULL, &v, &f) == E_INVALID && vec_dot(&v, &v, NULL) == E_INVALID);
	CHECK(vec_norm(NULL) != vec_norm(NULL) && vec_norm_1(NULL) != vec_norm_1(NULL));
	CHECK(angle_cos(&v, &v, NULL) == E_INVALID && angle_cos(NULL, &v, &f) == E_INVALID);
	CHECK(cross_product(&out, NULL, &v) == E_INVALID);
	CHECK(mat_mul_vec(&out, NULL, &v) == E_INVALID);
	CHECK(mat_mul_mat(&mo, &m, NULL) == E_INVALID);
	CHECK(mat_trace(NULL, &f) == E_INVALID && mat_trace(&m, NULL) == E_INVALID);
	CHECK(mat_transpose(&mo, NULL) == E_INVALID);
	CHECK(mat_conjugate_transpose(&mo, NULL) == E_INVALID);
	CHECK(mat_row_echelon(&mo, NULL) == E_INVALID);
	CHECK(mat_determinant(NULL, &f) == E_INVALID && mat_determinant(&m, NULL) == E_INVALID);
	CHECK(mat_inverse(&mo, NULL) == E_INVALID);
	CHECK(mat_rank(NULL, &rank) == E_INVALID && mat_rank(&m, NULL) == E_INVALID);
	CHECK(vec_to_string(NULL) == NULL && mat_to_string(NULL) == NULL);
	CHECK(column_major_text(NULL) == NULL);
	CHECK(vec_new(NULL, &g_real, 1) == E_INVALID && vec_new(&out, NULL, 1) == E_INVALID);
	CHECK(vec_from_f32(&out, 3, NULL) == E_INVALID && out.data == NULL);
	CHECK(mat_from_f32(&mo, 2, 2, NULL) == E_INVALID && mo.data == NULL);
	CHECK(vec_at(NULL, 0) == NULL && mat_at(NULL, 0, 0) == NULL);
	CHECK(vec_at(&v, 3) == NULL && mat_at(&m, 3, 0) == NULL && mat_at(&m, 0, 3) == NULL);
	CHECK(!vec_equal(NULL, &v) && !mat_equal(&m, NULL));
	vec_free(NULL);
	mat_free(NULL);
	vec_print(NULL);
	mat_print(NULL);
	free_all();
}

static void	test_shape_errors(void)
{
	t_vec	v2;
	t_vec	v3;
	t_vec	out;
	t_mat	m22;
	t_mat	m23;
	t_mat	mo;
	float	f;
	size_t	rank;
	t_vec	pair[2];
	float	co[2] = {1.f, 1.f};

	v2 = VF(1., 2.);
	v3 = VF(1., 2., 3.);
	m22 = MF(2, 2, 1., 2., 3., 4.);
	m23 = MF(2, 3, 1., 2., 3., 4., 5., 6.);
	/* the operands are left untouched by a failed operation */
	CHECK(vec_add(&v2, &v3) == E_SHAPE && vec_sub(&v2, &v3) == E_SHAPE);
	CHECK(mat_add(&m22, &m23) == E_SHAPE && mat_sub(&m22, &m23) == E_SHAPE);
	check_vstr(&v2, "[1.0]\n[2.0]");
	check_mstr(&m22, "[1.0, 2.0]\n[3.0, 4.0]");
	pair[0] = v2;
	pair[1] = v3;
	CHECK(vec_linear_combination(&out, pair, 2, co) == E_SHAPE && out.data == NULL);
	CHECK(vec_lerp(&out, &v2, &v3, 0.5f) == E_SHAPE && out.data == NULL);
	CHECK(mat_lerp(&mo, &m22, &m23, 0.5f) == E_SHAPE && mo.data == NULL);
	CHECK(vec_dot(&v2, &v3, &f) == E_SHAPE);
	CHECK(angle_cos(&v2, &v3, &f) == E_SHAPE);
	CHECK(cross_product(&out, &v2, &v2) == E_SHAPE && out.data == NULL);
	CHECK(cross_product(&out, &v3, &v2) == E_SHAPE);
	CHECK(mat_mul_vec(&out, &m22, &v3) == E_SHAPE && out.data == NULL);
	CHECK(mat_mul_mat(&mo, &m22, &m22) == E_OK);
	reg(mo.data);
	CHECK(mat_mul_mat(&mo, &m22, &m23) == E_OK);
	reg(mo.data);
	CHECK(mat_mul_mat(&mo, &m23, &m22) == E_SHAPE && mo.data == NULL);
	CHECK(mat_trace(&m23, &f) == E_NOT_SQUARE);
	CHECK(mat_determinant(&m23, &f) == E_NOT_SQUARE);
	CHECK(mat_inverse(&mo, &m23) == E_NOT_SQUARE && mo.data == NULL);
	CHECK(mat_rank(&m23, &rank) == E_OK && rank == 2);	/* any shape has a rank */
	/* real and complex data never mix */
	{
		t_vec	c;

		vec_new(&c, &g_complex, 2);
		reg(c.data);
		CHECK(vec_add(&v2, &c) == E_SHAPE);
		CHECK(vec_dot(&v2, &c, &f) == E_SHAPE);
	}
	CHECK(vec_reshape(&mo, &v3, 2, 2) == E_SHAPE && mo.data == NULL);
	CHECK(vec_reshape(&mo, &v3, 3, 1) == E_OK);
	reg(mo.data);
	CHECK(vec_reshape(&mo, &v3, (size_t)-1, 2) == E_SHAPE);
	free_all();
}

static void	test_empty_and_degenerate(void)
{
	t_vec	ev;
	t_vec	out;
	t_mat	em;
	t_mat	mo;
	t_mat	tall;
	float	f;
	size_t	rank;
	char	*s;

	CHECK(vec_new(&ev, &g_real, 0) == E_OK);
	reg(ev.data);
	CHECK(mat_new(&em, &g_real, 0, 0) == E_OK);
	reg(em.data);
	CHECK(vec_norm_1(&ev) + vec_norm(&ev) + vec_norm_inf(&ev) == 0.0f);
	CHECK(vec_dot(&ev, &ev, &f) == E_OK && f == 0.0f);
	CHECK(vec_lerp(&out, &ev, &ev, 0.5f) == E_OK && out.n == 0);
	reg(out.data);
	CHECK(vec_add(&ev, &ev) == E_OK);
	CHECK(angle_cos(&ev, &ev, &f) == E_INVALID);
	CHECK(mat_trace(&em, &f) == E_OK && f == 0.0f);
	CHECK(mat_determinant(&em, &f) == E_OK && f == 1.0f);
	CHECK(mat_rank(&em, &rank) == E_OK && rank == 0);
	CHECK(mat_transpose(&mo, &em) == E_OK && mo.rows == 0);
	reg(mo.data);
	CHECK(mat_row_echelon(&mo, &em) == E_OK);
	reg(mo.data);
	CHECK(mat_inverse(&mo, &em) == E_OK && mo.rows == 0);
	reg(mo.data);
	s = vec_to_string(&ev);
	CHECK(s && s[0] == '\0');
	free(s);
	s = mat_to_string(&em);
	CHECK(s && s[0] == '\0');
	free(s);
	CHECK(mat_new(&tall, &g_real, 3, 0) == E_OK);
	reg(tall.data);
	s = mat_to_string(&tall);
	CHECK(s && strcmp(s, "[]\n[]\n[]") == 0);
	free(s);
	CHECK(mat_transpose(&mo, &tall) == E_OK && mo.rows == 0 && mo.cols == 3);
	reg(mo.data);
	CHECK(mat_rank(&tall, &rank) == E_OK && rank == 0);
	/* 2x0 times 0x3 is a 2x3 matrix of zeros */
	{
		t_mat	a;
		t_mat	b;

		mat_new(&a, &g_real, 2, 0);
		mat_new(&b, &g_real, 0, 3);
		reg(a.data);
		reg(b.data);
		CHECK(mat_mul_mat(&mo, &a, &b) == E_OK && mo.rows == 2 && mo.cols == 3);
		reg(mo.data);
		check_mstr(&mo, "[0.0, 0.0, 0.0]\n[0.0, 0.0, 0.0]");
	}
	/* 1x1 and zero matrices */
	{
		t_mat	one = MF(1, 1, 4.);
		t_mat	zero = MF(2, 2, 0., 0., 0., 0.);

		CHECK(mat_inverse(&mo, &one) == E_OK);
		reg(mo.data);
		check_mstr(&mo, "[0.25]");
		CHECK(mat_inverse(&mo, &zero) == E_SINGULAR && mo.data == NULL);
		CHECK(mat_rank(&zero, &rank) == E_OK && rank == 0);
		CHECK(mat_determinant(&zero, &f) == E_OK && f == 0.0f);
		CHECK(mat_row_echelon(&mo, &zero) == E_OK);
		reg(mo.data);
		CHECK(mat_equal(&mo, &zero));
	}
	free_all();
}

static void	test_size_overflow(void)
{
	t_mat	m;
	t_vec	v;

	CHECK(mat_new(&m, &g_real, (size_t)-1, 2) == E_NOMEM && m.data == NULL);
	CHECK(mat_new(&m, &g_real, (size_t)-1, (size_t)-1) == E_NOMEM);
	CHECK(mat_new(&m, &g_real, (size_t)1 << 62, 1) == E_NOMEM);
	CHECK(vec_new(&v, &g_real, (size_t)-1) == E_NOMEM && v.data == NULL);
	CHECK(vec_new(&v, &g_complex, (size_t)-1 / 4) == E_NOMEM);
	vec_free(&v);
	vec_free(&v);	/* freeing twice is harmless */
	mat_free(&m);
	mat_free(&m);
}

static void	test_nan_and_infinity(void)
{
	t_mat	m;
	t_mat	out;
	t_vec	v;
	t_vec	vo;
	float	f;
	size_t	rank;
	float	nan_ = NAN;
	float	inf = INFINITY;

	m = MF(2, 2, nan_, 1., 2., inf);
	CHECK(mat_inverse(&out, &m) == E_INVALID && out.data == NULL);
	mat_determinant(&m, &f);
	CHECK(mat_rank(&m, &rank) == E_OK);
	CHECK(mat_row_echelon(&out, &m) == E_OK);
	reg(out.data);
	CHECK(mat_transpose(&out, &m) == E_OK);
	reg(out.data);
	CHECK(mat_trace(&m, &f) == E_OK);
	v = VF(nan_, inf, -inf, 1.);
	CHECK(vec_norm(&v) != vec_norm(&v));
	CHECK(vec_norm_1(&v) != vec_norm_1(&v) && vec_norm_inf(&v) != vec_norm_inf(&v));
	CHECK(vec_dot(&v, &v, &f) == E_OK);
	CHECK(vec_lerp(&vo, &v, &v, nan_) == E_OK);
	reg(vo.data);
	(void)angle_cos(&v, &v, &f);
	v = VF(inf, inf);
	CHECK(vec_norm(&v) == INFINITY);
	v = VF(nan_);
	CHECK(vec_norm(&v) != vec_norm(&v));
	{
		float	mx = 3.4028235e38f;
		t_mat	big = MF(2, 2, mx, mx, mx, mx);

		mat_determinant(&big, &f);
		mat_row_echelon(&out, &big);
		reg(out.data);
		CHECK(mat_mul_mat(&out, &big, &big) == E_OK);
		reg(out.data);
	}
	free_all();
}

static void	test_extreme_magnitudes(void)
{
	t_vec	v;
	t_vec	w;
	t_mat	m;
	t_mat	inv;
	float	c;
	size_t	rank;

	v = VF(1e-25f, 1e-25f);
	CHECK(near_f(vec_norm(&v) / 1.4142135e-25f, 1.0f, 1e-5f));
	v = VF(1e25f, 1e25f);
	CHECK(near_f(vec_norm(&v) / 1.4142135e25f, 1.0f, 1e-5f));
	v = VF(1.8e38f, 2.4e38f);	/* the squares overflow, the norm (3e38) does not */
	CHECK(near_f(vec_norm(&v) / 3e38f, 1.0f, 1e-5f));
	v = VF(1e30f, 1e30f);
	w = VF(2e30f, 2e30f);
	CHECK(angle_cos(&v, &w, &c) == E_OK && near_f(c, 1.0f, 1e-5f));
	m = MF(2, 2, 1e-30f, 0., 0., 1e-30f);
	CHECK(mat_inverse(&inv, &m) == E_OK);
	reg(inv.data);
	CHECK(near_f(((float *)inv.data)[0] / 1e30f, 1.0f, 1e-5f));
	m = MF(2, 2, 1e-30f, 2e-30f, 3e-30f, 4e-30f);
	CHECK(mat_rank(&m, &rank) == E_OK && rank == 2);
	m = MF(2, 2, 1e30f, 2e30f, 2e30f, 4e30f);
	CHECK(mat_rank(&m, &rank) == E_OK && rank == 1);
	m = MF(2, 2, 1e8f, 0., 0., 1.);	/* different scales, still full rank */
	CHECK(mat_rank(&m, &rank) == E_OK && rank == 2);
	m = MF(3, 3, 1., 2., 3., 4., 5., 6., 7., 8., 9.);	/* noise is not rank */
	CHECK(mat_rank(&m, &rank) == E_OK && rank == 2);
	CHECK(mat_inverse(&inv, &m) == E_SINGULAR);
	m = MF(3, 3, .1f, .2f, .3f, .4f, .5f, .6f, .7f, .8f, .9f);
	CHECK(mat_rank(&m, &rank) == E_OK && rank == 2);
	free_all();
}

static void	test_misc_properties(void)
{
	t_vec	z;
	t_vec	u;
	t_vec	r;
	t_mat	m;
	float	neg = -1.0f;

	/* negative zero never leaks into the output */
	z = VF(1., 0., 0.);
	CHECK(cross_product(&r, &z, &z) == E_OK);
	reg(r.data);
	check_vstr(&r, "[0.0]\n[0.0]\n[0.0]");
	u = VF(0.);
	vec_scl(&u, &neg);
	check_vstr(&u, "[0.0]");
	/* cancellation: a*a - c*1 where a*a = 1 + 2^-12 + 2^-26, c = 1 + 2^-12 */
	{
		float	a = 1.0f + 1.0f / 8192.0f;
		float	c = 1.0f + 1.0f / 4096.0f;
		float	one = 1.0f;
		float	res;

		CHECK(a * a - c * one == 0.0f);	/* the plain formula loses it */
		diff_of_products(&g_real, &res, &a, &a, &c, &one);
		CHECK(res == 1.0f / 67108864.0f);
	}
	/* reshape / flatten / column-major utilities */
	u = VF(1., 2., 3., 4.);
	CHECK(vec_reshape(&m, &u, 2, 2) == E_OK);
	reg(m.data);
	check_mstr(&m, "[1.0, 2.0]\n[3.0, 4.0]");
	CHECK(mat_to_vec(&r, &m) == E_OK);
	reg(r.data);
	CHECK(vec_equal(&r, &u));
	CHECK(mat_to_column_major(&r, &m) == E_OK);
	reg(r.data);
	check_vstr(&r, "[1.0]\n[3.0]\n[2.0]\n[4.0]");
	free_all();
}

int	main(void)
{
	test_format();
	test_subject_00_to_03();
	test_subject_04_to_09();
	test_subject_10_to_14();
	test_projection();
	test_real_laws();
	test_vector_laws();
	test_cross_laws();
	test_complex();
	test_null_pointers();
	test_shape_errors();
	test_empty_and_degenerate();
	test_size_overflow();
	test_nan_and_infinity();
	test_extreme_magnitudes();
	test_misc_properties();
	printf("%d/%d checks passed\n", g_total - g_fail, g_total);
	return (g_fail != 0);
}

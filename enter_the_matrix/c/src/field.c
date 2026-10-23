/*
** The scalar fields K, error messages and small utilities.
**
** g_real is the field of the reals, represented as float (f32 in the subject).
** g_complex is the bonus of exercise 15: a pair of floats with the usual
** field operations. The generic code of the other files only ever talks to a
** t_field, so it works for both unchanged.
**
** Mathematical functions: the subject forbids the math library except, per
** exercise, the fused multiply-add (fmaf), pow (powf) and tan (tanf). The
** absolute value is a comparison here, and the square root is powf(x, 0.5).
**
** About the complex operations: a complex product is four real products, so
** there is no single fused operation for it. fma on complex is the plain
** a * b + c on purpose: the two products that must cancel in conj(z) * z are
** then rounded identically, so the imaginary part of <v|v> is exactly 0.
** (Build with -ffp-contract=off so the compiler does not fuse them either.)
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

const char	*err_str(t_err e)
{
	if (e == E_OK)
		return ("ok");
	if (e == E_SHAPE)
		return ("incompatible shapes");
	if (e == E_NOT_SQUARE)
		return ("the matrix must be square");
	if (e == E_EMPTY)
		return ("empty input");
	if (e == E_SINGULAR)
		return ("the matrix is singular (it has no inverse)");
	if (e == E_INVALID)
		return ("invalid argument");
	return ("out of memory or size overflow");
}

/* ---------------------------------------------------------------------------
** Small utilities
** ------------------------------------------------------------------------ */

bool	fin_f32(float x)
{
	return (x == x && x - x == 0.0f);
}

float	sqrt_f32(float x)
{
	if (x < 0.0f)
		return (NAN);
	return (powf(x, 0.5f));
}

bool	mul_overflows(size_t a, size_t b, size_t *r)
{
	if (a != 0 && b > (size_t)-1 / a)
		return (true);
	*r = a * b;
	return (false);
}

void	*alloc_scalars(const t_field *f, size_t n)
{
	size_t	bytes;

	if (!f || mul_overflows(n, f->size, &bytes))
		return (NULL);
	return (calloc(bytes ? bytes : 1, 1));
}

/* ---------------------------------------------------------------------------
** Float formatting: shortest text that reads back as the same float, written
** like Rust's {:?} (1.0, 0.5, 27.3, 1e20, 1.5e-7).
** ------------------------------------------------------------------------ */

static int	put(char *buf, size_t cap, const char *s)
{
	return (snprintf(buf, cap, "%s", s));
}

/* digits (no dot) and decimal exponent of the shortest representation */
static void	shortest(float x, char *digits, int *exp10)
{
	char	tmp[48];
	int		p;
	int		i;
	int		n;

	for (p = 0; p < 9; p++)
	{
		snprintf(tmp, sizeof(tmp), "%.*e", p, (double)x);
		if (strtof(tmp, NULL) == x)
			break ;
	}
	n = 0;
	for (i = 0; tmp[i] && tmp[i] != 'e'; i++)
		if (tmp[i] >= '0' && tmp[i] <= '9')
			digits[n++] = tmp[i];
	digits[n] = '\0';
	*exp10 = atoi(strchr(tmp, 'e') + 1);
}

static void	positional(char *out, const char *d, int e)
{
	int	nd;
	int	i;

	nd = (int)strlen(d);
	if (e >= 0)
	{
		for (i = 0; i <= e; i++)
			*out++ = i < nd ? d[i] : '0';
		*out++ = '.';
		if (nd > e + 1)
			for (i = e + 1; i < nd; i++)
				*out++ = d[i];
		else
			*out++ = '0';
	}
	else
	{
		*out++ = '0';
		*out++ = '.';
		for (i = 0; i < -e - 1; i++)
			*out++ = '0';
		for (i = 0; i < nd; i++)
			*out++ = d[i];
	}
	*out = '\0';
}

int	fmt_f32(char *buf, size_t cap, float x)
{
	char	d[16];
	char	body[96];
	char	full[112];
	int		e;
	float	ax;

	if (x != x)
		return (put(buf, cap, "NaN"));
	if (!fin_f32(x))
		return (put(buf, cap, x < 0.0f ? "-inf" : "inf"));
	if (x == 0.0f)
		return (put(buf, cap, "0.0"));
	shortest(x, d, &e);
	ax = x < 0.0f ? -x : x;
	if (ax >= 1e-4f && ax < 1e16f)
		positional(body, d, e);
	else if (d[1])
		snprintf(body, sizeof(body), "%c.%se%d", d[0], d + 1, e);
	else
		snprintf(body, sizeof(body), "%ce%d", d[0], e);
	snprintf(full, sizeof(full), "%s%s", x < 0.0f ? "-" : "", body);
	return (put(buf, cap, full));
}

/* ---------------------------------------------------------------------------
** The real field
** ------------------------------------------------------------------------ */

# define RF(p) (*(const float *)(p))
# define RW(p) (*(float *)(p))

static void	r_zero(void *r)
{
	RW(r) = 0.0f;
}

static void	r_one(void *r)
{
	RW(r) = 1.0f;
}

static void	r_from(void *r, float x)
{
	RW(r) = x;
}

static void	r_add(void *r, const void *a, const void *b)
{
	RW(r) = RF(a) + RF(b);
}

static void	r_sub(void *r, const void *a, const void *b)
{
	RW(r) = RF(a) - RF(b);
}

static void	r_mul(void *r, const void *a, const void *b)
{
	RW(r) = RF(a) * RF(b);
}

static void	r_div(void *r, const void *a, const void *b)
{
	RW(r) = RF(a) / RF(b);
}

static void	r_neg(void *r, const void *a)
{
	RW(r) = -RF(a);
}

static void	r_fma(void *r, const void *a, const void *b, const void *c)
{
	RW(r) = fmaf(RF(a), RF(b), RF(c));
}

static void	r_conj(void *r, const void *a)
{
	RW(r) = RF(a);
}

static float	r_modulus(const void *a)
{
	return (RF(a) < 0.0f ? -RF(a) : RF(a));
}

static float	r_re(const void *a)
{
	return (RF(a));
}

static bool	r_is_finite(const void *a)
{
	return (fin_f32(RF(a)));
}

static bool	r_is_zero(const void *a)
{
	return (RF(a) == 0.0f);
}

static bool	r_eq(const void *a, const void *b)
{
	return (RF(a) == RF(b));
}

static int	r_format(char *buf, size_t cap, const void *a)
{
	return (fmt_f32(buf, cap, RF(a)));
}

const t_field	g_real = {"real", sizeof(float), r_zero, r_one, r_from, r_add,
	r_sub, r_mul, r_div, r_neg, r_fma, r_conj, r_modulus, r_re, r_is_finite,
	r_is_zero, r_eq, r_format};

/* ---------------------------------------------------------------------------
** The complex field (exercise 15)
** ------------------------------------------------------------------------ */

# define CF(p) (*(const t_complex *)(p))
# define CW(p) (*(t_complex *)(p))

static void	c_zero(void *r)
{
	CW(r).re = 0.0f;
	CW(r).im = 0.0f;
}

static void	c_one(void *r)
{
	CW(r).re = 1.0f;
	CW(r).im = 0.0f;
}

static void	c_from(void *r, float x)
{
	CW(r).re = x;
	CW(r).im = 0.0f;
}

static void	c_add(void *r, const void *a, const void *b)
{
	t_complex	x = CF(a);
	t_complex	y = CF(b);

	CW(r).re = x.re + y.re;
	CW(r).im = x.im + y.im;
}

static void	c_sub(void *r, const void *a, const void *b)
{
	t_complex	x = CF(a);
	t_complex	y = CF(b);

	CW(r).re = x.re - y.re;
	CW(r).im = x.im - y.im;
}

static void	c_mul(void *r, const void *a, const void *b)
{
	t_complex	x = CF(a);
	t_complex	y = CF(b);
	t_complex	z;

	z.re = x.re * y.re - x.im * y.im;
	z.im = x.re * y.im + x.im * y.re;
	CW(r) = z;
}

/* Smith's algorithm: no overflow when the squared modulus would overflow. */
static void	c_div(void *r, const void *a, const void *b)
{
	t_complex	x = CF(a);
	t_complex	y = CF(b);
	t_complex	z;
	float		q;
	float		den;

	if ((y.im < 0.0f ? -y.im : y.im) <= (y.re < 0.0f ? -y.re : y.re))
	{
		q = y.im / y.re;
		den = y.re + y.im * q;
		z.re = (x.re + x.im * q) / den;
		z.im = (x.im - x.re * q) / den;
	}
	else
	{
		q = y.re / y.im;
		den = y.re * q + y.im;
		z.re = (x.re * q + x.im) / den;
		z.im = (x.im * q - x.re) / den;
	}
	CW(r) = z;
}

static void	c_neg(void *r, const void *a)
{
	t_complex	x = CF(a);

	CW(r).re = -x.re;
	CW(r).im = -x.im;
}

static void	c_fma(void *r, const void *a, const void *b, const void *c)
{
	t_complex	p;
	t_complex	z = CF(c);

	c_mul(&p, a, b);
	CW(r).re = p.re + z.re;
	CW(r).im = p.im + z.im;
}

static void	c_conj(void *r, const void *a)
{
	t_complex	x = CF(a);

	CW(r).re = x.re;
	CW(r).im = -x.im;
}

static float	c_modulus(const void *p)
{
	t_complex	z = CF(p);
	float		a = z.re < 0.0f ? -z.re : z.re;
	float		b = z.im < 0.0f ? -z.im : z.im;
	float		m;
	float		x;
	float		y;

	if (a != a || b != b)
		return (NAN);
	m = a > b ? a : b;
	if (m == 0.0f || !fin_f32(m))
		return (m);
	x = a / m;
	y = b / m;
	return (m * sqrt_f32(fmaf(x, x, y * y)));
}

static float	c_re(const void *a)
{
	return (CF(a).re);
}

static bool	c_is_finite(const void *a)
{
	return (fin_f32(CF(a).re) && fin_f32(CF(a).im));
}

static bool	c_is_zero(const void *a)
{
	return (CF(a).re == 0.0f && CF(a).im == 0.0f);
}

static bool	c_eq(const void *a, const void *b)
{
	return (CF(a).re == CF(b).re && CF(a).im == CF(b).im);
}

static int	c_format(char *buf, size_t cap, const void *a)
{
	char	re[48];
	char	im[48];
	float	y;

	y = CF(a).im;
	fmt_f32(re, sizeof(re), CF(a).re);
	fmt_f32(im, sizeof(im), y < 0.0f ? -y : y);
	return (snprintf(buf, cap, "%s%c%si", re, y < 0.0f ? '-' : '+', im));
}

const t_field	g_complex = {"complex", sizeof(t_complex), c_zero, c_one, c_from,
	c_add, c_sub, c_mul, c_div, c_neg, c_fma, c_conj, c_modulus, c_re,
	c_is_finite, c_is_zero, c_eq, c_format};

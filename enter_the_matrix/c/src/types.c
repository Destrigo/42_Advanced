/*
** Vector and Matrix: a store of data plus the utility functions the subject
** recommends (size, squareness, reshape, printing). The mathematical
** operations live in the file of the exercise that introduces them.
**
** A vector holds n scalars; a matrix holds rows x cols scalars stored row by
** row. Both own their buffer: free them with vec_free / mat_free. A failed
** constructor leaves its out parameter zeroed (data == NULL), which is safe to
** free.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

/* ---------------------------------------------------------------------------
** Vectors
** ------------------------------------------------------------------------ */

t_err	vec_new(t_vec *out, const t_field *f, size_t n)
{
	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!f || f->size > K_MAX)
		return (E_INVALID);
	out->data = alloc_scalars(f, n);
	if (!out->data)
		return (E_NOMEM);
	out->f = f;
	out->n = n;
	return (E_OK);
}

t_err	vec_from_f32(t_vec *out, size_t n, const float *vals)
{
	t_err	e;

	e = vec_new(out, &g_real, n);
	if (e != E_OK)
		return (e);
	if (n && !vals)
	{
		vec_free(out);
		return (E_INVALID);
	}
	if (n)
		memcpy(out->data, vals, n * sizeof(float));
	return (E_OK);
}

t_err	vec_from_complex(t_vec *out, size_t n, const t_complex *vals)
{
	t_err	e;

	e = vec_new(out, &g_complex, n);
	if (e != E_OK)
		return (e);
	if (n && !vals)
	{
		vec_free(out);
		return (E_INVALID);
	}
	if (n)
		memcpy(out->data, vals, n * sizeof(t_complex));
	return (E_OK);
}

t_err	vec_clone(t_vec *out, const t_vec *v)
{
	t_err	e;

	if (!out)
		return (E_INVALID);
	if (!v || !v->f)
		return (memset(out, 0, sizeof(*out)), E_INVALID);
	e = vec_new(out, v->f, v->n);
	if (e == E_OK && v->n)
		memcpy(out->data, v->data, v->n * v->f->size);
	return (e);
}

void	vec_free(t_vec *v)
{
	if (!v)
		return ;
	free(v->data);
	memset(v, 0, sizeof(*v));
}

void	*vec_at(const t_vec *v, size_t i)
{
	if (!v || !v->f || i >= v->n)
		return (NULL);
	return (EL(v->f, v->data, i));
}

bool	vec_equal(const t_vec *a, const t_vec *b)
{
	size_t	i;

	if (!a || !b || a->f != b->f || a->n != b->n)
		return (false);
	for (i = 0; i < a->n; i++)
		if (!a->f->eq(CEL(a->f, a->data, i), CEL(b->f, b->data, i)))
			return (false);
	return (true);
}

/* ---------------------------------------------------------------------------
** Matrices
** ------------------------------------------------------------------------ */

t_err	mat_new(t_mat *out, const t_field *f, size_t rows, size_t cols)
{
	size_t	n;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!f || f->size > K_MAX)
		return (E_INVALID);
	if (mul_overflows(rows, cols, &n))
		return (E_NOMEM);
	out->data = alloc_scalars(f, n);
	if (!out->data)
		return (E_NOMEM);
	out->f = f;
	out->rows = rows;
	out->cols = cols;
	return (E_OK);
}

t_err	mat_from_f32(t_mat *out, size_t rows, size_t cols, const float *vals)
{
	t_err	e;

	e = mat_new(out, &g_real, rows, cols);
	if (e != E_OK)
		return (e);
	if (rows * cols != 0 && !vals)
	{
		mat_free(out);
		return (E_INVALID);
	}
	if (rows * cols != 0)
		memcpy(out->data, vals, rows * cols * sizeof(float));
	return (E_OK);
}

t_err	mat_from_complex(t_mat *out, size_t rows, size_t cols,
			const t_complex *vals)
{
	t_err	e;

	e = mat_new(out, &g_complex, rows, cols);
	if (e != E_OK)
		return (e);
	if (rows * cols != 0 && !vals)
	{
		mat_free(out);
		return (E_INVALID);
	}
	if (rows * cols != 0)
		memcpy(out->data, vals, rows * cols * sizeof(t_complex));
	return (E_OK);
}

t_err	mat_clone(t_mat *out, const t_mat *m)
{
	t_err	e;

	if (!out)
		return (E_INVALID);
	if (!m || !m->f)
		return (memset(out, 0, sizeof(*out)), E_INVALID);
	e = mat_new(out, m->f, m->rows, m->cols);
	if (e == E_OK && m->rows * m->cols != 0)
		memcpy(out->data, m->data, m->rows * m->cols * m->f->size);
	return (e);
}

void	mat_free(t_mat *m)
{
	if (!m)
		return ;
	free(m->data);
	memset(m, 0, sizeof(*m));
}

void	*mat_at(const t_mat *m, size_t i, size_t j)
{
	if (!m || !m->f || i >= m->rows || j >= m->cols)
		return (NULL);
	return (EL(m->f, m->data, i * m->cols + j));
}

bool	mat_is_square(const t_mat *m)
{
	return (m && m->rows == m->cols);
}

bool	mat_equal(const t_mat *a, const t_mat *b)
{
	size_t	i;

	if (!a || !b || a->f != b->f || a->rows != b->rows || a->cols != b->cols)
		return (false);
	for (i = 0; i < a->rows * a->cols; i++)
		if (!a->f->eq(CEL(a->f, a->data, i), CEL(b->f, b->data, i)))
			return (false);
	return (true);
}

/* ---------------------------------------------------------------------------
** Reshape, flatten, column-major
** ------------------------------------------------------------------------ */

t_err	vec_reshape(t_mat *out, const t_vec *v, size_t rows, size_t cols)
{
	size_t	n;
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!v || !v->f)
		return (E_INVALID);
	if (mul_overflows(rows, cols, &n) || n != v->n)
		return (E_SHAPE);
	e = mat_new(out, v->f, rows, cols);
	if (e == E_OK && n)
		memcpy(out->data, v->data, n * v->f->size);
	return (e);
}

t_err	mat_to_vec(t_vec *out, const t_mat *m)
{
	t_err	e;

	if (!out)
		return (E_INVALID);
	if (!m || !m->f)
		return (memset(out, 0, sizeof(*out)), E_INVALID);
	e = vec_new(out, m->f, m->rows * m->cols);
	if (e == E_OK && m->rows * m->cols != 0)
		memcpy(out->data, m->data, m->rows * m->cols * m->f->size);
	return (e);
}

/* The content column by column, the layout OpenGL / the display program use. */
t_err	mat_to_column_major(t_vec *out, const t_mat *m)
{
	size_t	i;
	size_t	j;
	t_err	e;

	e = mat_to_vec(out, m);
	if (e != E_OK)
		return (e);
	for (j = 0; j < m->cols; j++)
		for (i = 0; i < m->rows; i++)
			memcpy(EL(m->f, out->data, j * m->rows + i),
				CEL(m->f, m->data, i * m->cols + j), m->f->size);
	return (E_OK);
}

/* ---------------------------------------------------------------------------
** Text. -0.0 is printed as 0.0 (the formats of the fields already do it).
** ------------------------------------------------------------------------ */

typedef struct s_str
{
	char	*s;
	size_t	len;
	size_t	cap;
	bool	ok;
}	t_str;

static void	str_add(t_str *b, const char *text)
{
	size_t	n;
	char	*bigger;

	if (!b->ok)
		return ;
	n = strlen(text);
	if (b->len + n + 1 > b->cap)
	{
		while (b->len + n + 1 > b->cap)
			b->cap = b->cap ? b->cap * 2 : 64;
		bigger = realloc(b->s, b->cap);
		if (!bigger)
		{
			b->ok = false;
			return ;
		}
		b->s = bigger;
	}
	memcpy(b->s + b->len, text, n + 1);
	b->len += n;
}

static void	str_scalar(t_str *b, const t_field *f, const void *p)
{
	char	tmp[128];

	f->format(tmp, sizeof(tmp), p);
	str_add(b, tmp);
}

static char	*str_done(t_str *b)
{
	if (!b->ok)
	{
		free(b->s);
		return (NULL);
	}
	if (!b->s)
		str_add(b, "");
	return (b->s);
}

char	*vec_to_string(const t_vec *v)
{
	t_str	b;
	size_t	i;

	if (!v || !v->f)
		return (NULL);
	memset(&b, 0, sizeof(b));
	b.ok = true;
	for (i = 0; i < v->n; i++)
	{
		str_add(&b, i ? "\n[" : "[");
		str_scalar(&b, v->f, CEL(v->f, v->data, i));
		str_add(&b, "]");
	}
	return (str_done(&b));
}

char	*mat_to_string(const t_mat *m)
{
	t_str	b;
	size_t	i;
	size_t	j;

	if (!m || !m->f)
		return (NULL);
	memset(&b, 0, sizeof(b));
	b.ok = true;
	for (i = 0; i < m->rows; i++)
	{
		str_add(&b, i ? "\n[" : "[");
		for (j = 0; j < m->cols; j++)
		{
			if (j)
				str_add(&b, ", ");
			str_scalar(&b, m->f, CEL(m->f, m->data, i * m->cols + j));
		}
		str_add(&b, "]");
	}
	return (str_done(&b));
}

void	vec_print(const t_vec *v)
{
	char	*s;

	s = vec_to_string(v);
	puts(s ? s : "(null)");
	free(s);
}

void	mat_print(const t_mat *m)
{
	char	*s;

	s = mat_to_string(m);
	puts(s ? s : "(null)");
	free(s);
}

/*
** Exercise 00 - Add, Subtract and Scale
**
** The three basic operations of a vector space, for vectors and for matrices
** (matrices of a given shape are a vector space too):
**  - addition u + v, coordinate by coordinate (same size required);
**  - subtraction u - v;
**  - scaling a * u: every coordinate is multiplied by the scalar a.
**
** Like the prototypes of the subject, the functions work in place: self is
** the accumulator. No mathematical function is used, only + - *.
**
** Complexity: one pass over the coordinates, O(n) time and no extra memory.
**
** Sizes that do not match make no sense (undefined in the subject): the
** function returns an error and leaves self untouched. The scalar a points to
** a K of the same field as self (a float for a real vector).
*/

#include "internal.h"

void	buf_add(const t_field *f, void *d, const void *s, size_t n)
{
	size_t	i;

	for (i = 0; i < n; i++)
		f->add(EL(f, d, i), EL(f, d, i), CEL(f, s, i));
}

void	buf_sub(const t_field *f, void *d, const void *s, size_t n)
{
	size_t	i;

	for (i = 0; i < n; i++)
		f->sub(EL(f, d, i), EL(f, d, i), CEL(f, s, i));
}

void	buf_scl(const t_field *f, void *d, const void *a, size_t n)
{
	size_t	i;

	for (i = 0; i < n; i++)
		f->mul(EL(f, d, i), EL(f, d, i), a);
}

static t_err	check_vec(const t_vec *a, const t_vec *b)
{
	if (!a || !b || !a->f || !b->f || (a->n && !a->data) || (b->n && !b->data))
		return (E_INVALID);
	if (a->f != b->f || a->n != b->n)
		return (E_SHAPE);
	return (E_OK);
}

static t_err	check_mat(const t_mat *a, const t_mat *b)
{
	if (!a || !b || !a->f || !b->f)
		return (E_INVALID);
	if (a->f != b->f || a->rows != b->rows || a->cols != b->cols)
		return (E_SHAPE);
	return (E_OK);
}

t_err	vec_add(t_vec *self, const t_vec *v)
{
	t_err	e;

	e = check_vec(self, v);
	if (e == E_OK)
		buf_add(self->f, self->data, v->data, self->n);
	return (e);
}

t_err	vec_sub(t_vec *self, const t_vec *v)
{
	t_err	e;

	e = check_vec(self, v);
	if (e == E_OK)
		buf_sub(self->f, self->data, v->data, self->n);
	return (e);
}

t_err	vec_scl(t_vec *self, const void *a)
{
	if (!self || !self->f || !a)
		return (E_INVALID);
	buf_scl(self->f, self->data, a, self->n);
	return (E_OK);
}

t_err	mat_add(t_mat *self, const t_mat *v)
{
	t_err	e;

	e = check_mat(self, v);
	if (e == E_OK)
		buf_add(self->f, self->data, v->data, self->rows * self->cols);
	return (e);
}

t_err	mat_sub(t_mat *self, const t_mat *v)
{
	t_err	e;

	e = check_mat(self, v);
	if (e == E_OK)
		buf_sub(self->f, self->data, v->data, self->rows * self->cols);
	return (e);
}

t_err	mat_scl(t_mat *self, const void *a)
{
	if (!self || !self->f || !a)
		return (E_INVALID);
	buf_scl(self->f, self->data, a, self->rows * self->cols);
	return (E_OK);
}

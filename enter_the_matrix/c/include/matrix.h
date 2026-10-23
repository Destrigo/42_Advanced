#ifndef MATRIX_H
# define MATRIX_H

# include <stdbool.h>
# include <stddef.h>

/* ---------------------------------------------------------------------------
** Errors
**
** Every function that can fail returns a t_err (E_OK on success). When the
** subject says the behaviour is "undefined" (different sizes, non-square
** matrix...) the function returns an error and leaves its arguments untouched.
** The out parameters of a failed constructor-like function are zeroed, so
** vec_free / mat_free on them is always safe.
** ------------------------------------------------------------------------ */

typedef enum e_err
{
	E_OK = 0,
	E_SHAPE,		/* incompatible sizes / shapes (or different fields) */
	E_NOT_SQUARE,	/* the operation needs a square matrix */
	E_EMPTY,		/* the operation needs at least one element */
	E_SINGULAR,		/* the matrix has no inverse */
	E_INVALID,		/* any other nonsensical argument (NULL, NaN, ...) */
	E_NOMEM			/* allocation failed or size overflow */
}	t_err;

const char	*err_str(t_err e);

/* ---------------------------------------------------------------------------
** The field K of the scalars
**
** C has no generics, so the vector spaces are generic over a table of
** operations (what the Scalar trait is in the Rust version). Two fields are
** provided: g_real (K = float) and g_complex (K = t_complex, exercise 15).
** Scalars are handled through pointers (void *) to a K.
** ------------------------------------------------------------------------ */

typedef struct s_complex
{
	float	re;
	float	im;
}	t_complex;

typedef struct s_field
{
	const char	*name;
	size_t		size;	/* sizeof(K); the zero of K is all bits zero */
	void		(*zero)(void *r);
	void		(*one)(void *r);
	void		(*from_f32)(void *r, float x);
	void		(*add)(void *r, const void *a, const void *b);
	void		(*sub)(void *r, const void *a, const void *b);
	void		(*mul)(void *r, const void *a, const void *b);
	void		(*div)(void *r, const void *a, const void *b);
	void		(*neg)(void *r, const void *a);
	void		(*fma)(void *r, const void *a, const void *b, const void *c);
	void		(*conj)(void *r, const void *a);
	float		(*modulus)(const void *a);
	float		(*re)(const void *a);
	bool		(*is_finite)(const void *a);
	bool		(*is_zero)(const void *a);
	bool		(*eq)(const void *a, const void *b);
	int			(*format)(char *buf, size_t cap, const void *a);
}	t_field;

/* r = a * b + c with the fused multiply-add (for reals); r may alias c. */
extern const t_field	g_real;
extern const t_field	g_complex;

/* Shortest decimal text that reads back as x, in the style of the subject
** ("3.0", "0.5", "27.3", "1e20"). Returns the length that was written. */
int			fmt_f32(char *buf, size_t cap, float x);

/* ---------------------------------------------------------------------------
** Vector and Matrix: a store of data plus utility functions.
** The matrices are stored row by row, and the examples of the subject are
** written row by row too.
** ------------------------------------------------------------------------ */

typedef struct s_vec
{
	const t_field	*f;
	size_t			n;
	void			*data;
}	t_vec;

typedef struct s_mat
{
	const t_field	*f;
	size_t			rows;
	size_t			cols;
	void			*data;
}	t_mat;

/* constructors: zero-filled, or from C arrays (row by row for matrices) */
t_err		vec_new(t_vec *out, const t_field *f, size_t n);
t_err		vec_from_f32(t_vec *out, size_t n, const float *vals);
t_err		vec_from_complex(t_vec *out, size_t n, const t_complex *vals);
t_err		vec_clone(t_vec *out, const t_vec *v);
void		vec_free(t_vec *v);
void		*vec_at(const t_vec *v, size_t i);

t_err		mat_new(t_mat *out, const t_field *f, size_t rows, size_t cols);
t_err		mat_from_f32(t_mat *out, size_t rows, size_t cols, const float *vals);
t_err		mat_from_complex(t_mat *out, size_t rows, size_t cols,
				const t_complex *vals);
t_err		mat_clone(t_mat *out, const t_mat *m);
void		mat_free(t_mat *m);
void		*mat_at(const t_mat *m, size_t i, size_t j);
bool		mat_is_square(const t_mat *m);

/* equality of contents (same field, same shape, same values) */
bool		vec_equal(const t_vec *a, const t_vec *b);
bool		mat_equal(const t_mat *a, const t_mat *b);

/* utilities of the subject: reshape (row by row), flatten, column-major */
t_err		vec_reshape(t_mat *out, const t_vec *v, size_t rows, size_t cols);
t_err		mat_to_vec(t_vec *out, const t_mat *m);
t_err		mat_to_column_major(t_vec *out, const t_mat *m);

/* text: "[7.0]\n[10.0]" for a vector, "[8.0, 6.0]\n[1.0, 6.0]" for a matrix.
** The string is malloc'ed (NULL if it cannot be allocated). */
char		*vec_to_string(const t_vec *v);
char		*mat_to_string(const t_mat *m);
void		vec_print(const t_vec *v);
void		mat_print(const t_mat *m);

/* handy literals for real vectors / matrices (C99 variadic macros) */
# define VEC_F(out, ...) vec_from_f32((out), \
	sizeof((float[]){__VA_ARGS__}) / sizeof(float), (float[]){__VA_ARGS__})
# define MAT_F(out, rows, cols, ...) mat_from_f32((out), (rows), (cols), \
	(float[]){__VA_ARGS__})

/* ---------------------------------------------------------------------------
** Exercises
** ------------------------------------------------------------------------ */

/* ex00: in place, self is the accumulator */
t_err		vec_add(t_vec *self, const t_vec *v);
t_err		vec_sub(t_vec *self, const t_vec *v);
t_err		vec_scl(t_vec *self, const void *a);
t_err		mat_add(t_mat *self, const t_mat *v);
t_err		mat_sub(t_mat *self, const t_mat *v);
t_err		mat_scl(t_mat *self, const void *a);

/* ex01: u is an array of k vectors (or matrices), coefs an array of k scalars
** of the field of u (k floats for real vectors, k t_complex for complex ones:
** C cannot check the length of the array, that is up to the caller). */
t_err		vec_linear_combination(t_vec *out, const t_vec *u, size_t k,
				const void *coefs);
t_err		mat_linear_combination(t_mat *out, const t_mat *u, size_t k,
				const void *coefs);

/* ex02 */
float		lerp_f32(float u, float v, float t);
void		scalar_lerp(const t_field *f, void *out, const void *u,
				const void *v, float t);
t_err		vec_lerp(t_vec *out, const t_vec *u, const t_vec *v, float t);
t_err		mat_lerp(t_mat *out, const t_mat *u, const t_mat *v, float t);

/* ex03: out points to a K */
t_err		vec_dot(const t_vec *u, const t_vec *v, void *out);

/* ex04: norms are real numbers, whatever K is (NaN for a NULL vector) */
float		vec_norm_1(const t_vec *v);
float		vec_norm(const t_vec *v);
float		vec_norm_inf(const t_vec *v);

/* ex05 */
t_err		angle_cos(const t_vec *u, const t_vec *v, float *out);

/* ex06 */
t_err		cross_product(t_vec *out, const t_vec *u, const t_vec *v);

/* ex07 */
t_err		mat_mul_vec(t_vec *out, const t_mat *a, const t_vec *vec);
t_err		mat_mul_mat(t_mat *out, const t_mat *a, const t_mat *b);

/* ex08: out points to a K */
t_err		mat_trace(const t_mat *m, void *out);

/* ex09 */
t_err		mat_transpose(t_mat *out, const t_mat *m);
t_err		mat_conjugate_transpose(t_mat *out, const t_mat *m);

/* ex10: reduced row echelon form */
t_err		mat_row_echelon(t_mat *out, const t_mat *m);

/* ex11: out points to a K */
t_err		mat_determinant(const t_mat *m, void *out);

/* ex12: E_SINGULAR if the matrix has no inverse */
t_err		mat_inverse(t_mat *out, const t_mat *m);

/* ex13 */
t_err		mat_rank(const t_mat *m, size_t *out);

/* ex14: fov is the vertical field of view in radians; out is a 4x4 real
** matrix. column_major_text: one line per column, malloc'ed. */
t_err		projection(t_mat *out, float fov, float ratio, float near_plane,
				float far_plane);
char		*column_major_text(const t_mat *m);

#endif

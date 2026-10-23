#ifndef INTERNAL_H
# define INTERNAL_H

# include <stdint.h>
# include "matrix.h"

/* Scratch storage for one scalar of any field. */
# define K_MAX 16

typedef union u_k
{
	float		f;
	t_complex	c;
	char		raw[K_MAX];
}	t_k;

/* Pointer to the i-th scalar of a flat buffer. */
# define EL(f, base, i) ((char *)(base) + (i) * (f)->size)
# define CEL(f, base, i) ((const char *)(base) + (i) * (f)->size)

float	sqrt_f32(float x);		/* pow(x, 0.5): the only root allowed */
bool	fin_f32(float x);		/* neither NaN nor infinite */
bool	mul_overflows(size_t a, size_t b, size_t *r);
void	*alloc_scalars(const t_field *f, size_t n);	/* zeroed, NULL on error */

/* Operations on flat buffers of n scalars (shared by vectors and matrices). */
void	buf_add(const t_field *f, void *d, const void *s, size_t n);
void	buf_sub(const t_field *f, void *d, const void *s, size_t n);
void	buf_scl(const t_field *f, void *d, const void *a, size_t n);
void	buf_axpy(const t_field *f, void *d, const void *a, const void *s,
			size_t n);	/* d += a * s (fused) */

/* r = a*b - c*d, with the rounding error of c*d compensated (see ex06). */
void	diff_of_products(const t_field *f, void *r, const void *a, const void *b,
			const void *c, const void *d);

/* Reduced row echelon form of data (rows x cols, row by row), in place.
** Stores the rank in *rank. Plain + - * / only. See elim.c. */
t_err	rref_inplace(const t_field *f, void *data, size_t rows, size_t cols,
			size_t *rank);

#endif

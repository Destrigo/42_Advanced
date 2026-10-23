/*
** Exercise 08 - Trace
**
** The trace of a square matrix is the sum of its diagonal:
** Tr(A) = A_11 + A_22 + ... + A_nn. It is linear (Tr(A+B) = Tr(A)+Tr(B)),
** satisfies Tr(AB) = Tr(BA), does not change when the basis changes, and
** equals the sum of the eigenvalues.
**
** No mathematical function is allowed here, just additions.
**
** Complexity: O(n) time, O(1) extra memory. A matrix that is not square is
** undefined: E_NOT_SQUARE is returned. The result is written to *out, a K.
*/

#include <string.h>
#include "internal.h"

t_err	mat_trace(const t_mat *m, void *out)
{
	t_k		sum;
	size_t	i;

	if (!m || !m->f || !out)
		return (E_INVALID);
	if (!mat_is_square(m))
		return (E_NOT_SQUARE);
	m->f->zero(&sum);
	for (i = 0; i < m->rows; i++)
		m->f->add(&sum, &sum, CEL(m->f, m->data, i * m->cols + i));
	memcpy(out, &sum, m->f->size);
	return (E_OK);
}

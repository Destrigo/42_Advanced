/*
** Exercise 09 - Set evaluation
**
** Evaluate an RPN formula where every letter stands for a set of integers
** (A is the first set given, B the second, ...) and return the resulting set.
**
** Logic and set theory mirror each other, so each operator has a set
** counterpart. The universe is the union of all given sets:
**  - !a   complement:           universe \ a
**  - a&b  intersection
**  - a|b  union
**  - a^b  symmetric difference
**  - a>b  material condition:   !a | b
**  - a=b  equivalence:          (a & b) | (!a & !b)
**
** Idea: these operations are pointwise. An element u of the universe belongs
** to the result exactly when the boolean formula is true after replacing each
** letter with "u is in that set". So we build the (sorted, duplicate-free)
** universe and evaluate the ordinary boolean AST once per element.
**
** The number of distinct variables must equal the number of sets provided,
** and they must be A, B, C, ... without gaps. Otherwise an error is printed
** and false is returned.
*/

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

static int	cmp_int(const void *a, const void *b)
{
	int	x;
	int	y;

	x = *(const int *)a;
	y = *(const int *)b;
	return ((x > y) - (x < y));
}

static bool	contains(const int *set, size_t n, int v)
{
	size_t	i;

	i = 0;
	while (i < n)
		if (set[i++] == v)
			return (true);
	return (false);
}

/* Sorted, duplicate-free union of all the sets. */
static int	*universe(const int *const *sets, const size_t *sizes, size_t nsets,
				size_t *count)
{
	size_t	total;
	size_t	i;
	size_t	j;
	int		*u;

	total = 0;
	for (i = 0; i < nsets; i++)
		total += sizes[i];
	u = xmalloc((total + 1) * sizeof(int));
	total = 0;
	for (i = 0; i < nsets; i++)
		for (j = 0; j < sizes[i]; j++)
			u[total++] = sets[i][j];
	qsort(u, total, sizeof(int), cmp_int);
	*count = 0;
	for (i = 0; i < total; i++)
		if (*count == 0 || u[*count - 1] != u[i])
			u[(*count)++] = u[i];
	return (u);
}

bool	eval_set(const char *formula, const int *const *sets,
			const size_t *sizes, size_t nsets, int **out, size_t *out_size)
{
	t_node		*n;
	signed char	vals[26];
	int			*u;
	size_t		ucount;
	size_t		i;
	size_t		k;

	n = ast_parse(formula);
	if (!n)
		return (false);
	if (nsets > 26 || ast_vars(n) != (((uint32_t)1 << nsets) - 1))
	{
		fprintf(stderr, "Error: the formula needs exactly the variables A..%c "
			"for the %zu set(s) given\n", (char)('A' + (nsets ? nsets - 1 : 0)),
			nsets);
		ast_free(n);
		return (false);
	}
	u = universe(sets, sizes, nsets, &ucount);
	*out = u;
	*out_size = 0;
	for (i = 0; i < 26; i++)
		vals[i] = 0;
	for (i = 0; i < ucount; i++)
	{
		for (k = 0; k < nsets; k++)
			vals[k] = contains(sets[k], sizes[k], u[i]);
		if (ast_eval(n, vals) > 0)
			u[(*out_size)++] = u[i];
	}
	ast_free(n);
	return (true);
}

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
** universe and evaluate the ordinary boolean formula once per element, with
** the stack machine of rpn.c. Each input set is sorted once, so "u is in set
** k" is a binary search.
**
** The number of distinct variables must equal the number of sets provided,
** and they must be A, B, C, ... without gaps. Otherwise an error is printed
** and false is returned. Duplicates inside a set are ignored.
**
** Complexity: O(N log N) to sort the N input numbers, then one formula
** evaluation (plus one binary search per set) per distinct element.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

static int	cmp_int(const void *a, const void *b)
{
	int	x;
	int	y;

	x = *(const int *)a;
	y = *(const int *)b;
	return ((x > y) - (x < y));
}

/* Sorted copy of a set (duplicates removed). */
static int	*sorted_unique(const int *set, size_t n, size_t *count)
{
	int		*s;
	size_t	i;

	s = xmalloc((n + 1) * sizeof(int));
	if (n)
		memcpy(s, set, n * sizeof(int));
	qsort(s, n, sizeof(int), cmp_int);
	*count = 0;
	for (i = 0; i < n; i++)
		if (*count == 0 || s[*count - 1] != s[i])
			s[(*count)++] = s[i];
	return (s);
}

static bool	contains(const int *s, size_t n, int v)
{
	size_t	lo;
	size_t	hi;
	size_t	mid;

	lo = 0;
	hi = n;
	while (lo < hi)
	{
		mid = lo + (hi - lo) / 2;
		if (s[mid] == v)
			return (true);
		if (s[mid] < v)
			lo = mid + 1;
		else
			hi = mid;
	}
	return (false);
}

static bool	args_ok(const char *formula, const int *const *sets,
				const size_t *sizes, size_t nsets)
{
	size_t	i;

	if (!rpn_check(formula))
		return (false);
	if (nsets > 26 || rpn_vars(formula) != (((uint32_t)1 << nsets) - 1))
		return (fprintf(stderr, "Error: the formula needs exactly the "
				"variables A..%c for the %zu set(s) given\n",
				(char)('A' + (nsets ? nsets - 1 : 0)), nsets), false);
	if (nsets && (!sets || !sizes))
		return (fprintf(stderr, "Error: null sets\n"), false);
	for (i = 0; i < nsets; i++)
		if (sizes[i] && !sets[i])
			return (fprintf(stderr, "Error: null set %zu\n", i), false);
	return (true);
}

/* Evaluates the formula for every element of the universe u. */
static void	evaluate(const char *formula, int **s, size_t *sn, size_t nsets,
				const int *u, size_t ucount, int *out, size_t *out_size)
{
	signed char	vals[26];
	signed char	*stack;
	size_t		i;
	size_t		k;

	stack = xmalloc(strlen(formula));
	for (i = 0; i < 26; i++)
		vals[i] = 0;
	*out_size = 0;
	for (i = 0; i < ucount; i++)
	{
		for (k = 0; k < nsets; k++)
			vals[k] = contains(s[k], sn[k], u[i]);
		if (rpn_eval(formula, vals, stack) > 0)
			out[(*out_size)++] = u[i];
	}
	free(stack);
}

bool	eval_set(const char *formula, const int *const *sets,
			const size_t *sizes, size_t nsets, int **out, size_t *out_size)
{
	int		**s;
	size_t	*sn;
	int		*all;
	size_t	total;
	size_t	k;

	if (!args_ok(formula, sets, sizes, nsets))
		return (false);
	s = xmalloc((nsets + 1) * sizeof(*s));
	sn = xmalloc((nsets + 1) * sizeof(*sn));
	total = 0;
	for (k = 0; k < nsets; k++)
	{
		s[k] = sorted_unique(sets[k], sizes[k], &sn[k]);
		total += sn[k];
	}
	all = xmalloc((total + 1) * sizeof(int));
	for (k = total = 0; k < nsets; k++, total += sn[k - 1])
		memcpy(all + total, s[k], sn[k] * sizeof(int));
	qsort(all, total, sizeof(int), cmp_int);
	for (k = 0, *out_size = 0; k < total; k++)
		if (*out_size == 0 || all[*out_size - 1] != all[k])
			all[(*out_size)++] = all[k];
	*out = xmalloc((*out_size + 1) * sizeof(int));
	evaluate(formula, s, sn, nsets, all, *out_size, *out, out_size);
	for (k = 0; k < nsets; k++)
		free(s[k]);
	free(s);
	free(sn);
	free(all);
	return (true);
}

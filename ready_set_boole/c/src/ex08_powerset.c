/*
** Exercise 08 - Powerset
**
** Return the powerset P(A) of a set A: the set of all its subsets, including
** the empty set and A itself. A set with n elements has 2^n subsets.
**
** Idea: every subset corresponds to an n-bit mask (bit i set = element i is
** in the subset). Counting from 0 to 2^n - 1 therefore enumerates every
** combination exactly once (the input is assumed to have no duplicates).
** The order produced is {}, {a0}, {a1}, {a0,a1}, {a2}, ...
**
** Complexity: O(2^n) space for the result, as the subject allows. All the
** subsets live in one block of memory (the n * 2^(n-1) elements in total)
** plus two arrays of 2^n entries, so there are only a handful of allocations.
** Since the result grows so fast, the size is computed before allocating and
** requests above POWERSET_MAX_BYTES are refused with an error (NULL).
*/

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

#define POWERSET_MAX_BYTES	((size_t)2 << 30)

static size_t	popcount(size_t v)
{
	size_t	c;

	c = 0;
	while (v)
	{
		c += v & 1;
		v >>= 1;
	}
	return (c);
}

static bool	fits(size_t n)
{
	size_t	count;

	if (n > 40)
		return (false);
	count = (size_t)1 << n;
	/* count * (pointer + size) + n * count / 2 ints, checked against the cap */
	return (count <= POWERSET_MAX_BYTES / (sizeof(int *) + sizeof(size_t))
		&& (n * (count >> 1)) <= POWERSET_MAX_BYTES / sizeof(int)
		&& count * (sizeof(int *) + sizeof(size_t)) + n * (count >> 1)
		* sizeof(int) <= POWERSET_MAX_BYTES);
}

static void	fill(t_powerset *p, const int *set, size_t n)
{
	size_t	mask;
	size_t	i;
	int		*dst;

	dst = p->data;
	for (mask = 0; mask < p->count; mask++)
	{
		p->subsets[mask] = dst;
		p->sizes[mask] = popcount(mask);
		for (i = 0; i < n; i++)
			if ((mask >> i) & 1)
				*dst++ = set[i];
	}
}

t_powerset	*powerset(const int *set, size_t n)
{
	t_powerset	*p;

	if (n > 0 && !set)
		return (fprintf(stderr, "Error: null set\n"), NULL);
	if (!fits(n))
		return (fprintf(stderr, "Error: powerset of %zu elements is too large "
				"(over %zu bytes)\n", n, POWERSET_MAX_BYTES), NULL);
	p = xmalloc(sizeof(*p));
	p->count = (size_t)1 << n;
	p->subsets = malloc(p->count * sizeof(int *));
	p->sizes = malloc(p->count * sizeof(size_t));
	p->data = malloc((n * (p->count >> 1) + 1) * sizeof(int));
	if (!p->subsets || !p->sizes || !p->data)
	{
		fprintf(stderr, "Error: not enough memory for 2^%zu subsets\n", n);
		powerset_free(p);
		return (NULL);
	}
	fill(p, set, n);
	return (p);
}

void	powerset_free(t_powerset *p)
{
	if (!p)
		return ;
	free(p->data);
	free(p->subsets);
	free(p->sizes);
	free(p);
}

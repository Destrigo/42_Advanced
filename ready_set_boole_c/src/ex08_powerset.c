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
** Complexity: O(2^n) space for the result, as the subject allows. n is
** limited to 30 so that the result stays addressable.
*/

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

t_powerset	*powerset(const int *set, size_t n)
{
	t_powerset	*p;
	size_t		mask;
	size_t		i;
	size_t		k;

	if (n > 30)
	{
		fprintf(stderr, "Error: set too large for a powerset\n");
		return (NULL);
	}
	if (n > 0 && !set)
	{
		fprintf(stderr, "Error: null set\n");
		return (NULL);
	}
	p = xmalloc(sizeof(*p));
	p->count = (size_t)1 << n;
	p->subsets = calloc(p->count, sizeof(int *));
	p->sizes = calloc(p->count, sizeof(size_t));
	if (!p->subsets || !p->sizes)
	{
		fprintf(stderr, "Error: not enough memory for 2^%zu subsets\n", n);
		free(p->subsets);
		free(p->sizes);
		free(p);
		return (NULL);
	}
	for (mask = 0; mask < p->count; mask++)
	{
		p->sizes[mask] = 0;
		p->subsets[mask] = xmalloc((n + 1) * sizeof(int));
		for (i = 0; i < n; i++)
		{
			k = p->sizes[mask];
			if ((mask >> i) & 1)
			{
				p->subsets[mask][k] = set[i];
				p->sizes[mask]++;
			}
		}
	}
	return (p);
}

void	powerset_free(t_powerset *p)
{
	size_t	i;

	if (!p)
		return ;
	i = 0;
	while (i < p->count)
		free(p->subsets[i++]);
	free(p->subsets);
	free(p->sizes);
	free(p);
}

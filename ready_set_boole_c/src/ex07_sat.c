/*
** Exercise 07 - SAT
**
** Decide whether a formula is satisfiable, i.e. whether at least one
** assignment of its variables makes it true.
**
** Idea: brute force. Try all 2^n assignments (the bits of a counter), return
** true as soon as one evaluates to true, false if none does. SAT is the
** classic NP-complete problem, so no polynomial algorithm is known; the
** exercise accepts exponential time. Examples: "AB|" is satisfiable, "AA!&"
** (A and not A) and "AA^" (A xor A) are not.
**
** Invalid formulas print an error on stderr and return false.
**
** Complexity: O(2^n) assignments, each evaluated in linear time.
*/

#include "boole.h"

bool	sat(const char *formula)
{
	t_node		*n;
	signed char	vals[26];
	uint32_t	mask;
	uint32_t	row;
	uint32_t	count;
	int			k;
	bool		found;

	n = ast_parse(formula);
	if (!n)
		return (false);
	mask = ast_vars(n);
	count = 1;
	for (k = 0; k < 26; k++)
		if ((mask >> k) & 1)
			count <<= 1;
	found = false;
	for (row = 0; row < count && !found; row++)
	{
		uint32_t	bits = row;

		/* Spread the bits of row over the variables that actually appear. */
		for (k = 0; k < 26; k++)
		{
			vals[k] = 0;
			if ((mask >> k) & 1)
			{
				vals[k] = bits & 1;
				bits >>= 1;
			}
		}
		found = ast_eval(n, vals) > 0;
	}
	ast_free(n);
	return (found);
}

/*
** Exercise 07 - SAT
**
** Decide whether a formula is satisfiable, i.e. whether at least one
** assignment of its variables makes it true.
**
** Idea: brute force over the 2^n assignments (SAT is the classic
** NP-complete problem: no polynomial algorithm is known, and the exercise
** accepts exponential time). To keep that exponential as cheap as possible
** 64 assignments are evaluated at once: every variable is a 64-bit word where
** bit j is its value in assignment j, and & | ^ ! work on all 64 bits in
** parallel. The first 6 variables get the classic alternating patterns
** (0xAAAA..., 0xCCCC..., ...) and the others are constant per word, so 2^n
** assignments need only 2^(n-6) evaluations of the formula.
**
** Examples: "AB|" is satisfiable, "AA!&" (A and not A) and "AA^" (A xor A)
** are not. Invalid formulas print an error on stderr and return false.
**
** Complexity: O(2^n) assignments (O(2^n / 64) evaluations), each evaluation
** linear in the formula.
*/

#include <stdlib.h>
#include <string.h>
#include "boole.h"

static const uint64_t	g_pattern[6] = {
	0xAAAAAAAAAAAAAAAAull, 0xCCCCCCCCCCCCCCCCull, 0xF0F0F0F0F0F0F0F0ull,
	0xFF00FF00FF00FF00ull, 0xFFFF0000FFFF0000ull, 0xFFFFFFFF00000000ull};

bool	sat(const char *formula)
{
	uint64_t	vals[26];
	uint64_t	*stack;
	uint32_t	mask;
	uint64_t	chunk;
	uint64_t	valid;
	int			nv;
	int			k;
	int			v;
	bool		found;

	if (!rpn_check(formula))
		return (false);
	mask = rpn_vars(formula);
	nv = 0;
	for (k = 0; k < 26; k++)
		nv += (mask >> k) & 1;
	valid = (nv >= 6) ? ~(uint64_t)0 : (((uint64_t)1 << ((uint64_t)1 << nv)) - 1);
	stack = xmalloc(strlen(formula) * sizeof(*stack));
	found = false;
	for (chunk = 0; chunk < ((uint64_t)1 << (nv > 6 ? nv - 6 : 0)) && !found;
		chunk++)
	{
		for (k = 0, v = 0; k < 26; k++)
		{
			vals[k] = 0;
			if (!((mask >> k) & 1))
				continue ;
			vals[k] = (v < 6) ? g_pattern[v] : -((chunk >> (v - 6)) & 1);
			v++;
		}
		found = (rpn_eval_word(formula, vals, stack) & valid) != 0;
	}
	free(stack);
	return (found);
}

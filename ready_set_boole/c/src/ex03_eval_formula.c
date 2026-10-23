/*
** Exercise 03 - Boolean evaluation
**
** Evaluate a propositional formula written in reverse polish notation (RPN).
**
** The formula is checked and evaluated with the explicit-stack machine in
** rpn.c (see the comment at the top of that file): each character is
** handled once, so the cost is O(n) in the length of the formula, with no
** recursion, however long the input is.
**
** Operators: '!' not, '&' and, '|' or, '^' xor, '>' implication (!a | b),
** '=' equivalence (a == b). The only operands here are '0' and '1'.
**
** Invalid formulas (bad symbol, missing or leftover operands, variables)
** print an error on stderr and return false.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

bool	eval_formula(const char *formula)
{
	signed char	vals[26];
	signed char	*stack;
	int			r;
	int			i;

	if (!rpn_check(formula))
		return (false);
	i = 0;
	while (i < 26)
		vals[i++] = -1;
	stack = xmalloc(strlen(formula));
	r = rpn_eval(formula, vals, stack);
	free(stack);
	if (r < 0)
		fprintf(stderr, "Error: variables are not allowed here\n");
	return (r > 0);
}

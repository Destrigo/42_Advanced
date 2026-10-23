/*
** Exercise 03 - Boolean evaluation
**
** Evaluate a propositional formula written in reverse polish notation (RPN).
**
** The shared parser (ast.c) turns the string into a binary AST using a stack
** (see the comment at the top of that file), and the tree is evaluated
** recursively.
**
** Operators: '!' not, '&' and, '|' or, '^' xor, '>' implication (!a | b),
** '=' equivalence (a == b). The only operands here are '0' and '1'.
**
** Invalid formulas print an error on stderr and return false.
**
** Complexity: O(n) in the length of the formula.
*/

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

bool	eval_formula(const char *formula)
{
	signed char	vals[26];
	t_node		*n;
	int			r;
	int			i;

	n = ast_parse(formula);
	if (!n)
		return (false);
	i = 0;
	while (i < 26)
		vals[i++] = -1;
	r = ast_eval(n, vals);
	ast_free(n);
	if (r < 0)
		fprintf(stderr, "Error: variables are not allowed here\n");
	return (r > 0);
}

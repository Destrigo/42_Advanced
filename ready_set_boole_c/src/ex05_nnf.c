/*
** Exercise 05 - Negation Normal Form (NNF)
**
** Rewrite a formula so that it only uses variables, '!', '&' and '|', and
** every '!' sits directly on a variable.
**
** Idea: one recursive pass that carries a flag "this subtree is negated".
**  - !x just flips the flag (double negation elimination).
**  - De Morgan: !(a & b) = !a | !b, !(a | b) = !a & !b.
**  - Material condition: a > b = !a | b, so !(a > b) = a & !b.
**  - Equivalence: a = b = (a & b) | (!a & !b).
**  - Xor is the negation of equivalence: a ^ b = (a & !b) | (!a & b).
** A negated variable is emitted as "A!"; a plain one as "A".
**
** The result is logically equivalent to the input (checked in the tests by
** comparing truth tables). Size can grow for nested '=' / '^' since each
** side is needed in both polarities.
**
** Returns a malloc'ed string, or NULL if the formula is invalid.
*/

#include <stdlib.h>
#include "boole.h"

static t_node	*bin(char op, t_node *a, t_node *b)
{
	return (ast_new(N_BIN, op, a, b));
}

/* NNF of n (or of !n when neg), as a freshly allocated tree. */
t_node	*nnf_node(const t_node *n, bool neg)
{
	bool	same;

	if (n->type == N_CONST)
		return (ast_new(N_CONST, ((n->sym == '1') != neg) + '0', NULL, NULL));
	if (n->type == N_VAR && !neg)
		return (ast_new(N_VAR, n->sym, NULL, NULL));
	if (n->type == N_VAR)
		return (ast_new(N_NOT, '!', ast_new(N_VAR, n->sym, NULL, NULL), NULL));
	if (n->type == N_NOT)
		return (nnf_node(n->l, !neg));
	if (n->sym == '&' || n->sym == '|')
		return (bin((n->sym == '&') != neg ? '&' : '|',
				nnf_node(n->l, neg), nnf_node(n->r, neg)));
	if (n->sym == '>' && !neg)
		return (bin('|', nnf_node(n->l, true), nnf_node(n->r, false)));
	if (n->sym == '>')
		return (bin('&', nnf_node(n->l, false), nnf_node(n->r, true)));
	same = (n->sym == '=') != neg;
	if (same)
		return (bin('|', bin('&', nnf_node(n->l, false), nnf_node(n->r, false)),
				bin('&', nnf_node(n->l, true), nnf_node(n->r, true))));
	return (bin('|', bin('&', nnf_node(n->l, false), nnf_node(n->r, true)),
			bin('&', nnf_node(n->l, true), nnf_node(n->r, false))));
}

char	*negation_normal_form(const char *formula)
{
	t_node	*n;
	t_node	*res;
	char	*s;

	n = ast_parse(formula);
	if (!n)
		return (NULL);
	res = nnf_node(n, false);
	s = ast_to_rpn(res);
	ast_free(n);
	ast_free(res);
	return (s);
}

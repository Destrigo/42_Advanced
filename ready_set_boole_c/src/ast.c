/*
** Shared binary AST for propositional formulas written in reverse polish
** notation (RPN).
**
** In RPN the operator comes after its operands ("10&" is 1 AND 0), so no
** parentheses are needed. The parser reads the string left to right with a
** stack of nodes: constants and variables are pushed; '!' pops one node;
** every binary operator (& | ^ > =) pops two (right operand first) and
** pushes the combined node. A valid formula leaves exactly one node, the root
** of the tree.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

void	*xmalloc(size_t size)
{
	void	*p;

	p = malloc(size);
	if (!p)
	{
		perror("malloc");
		exit(1);
	}
	return (p);
}

t_node	*ast_new(t_type type, char sym, t_node *l, t_node *r)
{
	t_node	*n;

	n = xmalloc(sizeof(*n));
	n->type = type;
	n->sym = sym;
	n->l = l;
	n->r = r;
	return (n);
}

void	ast_free(t_node *n)
{
	if (!n)
		return ;
	ast_free(n->l);
	ast_free(n->r);
	free(n);
}

static t_node	*parse_fail(t_node **stack, size_t top, const char *msg, size_t i)
{
	size_t	k;

	k = 0;
	while (k < top)
		ast_free(stack[k++]);
	free(stack);
	if (i == (size_t)-1)
		fprintf(stderr, "Error: %s\n", msg);
	else
		fprintf(stderr, "Error: %s at position %zu\n", msg, i);
	return (NULL);
}

t_node	*ast_parse(const char *f)
{
	t_node	**st;
	size_t	top;
	size_t	i;
	t_node	*root;

	st = xmalloc((strlen(f) + 1) * sizeof(*st));
	top = 0;
	i = 0;
	while (f[i])
	{
		if (f[i] == '0' || f[i] == '1')
			st[top++] = ast_new(N_CONST, f[i], NULL, NULL);
		else if (f[i] >= 'A' && f[i] <= 'Z')
			st[top++] = ast_new(N_VAR, f[i], NULL, NULL);
		else if (f[i] == '!')
		{
			if (top < 1)
				return (parse_fail(st, top, "missing operand for '!'", i));
			st[top - 1] = ast_new(N_NOT, '!', st[top - 1], NULL);
		}
		else if (strchr("&|^>=", f[i]))
		{
			if (top < 2)
				return (parse_fail(st, top, "missing operand for operator", i));
			st[top - 2] = ast_new(N_BIN, f[i], st[top - 2], st[top - 1]);
			top--;
		}
		else
			return (parse_fail(st, top, "invalid symbol", i));
		i++;
	}
	if (top != 1)
		return (parse_fail(st, top, top ? "leftover operands" : "empty formula",
				(size_t)-1));
	root = st[0];
	free(st);
	return (root);
}

bool	ast_op(char op, bool a, bool b)
{
	if (op == '&')
		return (a & b);
	if (op == '|')
		return (a | b);
	if (op == '^')
		return (a ^ b);
	if (op == '>')
		return (!a | b);
	return (a == b);
}

int	ast_eval(const t_node *n, const signed char *vals)
{
	int	a;
	int	b;

	if (n->type == N_CONST)
		return (n->sym == '1');
	if (n->type == N_VAR)
		return (vals[n->sym - 'A']);
	a = ast_eval(n->l, vals);
	if (n->type == N_NOT)
		return (a < 0 ? -1 : !a);
	b = ast_eval(n->r, vals);
	if (a < 0 || b < 0)
		return (-1);
	return (ast_op(n->sym, a, b));
}

uint32_t	ast_vars(const t_node *n)
{
	if (n->type == N_CONST)
		return (0);
	if (n->type == N_VAR)
		return ((uint32_t)1 << (n->sym - 'A'));
	if (n->type == N_NOT)
		return (ast_vars(n->l));
	return (ast_vars(n->l) | ast_vars(n->r));
}

static size_t	rpn_len(const t_node *n)
{
	if (n->type == N_CONST || n->type == N_VAR)
		return (1);
	if (n->type == N_NOT)
		return (rpn_len(n->l) + 1);
	return (rpn_len(n->l) + rpn_len(n->r) + 1);
}

static char	*rpn_write(const t_node *n, char *dst)
{
	if (n->l)
		dst = rpn_write(n->l, dst);
	if (n->r)
		dst = rpn_write(n->r, dst);
	*dst = n->sym;
	return (dst + 1);
}

char	*ast_to_rpn(const t_node *n)
{
	char	*s;

	s = xmalloc(rpn_len(n) + 1);
	*rpn_write(n, s) = '\0';
	return (s);
}

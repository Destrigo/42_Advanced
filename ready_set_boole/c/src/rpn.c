/*
** Shared helpers for formulas written in reverse polish notation (RPN).
**
** In RPN the operator comes after its operands ("10&" is 1 AND 0), so no
** parentheses are needed and a formula can be checked and evaluated with a
** single left-to-right pass and a stack:
**  - 0, 1 and variables push a value;
**  - '!' replaces the top of the stack by its negation;
**  - a binary operator (& | ^ > =) pops two values (right operand first) and
**    pushes the result.
** A formula is valid when the stack never underflows and exactly one value is
** left at the end. Doing it this way (instead of building a recursive tree)
** means very long or very deep formulas cannot overflow the call stack.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

void	*xmalloc(size_t size)
{
	void	*p;

	p = malloc(size ? size : 1);
	if (!p)
	{
		perror("malloc");
		exit(1);
	}
	return (p);
}

void	*xcalloc(size_t count, size_t size)
{
	void	*p;

	p = calloc(count ? count : 1, size ? size : 1);
	if (!p)
	{
		perror("calloc");
		exit(1);
	}
	return (p);
}

void	*xrealloc(void *ptr, size_t size)
{
	void	*p;

	p = realloc(ptr, size ? size : 1);
	if (!p)
	{
		perror("realloc");
		exit(1);
	}
	return (p);
}

static bool	is_operand(char c)
{
	return (c == '0' || c == '1' || (c >= 'A' && c <= 'Z'));
}

static bool	is_binary(char c)
{
	return (c == '&' || c == '|' || c == '^' || c == '>' || c == '=');
}

bool	rpn_check(const char *f)
{
	size_t	depth;
	size_t	i;

	if (!f)
		return (fprintf(stderr, "Error: null formula\n"), false);
	depth = 0;
	for (i = 0; f[i]; i++)
	{
		if (is_operand(f[i]))
			depth++;
		else if (f[i] == '!' || is_binary(f[i]))
		{
			if (depth < (is_binary(f[i]) ? 2u : 1u))
				return (fprintf(stderr, "Error: missing operand for '%c' at "
						"position %zu\n", f[i], i), false);
			if (is_binary(f[i]))
				depth--;
		}
		else
			return (fprintf(stderr, "Error: invalid symbol at position %zu\n",
					i), false);
	}
	if (depth == 0)
		return (fprintf(stderr, "Error: empty formula\n"), false);
	if (depth != 1)
		return (fprintf(stderr, "Error: leftover operands\n"), false);
	return (true);
}

bool	rpn_op(char op, bool a, bool b)
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

uint32_t	rpn_vars(const char *f)
{
	uint32_t	mask;
	size_t		i;

	mask = 0;
	for (i = 0; f[i]; i++)
		if (f[i] >= 'A' && f[i] <= 'Z')
			mask |= (uint32_t)1 << (f[i] - 'A');
	return (mask);
}

int	rpn_eval(const char *f, const signed char *vals, signed char *st)
{
	size_t		top;
	size_t		i;
	signed char	a;
	signed char	b;

	top = 0;
	for (i = 0; f[i]; i++)
	{
		if (f[i] == '0' || f[i] == '1')
			st[top++] = (f[i] == '1');
		else if (f[i] >= 'A' && f[i] <= 'Z')
			st[top++] = vals[f[i] - 'A'];
		else if (f[i] == '!')
			st[top - 1] = (st[top - 1] < 0) ? -1 : !st[top - 1];
		else
		{
			b = st[--top];
			a = st[top - 1];
			st[top - 1] = (a < 0 || b < 0) ? -1 : rpn_op(f[i], a, b);
		}
	}
	return (st[0]);
}

uint64_t	rpn_eval_word(const char *f, const uint64_t *vals, uint64_t *st)
{
	size_t		top;
	size_t		i;
	uint64_t	a;
	uint64_t	b;

	top = 0;
	for (i = 0; f[i]; i++)
	{
		if (f[i] == '0' || f[i] == '1')
			st[top++] = (f[i] == '1') ? ~(uint64_t)0 : 0;
		else if (f[i] >= 'A' && f[i] <= 'Z')
			st[top++] = vals[f[i] - 'A'];
		else if (f[i] == '!')
			st[top - 1] = ~st[top - 1];
		else
		{
			b = st[--top];
			a = st[top - 1];
			if (f[i] == '&')
				st[top - 1] = a & b;
			else if (f[i] == '|')
				st[top - 1] = a | b;
			else if (f[i] == '^')
				st[top - 1] = a ^ b;
			else if (f[i] == '>')
				st[top - 1] = ~a | b;
			else
				st[top - 1] = ~(a ^ b);
		}
	}
	return (st[0]);
}

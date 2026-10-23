/*
** Exercise 06 - Conjunctive Normal Form (CNF)
**
** Rewrite a formula as an AND of ORs of literals (a literal is a variable or
** its negation), e.g. (A | B) & (B | !C).
**
** Idea: first convert to NNF (ex05), so only '&' '|' '!' on variables remain.
** Then represent the formula as a list of clauses (each clause a list of
** literals, meaning an OR):
**  - a & b: the clauses of a followed by the clauses of b.
**  - a | b: distribute OR over AND, i.e. every clause of a is merged with
**    every clause of b (A | (B & C) = (A | B) & (A | C)).
** Duplicate literals and duplicate clauses are dropped. Finally the clauses
** are written back in RPN, both ORs and ANDs right-nested ("ABC||", "ABC&&").
**
** The output can grow exponentially with the input (distribution). Constants
** '0'/'1' are not supported here: the exercise only uses variables.
**
** Returns a malloc'ed string, or NULL if the formula is invalid.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

/* A literal is encoded as (letter index << 1) | negated. */
typedef struct s_clause
{
	int		*lit;
	size_t	n;
}	t_clause;

typedef struct s_cnf
{
	t_clause	*c;
	size_t		n;
	size_t		cap;
}	t_cnf;

static void	cnf_free(t_cnf *s)
{
	size_t	i;

	i = 0;
	while (i < s->n)
		free(s->c[i++].lit);
	free(s->c);
	s->c = NULL;
	s->n = 0;
	s->cap = 0;
}

/* Takes ownership of clause; frees it if the set already contains it. */
static void	cnf_add(t_cnf *s, t_clause cl)
{
	size_t		i;
	t_clause	*bigger;

	i = 0;
	while (i < s->n)
	{
		if (s->c[i].n == cl.n
			&& memcmp(s->c[i].lit, cl.lit, cl.n * sizeof(int)) == 0)
		{
			free(cl.lit);
			return ;
		}
		i++;
	}
	if (s->n == s->cap)
	{
		s->cap = s->cap ? s->cap * 2 : 4;
		bigger = xmalloc(s->cap * sizeof(*bigger));
		if (s->n)
			memcpy(bigger, s->c, s->n * sizeof(*bigger));
		free(s->c);
		s->c = bigger;
	}
	s->c[s->n++] = cl;
}

static t_clause	unit(int lit)
{
	t_clause	cl;

	cl.lit = xmalloc(sizeof(int));
	cl.lit[0] = lit;
	cl.n = 1;
	return (cl);
}

/* Merge two clauses, keeping the first occurrence of each literal. */
static t_clause	merge(t_clause a, t_clause b)
{
	t_clause	m;
	size_t		i;
	size_t		j;

	m.lit = xmalloc((a.n + b.n) * sizeof(int));
	m.n = 0;
	i = 0;
	while (i < a.n + b.n)
	{
		int	l = i < a.n ? a.lit[i] : b.lit[i - a.n];

		j = 0;
		while (j < m.n && m.lit[j] != l)
			j++;
		if (j == m.n)
			m.lit[m.n++] = l;
		i++;
	}
	return (m);
}

/* Clauses of a formula already in NNF, appended to out. False on error. */
static bool	cnf_build(const t_node *n, t_cnf *out)
{
	t_cnf	a;
	t_cnf	b;
	size_t	i;
	size_t	j;
	bool	ok;

	if (n->type == N_VAR)
		cnf_add(out, unit((n->sym - 'A') << 1));
	else if (n->type == N_NOT && n->l->type == N_VAR)
		cnf_add(out, unit(((n->l->sym - 'A') << 1) | 1));
	else if (n->type == N_BIN && n->sym == '&')
		return (cnf_build(n->l, out) && cnf_build(n->r, out));
	else if (n->type == N_BIN && n->sym == '|')
	{
		memset(&a, 0, sizeof(a));
		memset(&b, 0, sizeof(b));
		ok = cnf_build(n->l, &a) && cnf_build(n->r, &b);
		for (i = 0; ok && i < a.n; i++)
			for (j = 0; j < b.n; j++)
				cnf_add(out, merge(a.c[i], b.c[j]));
		cnf_free(&a);
		cnf_free(&b);
		return (ok);
	}
	else
	{
		fprintf(stderr, "Error: unsupported node '%c' in CNF\n", n->sym);
		return (false);
	}
	return (true);
}

/* Literals of a clause joined by '|', clauses joined by '&'. */
static char	*cnf_emit(const t_cnf *s)
{
	size_t	i;
	size_t	j;
	size_t	total;
	char	*str;
	char	*p;

	total = 2;
	for (i = 0; i < s->n; i++)
		total += s->c[i].n * 3 + 1;
	str = xmalloc(total);
	p = str;
	for (i = 0; i < s->n; i++)
	{
		for (j = 0; j < s->c[i].n; j++)
		{
			*p++ = 'A' + (s->c[i].lit[j] >> 1);
			if (s->c[i].lit[j] & 1)
				*p++ = '!';
		}
		for (j = 1; j < s->c[i].n; j++)
			*p++ = '|';
	}
	for (i = 1; i < s->n; i++)
		*p++ = '&';
	*p = '\0';
	return (str);
}

char	*conjunctive_normal_form(const char *formula)
{
	t_node	*n;
	t_node	*nf;
	t_cnf	set;
	char	*res;

	n = ast_parse(formula);
	if (!n)
		return (NULL);
	nf = nnf_node(n, false);
	ast_free(n);
	memset(&set, 0, sizeof(set));
	res = NULL;
	if (cnf_build(nf, &set))
		res = cnf_emit(&set);
	cnf_free(&set);
	ast_free(nf);
	return (res);
}

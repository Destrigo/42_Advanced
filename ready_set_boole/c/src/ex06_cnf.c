/*
** Exercise 06 - Conjunctive Normal Form (CNF)
**
** Rewrite a formula as an AND of ORs of literals (a literal is a variable or
** its negation), e.g. (A | B) & (B | !C).
**
** Idea: represent every sub-formula as a list of clauses (each clause a list
** of literals, meaning an OR; the list means an AND), and process the RPN
** with a stack, as in ex05, keeping for every sub-formula x BOTH the CNF of
** x ("pos") and the CNF of !x ("neg"). With "union" = concatenating the
** clauses of two CNFs (their AND) and "or" = distributing OR over AND (every
** clause of one is merged with every clause of the other, because
** A | (B & C) = (A | B) & (A | C)):
**  - !x:    swap pos and neg.
**  - a & b: pos = union(a, b)         neg = or(!a, !b)
**  - a | b: pos = or(a, b)            neg = union(!a, !b)
**  - a > b: pos = or(!a, b)           neg = union(a, !b)
**  - a = b: pos = or(union(a, b), union(!a, !b))
**           neg = or(union(a, !b), union(!a, b))
**  - a ^ b: the same as '=', with pos and neg exchanged.
** Duplicate literals and duplicate clauses are dropped. Finally the clauses
** are written back in RPN, both ORs and ANDs right-nested ("ABC||", "ABC&&").
**
** There is no recursion, so any length of input is fine. The result can grow
** exponentially with the input (distribution); beyond CNF_MAX_CLAUSES
** clauses, NULL is returned instead of exhausting time and memory. Constants
** '0'/'1' are not supported here: the exercise only uses variables.
**
** Returns a malloc'ed string, or NULL if the formula is invalid / too large.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

#define CNF_MAX_CLAUSES	20000
#define CNF_MAX_WORK	4000000	/* max clause pairs merged by one '|' */

/* A literal is encoded as (letter index << 1) | negated. */
typedef struct s_clause
{
	int		*lit;
	size_t	n;
}	t_clause;

/* Clauses in insertion order plus an open-addressing hash table (indices + 1,
** 0 = empty) so duplicates are detected in O(1). */
typedef struct s_cnf
{
	t_clause	*c;
	size_t		n;
	size_t		cap;
	size_t		*tab;
	size_t		tabcap;
}	t_cnf;

typedef struct s_item
{
	t_cnf	pos;
	t_cnf	neg;
}	t_item;

static void	cnf_free(t_cnf *s)
{
	size_t	i;

	i = 0;
	while (i < s->n)
		free(s->c[i++].lit);
	free(s->c);
	free(s->tab);
	memset(s, 0, sizeof(*s));
}

static size_t	hash_clause(const t_clause *cl)
{
	size_t	h;
	size_t	i;

	h = 1469598103934665603ull;
	for (i = 0; i < cl->n; i++)
		h = (h ^ (size_t)cl->lit[i]) * 1099511628211ull;
	return (h ^ (h >> 29));
}

static bool	same(const t_clause *a, const t_clause *b)
{
	return (a->n == b->n && memcmp(a->lit, b->lit, a->n * sizeof(int)) == 0);
}

/* Index in tab of the slot holding cl, or of the empty slot where it goes. */
static size_t	find_slot(const t_cnf *s, const t_clause *cl)
{
	size_t	i;

	i = hash_clause(cl) & (s->tabcap - 1);
	while (s->tab[i] && !same(&s->c[s->tab[i] - 1], cl))
		i = (i + 1) & (s->tabcap - 1);
	return (i);
}

static void	grow_table(t_cnf *s)
{
	size_t	i;

	free(s->tab);
	s->tabcap = s->tabcap ? s->tabcap * 2 : 16;
	s->tab = xcalloc(s->tabcap, sizeof(*s->tab));
	for (i = 0; i < s->n; i++)
		s->tab[find_slot(s, &s->c[i])] = i + 1;
}

/* Takes ownership of cl; frees it if already present. False if over the cap. */
static bool	cnf_add(t_cnf *s, t_clause cl)
{
	if (s->tabcap && s->tab[find_slot(s, &cl)])
	{
		free(cl.lit);
		return (true);
	}
	if (s->n >= CNF_MAX_CLAUSES)
	{
		free(cl.lit);
		return (false);
	}
	if (s->n == s->cap)
	{
		s->cap = s->cap ? s->cap * 2 : 4;
		s->c = xrealloc(s->c, s->cap * sizeof(*s->c));
	}
	s->c[s->n++] = cl;
	if (s->n * 2 > s->tabcap)
		grow_table(s);
	else
		s->tab[find_slot(s, &s->c[s->n - 1])] = s->n;
	return (true);
}

static t_clause	clone(t_clause c)
{
	t_clause	r;

	r.n = c.n;
	r.lit = xmalloc(c.n * sizeof(int));
	memcpy(r.lit, c.lit, c.n * sizeof(int));
	return (r);
}

/* Merge two clauses, keeping the first occurrence of each literal. */
static t_clause	merge(t_clause a, t_clause b)
{
	t_clause	m;
	size_t		i;
	size_t		j;
	int			l;

	m.lit = xmalloc((a.n + b.n) * sizeof(int));
	m.n = 0;
	for (i = 0; i < a.n + b.n; i++)
	{
		l = i < a.n ? a.lit[i] : b.lit[i - a.n];
		for (j = 0; j < m.n && m.lit[j] != l; j++)
			;
		if (j == m.n)
			m.lit[m.n++] = l;
	}
	return (m);
}

/* out = a AND b (out must start empty; the caller frees it on failure). */
static bool	cnf_union(const t_cnf *a, const t_cnf *b, t_cnf *out)
{
	size_t	i;

	for (i = 0; i < a->n; i++)
		if (!cnf_add(out, clone(a->c[i])))
			return (false);
	for (i = 0; i < b->n; i++)
		if (!cnf_add(out, clone(b->c[i])))
			return (false);
	return (true);
}

/* out = a OR b, distributed. */
static bool	cnf_or(const t_cnf *a, const t_cnf *b, t_cnf *out)
{
	size_t	i;
	size_t	j;

	if (a->n * b->n > CNF_MAX_WORK)	/* bound the time spent on one product */
		return (false);
	for (i = 0; i < a->n; i++)
		for (j = 0; j < b->n; j++)
			if (!cnf_add(out, merge(a->c[i], b->c[j])))
				return (false);
	return (true);
}

/* eq = (a = b), xo = (a ^ b); both as CNF. */
static bool	cnf_eq_xor(const t_item *a, const t_item *b, t_cnf *eq, t_cnf *xo)
{
	t_cnf	t[4];
	bool	ok;

	memset(t, 0, sizeof(t));
	ok = cnf_union(&a->pos, &b->pos, &t[0]) && cnf_union(&a->neg, &b->neg, &t[1])
		&& cnf_or(&t[0], &t[1], eq)
		&& cnf_union(&a->pos, &b->neg, &t[2])
		&& cnf_union(&a->neg, &b->pos, &t[3]) && cnf_or(&t[2], &t[3], xo);
	cnf_free(&t[0]);
	cnf_free(&t[1]);
	cnf_free(&t[2]);
	cnf_free(&t[3]);
	return (ok);
}

static t_clause	unit(int lit)
{
	t_clause	cl;

	cl.lit = xmalloc(sizeof(int));
	cl.lit[0] = lit;
	cl.n = 1;
	return (cl);
}

/* Combines the top two items for a binary operator into r. */
static bool	combine(t_item *a, t_item *b, char c, t_item *r)
{
	t_cnf	eq;
	t_cnf	xo;
	bool	ok;

	if (c == '&')
		return (cnf_union(&a->pos, &b->pos, &r->pos)
			&& cnf_or(&a->neg, &b->neg, &r->neg));
	if (c == '|')
		return (cnf_or(&a->pos, &b->pos, &r->pos)
			&& cnf_union(&a->neg, &b->neg, &r->neg));
	if (c == '>')
		return (cnf_or(&a->neg, &b->pos, &r->pos)
			&& cnf_union(&a->pos, &b->neg, &r->neg));
	memset(&eq, 0, sizeof(eq));
	memset(&xo, 0, sizeof(xo));
	ok = cnf_eq_xor(a, b, &eq, &xo);
	r->pos = (c == '=') ? eq : xo;
	r->neg = (c == '=') ? xo : eq;
	return (ok);
}

/* Applies one RPN symbol. False on failure (stack items stay owned by st). */
static bool	step(t_item *st, size_t *top, char c)
{
	t_item	r;
	t_cnf	tmp;

	memset(&r, 0, sizeof(r));
	if (c == '0' || c == '1')
		return (fprintf(stderr, "Error: constants are not supported in CNF\n"),
			false);
	if (c >= 'A' && c <= 'Z')
	{
		cnf_add(&r.pos, unit((c - 'A') << 1));
		cnf_add(&r.neg, unit(((c - 'A') << 1) | 1));
		st[(*top)++] = r;
		return (true);
	}
	if (c == '!')
	{
		tmp = st[*top - 1].pos;
		st[*top - 1].pos = st[*top - 1].neg;
		st[*top - 1].neg = tmp;
		return (true);
	}
	if (!combine(&st[*top - 2], &st[*top - 1], c, &r))
		return (cnf_free(&r.pos), cnf_free(&r.neg), false);
	cnf_free(&st[*top - 1].pos);
	cnf_free(&st[*top - 1].neg);
	cnf_free(&st[*top - 2].pos);
	cnf_free(&st[*top - 2].neg);
	(*top)--;
	st[*top - 1] = r;
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
	t_item	*st;
	size_t	top;
	size_t	i;
	bool	ok;
	char	*res;

	if (!rpn_check(formula))
		return (NULL);
	st = xcalloc(strlen(formula), sizeof(*st));
	top = 0;
	ok = true;
	for (i = 0; formula[i] && ok; i++)
		ok = step(st, &top, formula[i]);
	if (!ok)
		fprintf(stderr, "Error: formula not supported or CNF too large "
			"(over %d clauses)\n", CNF_MAX_CLAUSES);
	res = ok ? cnf_emit(&st[0].pos) : NULL;
	for (i = 0; i < top; i++)
	{
		cnf_free(&st[i].pos);
		cnf_free(&st[i].neg);
	}
	free(st);
	return (res);
}

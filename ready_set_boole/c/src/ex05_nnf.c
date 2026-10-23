/*
** Exercise 05 - Negation Normal Form (NNF)
**
** Rewrite a formula so that it only uses variables, '!', '&' and '|', and
** every '!' sits directly on a variable.
**
** Idea: process the RPN left to right with a stack, and keep for every
** sub-formula x BOTH its NNF and the NNF of !x (called "pos" and "neg").
** Combining two sub-formulas a and b then only needs the rewrite rules:
**  - !x:    swap pos and neg (double negation elimination).
**  - a & b: pos = a & b          neg = !a | !b        (De Morgan)
**  - a | b: pos = a | b          neg = !a & !b
**  - a > b: pos = !a | b         neg = a & !b         (material condition)
**  - a = b: pos = (a & b) | (!a & !b)                 (equivalence)
**           neg = (a & !b) | (!a & b)
**  - a ^ b: the same as '=', with pos and neg exchanged.
** A negated variable is emitted as "A!"; a plain one as "A".
**
** The pieces are stored in an arena as a DAG (a node may be shared by
** several parents), so building the result is linear in the input and uses no
** recursion. Only writing it out expands the sharing, and that is where the
** result can become exponentially large for nested '=' / '^' (each side is
** needed in both polarities): the output is capped (NNF_MAX_OUT bytes) and
** NULL is returned beyond that instead of exhausting memory.
**
** Returns a malloc'ed string, or NULL if the formula is invalid / too large.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

#define NNF_MAX_OUT	(1u << 26)
#define NONE		UINT32_MAX

typedef struct s_dn
{
	char		sym;
	uint32_t	l;
	uint32_t	r;
}	t_dn;

typedef struct s_arena
{
	t_dn	*v;
	size_t	n;
	size_t	cap;
}	t_arena;

typedef struct s_pair
{
	uint32_t	pos;
	uint32_t	neg;
}	t_pair;

typedef struct s_frame
{
	uint32_t	n;
	char		seen;
}	t_frame;

typedef struct s_buf
{
	char	*s;
	size_t	len;
	size_t	cap;
}	t_buf;

static uint32_t	mk(t_arena *a, char sym, uint32_t l, uint32_t r)
{
	if (a->n == a->cap)
	{
		a->cap = a->cap ? a->cap * 2 : 64;
		a->v = xrealloc(a->v, a->cap * sizeof(*a->v));
	}
	a->v[a->n].sym = sym;
	a->v[a->n].l = l;
	a->v[a->n].r = r;
	return ((uint32_t)a->n++);
}

static void	swap_pair(t_pair *p)
{
	uint32_t	t;

	t = p->pos;
	p->pos = p->neg;
	p->neg = t;
}

/* Applies one RPN symbol to the stack of (pos, neg) pairs. */
static void	step(t_arena *a, t_pair *st, size_t *top, char c)
{
	t_pair	x;
	t_pair	y;
	t_pair	r;

	if (c == '0' || c == '1')
	{
		r.pos = mk(a, c, NONE, NONE);
		r.neg = mk(a, (c == '0') ? '1' : '0', NONE, NONE);
		st[(*top)++] = r;
		return ;
	}
	if (c >= 'A' && c <= 'Z')
	{
		r.pos = mk(a, c, NONE, NONE);
		r.neg = mk(a, '!', r.pos, NONE);
		st[(*top)++] = r;
		return ;
	}
	if (c == '!')
	{
		swap_pair(&st[*top - 1]);
		return ;
	}
	y = st[--(*top)];
	x = st[*top - 1];
	if (c == '&' || c == '|')
	{
		r.pos = mk(a, c, x.pos, y.pos);
		r.neg = mk(a, c == '&' ? '|' : '&', x.neg, y.neg);
	}
	else if (c == '>')
	{
		r.pos = mk(a, '|', x.neg, y.pos);
		r.neg = mk(a, '&', x.pos, y.neg);
	}
	else
	{
		r.pos = mk(a, '|', mk(a, '&', x.pos, y.pos), mk(a, '&', x.neg, y.neg));
		r.neg = mk(a, '|', mk(a, '&', x.pos, y.neg), mk(a, '&', x.neg, y.pos));
		if (c == '^')
			swap_pair(&r);
	}
	st[*top - 1] = r;
}

static bool	buf_push(t_buf *b, char c)
{
	if (b->len >= NNF_MAX_OUT)
		return (false);
	if (b->len + 2 > b->cap)
	{
		b->cap *= 2;
		b->s = xrealloc(b->s, b->cap);
	}
	b->s[b->len++] = c;
	return (true);
}

/* Writes the DAG rooted at root as RPN (post-order, explicit stack). */
static char	*emit(const t_arena *a, uint32_t root)
{
	t_buf	b;
	t_frame	*fr;
	t_frame	f;
	size_t	sp;
	size_t	cap;

	b.cap = 256;
	b.len = 0;
	b.s = xmalloc(b.cap);
	cap = 64;
	fr = xmalloc(cap * sizeof(*fr));
	fr[0] = (t_frame){root, 0};
	sp = 1;
	while (sp > 0)
	{
		f = fr[--sp];
		if (a->v[f.n].l != NONE && !f.seen)
		{
			if (sp + 3 > cap)
				fr = xrealloc(fr, (cap *= 2) * sizeof(*fr));
			fr[sp++] = (t_frame){f.n, 1};
			if (a->v[f.n].r != NONE)
				fr[sp++] = (t_frame){a->v[f.n].r, 0};
			fr[sp++] = (t_frame){a->v[f.n].l, 0};
		}
		else if (!buf_push(&b, a->v[f.n].sym))
		{
			fprintf(stderr, "Error: result too large (over %u bytes)\n",
				NNF_MAX_OUT);
			free(b.s);
			free(fr);
			return (NULL);
		}
	}
	b.s[b.len] = '\0';
	free(fr);
	return (b.s);
}

char	*negation_normal_form(const char *formula)
{
	t_arena	a;
	t_pair	*st;
	size_t	top;
	size_t	i;
	char	*res;

	if (!rpn_check(formula))
		return (NULL);
	memset(&a, 0, sizeof(a));
	st = xmalloc(strlen(formula) * sizeof(*st));
	top = 0;
	for (i = 0; formula[i]; i++)
		step(&a, st, &top, formula[i]);
	res = emit(&a, st[0].pos);
	free(st);
	free(a.v);
	return (res);
}

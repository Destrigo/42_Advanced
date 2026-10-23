/* Self-checking tests for every exercise. Run with `make test`.
** Besides the subject examples this tries hard to break each exercise:
** invalid and hostile input, million-character formulas, exponential
** blow-ups, huge sets, NaN, ... Error messages on stderr are expected. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

static int	g_fail;
static int	g_total;

#define CHECK(cond) do { g_total++; if (!(cond)) { g_fail++; \
	printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)

static void	check_str(char *got, const char *want, const char *in)
{
	g_total++;
	if (!got || strcmp(got, want) != 0)
	{
		g_fail++;
		printf("FAIL %s: got '%s', want '%s'\n", in, got ? got : "(null)", want);
	}
	free(got);
}

static uint64_t	g_rng = 88172645463325252ull;

static uint32_t	rnd(void)
{
	g_rng ^= g_rng << 13;
	g_rng ^= g_rng >> 7;
	g_rng ^= g_rng << 17;
	return ((uint32_t)(g_rng >> 11));
}

/* Same truth table for two valid formulas (up to 8 distinct variables). */
static bool	equivalent(const char *f, const char *g)
{
	signed char	vals[26];
	signed char	*sf = malloc(strlen(f));
	signed char	*sg = malloc(strlen(g));
	uint32_t	mask = rpn_vars(f) | rpn_vars(g);
	uint32_t	row;
	int			k;
	int			bit;
	bool		ok = true;

	for (row = 0; row < 256 && ok; row++)
	{
		for (k = 0, bit = 0; k < 26; k++)
			vals[k] = ((mask >> k) & 1) ? (row >> bit++) & 1 : 0;
		ok = rpn_eval(f, vals, sf) == rpn_eval(g, vals, sg);
	}
	free(sf);
	free(sg);
	return (ok);
}

static bool	no_extra_ops(const char *s)
{
	return (strpbrk(s, "^>=01") == NULL);
}

/* Random valid RPN formula over A..(A+nv-1) with about `ops` operators. */
static size_t	gen(char *out, size_t pos, int ops, int nv)
{
	if (ops <= 0 || rnd() % 5 == 0)
	{
		out[pos++] = 'A' + rnd() % nv;
		return (pos);
	}
	if (rnd() % 4 == 0)
	{
		pos = gen(out, pos, ops - 1, nv);
		out[pos++] = '!';
		return (pos);
	}
	pos = gen(out, pos, (ops - 1) / 2, nv);
	pos = gen(out, pos, ops - 1 - (ops - 1) / 2, nv);
	out[pos++] = "&|^>="[rnd() % 5];
	return (pos);
}

/* "<first>" followed by n times "<tail>" (e.g. a left-nested chain). */
static char	*repeat(const char *first, const char *tail, size_t n)
{
	size_t	lf = strlen(first);
	size_t	lt = strlen(tail);
	char	*s = malloc(lf + n * lt + 1);
	size_t	i;

	memcpy(s, first, lf);
	for (i = 0; i < n; i++)
		memcpy(s + lf + i * lt, tail, lt);
	s[lf + n * lt] = '\0';
	return (s);
}

static void	test_arith(void)
{
	uint32_t	n;
	uint32_t	a;
	uint32_t	b;
	uint32_t	expected[] = {0, 1, 3, 2, 6, 7, 5, 4, 12};
	int			i;

	CHECK(adder(0, 0) == 0);
	CHECK(adder(1, 2) == 3);
	CHECK(adder(255, 1) == 256);
	CHECK(adder(123456, 654321) == 777777);
	CHECK(adder(UINT32_MAX, 0) == UINT32_MAX);
	CHECK(adder(UINT32_MAX, 1) == 0);			/* wraps like u32 */
	CHECK(adder(UINT32_MAX, UINT32_MAX) == UINT32_MAX - 1);
	CHECK(multiplier(0, 5) == 0);
	CHECK(multiplier(5, 0) == 0);
	CHECK(multiplier(7, 6) == 42);
	CHECK(multiplier(1000, 1000) == 1000000);
	CHECK(multiplier(UINT32_MAX, UINT32_MAX) == 1);	/* wraps: (2^32-1)^2 mod 2^32 */
	for (i = 0; i < 200000; i++)
	{
		a = rnd() << (rnd() % 20);
		b = rnd() << (rnd() % 20);
		CHECK(adder(a, b) == (uint32_t)(a + b));
		CHECK(multiplier(a, b) == (uint32_t)(a * b));
		if (g_fail)
			return ;
	}
	for (n = 0; n <= 8; n++)
		CHECK(gray_code(n) == expected[n]);
	CHECK(gray_code(UINT32_MAX) == 0x80000000u);
	for (n = 0; n < 1000000; n++)	/* neighbours differ by exactly one bit */
	{
		uint32_t	d = gray_code(n) ^ gray_code(n + 1);

		CHECK(d != 0 && (d & (d - 1)) == 0);
		if (g_fail)
			return ;
	}
	CHECK((gray_code(UINT32_MAX) ^ gray_code(0)) == 0x80000000u);
}

static void	test_eval(void)
{
	size_t	n = 1000000;
	char	*s;

	CHECK(!eval_formula("10&"));
	CHECK(eval_formula("10|"));
	CHECK(eval_formula("11>"));
	CHECK(!eval_formula("10="));
	CHECK(eval_formula("1011||="));
	CHECK(eval_formula("10|1&"));
	CHECK(eval_formula("101|&"));
	CHECK(!eval_formula("1!0^"));
	CHECK(eval_formula("1"));
	CHECK(!eval_formula("0"));
	CHECK(eval_formula("0!"));
	CHECK(eval_formula("0!!!"));
	CHECK(eval_formula("01>"));
	CHECK(!eval_formula("10>"));
	/* invalid -> false, with a message on stderr, never a crash */
	CHECK(!eval_formula(NULL));
	CHECK(!eval_formula(""));
	CHECK(!eval_formula("1&"));
	CHECK(!eval_formula("!"));
	CHECK(!eval_formula("11"));
	CHECK(!eval_formula("1 1&"));
	CHECK(!eval_formula("11&\n"));
	CHECK(!eval_formula("\xff"));
	CHECK(!eval_formula("A"));		/* variables are not allowed in ex03 */
	CHECK(!eval_formula("1a&"));
	/* million-character formulas: right-nested and left-nested */
	s = repeat("", "1", n);
	s = realloc(s, 2 * n + 1);
	memset(s + n, '&', n - 1);
	s[2 * n - 1] = '\0';
	CHECK(eval_formula(s));
	free(s);
	s = repeat("1", "0|", n);
	CHECK(eval_formula(s));
	free(s);
	s = repeat("1", "!", n);		/* an even number of negations */
	CHECK(eval_formula(s));
	free(s);
}

static void	test_normal_forms(void)
{
	const char	*nnf_in[] = {"AB^", "AB^!", "AB=!", "AB>!", "A!!", "AB^C=!",
		"ABC>=C^", "A", "A!"};
	const char	*cnf_in[] = {"AB^", "AB=", "AB>C=", "AB^C|D&!", "ABC^=!", "A",
		"A!"};
	size_t		i;
	char		*r;

	check_str(negation_normal_form("AB&!"), "A!B!|", "AB&!");
	check_str(negation_normal_form("AB|!"), "A!B!&", "AB|!");
	check_str(negation_normal_form("AB>"), "A!B|", "AB>");
	check_str(negation_normal_form("AB="), "AB&A!B!&|", "AB=");
	check_str(negation_normal_form("AB|C&!"), "A!B!&C!|", "AB|C&!");
	check_str(negation_normal_form("A"), "A", "A");
	check_str(negation_normal_form("A!"), "A!", "A!");
	check_str(negation_normal_form("A!!"), "A", "A!!");
	check_str(negation_normal_form("A!!!"), "A!", "A!!!");
	for (i = 0; i < sizeof(nnf_in) / sizeof(*nnf_in); i++)
	{
		r = negation_normal_form(nnf_in[i]);
		CHECK(r && no_extra_ops(r) && equivalent(nnf_in[i], r));
		free(r);
	}
	check_str(conjunctive_normal_form("AB&!"), "A!B!|", "AB&!");
	check_str(conjunctive_normal_form("AB|!"), "A!B!&", "AB|!");
	check_str(conjunctive_normal_form("AB|C&"), "AB|C&", "AB|C&");
	check_str(conjunctive_normal_form("AB|C|D|"), "ABCD|||", "AB|C|D|");
	check_str(conjunctive_normal_form("AB&C&D&"), "ABCD&&&", "AB&C&D&");
	check_str(conjunctive_normal_form("AB&!C!|"), "A!B!C!||", "AB&!C!|");
	check_str(conjunctive_normal_form("AB|!C!&"), "A!B!C!&&", "AB|!C!&");
	check_str(conjunctive_normal_form("ABCD&|&"), "ABC|BD|&&", "ABCD&|&");
	check_str(conjunctive_normal_form("A"), "A", "A");
	check_str(conjunctive_normal_form("AA&"), "A", "AA&");
	for (i = 0; i < sizeof(cnf_in) / sizeof(*cnf_in); i++)
	{
		r = conjunctive_normal_form(cnf_in[i]);
		CHECK(r && no_extra_ops(r) && equivalent(cnf_in[i], r));
		free(r);
	}
	/* invalid input */
	CHECK(negation_normal_form(NULL) == NULL);
	CHECK(negation_normal_form("") == NULL);
	CHECK(negation_normal_form("AB") == NULL);
	CHECK(negation_normal_form("A&") == NULL);
	CHECK(negation_normal_form("a") == NULL);
	CHECK(conjunctive_normal_form(NULL) == NULL);
	CHECK(conjunctive_normal_form("") == NULL);
	CHECK(conjunctive_normal_form("A1&") == NULL);	/* constants unsupported */
}

static void	test_random_formulas(void)
{
	char	buf[512];
	char	*a;
	char	*b;
	size_t	n;
	int		i;

	for (i = 0; i < 5000; i++)
	{
		n = gen(buf, 0, rnd() % 9, 1 + rnd() % 4);
		buf[n] = '\0';
		a = negation_normal_form(buf);
		b = conjunctive_normal_form(buf);
		if (a)
			CHECK(no_extra_ops(a) && equivalent(buf, a));
		if (b)
			CHECK(no_extra_ops(b) && equivalent(buf, b));
		CHECK(a != NULL);
		free(a);
		free(b);
		if (g_fail)
		{
			printf("on formula %s\n", buf);
			return ;
		}
	}
}

static void	test_big_formulas(void)
{
	size_t	n = 1000000;
	char	*s;
	char	*r;
	int		i;

	/* "AAA...&&&": one million operands */
	s = repeat("", "A", n);
	s = realloc(s, 2 * n + 1);
	memset(s + n, '&', n - 1);
	s[2 * n - 1] = '\0';
	CHECK(sat(s));
	r = negation_normal_form(s);
	CHECK(r && strlen(r) == 2 * n - 1);
	free(r);
	check_str(conjunctive_normal_form(s), "A", "A x 1e6 &");
	free(s);
	/* left-nested chain of alternating variables */
	s = repeat("A", "B|C&", n / 4);
	CHECK(sat(s));
	r = conjunctive_normal_form(s);
	CHECK(r != NULL);
	free(r);
	free(s);
	/* nested '=' doubles in size at every level: refused, not a hang/OOM */
	s = malloc(200);
	strcpy(s, "AB=");
	for (i = 0; i < 60; i++)
		strcat(s, (i & 1) ? "C=" : "D=");
	r = negation_normal_form(s);
	CHECK(r == NULL);
	free(r);
	r = conjunctive_normal_form(s);
	CHECK(r == NULL);
	free(r);
	CHECK(sat(s));
	free(s);
}

static void	test_sat(void)
{
	char	all[64];
	char	unsat[64];
	int		i;

	CHECK(sat("AB|"));
	CHECK(sat("AB&"));
	CHECK(!sat("AA!&"));
	CHECK(!sat("AA^"));
	CHECK(sat("A"));
	CHECK(sat("A!"));
	CHECK(!sat("AA!&B&"));
	CHECK(sat("AB^"));
	CHECK(!sat("AB=A!B&"));
	CHECK(!sat("AB"));	/* invalid */
	CHECK(!sat(NULL));
	CHECK(!sat(""));
	/* all 26 variables: 2^26 assignments, must still be quick */
	for (i = 0; i < 26; i++)
		all[i] = 'A' + i;
	memset(all + 26, '&', 25);
	all[51] = '\0';
	CHECK(sat(all));
	/* unsatisfiable only because of one clash among 26 variables */
	strcpy(unsat, "AA!&");
	for (i = 1; i < 26; i++)
	{
		unsat[4 + 2 * (i - 1)] = 'A' + i;
		unsat[5 + 2 * (i - 1)] = '&';
	}
	unsat[4 + 2 * 25] = '\0';
	CHECK(!sat(unsat));
}

static void	test_sets(void)
{
	t_powerset	*p;
	int			in[] = {1, 2, 3};
	int			big[24] = {0};
	const int	a[] = {0, 1, 2}, b[] = {0, 3, 4}, c[] = {3, 4, 5};
	const int *const	ab[] = {a, b};
	const int *const	ac[] = {a, c};
	const size_t		sz[] = {3, 3};
	int			*out;
	size_t		n;
	size_t		i;

	p = powerset(in, 3);
	CHECK(p && p->count == 8 && p->sizes[0] == 0 && p->sizes[7] == 3);
	CHECK(p->subsets[5][0] == 1 && p->subsets[5][1] == 3);
	powerset_free(p);
	p = powerset(NULL, 0);
	CHECK(p && p->count == 1 && p->sizes[0] == 0);
	powerset_free(p);
	for (i = 0; i < 24; i++)
		big[i] = (int)i;
	p = powerset(big, 20);
	CHECK(p && p->count == (size_t)1 << 20 && p->sizes[p->count - 1] == 20
		&& p->subsets[p->count - 1][19] == 19);
	powerset_free(p);
	CHECK(powerset(NULL, 3) == NULL);
	CHECK(powerset(in, 31) == NULL);	/* too big for any memory */
	CHECK(powerset(in, 64) == NULL);
	CHECK(powerset(in, (size_t)-1) == NULL);
	powerset_free(NULL);

	CHECK(eval_set("AB&", ab, sz, 2, &out, &n) && n == 1 && out[0] == 0);
	free(out);
	CHECK(eval_set("AB|", ac, sz, 2, &out, &n) && n == 6 && out[5] == 5);
	free(out);
	CHECK(eval_set("A!", ab, sz, 1, &out, &n) && n == 0);
	free(out);
	CHECK(eval_set("AB^", ab, sz, 2, &out, &n) && n == 4 && out[0] == 1);
	free(out);
	CHECK(eval_set("A!B&", ab, sz, 2, &out, &n) && n == 2 && out[0] == 3);
	free(out);
	CHECK(eval_set("AB>", ab, sz, 2, &out, &n) && n == 3);	/* !A | B = {0,3,4} */
	free(out);
	CHECK(!eval_set("AB&", ab, sz, 1, &out, &n));	/* wrong set count */
	CHECK(!eval_set("AC&", ab, sz, 2, &out, &n));	/* gap in variables */
	CHECK(!eval_set("AB", ab, sz, 2, &out, &n));	/* invalid formula */
	CHECK(!eval_set(NULL, NULL, NULL, 0, &out, &n));
	CHECK(!eval_set("A", NULL, NULL, 1, &out, &n));
}

static void	test_sets_big(void)
{
	size_t		n = 200000;
	int			*x = malloc(n * sizeof(int));
	int			*y = malloc(n * sizeof(int));
	const int	*sets[2];
	size_t		sizes[2] = {n, n};
	int			*out;
	size_t		cnt;
	size_t		i;

	/* x = evens (with duplicates and negatives), y = multiples of 3 */
	for (i = 0; i < n; i++)
	{
		x[i] = ((int)(i % (n / 2)) - 50000) * 2;
		y[i] = ((int)i - 100000) * 3;
	}
	x[0] = INT32_MIN;
	y[0] = INT32_MAX;
	sets[0] = x;
	sets[1] = y;
	CHECK(eval_set("AB&", sets, sizes, 2, &out, &cnt));
	for (i = 1; i < cnt; i++)
		CHECK(out[i - 1] < out[i] && out[i] % 6 == 0);
	free(out);
	CHECK(eval_set("AB|", sets, sizes, 2, &out, &cnt) && cnt > n);
	for (i = 1; i < cnt; i++)
		CHECK(out[i - 1] < out[i]);
	free(out);
	free(x);
	free(y);
}

static void	test_curve(void)
{
	uint32_t	x;
	uint32_t	y;
	uint16_t	rx;
	uint16_t	ry;
	uint32_t	z;
	double		v;

	CHECK(map(0, 0) == 0.0);
	CHECK(map(1, 0) == 1.0 / 4294967296.0);
	CHECK(map(0, 1) == 2.0 / 4294967296.0);
	CHECK(map(65535, 65535) < 1.0);
	CHECK(map(65535, 65535) > 0.9999999);
	for (x = 0; x <= 65535; x += 251)
		for (y = 0; y <= 65535; y += 257)
		{
			CHECK(reverse_map(map(x, y), &rx, &ry) && rx == x && ry == y);
			if (g_fail)
				return ;
		}
	for (z = 0; z < UINT32_MAX - 65537; z += 65537)
	{
		v = (double)z / 4294967296.0;
		CHECK(reverse_map(v, &rx, &ry) && map(rx, ry) == v);
	}
	/* out of range / hostile values: error, outputs zeroed, no crash */
	rx = ry = 7;
	CHECK(!reverse_map(1.0, &rx, &ry) && rx == 0 && ry == 0);
	CHECK(!reverse_map(-0.5, &rx, &ry));
	CHECK(!reverse_map(1e300, &rx, &ry));
	CHECK(!reverse_map(-1e300, &rx, &ry));
	{
		volatile double	zero = 0.0;	/* volatile: no compile-time 0/0 */

		CHECK(!reverse_map(zero / zero, &rx, &ry));	/* NaN */
		CHECK(!reverse_map(1.0 / zero, &rx, &ry));	/* +inf */
		CHECK(!reverse_map(-1.0 / zero, &rx, &ry));	/* -inf */
	}
	CHECK(reverse_map(-0.0, &rx, &ry) && rx == 0 && ry == 0);
	CHECK(reverse_map(0.9999999999999999, &rx, &ry) && rx == 65535
		&& ry == 65535);
	CHECK(!reverse_map(0.5, NULL, &ry));
	/* bijective: a full row of the grid maps to distinct values */
	{
		double	prev = -1.0;

		for (x = 0; x < 65536; x++)
		{
			v = map(x, 12345);
			CHECK(v != prev);
			prev = v;
		}
	}
}

int	main(void)
{
	test_arith();
	test_eval();
	test_normal_forms();
	test_random_formulas();
	test_big_formulas();
	test_sat();
	test_sets();
	test_sets_big();
	test_curve();
	printf("%d/%d checks passed\n", g_total - g_fail, g_total);
	return (g_fail != 0);
}

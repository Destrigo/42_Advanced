/* Self-checking tests for every exercise. Run with `make test`. */

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

/* Same truth table for two formulas over A..Z (by brute force). */
static bool	equivalent(const char *f, const char *g)
{
	t_node		*a = ast_parse(f);
	t_node		*b = ast_parse(g);
	signed char	vals[26];
	uint32_t	mask = ast_vars(a) | ast_vars(b);
	uint32_t	row;
	int			k;
	int			bit;
	bool		ok = true;

	for (row = 0; row < 1u << 8 && ok; row++)
	{
		for (k = 0, bit = 0; k < 26; k++)
			vals[k] = ((mask >> k) & 1) ? (row >> bit++) & 1 : 0;
		ok = ast_eval(a, vals) == ast_eval(b, vals);
	}
	ast_free(a);
	ast_free(b);
	return (ok);
}

static bool	no_extra_ops(const char *s)
{
	return (strpbrk(s, "^>=01") == NULL);
}

static void	test_arith(void)
{
	uint32_t	n;
	uint32_t	expected[] = {0, 1, 3, 2, 6, 7, 5, 4, 12};

	CHECK(adder(0, 0) == 0);
	CHECK(adder(1, 2) == 3);
	CHECK(adder(255, 1) == 256);
	CHECK(adder(123456, 654321) == 777777);
	CHECK(adder(UINT32_MAX, 0) == UINT32_MAX);
	CHECK(multiplier(0, 5) == 0);
	CHECK(multiplier(7, 6) == 42);
	CHECK(multiplier(1000, 1000) == 1000000);
	CHECK(multiplier(65535, 65537) == (uint32_t)(65535u * 65537u));
	for (n = 0; n <= 8; n++)
		CHECK(gray_code(n) == expected[n]);
}

static void	test_logic(void)
{
	const char	*nnf_in[] = {"AB^", "AB^!", "AB=!", "AB>!", "A!!", "AB^C=!",
		"ABC>=C^"};
	const char	*cnf_in[] = {"AB^", "AB=", "AB>C=", "AB^C|D&!", "ABC^=!"};
	size_t		i;
	char		*r;

	CHECK(!eval_formula("10&"));
	CHECK(eval_formula("10|"));
	CHECK(eval_formula("11>"));
	CHECK(!eval_formula("10="));
	CHECK(eval_formula("1011||="));
	CHECK(eval_formula("10|1&"));
	CHECK(eval_formula("101|&"));
	CHECK(!eval_formula("1!0^"));
	CHECK(!eval_formula("1&"));	/* invalid -> false (message on stderr) */

	check_str(negation_normal_form("AB&!"), "A!B!|", "AB&!");
	check_str(negation_normal_form("AB|!"), "A!B!&", "AB|!");
	check_str(negation_normal_form("AB>"), "A!B|", "AB>");
	check_str(negation_normal_form("AB="), "AB&A!B!&|", "AB=");
	check_str(negation_normal_form("AB|C&!"), "A!B!&C!|", "AB|C&!");
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
	for (i = 0; i < sizeof(cnf_in) / sizeof(*cnf_in); i++)
	{
		r = conjunctive_normal_form(cnf_in[i]);
		CHECK(r && no_extra_ops(r) && equivalent(cnf_in[i], r));
		free(r);
	}

	CHECK(sat("AB|"));
	CHECK(sat("AB&"));
	CHECK(!sat("AA!&"));
	CHECK(!sat("AA^"));
	CHECK(!sat("AB"));	/* invalid */
}

static void	test_sets(void)
{
	t_powerset	*p;
	int			in[] = {1, 2, 3};
	int			big[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const int	a[] = {0, 1, 2}, b[] = {0, 3, 4}, c[] = {3, 4, 5};
	const int *const	ab[] = {a, b};
	const int *const	ac[] = {a, c};
	const size_t		sz[] = {3, 3};
	int			*out;
	size_t		n;

	p = powerset(in, 3);
	CHECK(p && p->count == 8 && p->sizes[0] == 0 && p->sizes[7] == 3);
	CHECK(p->subsets[5][0] == 1 && p->subsets[5][1] == 3);
	powerset_free(p);
	p = powerset(NULL, 0);
	CHECK(p && p->count == 1 && p->sizes[0] == 0);
	powerset_free(p);
	p = powerset(big, 10);
	CHECK(p && p->count == 1024);
	powerset_free(p);

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
	CHECK(!eval_set("AB&", ab, sz, 1, &out, &n));	/* wrong set count */
	CHECK(!eval_set("AC&", ab, sz, 2, &out, &n));	/* gap in variables */
}

/* Hostile inputs: must fail cleanly, never crash. */
static void	test_defensive(void)
{
	size_t	n = 200000;
	char	*deep = malloc(2 * n + 1);
	char	*ok = malloc(2 * 400 + 2);
	char	*r;
	size_t	i;
	int		*out;
	size_t	out_n;

	for (i = 0; i < n; i++)
	{
		deep[i] = 'A';
		deep[n + i] = '&';
	}
	deep[2 * n] = '\0';
	CHECK(!sat(deep));			/* 200000 levels: rejected, no stack overflow */
	CHECK(negation_normal_form(deep) == NULL);
	CHECK(conjunctive_normal_form(deep) == NULL);
	for (i = 0; i < 400; i++)
		ok[i] = 'A';
	for (i = 0; i < 399; i++)
		ok[400 + i] = '&';
	ok[799] = '\0';
	CHECK(sat(ok));				/* 400 levels: still fine */
	r = conjunctive_normal_form(ok);
	CHECK(r && strcmp(r, "A") == 0);	/* duplicates collapse */
	free(r);
	free(deep);
	free(ok);

	CHECK(!eval_formula(NULL));
	CHECK(!eval_formula(""));
	CHECK(!eval_formula("1 1&"));
	CHECK(!eval_formula("\xff"));
	CHECK(!sat(NULL));
	CHECK(negation_normal_form(NULL) == NULL);
	CHECK(conjunctive_normal_form("") == NULL);
	CHECK(conjunctive_normal_form("A1&") == NULL);	/* constants unsupported */
	CHECK(powerset(NULL, 3) == NULL);
	CHECK(powerset(NULL, 31) == NULL);
	CHECK(!eval_set(NULL, NULL, NULL, 0, &out, &out_n));
	print_truth_table("A&");	/* invalid: message only */
	print_truth_table(NULL);
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
	CHECK(!reverse_map(1.0, &rx, &ry));
	CHECK(!reverse_map(-0.5, &rx, &ry));
}

int	main(void)
{
	test_arith();
	test_logic();
	test_sets();
	test_defensive();
	test_curve();
	printf("%d/%d checks passed\n", g_total - g_fail, g_total);
	return (g_fail != 0);
}

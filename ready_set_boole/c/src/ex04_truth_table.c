/*
** Exercise 04 - Truth table
**
** Print the truth table of a formula whose variables are the letters A..Z.
**
** Idea: collect the distinct variables (sorted alphabetically, one column
** each), then enumerate all 2^n assignments with a counter whose bits are the
** variable values: the first variable is the most significant bit, so rows
** come out in the usual 000, 001, 010, ... order. Each row evaluates the
** formula with that assignment (stack machine, see rpn.c) and prints the
** result in the last column ("="). Each row is built in a small buffer and
** written with one call.
**
** Complexity: O(2^n) rows, each evaluated in time linear in the formula.
** Invalid formulas print an error on stderr and nothing on stdout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

static void	print_header(const char *vars, int cnt)
{
	int	k;

	printf("|");
	for (k = 0; k < cnt; k++)
		printf(" %c |", vars[k]);
	printf(" = |\n|");
	for (k = 0; k <= cnt; k++)
		printf("---|");
	printf("\n");
}

void	print_truth_table(const char *formula)
{
	signed char	vals[26];
	char		vars[26];
	char		line[26 * 4 + 16];
	signed char	*stack;
	uint32_t	row;
	char		*p;
	int			cnt;
	int			k;

	if (!rpn_check(formula))
		return ;
	cnt = 0;
	for (k = 0; k < 26; k++)
	{
		vals[k] = -1;
		if ((rpn_vars(formula) >> k) & 1)
			vars[cnt++] = 'A' + k;
	}
	print_header(vars, cnt);
	stack = xmalloc(strlen(formula));
	for (row = 0; row < ((uint32_t)1 << cnt); row++)
	{
		p = line;
		*p++ = '|';
		for (k = 0; k < cnt; k++)
		{
			vals[vars[k] - 'A'] = (row >> (cnt - 1 - k)) & 1;
			p += sprintf(p, " %d |", vals[vars[k] - 'A']);
		}
		sprintf(p, " %d |\n", rpn_eval(formula, vals, stack));
		fputs(line, stdout);
	}
	free(stack);
}

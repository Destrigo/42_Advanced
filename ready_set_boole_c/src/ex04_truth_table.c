/*
** Exercise 04 - Truth table
**
** Print the truth table of a formula whose variables are the letters A..Z.
**
** Idea: collect the distinct variables (sorted alphabetically, one column
** each), then enumerate all 2^n assignments with a counter whose bits are the
** variable values: the first variable is the most significant bit, so rows
** come out in the usual 000, 001, 010, ... order. Each row evaluates the AST
** with that assignment and prints the result in the last column ("=").
**
** Complexity: O(2^n) rows, each evaluated in time linear in the formula.
*/

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

void	print_truth_table(const char *formula)
{
	t_node		*n;
	signed char	vals[26];
	char		vars[26];
	int			cnt;
	int			k;
	uint32_t	row;

	n = ast_parse(formula);
	if (!n)
		return ;
	cnt = 0;
	k = 0;
	while (k < 26)
	{
		vals[k] = -1;
		if ((ast_vars(n) >> k) & 1)
			vars[cnt++] = 'A' + k;
		k++;
	}
	printf("|");
	for (k = 0; k < cnt; k++)
		printf(" %c |", vars[k]);
	printf(" = |\n|");
	for (k = 0; k <= cnt; k++)
		printf("---|");
	printf("\n");
	for (row = 0; row < ((uint32_t)1 << cnt); row++)
	{
		printf("|");
		for (k = 0; k < cnt; k++)
		{
			vals[vars[k] - 'A'] = (row >> (cnt - 1 - k)) & 1;
			printf(" %d |", vals[vars[k] - 'A']);
		}
		printf(" %d |\n", ast_eval(n, vals));
	}
	ast_free(n);
}

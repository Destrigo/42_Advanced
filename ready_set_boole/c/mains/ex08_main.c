#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex08 [n ...]  -- without arguments uses the set {1, 2, 3}. */
int	main(int argc, char **argv)
{
	int			def[3] = {1, 2, 3};
	int			*set;
	size_t		n;
	size_t		i;
	size_t		j;
	t_powerset	*p;

	n = 3;
	set = def;
	if (argc > 1)
	{
		n = argc - 1;
		set = malloc(n * sizeof(int));
		for (i = 0; i < n; i++)
			set[i] = atoi(argv[i + 1]);
	}
	p = powerset(set, n);
	if (p)
	{
		for (i = 0; i < p->count; i++)
		{
			printf("{");
			for (j = 0; j < p->sizes[i]; j++)
				printf(j ? ", %d" : "%d", p->subsets[i][j]);
			printf("}\n");
		}
	}
	powerset_free(p);
	if (set != def)
		free(set);
	return (0);
}

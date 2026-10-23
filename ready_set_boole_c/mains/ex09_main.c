#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boole.h"

static void	show(const char *f, const int *const *sets, const size_t *sizes,
				size_t nsets)
{
	int		*out;
	size_t	n;
	size_t	i;

	if (!eval_set(f, sets, sizes, nsets, &out, &n))
		return ;
	printf("%s -> [", f);
	for (i = 0; i < n; i++)
		printf(i ? ", %d" : "%d", out[i]);
	printf("]\n");
	free(out);
}

/* Usage: ex09 <formula> <set> <set> ...   where a set is "0,1,2".
** Without arguments runs the subject examples. */
int	main(int argc, char **argv)
{
	const int		a[] = {0, 1, 2}, b[] = {0, 3, 4}, c[] = {3, 4, 5};
	const int *const	s1[] = {a, b};
	const int *const	s2[] = {a, c};
	const size_t		sz[] = {3, 3};
	int					**sets;
	size_t				*sizes;
	char				*tok;
	int					i;

	if (argc < 2)
	{
		show("AB&", s1, sz, 2);
		show("AB|", s2, sz, 2);
		show("A!", s1, sz, 1);
		return (0);
	}
	sets = malloc(argc * sizeof(int *));
	sizes = calloc(argc, sizeof(size_t));
	for (i = 2; i < argc; i++)
	{
		sets[i - 2] = malloc(strlen(argv[i]) * sizeof(int) + sizeof(int));
		for (tok = strtok(argv[i], ","); tok; tok = strtok(NULL, ","))
			sets[i - 2][sizes[i - 2]++] = atoi(tok);
	}
	show(argv[1], (const int *const *)sets, sizes, argc - 2);
	for (i = 2; i < argc; i++)
		free(sets[i - 2]);
	free(sets);
	free(sizes);
	return (0);
}

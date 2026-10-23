#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

static void	show(const char *f)
{
	char	*r;

	r = conjunctive_normal_form(f);
	printf("%s -> %s\n", f, r ? r : "(invalid)");
	free(r);
}

/* Usage: ex06 [formula ...]  -- without arguments runs the subject examples. */
int	main(int argc, char **argv)
{
	const char	*ex[] = {"AB&!", "AB|!", "AB|C&", "AB|C|D|", "AB&C&D&",
		"AB&!C!|", "AB|!C!&", "ABCD&|&", NULL};
	int			i;

	if (argc > 1)
		for (i = 1; i < argc; i++)
			show(argv[i]);
	else
		for (i = 0; ex[i]; i++)
			show(ex[i]);
	return (0);
}

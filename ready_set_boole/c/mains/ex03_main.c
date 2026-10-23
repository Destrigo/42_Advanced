#include <stdio.h>
#include "boole.h"

/* Usage: ex03 [formula ...]  -- without arguments runs the subject examples. */
int	main(int argc, char **argv)
{
	const char	*ex[] = {"10&", "10|", "11>", "10=", "1011||=", NULL};
	int			i;

	if (argc > 1)
		for (i = 1; i < argc; i++)
			printf("%s -> %s\n", argv[i], eval_formula(argv[i]) ? "true" : "false");
	else
		for (i = 0; ex[i]; i++)
			printf("%s -> %s\n", ex[i], eval_formula(ex[i]) ? "true" : "false");
	return (0);
}

#include <stdio.h>
#include "boole.h"

/* Usage: ex07 [formula ...]  -- without arguments runs the subject examples. */
int	main(int argc, char **argv)
{
	const char	*ex[] = {"AB|", "AB&", "AA!&", "AA^", NULL};
	int			i;

	if (argc > 1)
		for (i = 1; i < argc; i++)
			printf("%s -> %s\n", argv[i], sat(argv[i]) ? "true" : "false");
	else
		for (i = 0; ex[i]; i++)
			printf("%s -> %s\n", ex[i], sat(ex[i]) ? "true" : "false");
	return (0);
}

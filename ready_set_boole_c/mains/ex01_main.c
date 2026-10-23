#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex01 [a b]  -- without arguments runs a few examples. */
int	main(int argc, char **argv)
{
	uint32_t	ex[4][2] = {{0, 5}, {1, 5}, {7, 6}, {1000, 1000}};
	int			i;

	if (argc == 3)
	{
		printf("%u\n", multiplier(strtoul(argv[1], NULL, 10),
				strtoul(argv[2], NULL, 10)));
		return (0);
	}
	for (i = 0; i < 4; i++)
		printf("%u * %u = %u\n", ex[i][0], ex[i][1],
			multiplier(ex[i][0], ex[i][1]));
	return (0);
}

#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex10 [x y]  -- without arguments runs a few examples. */
int	main(int argc, char **argv)
{
	uint16_t	ex[5][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {65535, 65535}};
	int			i;

	if (argc == 3)
	{
		printf("%.17g\n", map(atoi(argv[1]), atoi(argv[2])));
		return (0);
	}
	for (i = 0; i < 5; i++)
		printf("map(%u, %u) = %.17g\n", ex[i][0], ex[i][1],
			map(ex[i][0], ex[i][1]));
	return (0);
}

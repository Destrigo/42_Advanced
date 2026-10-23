#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex11 [n]  -- without arguments round-trips a few points. */
int	main(int argc, char **argv)
{
	uint16_t	ex[5][2] = {{0, 0}, {1, 0}, {0, 1}, {1234, 4321}, {65535, 65535}};
	uint16_t	x;
	uint16_t	y;
	double		n;
	int			i;

	if (argc == 2)
	{
		if (reverse_map(strtod(argv[1], NULL), &x, &y))
			printf("(%u, %u)\n", x, y);
		return (0);
	}
	for (i = 0; i < 5; i++)
	{
		n = map(ex[i][0], ex[i][1]);
		reverse_map(n, &x, &y);
		printf("(%u, %u) -> %.17g -> (%u, %u)\n", ex[i][0], ex[i][1], n, x, y);
	}
	return (0);
}

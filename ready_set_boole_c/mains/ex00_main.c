#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex00 [a b]  -- without arguments runs a few examples. */
int	main(int argc, char **argv)
{
	uint32_t	ex[4][2] = {{0, 0}, {1, 2}, {255, 1}, {123456, 654321}};
	int			i;

	if (argc == 3)
	{
		printf("%u\n", adder(strtoul(argv[1], NULL, 10),
				strtoul(argv[2], NULL, 10)));
		return (0);
	}
	for (i = 0; i < 4; i++)
		printf("%u + %u = %u\n", ex[i][0], ex[i][1], adder(ex[i][0], ex[i][1]));
	return (0);
}

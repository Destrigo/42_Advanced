#include <stdio.h>
#include <stdlib.h>
#include "boole.h"

/* Usage: ex02 [n ...]  -- without arguments prints gray_code(0..8). */
int	main(int argc, char **argv)
{
	uint32_t	n;
	int			i;

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			n = strtoul(argv[i], NULL, 10);
			printf("gray_code(%u) = %u\n", n, gray_code(n));
		}
		return (0);
	}
	for (n = 0; n <= 8; n++)
		printf("gray_code(%u) = %u\n", n, gray_code(n));
	return (0);
}

/* Usage: ex14 [fov_degrees ratio near far]
** Prints the projection matrix, then the text to put in the `proj` file of the
** display program (one line per column). */

#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int	main(int argc, char **argv)
{
	float	p[4] = {60.0f, 16.0f / 9.0f, 0.1f, 100.0f};
	t_mat	m;
	t_err	e;
	char	*text;
	int		i;

	if (argc == 5)
		for (i = 0; i < 4; i++)
			p[i] = strtof(argv[i + 1], NULL);
	e = projection(&m, p[0] * 3.14159265358979f / 180.0f, p[1], p[2], p[3]);
	if (e != E_OK)
	{
		fprintf(stderr, "Error: %s\n", err_str(e));
		return (1);
	}
	mat_print(&m);
	text = column_major_text(&m);
	printf("\ncolumn-major (the `proj` file):\n%s", text ? text : "");
	free(text);
	mat_free(&m);
	return (0);
}

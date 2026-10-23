#include "boole.h"

/* Usage: ex04 [formula]  -- defaults to "AB&C|". */
int	main(int argc, char **argv)
{
	print_truth_table(argc > 1 ? argv[1] : "AB&C|");
	return (0);
}

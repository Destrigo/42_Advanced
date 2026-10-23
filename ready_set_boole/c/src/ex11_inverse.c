/*
** Exercise 11 - Inverse function (bonus)
**
** Decode a real in [0, 1) back into the (x, y) point that map() produced, so
** that reverse_map(map(x, y)) == (x, y) and map(reverse_map(n)) == n.
**
** Idea: undo each step of ex10 in reverse order. Multiply by 2^32 to recover
** the 32-bit Z-order index (exact, since n is a multiple of 2^-32), then
** de-interleave it: x is the bits at even positions, y the bits at odd
** positions (the index shifted right by one). Gathering bits uses the same
** magic masks as ex10, applied in the opposite direction.
*/

#include <stdio.h>
#include "boole.h"

/* Inverse of spread() in ex10: gathers the bits at even positions. */
static uint16_t	compact(uint32_t z)
{
	z &= 0x55555555u;
	z = (z | (z >> 1)) & 0x33333333u;
	z = (z | (z >> 2)) & 0x0F0F0F0Fu;
	z = (z | (z >> 4)) & 0x00FF00FFu;
	z = (z | (z >> 8)) & 0x0000FFFFu;
	return ((uint16_t)z);
}

bool	reverse_map(double n, uint16_t *x, uint16_t *y)
{
	uint32_t	z;

	if (!x || !y)
		return (fprintf(stderr, "Error: null output pointer\n"), false);
	*x = 0;
	*y = 0;
	if (!(n >= 0.0 && n < 1.0))
	{
		fprintf(stderr, "Error: %g is outside [0, 1)\n", n);
		return (false);
	}
	z = (uint32_t)(n * 4294967296.0);
	*x = compact(z);
	*y = compact(z >> 1);
	return (true);
}

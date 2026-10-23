/*
** Exercise 10 - Curve (bonus)
**
** Map a point (x, y) in [0, 2^16) x [0, 2^16) to a unique real in [0, 1).
** This is the inverse of a space-filling curve: it stores a 2D space on a
** line.
**
** Idea: Z-order (Lebesgue / Morton) curve. Interleave the bits of x and y:
** bit i of x goes to bit 2i and bit i of y to bit 2i+1 of a 32-bit index z.
** Nearby points tend to get nearby indexes, which is why GPUs use it for
** texture layout. The index is then divided by 2^32 to land in [0, 1). A
** uint32_t fits exactly in the 53-bit mantissa of a double, so no precision
** is lost and the mapping stays bijective.
**
** The bit interleaving is done in O(1) with the classic "magic number"
** masks (spread bits by 8, 4, 2, 1 positions).
*/

#include "boole.h"

/* Spreads the 16 bits of v so that bit i lands on bit 2i. */
static uint32_t	spread(uint16_t v)
{
	uint32_t	x;

	x = v;
	x = (x | (x << 8)) & 0x00FF00FFu;
	x = (x | (x << 4)) & 0x0F0F0F0Fu;
	x = (x | (x << 2)) & 0x33333333u;
	x = (x | (x << 1)) & 0x55555555u;
	return (x);
}

double	map(uint16_t x, uint16_t y)
{
	uint32_t	z;

	z = spread(x) | (spread(y) << 1);
	return ((double)z / 4294967296.0);
}

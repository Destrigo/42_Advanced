/*
** Exercise 01 - Multiplier
**
** Multiply two u32 using only bitwise operators (and the adder from ex00).
**
** Idea: schoolbook binary multiplication. Write b as a sum of powers of two;
** for every bit set in b, add (a << position) to the result. In the loop we
** look at the lowest bit of b, add a if it is 1, then shift a left and b
** right so the next bit is examined.
**
** Complexity: the loop runs at most 32 times (width of b), each step calls
** the adder (at most 32 steps), so the cost is bounded by a constant: O(1)
** time and space for fixed-width integers. Overflow wraps modulo 2^32.
*/

#include "boole.h"

uint32_t	multiplier(uint32_t a, uint32_t b)
{
	uint32_t	result;

	result = 0;
	while (b != 0)
	{
		if (b & 1)
			result = adder(result, a);
		a <<= 1;
		b >>= 1;
	}
	return (result);
}

/*
** Exercise 02 - Gray code
**
** Convert an integer to its Gray code.
**
** A Gray code is an ordering of binary numbers where two consecutive values
** differ by exactly one bit (useful to avoid glitches in encoders and in
** Karnaugh maps). The binary-reflected version is obtained by XOR-ing the
** number with itself shifted right by one: g = n ^ (n >> 1). Each output
** bit is the XOR of two adjacent input bits, so going from n to n+1 changes
** exactly one output bit.
**
** Complexity: O(1).
*/

#include "boole.h"

uint32_t	gray_code(uint32_t n)
{
	return (n ^ (n >> 1));
}

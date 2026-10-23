/*
** Exercise 00 - Adder
**
** Add two u32 using only & | ^ << >>, assignment and comparisons.
**
** Idea: this is how a hardware adder works. XOR adds two bits without
** looking at the carry (1^1 = 0, 1^0 = 1), AND finds the positions where a
** carry is produced (1&1), and shifting that left by one moves each carry to
** the next bit. Adding the carry is the same problem again, so we repeat
** "sum = a ^ b, carry = (a & b) << 1" until the carry is zero.
**
** Complexity: at most 32 iterations (the carry moves up one bit per step),
** i.e. O(log n) in the value of the inputs, O(1) space.
*/

#include "boole.h"

uint32_t	adder(uint32_t a, uint32_t b)
{
	uint32_t	carry;

	while (b != 0)
	{
		carry = (a & b) << 1;
		a ^= b;
		b = carry;
	}
	return (a);
}

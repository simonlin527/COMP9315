// test Bits dt

#include <stdio.h>
#include "defs.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"

int main(int argc, char **argv)
{
	Bits b = newBits(60);
	printf("all 0's showbits: "); showBits(b); printf("\n");
	setBit(b, 5);
	printf("set 5 showbits: "); showBits(b); printf("\n");
	setBit(b, 0);
	setBit(b, 50);
	setBit(b, 59);
	printf("set pos 0, 5, 50, 59: "); showBits(b); printf("\n");
	if (bitIsSet(b,0)) printf("Bit 0 is set\n");
	if (bitIsSet(b,5)) printf("Bit 5 is set\n");
	if (bitIsSet(b,10)) printf("Bit 10 is set\n");
	if (bitIsSet(b,50)) printf("Bit 50 is set\n");
	if (bitIsSet(b,59)) printf("Bit 59 is set\n");
	setAllBits(b);
	printf("set all bits: "); showBits(b); printf("\n");
	unsetBit(b, 40);
	printf("unset pos 40:"); showBits(b); printf("\n");
	if (bitIsSet(b,20)) printf("Bit 20 is set\n");
	if (bitIsSet(b,40)) printf("Bit 40 is set\n");
	unsetAllBits(b);
	printf("unset all bits: "); showBits(b); printf("\n");
	setBit(b, 59);
	printf("set pos 59:"); showBits(b); printf("\n");
	if (bitIsSet(b,59)) printf("Bit 59 is set\n");
	setBit(b, 0);
	setBit(b, 2);
	setBit(b, 4);
	setBit(b, 6);
	setBit(b, 8);
	setBit(b, 10);
	Bits a = newBits(60);
	setBit(a, 1);
	setBit(a, 3);
	setBit(a, 5);
	setBit(a, 7);
	setBit(a, 8);
	setBit(a, 10);

	printf("b:"); showBits(b); printf("\n");
	printf("a:"); showBits(a); printf("\n");
	printf("a,b is subset: %d\n", isSubset(a,b));
	andBits(b, a);
	// orBits(b, a);
	printf("b & a:"); showBits(b); printf("\n");
	Bits c = b;
	printf("c:"); showBits(c); printf("\n");
	printf("a,c is subset: %d\n", isSubset(a,c));
	printf("c,a is subset: %d\n", isSubset(c,a));

	// Bits c = newBits(64);
	// memcpy(c->bitstring, "0000000000000000001000000000010000000001000001100000100000100010", c->nbytes);
	// printf("size: %lu, bitstring c:", sizeof(c->bitstring)); showBits(c); printf("\n");
	// Bits d = newBits(64);
	// memcpy(d->bitstring, "0111000001110010000001001010111000011001001011101010000100010101", d->nbits);
	// printf("c is subset of d: %d\n", isSubset(c,d));

	// Bits a = newBits(12);
	// printf("all 0's showbits: "); showBits(a); printf("\n");
	// setBit(a, 5);
	// printf("set 5 showbits: "); showBits(a); printf("\n");
	// setBit(a, 11);
	// printf("set 5 showbits: "); showBits(a); printf("\n");

	return 0;
}

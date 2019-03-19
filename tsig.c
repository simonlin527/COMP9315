// tsig.c ... functions on Tuple Signatures (tsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <unistd.h>
#include <string.h>
#include <math.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"
#include "tuple.h"

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
	int k, m;
	m = tsigBits(r); // signature size is set as tm in reln
	// otherwise you calculate m like this
	// m = (int)((1/log(2)) * (1/log(2)) * (float)r->params.nattrs * log(1/r->params.pF));
	k = r->params.tk; // k is set in rel->params.tk
	// otherwise you calculate k like this
	// k = (int)(1/log(2) * log(1/r->params.pF));
	Bits desc = newBits(m);
	char **attributes = tupleVals(r, t);
	for (int i=0; i< r->params.nattrs; i++){
		// printf("attribute:%s, ", attributes[i]);
		Bits cw = codeword(attributes[i], m, k);
		// printf("codeword is:"); showBits(cw); printf("\n");
		orBits(desc, cw);
		// printf("desc is:"); showBits(desc); printf("\n");
	}
	// printf("created signature is:"); showBits(desc); printf("\n");
	return desc;
}

//code word function, "borrowed" from the lecture notes and then modified
Bits codeword(char *attr_value, int m, int k)
{
	int  nbits = 0;   // count of set bits
	Bits cword = newBits(m);   // assuming m <= 32 bits
	srandom(hash_any(attr_value, strlen(attr_value)));
	
	if (strcmp(attr_value, "?") != 0){ // if it's a ?, then set all next few bits to 0
		while (nbits < k) {
			int i = random() % m;
			setBit(cword, i);
			nbits++;
		}
	}
	// printf("attr_value:%s, strelen:%lu, cword:", attr_value, strlen(attr_value)); showBits(cword); printf("\n");
	return cword;  // m-bits with k 1-bits and m-k 0-bits
}

// find "matching" pages using tuple signatures
void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
 	//TODO
	Bits querysig = makeTupleSig(q->rel, q->qstring);
	Page p; Bits sig = newBits(tsigBits(q->rel)); PageID pid, i;
	for (pid = 0; pid < nTsigPages(q->rel); pid++) {
		p = getPage(tsigFile(q->rel), pid);
		for (i = 0; i < pageNitems(p); i++){
			getBits(p, i, sig);
			//if query signature is subset of retrieved signature, then match
			if (isSubset(querysig, sig) == TRUE){
				// printf("sig on pid:%d, i:%d, pagenum: %d,sig:", pid, i, q->nsigs/maxTupsPP(q->rel)); showBits(sig); putchar('\n');
				// get the page num for tuple, and set that bit
				setBit(q->pages, q->nsigs/maxTupsPP(q->rel));
			}
			q->nsigs++;
		}
		q->nsigpages++;
	}

 	// The printf below is primarily for debugging
 	// Remove it before submitting this function
// 	printf("Matched Pages: "); showBits(q->pages); putchar('\n');
}

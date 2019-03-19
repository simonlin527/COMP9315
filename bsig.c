// bsig.c ... functions on Tuple Signatures (bsig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q)
{
	// assume we've already had slices, bm bits long and pm pages
	assert(q != NULL);

	// query: pm bits long;
	Bits qsig = makePageSig(q->rel, q->qstring);
	// match bits, initialization: bm bits long; all 1
	Bits matches = newBits(bsigBits(q->rel));
	// Bits bsig_page = newBits(nBsigPages(q->rel));
	setAllBits(matches);
	// For qsig, each bit represents one page, total length = pm
	// so the total time reading bsig would be the number of i in the qsig
	Page p; int pid, pidd;
	pidd = -1;
	for (int i=0; i < psigBits(q->rel); i++){
		// for query, if at location i bit = 1, fetch the bit slice at i
		// showBits(qsig);
		if (bitIsSet(qsig, i)) {
			// fetch slice [i]
			// printf("i: %d,", i);
			Bits slice = newBits(bsigBits(q->rel));
			// printf("nnnnn\n");
			pid = i/maxBsigsPP(q->rel);
			if (pid > pidd) {
				pidd = pidd+1;
				q->nsigpages++;
			}
			p = getPage(bsigFile(q->rel), i/maxBsigsPP(q->rel));
			getBits(p, i%maxBsigsPP(q->rel), slice);
			andBits(matches, slice);
			// set bsig_page bit
			// setBit(bsig_page, i/maxBsigsPP(q->rel));
			q->nsigs++;
		}
	}
	// matches: bm length
	// each match means return 1 page
	for (int j=0; j < nPages(q->rel); j++) {
		if (bitIsSet(matches, j)){
			// copy from psig.c
			// pid = j
			setBit(q->pages, j);
			// Page p; Bits sig = newBits(psigBits(q->rel)); int index = 0;
			// // get a particular page
			// printf("working???\n");
			// printf("j:%d\n", j);
			// p = getPage(psigFile(q->rel), j/maxPsigsPP(q->rel));
			// for (int i = 0; i < pageNitems(p); i++){
			// 	getBits(p, i, sig);
			// 	//if query signature is subset of retrieved signature, then match
			// 	if (isSubset(qsig, sig) == TRUE){
			// 		// printf("sig on pid:%d, i:%d, pagenum: %d,sig:", pid, i, q->nsigs/maxTupsPP(q->rel)); showBits(sig); putchar('\n');
			// 		// get the page num for tuple, and set that bit
			// 		setBit(q->pages, index);
			// 	}
			// 	// q->nsigs++;
			// 	index++;
		}
		// fetch page j and scan the page for matching
	}
}

	//
	//
	// for (int i=0; i< nBsigPages(q->rel); i++){
	// 	if (bitIsSet(bsig_page, i)) q->nsigpages++;
	// }
	// // Then we have the matches (length = bm)
	// // for each bit j in matches, if bit = 1,
	// // fetch page j and scan page j for matching
	// orBits(q->pages, matches);
// }

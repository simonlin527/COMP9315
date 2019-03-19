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
	// assume we've already have slices, bm bits long and pm pages
	assert(q != NULL);

	// query: pm bits long
	Bits qsig = makePageSig(q->rel, q->qstring);
	// match bits, initialization: bm bits long; all 1
	Bits matches = newBits(bsigBits(q->rel));
	Bits bsig_page = newBits(nBsigPages(q->rel));
	setAllBits(matches);
	// For qsig, each bit represents one page, total length = pm
	for (int i=0; i < psigBits(q->rel); i++){
		// for query, if at location i bit = 1, fetch the bit slice at i
		if (bitIsSet(qsig, i)) {
			// fetch slice [i]
			Page p;
			Bits slice = newBits(bsigBits(q->rel));
			p = getPage(bsigFile(q->rel), i/maxBsigsPP(q->rel));
			getBits(p, i%maxBsigsPP(q->rel), slice);
			// matches = matches & slice;
			andBits(matches, slice);
			// set bsig_page bit 
			setBit(bsig_page, i/maxBsigsPP(q->rel));
			q->nsigs++;
		}
	}
	for (int i=0; i< nBsigPages(q->rel); i++){
		if (bitIsSet(bsig_page, i)) q->nsigpages++;
	}
	// Then we have the matches (length = bm)
	// for each bit j in matches, if bit = 1,
	// fetch page j and scan page j for matching
	orBits(q->pages, matches);
}

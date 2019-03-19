// psig.c ... functions on page signatures (psig's)
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "bits.h"
#include "tsig.h"

Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
	// Bits page_sig = newBits(psigBits(r));
	int k, m;
	m = psigBits(r); // signature size is set as tm in reln
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
	return desc; // remove this
}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	//TODO
	Bits querysig = makePageSig(q->rel, q->qstring);
	// printf("querysig is:"); showBits(querysig); printf("\n");
	Page p; Bits sig = newBits(psigBits(q->rel)); PageID pid, i; int index = 0;
	for (pid = 0; pid < nPsigPages(q->rel); pid++) {
		p = getPage(psigFile(q->rel), pid);
		for (i = 0; i < pageNitems(p); i++){
			getBits(p, i, sig);
			//if query signature is subset of retrieved signature, then match
			if (isSubset(querysig, sig) == TRUE){
				// printf("sig on pid:%d, i:%d, pagenum: %d,sig:", pid, i, q->nsigs/maxTupsPP(q->rel)); showBits(sig); putchar('\n');
				// get the page num for tuple, and set that bit
				setBit(q->pages, index);
			}
			q->nsigs++;
			index++;
		}
		q->nsigpages++;
	}
}

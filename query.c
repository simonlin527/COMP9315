// query.c ... query scan functions
// part of SIMC signature files
// Manage creating and using Query objects
// Written by John Shepherd, September 2018

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"
#include "tsig.h"
#include "psig.h"
#include "bsig.h"

// check whether a query is valid for a relation
// e.g. same number of attributes

int checkQuery(Reln r, char *q)
{
	if (*q == '\0') return 0;
	char *c;
	int nattr = 1;
	for (c = q; *c != '\0'; c++)
		if (*c == ',') nattr++;
	return (nattr == nAttrs(r));
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q, char sigs)
{
	Query new = malloc(sizeof(QueryRep));
	assert(new != NULL);
	if (!checkQuery(r,q)) return NULL;
	new->rel = r;
	new->qstring = q;
	new->nsigs = new->nsigpages = 0;
	new->ntuples = new->ntuppages = new->nfalse = 0;
	new->pages = newBits(nPages(r));
	switch (sigs) {
	case 't': findPagesUsingTupSigs(new); break;
	case 'p': findPagesUsingPageSigs(new); break;
	case 'b': findPagesUsingBitSlices(new); break;
	default:  setAllBits(new->pages); break;
	}
	new->curpage = 0;
	return new;
}

// scan through selected pages (q->pages)
// search for matching tuples and show each
// accumulate query stats

void scanAndDisplayMatchingTuples(Query q)
{
	assert(q != NULL);
	//TODO
	// printf("TESTING THIS SHIT\n");
	// printf("name of relation: %s\n", (q->rel)->name);
	// printf("query string:%s\n", q->qstring);

	// open the relation as r
	Reln r = openRelation((q->rel)->name);
	assert(r != NULL);
	// printf("Created new relation\n");

	// turn query string into a tuple for comparison purposes
	// printf("Creating new tuple based on query\n");
	Tuple target = malloc(strlen(q->qstring)+1);
	strcpy(target, q->qstring);
	assert(target != NULL);
	// printf("Created new tuple: %s\n", target);
	// printf("test tuple: "); showTuple(r, target); printf("\n");

	// scan data pages, printing each tuple

	Page p; Tuple t; int pid, i; Bool page_flag;
	for (pid = 0; pid < nPages(r); pid++)
	{
		page_flag = FALSE;
		// If pid is not in matching pages, then we skip that page
		if (bitIsSet(q->pages, pid) == FALSE) continue;
		// Else we retrieve page and compare tuple
		p = getPage(dataFile(r), pid);
		for (i = 0; i < pageNitems(p); i++)
		{
			t = getTupleFromPage(r, p, i);
			q->ntuples++;
			// match tuple
			if (tupleMatch(r, target, t) == TRUE)
			{
				page_flag = TRUE;
// 				printf("Found in page: %d, ", pid);
				showTuple(r, t);
			}
		}
		if (page_flag == FALSE) q->nfalse++; // no match found in that page
		q->ntuppages++;
	}

	//clean up
	free(target);
	closeRelation(r);
	// closeQuery(q); // don't do this, cos query is still needed
}

// print statistics on query

void queryStats(Query q)
{
	printf("# signatures read:   %d\n", q->nsigs);
	printf("# sig pages read:    %d\n", q->nsigpages);
	printf("# tuples examined:   %d\n", q->ntuples);
	printf("# data pages read:   %d\n", q->ntuppages);
	printf("# false match pages: %d\n", q->nfalse);
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q->pages);
	free(q);
}

// reln.c ... functions on Relations
// part of SIMC signature files
// Written by John Shepherd, September 2018

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "reln.h"
#include "page.h"
#include "tuple.h"
#include "tsig.h"
#include "psig.h"
#include "bits.h"
#include "hash.h"
// open a file with a specified suffix
// - always open for both reading and writing

File openFile(char *name, char *suffix)
{
	char fname[MAXFILENAME];
	sprintf(fname,"%s.%s",name,suffix);
	File f = open(fname,O_RDWR|O_CREAT,0644);
	assert(f >= 0);
	return f;
}

// create a new relation (five files)
// data file has one empty data page

Status newRelation(char *name, Count nattrs, float pF,
                   Count tk, Count tm, Count pm, Count bm)
{
	Reln r = malloc(sizeof(RelnRep));
	RelnParams *p = &(r->params);
	assert(r != NULL);
	// r->name = malloc(strlen(name)+1);
	strcpy(r->name, name);
	// r->name = name;
	// printf("NAME: %s\n", r->name);
	assert(r->name != NULL);

	p->nattrs = nattrs;
	p->pF = pF,
	p->tupsize = 28 + 7*(nattrs-2);
	Count available = (PAGESIZE-sizeof(Count));
	p->tupPP = available/p->tupsize;
	p->tk = tk;
	if (tm%8 > 0) tm += 8-(tm%8); // round up to byte size
	p->tm = tm; p->tsigSize = tm/8; p->tsigPP = available/(tm/8);
	if (pm%8 > 0) pm += 8-(pm%8); // round up to byte size
	p->pm = pm; p->psigSize = pm/8; p->psigPP = available/(pm/8);
	if (p->psigPP < 2) { free(r); return -1; }
	if (bm%8 > 0) bm += 8-(bm%8); // round up to byte size
	p->bm = bm; p->bsigSize = bm/8; p->bsigPP = available/(bm/8);
	if (p->bsigPP < 2) { free(r); return -1; }
	r->infof = openFile(name,"info");
	r->dataf = openFile(name,"data");
	r->tsigf = openFile(name,"tsig");
	r->psigf = openFile(name,"psig");
	r->bsigf = openFile(name,"bsig");
	addPage(r->dataf); p->npages = 1; p->ntups = 0;
	addPage(r->tsigf); p->tsigNpages = 1; p->ntsigs = 0;
	addPage(r->psigf); p->psigNpages = 1; p->npsigs = 0;
	addPage(r->bsigf); p->bsigNpages = 1; p->nbsigs = 0; // replace this // No need?
	// Create a file containing "pm" all-zeroes bit-strings,
    // each of which has length "bm" bits
	PageID pid; Page page;
	for (int i=0; i<pm; i++){
		Bits bitslice = newBits(bm);
		// add sig to last page
		pid = p->bsigNpages-1;
		page = getPage(r->bsigf, pid);
		// check if room on last page; if not add new page
		if (pageNitems(page) == maxBsigsPP(r)) {
			addPage(r->bsigf);
			p->bsigNpages++;
			pid++;
			free(page);
			page = newPage();
			if (page == NULL) return NO_PAGE;
		}
		putBits(page, pageNitems(page), bitslice); // add signature to page
		p->nbsigs++;  // written to disk in closeRelation()
		putPage(r->bsigf, pid, page);
		//clean up
		freeBits(bitslice);
	}

	// Clean up
	closeRelation(r);
	return 0;
}

// check whether a relation already exists

Bool existsRelation(char *name)
{
	char fname[MAXFILENAME];
	sprintf(fname,"%s.info",name);
	File f = open(fname,O_RDONLY);
	if (f < 0)
		return FALSE;
	else {
		close(f);
		return TRUE;
	}
}

// set up a relation descriptor from relation name
// open files, reads information from rel.info

Reln openRelation(char *name)
{
	Reln r = malloc(sizeof(RelnRep));
	assert(r != NULL);
	// r->name = name;
	strcpy(r->name, name);
	r->infof = openFile(name,"info");
	r->dataf = openFile(name,"data");
	r->tsigf = openFile(name,"tsig");
	r->psigf = openFile(name,"psig");
	r->bsigf = openFile(name,"bsig");
	read(r->infof, &(r->params), sizeof(RelnParams));
	return r;
}

// release files and descriptor for an open relation
// copy latest information to .info file
// note: we don't write ChoiceVector since it doesn't change

void closeRelation(Reln r)
{
	// make sure updated global data is put in info file
	lseek(r->infof, 0, SEEK_SET);
	int n = write(r->infof, &(r->params), sizeof(RelnParams));
	assert(n == sizeof(RelnParams));
	close(r->infof); close(r->dataf);
	close(r->tsigf); close(r->psigf); close(r->bsigf);
	free(r);
}

// insert a new tuple into a relation
// returns page where inserted
// returns NO_PAGE if insert fails completely

PageID addToRelation(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL && strlen(t) == tupSize(r));
	Page p;  PageID pid;
	RelnParams *rp = &(r->params);

	// add tuple to last page
	pid = rp->npages-1;
	p = getPage(r->dataf, pid);
	// check if room on last page; if not add new page
	if (pageNitems(p) == rp->tupPP) {
		addPage(r->dataf);
		rp->npages++;
		pid++;
		free(p);
		p = newPage();
		if (p == NULL) return NO_PAGE;
	}
	addTupleToPage(r, p, t);
	rp->ntups++;  //written to disk in closeRelation()
	putPage(r->dataf, pid, p);


	// compute tuple signature and add to tsigf
	//TODO
	Bits sig = makeTupleSig(r, t);
	// add sig to last page
	pid = rp->tsigNpages-1;
	p = getPage(r->tsigf, pid);
	// check if room on last page; if not add new page
	if (pageNitems(p) == rp->tsigPP) {
		addPage(r->tsigf);
		rp->tsigNpages++;
		pid++;
		free(p);
		p = newPage();
		if (p == NULL) return NO_PAGE;
	}
	putBits(p, pageNitems(p), sig); // add signature to page
	rp->ntsigs++;  // written to disk in closeRelation()
	putPage(r->tsigf, pid, p);
	//clean up
	freeBits(sig);


	// compute page signature and add to psigf
	Bits psig = makePageSig(r, t);
	// add sig to last page
	pid = rp->psigNpages-1;
	p = getPage(r->psigf, pid);
	// remember page sig index from tuple page ID
	PageID index = pageNitems(p)-1;
	if (nPsigs(r) == nPages(r)){
		// retrieve signature and do orBits(retrieved, psig)
		Bits ret = newBits(psigBits(r));
		getBits(p, index, ret);
		orBits(ret, psig);
		// putBits adds one fucking item so we use editBits
		// which is just putBits but without the addItem function in it lmao
		editBits(p, pageNitems(p)-1, ret);
		putPage(r->psigf, pid, p);
		freeBits(ret);
	}
	else{
		// add new page sig to psigf
		// check if room on last page; if not add new page
		if (pageNitems(p) == rp->psigPP) {
			addPage(r->psigf);
			rp->psigNpages++;
			pid++;
			free(p);
			p = newPage();
			if (p == NULL) return NO_PAGE;
		}
		//Read signature based on tuple page ID
		putBits(p, pageNitems(p), psig); // add signature to page
		rp->npsigs++;
		putPage(r->psigf, pid, p);
	}


	// printf("psig: "); showBits(psig); printf("\n");
	// use page signature to update bit-slices
	pid = nPsigs(r)-1;
	for (int i = 0; i < psigBits(r); i++){
		if (bitIsSet(psig, i)==TRUE){
			// Slice = i'th slice from bsig
			Bits slice = newBits(bsigBits(r));
			// Get bsig page num => i / maxbisgPP
			p = getPage(r->bsigf, i/maxBsigsPP(r));
			getBits(p, i%maxBsigsPP(r), slice);
			// set pid'th bit in bit slice
			setBit(slice, pid);
			// put bit slice back where we got it
			editBits(p, i%maxBsigsPP(r), slice);
			putPage(r->bsigf, i/maxBsigsPP(r), p);
			// printf("i: %d, slice: ", i); showBits(slice); printf("\n");
			freeBits(slice);
		}
	}

	//Cleanup
	freeBits(psig);

	return nPages(r)-1;
}

// displays info about open Reln (for debugging)

void relationStats(Reln r)
{
	RelnParams *p = &(r->params);
	printf("Global Info:\n");
	printf("Dynamic:\n");
    printf("  #items:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
			p->ntups, p->ntsigs, p->npsigs, p->nbsigs);
    printf("  #pages:  tuples: %d  tsigs: %d  psigs: %d  bsigs: %d\n",
			p->npages, p->tsigNpages, p->psigNpages, p->bsigNpages);
	printf("Static:\n");
    printf("  tups   #attrs: %d  size: %d bytes  max/page: %d\n",
			p->nattrs, p->tupsize, p->tupPP);
	printf("  sigs   bits/attr: %d\n", p->tk);
	printf("  tsigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->tm, p->tsigSize, p->tsigPP);
	printf("  psigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->pm, p->psigSize, p->psigPP);
	printf("  bsigs  size: %d bits (%d bytes)  max/page: %d\n",
			p->bm, p->bsigSize, p->bsigPP);
}

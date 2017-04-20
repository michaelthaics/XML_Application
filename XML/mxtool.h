/********
mxtool.h -- header file for mxtool.c
Last updated:  Oct 9th, 2014

Created: Oct 2nd, 2014 (copied from CIS2750 Assignment 2 Files)
Author: Michael Thai #0808957
Contact: mthai@mail.uoguelph.ca
********/
#ifndef MXTOOL_H_
#define MXTOOL_H_ 1

#include "mxutil.h"


/* command execution functions

   Return values: EXIT_SUCCESS or EXIT_FAILURE (stdlib.h) */

int review( const XmElem *top, FILE *outfile );

int concat( const XmElem *top1, const XmElem *top2, FILE *outfile );

enum SELECTOR { KEEP, DISCARD };
int selects( const XmElem *top, const enum SELECTOR sel, const char *pattern, FILE *outfile );

int libFormat( const XmElem *top, FILE *outfile );

int bibFormat( const XmElem *top, FILE *outfile );


/* helper functions (will be tested individually) */

int match( const char *data, const char *regex );

typedef char *BibData[4];
enum BIBFIELD { AUTHOR=0, TITLE, PUBINFO, CALLNUM };
void marc2bib( const XmElem *mrec, BibData bdata );

void sortRecs( XmElem *collection, const char *keys[] );

#endif

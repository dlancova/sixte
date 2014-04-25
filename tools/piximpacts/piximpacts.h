/*
   This file is part of SIXTE.

   SIXTE is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   any later version.

   SIXTE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   For a copy of the GNU General Public License see
   <http://www.gnu.org/licenses/>.


   Copyright 2014 Thorsten Brand, FAU
*/

#ifndef PIXIMP_H
#define PIXIMP_H 1

#include "sixt.h"
#include "gti.h"
#include "impactfile.h"
#include "advdet.h"
#include "pixelimpactfile.h"

#define TOOLSUB piximpacts_main
#include "headas_main.c"

////////////////////////////////////////////////////////////////////////
// Type declarations.
////////////////////////////////////////////////////////////////////////


struct Parameters {
  char ImpactList[MAXFILENAME];
  char PixImpList[MAXFILENAME];
  char XMLFile[MAXFILENAME];
  
  char clobber;
};


////////////////////////////////////////////////////////////////////////
// Function declarations.
////////////////////////////////////////////////////////////////////////


// Reads the program parameters using PIL
int getpar(struct Parameters* const parameters);

// Reads standard sixt keywords
void sixt_read_fits_stdkeywords(fitsfile* const ifptr,
			       char* const telescop,
			       char* const instrume,
			       char* const filter,
			       char* const ancrfile,
			       char* const respfile,
			       double *mjdref,
			       double *timezero,
			       double *tstart,
			       double *tstop, 
			       int* const status);

#endif /* PIXIMP_H */
#ifndef IMPACTLISTFILE_H
#define IMPACTLISTFILE_H 1

#include "sixt.h"
#include "impact.h"
#include "point.h"


/////////////////////////////////////////////////////////////////
// Type Declarations.
/////////////////////////////////////////////////////////////////


typedef struct {
  /** Pointer to the FITS file. */
  fitsfile* fptr;
  
  /** Total number of rows in the FITS file. */
  long nrows;

  /** Number of the current row in the FITS file. The numbering
      starts at 1 for the first line. If row is equal to 0, no row
      has been read or written so far. */
  long row;

  /** Column numbers in the FITS binary table. */
  int ctime, cenergy, cx, cy, cph_id, csrc_id;

} ImpactListFile;


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


/** Constructor. Returns a pointer to an empty ImpactListFile data
    structure. */
ImpactListFile* newImpactListFile(int* const status);

/** Destructor. */
void freeImpactListFile(ImpactListFile** const file, int* const status);

/** Open an existing ImpactListFile. */
ImpactListFile* openImpactListFile(const char* const filename,
				   const int mode, int* const status);

/** Create and open a new ImpactListFile. The new file is generated
    according to the specified template. */
ImpactListFile* openNewImpactListFile(const char* const filename,
				      const char clobber,
				      int* const status);

/** Return the next impact from the file. Increments the internal row
    counter by 1 (e.g. if 'row==0' at the beginning of the function
    call, the first row from the FITS table is read and the counter is
    increased to 'row==1'). */
void getNextImpactFromFile(ImpactListFile* const file, 
			   Impact* const impact, 
			   int* const status);

/** Append a new entry to the ImpactListFile. */
void addImpact2File(ImpactListFile* const ilf, 
		    Impact* const impact, 
		    int* const status);


#endif /* IMPACTLISTFILE_H */
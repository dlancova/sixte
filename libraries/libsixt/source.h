#ifndef SOURCE_H
#define SOURCE_H 1

#include "sixt.h"

#include "linkedpholist.h"
#include "photon.h"


/////////////////////////////////////////////////////////////////
// Type Declarations.
/////////////////////////////////////////////////////////////////


/** Photon energy spectrum of an X-ray source. */
typedef struct {

  /** Coordinates of source position. */
  double ra, dec;

  /** Source extension [rad]. */
  float extension;

  /** Row number of the source in the SimputCatalog. Numbering starts
      at line 1. */
  long row;

  /** Time of the emission of the last photon. */
  double* t_next_photon;

} Source;


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


/** Constructor. */
Source* newSource(int* const status);

/** Destructor. */
void freeSource(Source** const src);

/** Create photons for a particular source in the specified time
    interval. */
LinkedPhoListElement* getXRayPhotons(Source* const src, 
				     SimputCatalog* const simput,
				     const double t0, const double t1,
				     const double mjdref,
				     int* const status);

/** Sort the list of Source objects with the specified number of
    entries with respect to the requested coordinate axis using a
    quick sort algorithm. */
void quicksortSources(Source* const list, const long left, 
		      const long right, const int axis);


#endif /* SOURCE_H */

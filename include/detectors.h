#ifndef DETECTORS_H
#define DETECTORS_H (1)


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

// The GNU Scientific Library Errorfunction is used to calculate charge 
// distribution of split events (assuming a Gaussian shape for the carge cloud).
#include <gsl/gsl_sf_erf.h>

// HEAdas header files
#include "pil.h"
#include "headas.h"
#include "headas_error.h"
#include "fitsio.h"


////////////////////////////////////////////////////////


#define INVALID_PIXEL (-1)   // flags an invalid pixel
#define HTRS_N_PIXELS (37)   // Total number of pixels in the HTRS array


int n_events, n_dead, n_interframe, n_outside;


#include "detectors.types.h"

#include "eventlist.h"

#include "detectors.def.h"


#endif  /* DETECTORS_H */

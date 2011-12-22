#ifndef PHGEN_H
#define PHGEN_H 1

#include "sixt.h"
#include "attitudecatalog.h"
#include "gendet.h"
#include "photon.h"
#include "sourcecatalog.h"


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


int phgen(AttitudeCatalog* const ac,
	  SourceCatalog* const srccat,
	  const double t0, 
	  const double exposure,
	  const double mjdref,
	  const float fov,
	  Photon* const ph,
	  int* const status);


#endif /* PHGEN_H */

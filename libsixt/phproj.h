#ifndef PHPROJ_H
#define PHPROJ_H 1

#include "sixt.h"
#include "attitudecatalog.h"
#include "pattern.h"
#include "patternfile.h"
#include "gendet.h"
#include "point.h"
#include "vector.h"


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


void phproj(GenDet* const det,
	    AttitudeCatalog* const ac,
	    PatternFile* const plf,
	    const double t0,
	    const double exposure,
	    int* const status);


#endif /* PHPROJ_H */
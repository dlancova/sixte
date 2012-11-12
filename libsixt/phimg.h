#ifndef PHIMG_H
#define PHIMG_H 1

#include "sixt.h"
#include "attitudecatalog.h"
#include "check_fov.h"
#include "gendet.h"
#include "photon.h"
#include "impact.h"


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


int phimg(const GenDet* const det,
	  AttitudeCatalog* const ac,
	  Photon* const ph,
	  Impact* const imp,
	  int* const status);


#endif /* PHIMG_H */
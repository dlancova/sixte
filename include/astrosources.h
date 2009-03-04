/** 
 * Contains all FUNCTION declarations needed to handle source catalogs.
 * The variable and TYPE definitions can be found in 'astrosources.types.h'.
 */

#ifndef ASTROSOURCES_H
#define ASTROSOURCES_H 1

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include "fitsio.h"
#include "headas.h"
#include "headas_error.h"

#include "fits_pha.h"
#include "fits_ctlg.h"
#include "spectrum.h"
#include "detectors.h"
#include "psf.h"
#include "vector.h"


#include "astrosources.types.h"



// Function loads the required source catalogs from the specified FITS files and
// stores the source data in an array. Additionally it allocates the memory for the
// preselected catalog.
int get_source_catalogs(struct source_cat_entry **selected_catalog, 
			const int n_sourcefiles, fitsfile **sourcefiles,
			int columns[5][3], 
			char source_filename[MAX_NSOURCEFILES][FILENAME_LENGTH]);


// Releases the memory which has been allocated to store 
// the source catalogs (all sources and preselected).
void free_source_catalogs(fitsfile **sourcefiles, const int n_sourcefiles,
			  struct source_cat_entry **selected_catalog, int *status);


// Get the preselected source catalogs with sources along the path of the 
// telescope axis over the sky.
int get_preselected_catalog(struct source_cat_entry *selected_catalog, 
			    long *nsources, const int n_sourcefiles,
			    fitsfile **sourcefiles, int columns[5][3], 
			    struct vector telescope_direction,
			    const double pre_max_align, struct Spectrum_Store, 
			    const int Nspectra);




// Function opens the specified point-source catalog files.
PointSourceFiles* get_PointSourceFiles(int* status, int nfiles, ...);
// Close the point.source catalog files.
void free_PointSourceFiles(PointSourceFiles* psf, int* status);


#endif /* ASTROSOURCES_H */


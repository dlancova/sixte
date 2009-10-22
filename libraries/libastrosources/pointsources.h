#ifndef POINTSOURCES_H
#define POINTSOURCES_H (1)

#include "sixt.h"
#include "spectrum.h"
#include "vector.h"
#include "linlightcurve.h"


// Maximum number of sources in the preselected source catalog:
#define MAX_N_POINTSOURCES 1000000


/** Contains all data to specify the properties of a point source. */
typedef struct {
  float ra, dec; /**< Right ascension and declination of the source [rad]. */

  /** Average photon rate [photons/s]. */
  float rate; 
  /** Type of the light curve particular for this PointSource.
   * There are different possible values:
   * T_LC_CONSTANT (=0) means a constant light curve.
   * T_LC_TIMMER_KOENIG (=-1) means a light curve with red noise according 
   * to Timmer & Koenig (1995).
   * Positive values can be used to designate a particular FITS file containing 
   * a light curve. */
  long lc_type;
  /** Pointer to object with Piece-wise linear light curve for this X-ray source. */
  LinLightCurve* lc; 

  /** Index of the source spectrum within the SpectrumStore.
   * This number represents the index of the source spectrum within the SpectrumStore
   * of the PointSourceCatalog. 
   * Warning: the spectrum_index starts  at 1 (not at 0). */
  long spectrum_index; 

  Spectrum *spectrum; /**< Pointer to source spectrum. */

  /** Time of last photon created for this source. */
  double t_last_photon; 

} PointSource;

/** Contains all PointSources from one and the same FITS file. 
 * This data structure is generated from the sources sorted out of the 
 * PointSourceFile objects in the PointSourceFileCatalog. */
typedef struct {
  /** Number of PointSource objects contained in the PointSourceCatalog. */
  long nsources;
  /** Array containing the individual PointSource objects. 
   * Length of the array is nsources. */
  PointSource* sources;
} PointSourceCatalog;



/** PointSourceFile containing information about X-ray point sources.
 * Provides information about a FITS file with point sources. */
typedef struct {
  fitsfile* fptr;

  long nrows; /**< Number of rows / PointSources in the FITS table. */

  /** Column names of the point source specific data columns in the FITS table. 
   * If a column is not available in the FITS file, the corresponding variable is 0. */
  int cra, cdec, crate, cspectrum, clightcurve;

  /** SpectrumStore containing the spectra that are used in this PointSourceCatalog. */
  SpectrumStore spectrumstore;
} PointSourceFile;


/** Collection of point source FITS files.
 * Used as input for the point source selection algorithm. */
typedef struct {
  int nfiles;
  PointSourceFile** files;
} PointSourceFileCatalog;


/////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////


/** Constructor. */
PointSourceFileCatalog* get_PointSourceFileCatalog();
/** Destructor. */
void free_PointSourceFileCatalog(PointSourceFileCatalog*);

/** Constructor returning a pointer to an empty PointSourceFile object. */
PointSourceFile* get_PointSourceFile();
/** Enhanced constructor specifying a source file and the right HDU number to be loaded. */
PointSourceFile* get_PointSourceFile_fromFile(char* filename, int hdu, int* status);
/** Destructor. */
void free_PointSourceFile(PointSourceFile*);

/** Selects point sources along the path of the telescope axis over the sky
 * and returns a PointSourceCatalog containing the individual PointSource objects. 
 * The selection is done based on a normal vector perpendicular to the scan plane and
 * the cos(sin?)-value of the maximum angle between the source direction and the scan plane.
 * For each selected source a Spectrum and a Lightcurve are assigned. */
PointSourceCatalog* get_PointSourceCatalog(PointSourceFileCatalog*, Vector normal_vector, 
					   const double max_align, int* status);
/** Destructor. */
void free_PointSourceCatalog(PointSourceCatalog* psc);

/** Reads a table from a FITS source catalogue. 
 * The right ascension and declination of the return PointSource structure are 
 * given in [rad]. */
int get_PointSourceTable_Row(PointSourceFile*, long row, PointSource*, int* status);

/** Add a new PointSourceFile object to the PointSourceFileCatalog.
 * The function takes care of allocating enough memory,
 * loads the FITS HDU specified by the filename and the HDU number, and
 * adds it to the catalog.
 * The return value is the error status.
 */
int addPointSourceFile2Catalog(PointSourceFileCatalog* psfc, char* filename, 
			       int sources_hdu);


#endif /* POINTSOURCES_H */


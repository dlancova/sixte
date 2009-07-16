#ifndef DETECTORS_DEF_H
#define DETECTORS_DEF_H (1)

#include "photon.h"
#include "point.h"

#include "detectors.types.h"


// This routine is called after each photon event. It takes care of the detector 
// action. I.e. if the integration time is over, it reads out the detector array 
// and clears the pixels. It can also perform the readout of individual detector 
// lines, depending on the detector model (e.g. DEPFET for WFI in contrast to 
// framestore for eROSITA). If the exposure time is not exceeded it simply does 
// nothing.
//void detector_action(Detector*, double time, struct Eventlist_File*, int* status);

/*inline void tes_detector_action(void*, double time, struct Eventlist_File*, 
				int *status); 
inline void htrs_detector_action(void*, double time, 
struct Eventlist_File*, int *status); */


/** This routine clears the entire detector pixel array 
 * (i.e., all created charges are removed, e.g., after read out). */
inline void clear_detector(Detector*);


/** This routine clears a particular line of the detector pixel array 
 * (i.e., all created charges are removed, e.g., after read out). */
inline void clear_detector_line(Detector*, int line);



/** Get the PHA channel that corresponds to a particular charge 
 * according to the EBOUNDS table. */
long get_channel(float, Detector*);

/** Get the charge corresponding to a particular PHA channel according to 
 * the EBOUNDS table. */
float get_energy(long, Detector*);


/** Constructor. Allocates memory for the general detector data structure, but
 * not for the detector-specific elements that are different for the individual
 * detector types. */
Detector* get_Detector(int*);
/** Constructor. Allocates memory for the pixel array. */
int get_DetectorPixels(Detector*, int*);
// Destructor: function releases memory of detector.
//void free_Detector(Detector* detector); // TODO



// Load the detector response MATRIX and the EBOUNDS table from 
// a RMF FITS file and assign them to the detector data structure.
int detector_assign_rsp(Detector *, char *);

// Load the detector response matrix from the given RMF file.
//int get_rmf(Detector *, char* rmf_name);
// Returns a detector PHA channel for the given photon energy according to the RMF.
// Caution: This PHA channel doesn't have to be equivalent to the photon energy. 
// Depending on the detector redistribution matrix the energy can result in one 
// of several possible PHA channels with certain probability.
// If the energy is above the highest available energy bin in the RMF, the 
// return value is "-1".
//long detector_rmf(float energy, mRMF*);
// Release memory of detector response matrix.
//void free_rmf(mRMF *);


// This function returns '1', if the specified detector pixel is active at 
// the time 'time'. If the pixel is, e.g., cleared at this time it cannot 
// receive a charge. In that case the function returns '0'.
int detector_active(int x, int y, Detector*, double time);
int htrs_detector_active(int x, int y, Detector*, double time);


// This function determines the integer pixel coordinates for a given 
// 2D floating point. The point lies within the hexagonal HTRS pixel.
int htrs_get_pixel(Detector*, struct Point2d, 
		   int* x, int* y, double* fraction);


// This routine performs the initialization of the HTRS
// detector. The return value is the error status.
int htrs_init_Detector(Detector*);
// Destructor: release all dynamically allocated memory in the HTRS
// detector structure.
void htrs_free_Detector(Detector*);


// Determines the pixel coordinates for a given point on a 2D array of 
// square pixels. The function returns all pixels that are affected due 
// to a splitting of the charge cloud. The coordinates of the affected 
// pixels are stored in the x[] and y[] arrays, the charge fraction in 
// each pixel in fraction[]. The number of totally affected 
// pixels is given by the function's return value. 
int get_pixel_square(Detector*, struct Point2d, 
		     int* x, int* y, double* fraction);


/*
// Get memory for detector EBOUNDS matrix and fill it with data from FITS file.
int get_ebounds(Ebounds*, int *Nchannels, const char filename[]);
// Release memory of detector EBOUNDS.
void free_ebounds(Ebounds *);
*/


#endif  /*  DETECTORS_DEF_H */


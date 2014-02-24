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


   Copyright 2007-2014 Christian Schmid, FAU
*/

#ifndef SQUARE_PIXELS_H
#define SQUARE_PIXELS_H 1

#include "sixt.h"
#include "point.h"
#include "genericdetector.h"
#include "gaussianchargecloud.h"
#include "exponentialchargecloud.h"


////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////


/** One square detector pixel. */
typedef struct {
  /** Charge stored in this detector pixel (actually the photon energy
      in [keV]). */
  float charge;

  /** Flag indicating whether the event pattern is valid (value "1", generated by
      a single photon) or not (value "-1"). */
  int valid_flag;

} SquarePixel;


/** Data specific for detectos with an array of square pixels. */
typedef struct {
  /** Width of the detector pixel array in x-direction (number of
     [integer pixels]). */
  int xwidth; 
  /** Width of the detector pixel array in y-direction (number of
      [integer pixels]). */
  int ywidth; 

  /** Offset of the detector pixel array in x-direction in [integer
      pixels]. The physical origin of the detector (at the center of
      the detector) has the array x-index 'xoffset'. */
  int xoffset; 
  /** Offset of the detector pixel array in y-direction in [integer
      pixels]. The physical origin of the detector (at the center of
      the detector) has the array y-index 'yoffset'. */
  int yoffset; 

  /** Width of a single pixel in x-direction [m]. */
  double xpixelwidth; 
  /** Width of a single pixel in y-direction [m]. */
  double ypixelwidth; 

  /** Array of square pixels. The charge generated by incident
      photons is stored in these pixels in order to be read out
      afterwards. The dimensions of the array are xwidth x ywidth. */
  SquarePixel** array;

  /** Flags for the individual detector lines whether they have to be
      read out. If no pixel within a line is affected after a
      read-out process, the line can be neglected in the following
      read-out, as there are no charges to be detected.  This array
      contains an entry for each detector line, which is set to 0, if
      no pixel in the line contains charge, and is set to 1, if any
      pixel contains charge. The length of the array is ywidth. */
  int* line2readout;
  double DCU_length, DCU_gap, DCA_gap;

} SquarePixels;


struct SquarePixelsParameters {
  int xwidth, ywidth;
  double DCU_length, DCU_gap, DCA_gap;
  double xpixelwidth, ypixelwidth;
};


/////////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////////


/** Constructor for the SquarePixels data structure. */
SquarePixels* newSquarePixels(struct SquarePixelsParameters* spp, int* const status);

/** Destructor of the SquarePixels data structure. E.g. release
    allocated memory. */
void destroySquarePixels(SquarePixels** const sp);

/** Clear the array of SquarePixels. */
void clearSquarePixels(SquarePixels* const sp);

/** Clear on line of Pixels. A line is defined to have constant
    detector x-coordinate. */
void clearLineSquarePixels(SquarePixels* const sp, const int line);

/** Determine the split ratios among neighboring pixels for a photon
    impact on an array of square pixels. The charge cloud is assumed
    to have a Gaussian shape. If the main event lies inside the
    detector, the function return value is between 1 and 4, although
    some of the split partners may be invalid pixels nevertheless. If
    the main event lies outside the detector, the function return
    value is 0. */
int getSquarePixelsGaussianSplits(SquarePixels* sp, 
				  GaussianChargeCloud* gcc, 
				  struct Point2d position, 
				  int* x, int* y, double* fraction);

/** Determine the split ratios among neighboring pixels for a photon
    impact on an array of square pixels. The charge distribution is
    determined by an exponential model proposed by Konrad Dennerl. If
    the main event lies inside the detector, the function return value
    is between 1 and 4, although some of the split partners may be
    invalid pixels nevertheless. If the main event lies outside the
    detector, the function return value is 0. */
int getSquarePixelsExponentialSplits(SquarePixels* sp, 
				     ExponentialChargeCloud* ecc, 
				     struct Point2d position, 
				     int* x, int* y, double* fraction);

/** Determine the pixel that is hit by a photon impact. The affected
    pixel is returned by the x and y parameters (pixel index starting at
    0). The return value is 1, if a valid pixel is hit. Otherwise it
    is zero. In the latter case the pixel coordinates x and y are
    undefined. */
int getSquarePixel(SquarePixels* sp, struct Point2d position, int* x, int* y);


/** Update the valid flag for a newly created event pattern. */
void SPupdateValidFlag(SquarePixels* sp, int* x, int* y, int nsplits);

/** Set the valid flag of a particular pixel to INVALID (-1) and also
    do this for all surrounding neighbors with a valid flag > 0. */
void SPsetInvalidFlag(SquarePixels* sp, int x, int y);

/** Add a charge [keV] (!) to a pixel in the SquarePixel array. Check
    if the pixel is a valid pixel. If not simply do nothing. */
void SPaddCharge(SquarePixels* sp, int x, int y, float charge);


#endif /* SQUARE_PIXELS_H */


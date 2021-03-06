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


   Copyright 2007-2014 Christian Schmid, Mirjam Oertel, FAU
   Copyright 2015-2019 Remeis-Sternwarte, Friedrich-Alexander-Universitaet
                       Erlangen-Nuernberg
*/

#include "comadetector.h"


CoMaDetector* getCoMaDetector(struct CoMaDetectorParameters* parameters,
			      int* status)
{
  // Allocate memory for the CoMaDetector object.
  CoMaDetector* det=(CoMaDetector*)malloc(sizeof(CoMaDetector));
  if (NULL==det) {
    *status=EXIT_FAILURE;
    SIXT_ERROR("could not allocate memory for CoMaDetector");
    return(det);
  }

  // Set initial values.
  det->pixels   =NULL;
  det->eventfile=NULL;

  // Call the initialization routines of the underlying data structures.
  det->pixels=newSquarePixels(&parameters->pixels, status);
  CHECK_STATUS_RET(*status, det);

  // Create and open new event list file.
  det->eventfile=openNewCoMaEventFile(parameters->eventfile_filename,
				      parameters->eventfile_template,
				      status);
  CHECK_STATUS_RET(*status, det);

  return(det);
}


void freeCoMaDetector(CoMaDetector* det)
{
  if (NULL!=det) {
    // Call the clean-up routines of the underlying data structures.
    destroySquarePixels(&det->pixels);
    closeCoMaEventFile(det->eventfile);
    // Free the allocated memory.
    free(det);
  }
}


int addImpact2CoMaDetector(CoMaDetector* det, Impact* impact)
{
  int status=EXIT_SUCCESS;

  // Determine the affected pixel.
  int x, y; // Detector RAWX and RAWY.
  if (getSquarePixel(det->pixels, impact->position, &x, &y)>0) {
    CoMaEvent event={.time  =impact->time,
		     .charge=impact->energy,
		     .rawx  =x,
		     .rawy  =y };

    // Add event to event file.
    status=addCoMaEvent2File(det->eventfile, &event);
    if (EXIT_SUCCESS!=status) return(status);
  }

  return(status);
}

int addImpact2CoMaDetector_protoMirax(CoMaDetector* det, Impact* impact)
{
  int status=EXIT_SUCCESS;

  // Determine the affected pixel.
  int x, y; // Detector RAWX and RAWY.
  if (getSquarePixel_protoMirax(det->pixels, impact->position, &x, &y)>0) {
    CoMaEvent event={.time  =impact->time,
		     .charge=impact->energy,
		     .rawx  =x,
		     .rawy  =y };

    // Add event to event file.
    status=addCoMaEvent2File(det->eventfile, &event);
    if (EXIT_SUCCESS!=status) return(status);
  }

  return(status);
}

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


   Copyright 2014 Thorsten Brand, FAU
   Copyright 2015-2019 Remeis-Sternwarte, Friedrich-Alexander-Universitaet
                       Erlangen-Nuernberg
*/

#include "piximpacts.h"


////////////////////////////////////
/** Main procedure. */
int piximpacts_main() {

  // Containing all programm parameters read by PIL.
  struct Parameters par;

  // Detector data structure.
  AdvDet *det=NULL;

  // Input impact list.
  ImpactFile* ilf=NULL;

  // Pixel impact list
  PixImpFile* plf=NULL;

  // Error status.
  int status=EXIT_SUCCESS;

  // Keywords

  char telescop[MAXMSG];
  char instrume[MAXMSG];
  char filter[MAXMSG];
  char ancrfile[MAXMSG];
  char respfile[MAXMSG];
  double mjdref;
  double timezero;
  double tstart;
  double tstop;

  // Register HEATOOL:
  set_toolname("pixevents");
  set_toolversion("0.05");

  do { // Beginning of the ERROR handling loop (will at
       // most be run once).

    // Read parameters using PIL library.
    status=getpar(&par);
    CHECK_STATUS_BREAK(status);

    headas_chat(3, "initialize ...\n");

    // Load detector information
    det=loadAdvDet(par.XMLFile, &status);
    CHECK_STATUS_BREAK(status);

    // Determine the impact list file.
    char impactlist_filename[MAXFILENAME];
    strcpy(impactlist_filename, par.ImpactList);

    // Determine the event list output file.
    char piximplist_filename[MAXFILENAME];
    strcpy(piximplist_filename, par.PixImpList);

    // Open the FITS file with the input impact list:
    ilf=openImpactFile(impactlist_filename, READONLY, &status);
    CHECK_STATUS_BREAK(status);

    // Read keywords from input file
    sixt_read_fits_stdkeywords_obsolete(ilf->fptr,
			       telescop,
			       instrume,
			       filter,
			       ancrfile,
			       respfile,
			       &mjdref,
			       &timezero,
			       &tstart,
			       &tstop,
			       &status);
    CHECK_STATUS_BREAK(status);

    // Open the output file
    plf=openNewPixImpFile(piximplist_filename,
			   telescop,
			   instrume,
			   filter,
			   ancrfile,
			   respfile,
			   par.XMLFile,
			   impactlist_filename,
			   mjdref,
			   timezero,
			   tstart,
			   tstop,
			   par.clobber,
			   &status);
    CHECK_STATUS_BREAK(status);

    int ii;

    while (ilf->row<ilf->nrows){
      // event pixel index array
      long *pixindex=NULL;

      // detector impact
      Impact detimp;

      // pixel impact array
      PixImpact *piximp=NULL;

      // load next impact
      getNextImpactFromFile(ilf, &detimp, &status);
      CHECK_STATUS_BREAK(status);


      // calculate pixel impact parameters
      int newPixImpacts=AdvImpactList(det, &detimp, &piximp);


      if(newPixImpacts>0){
	for(ii=0; ii<newPixImpacts; ii++){
	  addImpact2PixImpFile(plf, &(piximp[ii]), &status);
	}
      }


      free(pixindex);
      free(piximp);
    }

    // Copy the GTI extension into the new file
    CHECK_STATUS_BREAK(status);
    if(!fits_movnam_hdu(ilf->fptr, BINARY_TBL, "STDGTI", 0, &status)){
      fits_copy_hdu(ilf->fptr, plf->fptr, 0, &status);
      CHECK_STATUS_BREAK(status);
    }else{
      status=EXIT_SUCCESS;
    }

  } while(0); // END of the error handling loop.

  // --- END of pixel detection ---

  // --- Cleaning up ---
  headas_chat(3, "cleaning up ...\n");
  freePixImpFile(&plf, &status);
  freeImpactFile(&ilf, &status);

  destroyAdvDet(&det);

  if (EXIT_SUCCESS==status) {
    headas_chat(3, "finished successfully!\n\n");
    return(EXIT_SUCCESS);
  } else {
    return(EXIT_FAILURE);
  }

}

int getpar(struct Parameters* const par)
{
  // String input buffer.
  char* sbuffer=NULL;

  // Error status.
  int status=EXIT_SUCCESS;

  // Read all parameters via the ape_trad_ routines.

  status=ape_trad_query_file_name("ImpactList", &sbuffer);
  if (EXIT_SUCCESS!=status) {
    SIXT_ERROR("failed reading the name of the impact list");
    return(status);
  }
  strcpy(par->ImpactList, sbuffer);
  free(sbuffer);

  status=ape_trad_query_string("PixImpList", &sbuffer);
  if (EXIT_SUCCESS!=status) {
    SIXT_ERROR("failed reading the name of the pixel impact list");
    return(status);
  }
  strcpy(par->PixImpList, sbuffer);
  free(sbuffer);

  status=ape_trad_query_string("XMLFile", &sbuffer);
  if (EXIT_SUCCESS!=status) {
    SIXT_ERROR("failed reading the name of the XML file");
    return(status);
  }
  strcpy(par->XMLFile, sbuffer);
  free(sbuffer);

  status=ape_trad_query_bool("clobber", &par->clobber);
  if (EXIT_SUCCESS!=status) {
    SIXT_ERROR("failed reading the clobber parameter");
    return(status);
  }

  status=ape_trad_query_bool("history", &par->history);
  if (EXIT_SUCCESS!=status) {
    SIXT_ERROR("failed reading the history parameter");
    return(status);
  }

  return(status);
}

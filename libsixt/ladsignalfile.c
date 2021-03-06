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
   Copyright 2015-2019 Remeis-Sternwarte, Friedrich-Alexander-Universitaet
                       Erlangen-Nuernberg
*/

#include "ladsignalfile.h"


LADSignalFile* newLADSignalFile(int* const status)
{
  LADSignalFile* file=
    (LADSignalFile*)malloc(sizeof(LADSignalFile));
  CHECK_NULL_RET(file, *status,
		 "memory allocation for LADSignalFile failed", file);

  // Initialize pointers with NULL.
  file->fptr=NULL;

  // Initialize values.
  file->nrows=0;
  file->row  =0;
  file->ctime=0;
  file->csignal=0;
  file->cpanel =0;
  file->cmodule=0;
  file->celement=0;
  file->canode =0;
  file->cph_id =0;
  file->csrc_id=0;

  return(file);
}


void freeLADSignalFile(LADSignalFile** const file, int* const status)
{
  if (NULL!=*file) {
    if (NULL!=(*file)->fptr) {
      fits_close_file((*file)->fptr, status);
    }
    free(*file);
    *file=NULL;
  }
}


LADSignalFile* openNewLADSignalFile(const char* const filename,
				    const char clobber,
				    int* const status)
{
  LADSignalFile* file=newLADSignalFile(status);
  CHECK_STATUS_RET(*status, file);

  // Check if the file already exists.
  int exists;
  fits_file_exists(filename, &exists, status);
  CHECK_STATUS_RET(*status, file);
  if (0!=exists) {
    if (0!=clobber) {
      // Delete the file.
      remove(filename);
    } else {
      // Throw an error.
      char msg[MAXMSG];
      sprintf(msg, "file '%s' already exists", filename);
      SIXT_ERROR(msg);
      *status=EXIT_FAILURE;
      return(file);
    }
  }

  // Create a new LADSignal list FITS file from the template file.
  char buffer[MAXFILENAME];
  sprintf(buffer, "%s(%s%s)", filename, SIXT_DATA_PATH,
	  "/templates/ladsignalfile.tpl");
  fits_create_file(&file->fptr, buffer, status);
  CHECK_STATUS_RET(*status, file);

  // Set the time-keyword in the header.
  char datestr[MAXMSG];
  int timeref;
  fits_get_system_time(datestr, &timeref, status);
  CHECK_STATUS_RET(*status, file);
  fits_update_key(file->fptr, TSTRING, "DATE", datestr,
		  "File creation date", status);
  CHECK_STATUS_RET(*status, file);

  // Add header information about program parameters.
  // The second parameter "1" means that the headers are written
  // to the first extension.
  HDpar_stamp(file->fptr, 1, status);
  CHECK_STATUS_RET(*status, file);

  // Move to the binary table extension.
  fits_movabs_hdu(file->fptr, 2, 0, status);
  CHECK_STATUS_RET(*status, file);

  // Close the file.
  freeLADSignalFile(&file, status);
  CHECK_STATUS_RET(*status, file);

  // Re-open the file.
  file=openLADSignalFile(filename, READWRITE, status);
  CHECK_STATUS_RET(*status, file);

  return(file);
}


LADSignalFile* openLADSignalFile(const char* const filename,
				 const int mode,
				 int* const status)
{
  LADSignalFile* file=newLADSignalFile(status);
  CHECK_STATUS_RET(*status, file);

  headas_chat(4, "open event list file '%s' ...\n", filename);
  fits_open_table(&file->fptr, filename, mode, status);
  CHECK_STATUS_RET(*status, file);

  // Determine the row numbers.
  file->row=0;
  fits_get_num_rows(file->fptr, &file->nrows, status);
  CHECK_STATUS_RET(*status, file);

  // Determine the column numbers.
  fits_get_colnum(file->fptr, CASEINSEN, "TIME", &file->ctime, status);
  fits_get_colnum(file->fptr, CASEINSEN, "SIGNAL", &file->csignal, status);
  fits_get_colnum(file->fptr, CASEINSEN, "PANEL", &file->cpanel, status);
  fits_get_colnum(file->fptr, CASEINSEN, "MODULE", &file->cmodule, status);
  fits_get_colnum(file->fptr, CASEINSEN, "ELEMENT", &file->celement, status);
  fits_get_colnum(file->fptr, CASEINSEN, "ANODE", &file->canode, status);
  fits_get_colnum(file->fptr, CASEINSEN, "PH_ID", &file->cph_id, status);
  fits_get_colnum(file->fptr, CASEINSEN, "SRC_ID", &file->csrc_id, status);
  CHECK_STATUS_RET(*status, file);

  // Check if the vector length of the PH_ID and SRC_ID columns is equivalent
  // with the corresponding array lengths in the LADSignal data structure.
  int typecode;
  long repeat, width;
  // PH_ID.
  fits_get_coltype(file->fptr, file->cph_id, &typecode, &repeat,
		   &width, status);
  CHECK_STATUS_RET(*status, file);
  if (repeat!=NLADSIGNALPHOTONS) {
    // Throw an error.
    *status = EXIT_FAILURE;
    char msg[MAXMSG];
    sprintf(msg, "maximum number of photons contributing "
	    "to a single event is different "
	    "in the simulation (%d) and in the event list "
	    "template file (%ld)", NLADSIGNALPHOTONS, repeat);
    SIXT_ERROR(msg);
    return(file);
  }
  // SRC_ID.
  fits_get_coltype(file->fptr, file->csrc_id, &typecode, &repeat,
		   &width, status);
  CHECK_STATUS_RET(*status, file);
  if (repeat!=NLADSIGNALPHOTONS) {
    // Throw an error.
    *status = EXIT_FAILURE;
    char msg[MAXMSG];
    sprintf(msg, "maximum number of photons contributing "
	    "to a single event is different "
	    "in the simulation (%d) and in the event list "
	    "template file (%ld)!\n", NLADSIGNALPHOTONS, repeat);
    SIXT_ERROR(msg);
    return(file);
  }

  return(file);
}


void addLADSignal2File(LADSignalFile* const file,
		       LADSignal* const event,
		       int* const status)
{
  // Check if the event file has been opened.
  CHECK_NULL_VOID(file, *status, "no event file opened");
  CHECK_NULL_VOID(file->fptr, *status, "no event file opened");

  file->nrows++;
  file->row=file->nrows;

  // Write the data.
  updateLADSignalInFile(file, file->row, event, status);
  CHECK_STATUS_VOID(*status);
}


void getLADSignalFromFile(const LADSignalFile* const file,
			  const int row, LADSignal* const event,
			  int* const status)
{
  // Check if the file has been opened.
  CHECK_NULL_VOID(file, *status, "no event file opened");
  CHECK_NULL_VOID(file->fptr, *status, "no event file opened");

  // Check if there is still a row available.
  if (row > file->nrows) {
    *status=EXIT_FAILURE;
    SIXT_ERROR("event file contains no further entries");
    return;
  }

  // Read in the data.
  int anynul=0;
  double dnull=0.;
  float fnull=0.;
  long lnull=0;

  fits_read_col(file->fptr, TDOUBLE, file->ctime, row, 1, 1,
		&dnull, &event->time, &anynul, status);
  fits_read_col(file->fptr, TFLOAT, file->csignal, row, 1, 1,
		&fnull, &event->signal, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cpanel, row, 1, 1,
		&lnull, &event->panel, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cmodule, row, 1, 1,
		&lnull, &event->module, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->celement, row, 1, 1,
		&lnull, &event->element, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->canode, row, 1, 1,
		&lnull, &event->anode, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cph_id, row, 1, NLADSIGNALPHOTONS,
		&lnull, &event->ph_id, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->csrc_id, row, 1, NLADSIGNALPHOTONS,
		&lnull, &event->src_id, &anynul, status);
  CHECK_STATUS_VOID(*status);

  // Check if an error occurred during the reading process.
  if (0!=anynul) {
    *status=EXIT_FAILURE;
    SIXT_ERROR("reading from ImpactFile failed");
    return;
  }
}


void updateLADSignalInFile(const LADSignalFile* const file,
			   const int row, LADSignal* const event,
			   int* const status)
{
  fits_write_col(file->fptr, TDOUBLE, file->ctime, row,
		 1, 1, &event->time, status);
  fits_write_col(file->fptr, TFLOAT, file->csignal, row,
		 1, 1, &event->signal, status);
  fits_write_col(file->fptr, TLONG, file->cpanel, row,
		 1, 1, &event->panel, status);
  fits_write_col(file->fptr, TLONG, file->cmodule, row,
		 1, 1, &event->module, status);
  fits_write_col(file->fptr, TLONG, file->celement, row,
		 1, 1, &event->element, status);
  fits_write_col(file->fptr, TLONG, file->canode, row,
		 1, 1, &event->anode, status);
  fits_write_col(file->fptr, TLONG, file->cph_id, row,
		 1, NLADSIGNALPHOTONS, &event->ph_id, status);
  fits_write_col(file->fptr, TLONG, file->csrc_id, row,
		 1, NLADSIGNALPHOTONS, &event->src_id, status);
  CHECK_STATUS_VOID(*status);
}

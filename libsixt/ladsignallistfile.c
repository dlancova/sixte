#include "ladsignallistfile.h"


LADSignalListFile* newLADSignalListFile(int* const status)
{
  LADSignalListFile* file=
    (LADSignalListFile*)malloc(sizeof(LADSignalListFile));
  CHECK_NULL_RET(file, *status, 
		 "memory allocation for LADSignalListFile failed", file);

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


void freeLADSignalListFile(LADSignalListFile** const file, 
			   int* const status)
{
  if (NULL!=*file) {
    if (NULL!=(*file)->fptr) {
      fits_close_file((*file)->fptr, status);
    }
    free(*file);
    *file=NULL;
  }
}


LADSignalListFile* openNewLADSignalListFile(const char* const filename,
					    const char clobber,
					    int* const status)
{
  LADSignalListFile* file=newLADSignalListFile(status);
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
  sprintf(buffer, "%s(%s%s)", filename, SIXT_DATA_PATH, "/templates/ladsignallist.tpl");
  fits_create_file(&file->fptr, buffer, status);
  CHECK_STATUS_RET(*status, file);

  // Set the time-keyword in the LADSignal List Header.
  // See also: Stevens, "Advanced Programming in the UNIX environment",
  // p. 155 ff.
  time_t current_time;
  if (0 != time(&current_time)) {
    struct tm* current_time_utc = gmtime(&current_time);
    if (NULL != current_time_utc) {
      char current_time_str[MAXMSG];
      if (strftime(current_time_str, MAXMSG, "%Y-%m-%dT%H:%M:%S", 
		   current_time_utc) > 0) {
	// Return value should be == 19 !
	if (fits_update_key(file->fptr, TSTRING, "DATE-OBS", current_time_str, 
			    "Start Time (UTC) of exposure", status)) 
	  return(file);
      }
    }
  } 
  // END of writing time information to the header.

  // Add header information about program parameters.
  // The second parameter "1" means that the headers are written
  // to the first extension.
  HDpar_stamp(file->fptr, 1, status);
  CHECK_STATUS_RET(*status, file);

  // Move to the binary table extension.
  fits_movabs_hdu(file->fptr, 2, 0, status);
  CHECK_STATUS_RET(*status, file);

  // Close the file.
  freeLADSignalListFile(&file, status);
  CHECK_STATUS_RET(*status, file);

  // Re-open the file.
  file=openLADSignalListFile(filename, READWRITE, status);
  CHECK_STATUS_RET(*status, file);
  
  return(file);
}


LADSignalListFile* openLADSignalListFile(const char* const filename,
					 const int mode, 
					 int* const status)
{
  LADSignalListFile* file = newLADSignalListFile(status);
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


void addLADSignal2File(LADSignalListFile* const file, 
		       LADSignal* const event, 
		       int* const status)
{
  // Check if the event file has been opened.
  CHECK_NULL_VOID(file, *status, "no event file opened");
  CHECK_NULL_VOID(file->fptr, *status, "no event file opened");

  // Insert a new, empty row to the table:
  fits_insert_rows(file->fptr, file->row, 1, status);
  CHECK_STATUS_VOID(*status);
  file->nrows++;
  file->row = file->nrows;

  // Write the data.
  updateLADSignalInFile(file, file->row, event, status);
  CHECK_STATUS_VOID(*status);
}


void getLADSignalFromFile(const LADSignalListFile* const file,
			  const int row, LADSignal* const event,
			  int* const status)
{
  // Check if the file has been opened.
  CHECK_NULL_VOID(file, *status, "no event file opened");
  CHECK_NULL_VOID(file->fptr, *status, "no event file opened");

  // Check if there is still a row available.
  if (row > file->nrows) {
    *status = EXIT_FAILURE;
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
    *status = EXIT_FAILURE;
    SIXT_ERROR("reading from ImpactListFile failed");
    return;
  }
}


void updateLADSignalInFile(const LADSignalListFile* const file,
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

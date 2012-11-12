#include "eventlistfile.h"


EventListFile* newEventListFile(int* const status)
{
  EventListFile* file = (EventListFile*)malloc(sizeof(EventListFile));
  CHECK_NULL_RET(file, *status, "memory allocation for EventFile failed", 
		 file);

  // Initialize pointers with NULL.
  file->fptr=NULL;

  // Initialize values.
  file->nrows=0;
  file->ctime=0;
  file->cframe =0;
  file->cpha   =0;
  file->csignal=0;
  file->crawx  =0;
  file->crawy  =0;
  file->cph_id =0;
  file->csrc_id=0;

  return(file);
}


void freeEventListFile(EventListFile** const file, int* const status)
{
  if (NULL!=*file) {
    if (NULL!=(*file)->fptr) {
      // If the file was opened in READWRITE mode, calculate
      // the check sum an append it to the FITS header.
      int mode;
      fits_file_mode((*file)->fptr, &mode, status);
      if (READWRITE==mode) {
	fits_write_chksum((*file)->fptr, status);
      }
      fits_close_file((*file)->fptr, status);
    }
    free(*file);
    *file=NULL;
  }
}


EventListFile* openNewEventListFile(const char* const filename,
				    const char clobber,
				    int* const status)
{
  EventListFile* file=newEventListFile(status);
  if (EXIT_SUCCESS!=*status) return(file);

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

  // Create a new event list FITS file from the template file.
  char buffer[MAXFILENAME];
  sprintf(buffer, "%s(%s%s)", filename, SIXT_DATA_PATH, 
	  "/templates/eventlist.tpl");
  fits_create_file(&file->fptr, buffer, status);
  CHECK_STATUS_RET(*status, file);

  // Set the time-keyword in the Event List Header.
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
  // END of writing time information to Event File FITS header.

  // Add header information about program parameters.
  // The second parameter "1" means that the headers are written
  // to the first extension.
  HDpar_stamp(file->fptr, 1, status);
  if (EXIT_SUCCESS!=*status) return(file);

  // Close the file.
  freeEventListFile(&file, status);
  if (EXIT_SUCCESS!=*status) return(file);

  // Re-open the file.
  file=openEventListFile(filename, READWRITE, status);
  if (EXIT_SUCCESS!=*status) return(file);
  
  return(file);
}


EventListFile* openEventListFile(const char* const filename,
				 const int mode, int* const status)
{
  EventListFile* file=newEventListFile(status);
  CHECK_STATUS_RET(*status, file);

  headas_chat(4, "open event list file '%s' ...\n", filename);
  fits_open_table(&file->fptr, filename, mode, status);
  CHECK_STATUS_RET(*status, file);

  // Determine the row numbers.
  fits_get_num_rows(file->fptr, &file->nrows, status);

  // Determine the column numbers.
  fits_get_colnum(file->fptr, CASEINSEN, "TIME", &file->ctime, status);
  fits_get_colnum(file->fptr, CASEINSEN, "FRAME", &file->cframe, status);
  fits_get_colnum(file->fptr, CASEINSEN, "PHA", &file->cpha, status);
  fits_get_colnum(file->fptr, CASEINSEN, "SIGNAL", &file->csignal, status);
  fits_get_colnum(file->fptr, CASEINSEN, "RAWX", &file->crawx, status);
  fits_get_colnum(file->fptr, CASEINSEN, "RAWY", &file->crawy, status);
  fits_get_colnum(file->fptr, CASEINSEN, "PH_ID", &file->cph_id, status);
  fits_get_colnum(file->fptr, CASEINSEN, "SRC_ID", &file->csrc_id, status);
  CHECK_STATUS_RET(*status, file);

  // Check if the vector length of the PH_ID and SRC_ID columns is equivalent 
  // with the corresponding array lengths in the Event data structure.
  int typecode;
  long repeat, width;
  // PH_ID.
  fits_get_coltype(file->fptr, file->cph_id, &typecode, &repeat,
		   &width, status);
  CHECK_STATUS_RET(*status, file);
  if (repeat!=NEVENTPHOTONS) {
    // Throw an error.
    *status = EXIT_FAILURE;
    char msg[MAXMSG];
    sprintf(msg, "Error: the maximum number of photons contributing "
	    "to a single event is different for the parameter set "
	    "in the simulation (%d) and in the event list "
	    "template file (%ld)!\n", NEVENTPHOTONS, repeat);
    HD_ERROR_THROW(msg, *status);
    return(file);
  }
  // SRC_ID.
  fits_get_coltype(file->fptr, file->csrc_id, &typecode, &repeat,
		   &width, status);
  CHECK_STATUS_RET(*status, file);
  if (repeat!=NEVENTPHOTONS) {
    // Throw an error.
    *status = EXIT_FAILURE;
    char msg[MAXMSG];
    sprintf(msg, "Error: the maximum number of photons contributing "
	    "to a single event is different for the parameter set "
	    "in the simulation (%d) and in the event list "
	    "template file (%ld)!\n", NEVENTPHOTONS, repeat);
    HD_ERROR_THROW(msg, *status);
    return(file);
  }

  return(file);
}


void addEvent2File(EventListFile* const file, Event* const event, 
		   int* const status)
{
  // Check if the event file has been opened.
  CHECK_NULL_VOID(file, *status, "event file not open");
  CHECK_NULL_VOID(file->fptr, *status, "event file not open");

  // Insert a new, empty row to the table:
  fits_insert_rows(file->fptr, file->nrows, 1, status);
  CHECK_STATUS_VOID(*status);
  file->nrows++;

  // Write the data.
  updateEventInFile(file, file->nrows, event, status);
  CHECK_STATUS_VOID(*status);
}


void getEventFromFile(const EventListFile* const file,
		      const int row, Event* const event,
		      int* const status)
{
  // Check if the file has been opened.
  CHECK_NULL_VOID(file, *status, "event file not open");
  CHECK_NULL_VOID(file->fptr, *status, "event file not open");

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
  int inull=0;

  fits_read_col(file->fptr, TDOUBLE, file->ctime, row, 1, 1, 
		&dnull, &event->time, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cframe, row, 1, 1, 
		&lnull, &event->frame, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cpha, row, 1, 1, 
		&lnull, &event->pha, &anynul, status);
  fits_read_col(file->fptr, TFLOAT, file->csignal, row, 1, 1, 
		&fnull, &event->signal, &anynul, status);
  fits_read_col(file->fptr, TINT, file->crawx, row, 1, 1, 
		&inull, &event->rawx, &anynul, status);
  fits_read_col(file->fptr, TINT, file->crawy, row, 1, 1, 
		&inull, &event->rawy, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->cph_id, row, 1, NEVENTPHOTONS, 
		&lnull, &event->ph_id, &anynul, status);
  fits_read_col(file->fptr, TLONG, file->csrc_id, row, 1, NEVENTPHOTONS, 
		&lnull, &event->src_id, &anynul, status);
  CHECK_STATUS_VOID(*status);

  // Check if an error occurred during the reading process.
  if (0!=anynul) {
    *status=EXIT_FAILURE;
    SIXT_ERROR("reading from EventListFile failed");
    return;
  }
}


void updateEventInFile(const EventListFile* const file,
		       const int row, Event* const event,
		       int* const status)
{
  fits_write_col(file->fptr, TDOUBLE, file->ctime, row, 
		 1, 1, &event->time, status);
  fits_write_col(file->fptr, TLONG, file->cframe, row, 
		 1, 1, &event->frame, status);
  fits_write_col(file->fptr, TLONG, file->cpha, row, 
		 1, 1, &event->pha, status);
  fits_write_col(file->fptr, TFLOAT, file->csignal, row, 
		 1, 1, &event->signal, status);
  fits_write_col(file->fptr, TINT, file->crawx, row, 
		 1, 1, &event->rawx, status);
  fits_write_col(file->fptr, TINT, file->crawy, row, 
		 1, 1, &event->rawy, status);
  fits_write_col(file->fptr, TLONG, file->cph_id, row, 
		 1, NEVENTPHOTONS, &event->ph_id, status);
  fits_write_col(file->fptr, TLONG, file->csrc_id, row, 
		 1, NEVENTPHOTONS, &event->src_id, status);
  CHECK_STATUS_VOID(*status);
}

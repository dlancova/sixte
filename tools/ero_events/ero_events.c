#include "ero_events.h"


int ero_events_main() 
{
  // Containing all programm parameters read by PIL
  struct Parameters par; 

  // Input event list file.
  EventListFile* gelf=NULL;

  // File pointer to the output eROSITA event file. 
  fitsfile* fptr=NULL;

  // Error status.
  int status=EXIT_SUCCESS; 


  // Register HEATOOL:
  set_toolname("ero_events");
  set_toolversion("0.01");


  do { // Beginning of the ERROR handling loop (will at most be run once).

    // --- Initialization ---

    headas_chat(3, "initialization ...\n");

    // Read parameters using PIL library:
    if ((status=getpar(&par))) break;

    // Set the input event file.
    gelf=openEventListFile(par.genEventList, READONLY, &status);
    if (EXIT_SUCCESS!=status) break;


    // Create and open the output eROSITA event file.
    // Filename of the template file.
    char template[MAXMSG];
    strcpy(template, par.fits_templates);
    strcat(template, "/");
    strcat(template, "eroeventlist.tpl");

    // Remove old file, if it exists.
    remove(par.eroEventList);

    // Create and open a new FITS file using the template.
    char buffer[MAXMSG];
    sprintf(buffer, "%s(%s)", par.eroEventList, template);
    headas_chat(4, "create new eROSITA event list file '%s' from template '%s' ...\n", 
		par.eroEventList, template);
    fits_create_file(&fptr, buffer, &status);
    if (EXIT_SUCCESS!=status) break;

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
	  fits_update_key(fptr, TSTRING, "DATE-OBS", current_time_str, 
			  "Start Time (UTC) of exposure", &status);
	  if (EXIT_SUCCESS!=status) break;
	}
      }
    } 
    // END of writing time information to Event File FITS header.

    // Determine the column numbers.
    int ctime, crawx, crawy, cframe, cpha, cenergy, cra, cdec;
    fits_get_colnum(fptr, CASEINSEN, "TIME", &ctime, &status);
    fits_get_colnum(fptr, CASEINSEN, "FRAME", &cframe, &status);
    fits_get_colnum(fptr, CASEINSEN, "PHA", &cpha, &status);
    fits_get_colnum(fptr, CASEINSEN, "ENERGY", &cenergy, &status);
    fits_get_colnum(fptr, CASEINSEN, "RAWX", &crawx, &status);
    fits_get_colnum(fptr, CASEINSEN, "RAWY", &crawy, &status);
    fits_get_colnum(fptr, CASEINSEN, "RA", &cra, &status);
    fits_get_colnum(fptr, CASEINSEN, "DEC", &cdec, &status);
    if (EXIT_SUCCESS!=status) break;

    // --- END of Initialization ---

    
    // --- Beginning of Copy Process ---

    headas_chat(3, "start copy process ...\n");

    // Loop over all events in the FITS file. 
    long row;
    for (row=0; row<=gelf->nrows; row++) {
      
      // Read the next event from the input file.
      Event event;
      getEventFromFile(gelf, row+1, &event, &status);
      CHECK_STATUS_BREAK(status);

      // Store the event in the output file.
      fits_insert_rows(fptr, row, 1, &status);
      CHECK_STATUS_BREAK(status);

      fits_write_col(fptr, TDOUBLE, ctime, row+1, 1, 1, &event.time, &status);
      fits_write_col(fptr, TLONG, cframe, row+1, 1, 1, &event.frame, &status);
      fits_write_col(fptr, TLONG, cpha, row+1, 1, 1, &event.pha, &status);
      float energy = event.charge * 1000.; // [eV]
      fits_write_col(fptr, TFLOAT, cenergy, row+1, 1, 1, &energy, &status);
      int rawx = event.rawx+1;
      fits_write_col(fptr, TINT, crawx, row+1, 1, 1, &rawx, &status);
      int rawy = event.rawy+1;
      fits_write_col(fptr, TINT, crawy, row+1, 1, 1, &rawy, &status);

      // TODO

      long ra = (long)(event.ra/1.e-6);
      if (event.ra < 0.) ra--;
      fits_write_col(fptr, TLONG, cra, row+1, 1, 1, &ra, &status);

      long dec = (long)(event.dec/1.e-6);
      if (event.dec < 0.) dec--;
      fits_write_col(fptr, TLONG, cdec, row+1, 1, 1, &dec, &status);
      
      // X
      // Y
      // CCDNR (PIL parameter)
      CHECK_STATUS_BREAK(status);

    }
    CHECK_STATUS_BREAK(status);
    // END of loop over all events in the FITS file.

  } while(0); // END of the error handling loop.


  // --- Cleaning up ---
  headas_chat(3, "cleaning up ...\n");

  // Close the files.
  freeEventListFile(&gelf, &status);
  if (NULL!=fptr) fits_close_file(fptr, &status);
  
  if (status == EXIT_SUCCESS) headas_chat(3, "finished successfully\n\n");
  return(status);
}


int getpar(struct Parameters* const par)
{
  // String input buffer.
  char* sbuffer=NULL;

  // Error status.
  int status=EXIT_SUCCESS;

  status=ape_trad_query_file_name("genEventList", &sbuffer);
  if (EXIT_SUCCESS!=status) {
    HD_ERROR_THROW("Error reading the name of the input event list!\n", status);
    return(status);
  } 
  strcpy(par->genEventList, sbuffer);
  free(sbuffer);

  status=ape_trad_query_file_name("eroEventList", &sbuffer);
  if (EXIT_SUCCESS!=status) {
    HD_ERROR_THROW("Error reading the name of the output event list!\n", status);
    return(status);
  } 
  strcpy(par->eroEventList, sbuffer);
  free(sbuffer);

  status=ape_trad_query_bool("clobber", &par->clobber);
  if (EXIT_SUCCESS!=status) {
    HD_ERROR_THROW("Error reading the clobber parameter!\n", status);
    return(status);
  }

  // Get the name of the FITS template directory
  // from the environment variable.
  if (NULL!=(sbuffer=getenv("SIXT_FITS_TEMPLATES"))) {
    strcpy(par->fits_templates, sbuffer);
    // Note: the char* pointer returned by getenv should not
    // be modified nor free'd.
  } else {
    status = EXIT_FAILURE;
    HD_ERROR_THROW("Error reading the environment variable 'SIXT_FITS_TEMPLATES'!\n", 
		   status);
    return(status);
  }

  return(status);
}



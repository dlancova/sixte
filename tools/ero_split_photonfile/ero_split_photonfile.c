#include "ero_split_photonfile.h"


int ero_split_photonfile_getpar(struct Parameters* parameters)
{
  int status = EXIT_SUCCESS;

  if ((status = PILGetFname("input_filename", parameters->input_filename))) {
    HD_ERROR_THROW("Error reading the name of the input file!\n", status);
  }

  else if ((status = PILGetFname("output_prefix", parameters->output_prefix))) {
    HD_ERROR_THROW("Error reading the prefix for the output files!\n", status);
  }

  // Get the name of the FITS template directory.
  // First try to read it from the environment variable.
  // If the variable does not exist, read it from the PIL.
  char* buffer;
  if (NULL!=(buffer=getenv("SIXT_FITS_TEMPLATES"))) {
    strcpy(parameters->photonlist_template, buffer);
  } else {
    if ((status = PILGetFname("fits_templates", parameters->photonlist_template))) {
      HD_ERROR_THROW("Error reading the path of the FITS templates!\n", status);
      
    }
  }
  if (EXIT_SUCCESS!=status) return(status);
  // Set the photon list template file:
  strcat(parameters->photonlist_template, "/photonlist.tpl");

  return(status);
}



int ero_split_photonfile_main() {
  
  struct Parameters parameters;
  // Input photon list.
  PhotonListFile inputfile;
  // Array of output photon files.
  PhotonListFile outputfiles[7];
  int filecounter;

  int status = EXIT_SUCCESS;

  // Register HEATOOL
  set_toolname("ero_split_photonfile");
  set_toolversion("0.01");
  
  do { // ERROR handling loop

    // Read parameters by PIL.
    status = ero_split_photonfile_getpar(&parameters);
    if (EXIT_SUCCESS!=status) break;
    
    // Initialize HEADAS random number generator and GSL generator for 
    // Gaussian distribution.
    HDmtInit(SIXT_HD_RANDOM_SEED);

    // Open the INPUT file.
    status = openPhotonListFile(&inputfile, 
				parameters.input_filename, 
				READONLY);
    if (EXIT_SUCCESS!=status) break;

    // Create and open new output photon list files.
    char filename[MAXMSG];
    for (filecounter=0; filecounter<7; filecounter++) {
      sprintf(filename, "%s%d.fits", 
	      parameters.output_prefix, filecounter);
      status = openNewPhotonListFile(&outputfiles[filecounter], 
				     filename, 
				     parameters.photonlist_template);
      if (EXIT_SUCCESS!=status) break;
    }
    if (EXIT_SUCCESS!=status) break;


    // Copy header keywords.
    struct HKeys {
      double refxcrvl;
      double refycrvl;
      char attitude[MAXMSG];
    } hkeys;

    // Read from input file.
    char comment[MAXMSG]; // String buffer.
    if (fits_read_key(inputfile.fptr, TDOUBLE, "REFXCRVL", 
		      &hkeys.refxcrvl, comment, &status)) break;    
    if (fits_read_key(inputfile.fptr, TDOUBLE, "REFYCRVL", 
		      &hkeys.refycrvl, comment, &status)) break;    
    if (fits_read_key(inputfile.fptr, TSTRING, "ATTITUDE", 
		      hkeys.attitude, comment, &status)) break;

    // Write to output files.
    for (filecounter=0; filecounter<7; filecounter++) {
      if (fits_update_key(outputfiles[filecounter].fptr, TDOUBLE, "REFXCRVL", 
			  &hkeys.refxcrvl, "", &status)) break;
      if (fits_update_key(outputfiles[filecounter].fptr, TDOUBLE, "REFYCRVL", 
			  &hkeys.refycrvl, "", &status)) break;
      if (fits_update_key(outputfiles[filecounter].fptr, TSTRING, "ATTITUDE", 
			  hkeys.attitude, "name of the attitude FITS file", 
			  &status)) break;
    }
    // END of copying header keywords.


    // Copy photon list entries.
    Photon photon={.time=0.};
    int rnd_file;
    while (inputfile.row<inputfile.nrows) {

      if (EXIT_SUCCESS!=status) break;
      
      // Read an entry from the photon list:
      status = PhotonListFile_getNextRow(&inputfile, &photon);
      if (status!=EXIT_SUCCESS) break;
      
      // Get a random file index [0-6].
      rnd_file = (int)(sixt_get_random_number()*7.);
      
      // Append the photon to the randomly chosen file.
      status = addPhoton2File(&outputfiles[rnd_file], &photon);
      if (status!=EXIT_SUCCESS) break;
    }
    if (EXIT_SUCCESS!=status) break;
    // END of loop over all entries in the input photon list file.

  } while(0); // End of error handling loop


  // --- Clean Up ---

  // Release HEADAS random number generator.
  HDmtFree();

  // Close the photon list files:
  closePhotonListFile(&inputfile);
  for(filecounter=0; filecounter<7; filecounter++) {
    closePhotonListFile(&outputfiles[filecounter]);
  }
  
  return(status);
}


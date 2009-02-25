#if HAVE_CONFIG_H
#include <config.h>
#else
#error "Do not compile outside Autotools!"
#endif

#include "photon_imaging.h"



////////////////////////////////////
// Main procedure.
int photon_imaging_main() {
  double t0;        // starting time of the simulation
  double timespan;  // time span of the simulation

  char orbit_filename[FILENAME_LENGTH];   // filename of orbit file
  char attitude_filename[FILENAME_LENGTH];// filename of the attitude file
  long sat_nentries;                      // number of entries in the orbit array 
                                          // ( <= orbit_nrows )
  struct Telescope *sat_catalog=NULL;     // catalog with orbit and attitude data 
                                          // over a certain timespan

  char photonlist_filename[FILENAME_LENGTH]; // input: photon list
  fitsfile *photonlist_fptr=NULL;            // FITS file pointer
  long photonlist_nrows;                     // number of entries in the photon list

  struct Telescope telescope; // Telescope data (like FOV diameter or focal length)
  struct Detector detector;   // Detector data structure (containing the 
                              // pixel array, its width, ...)
  struct PSF_Store psf_store; // Storage for the PSF (Point Spread Function) data 
                              // (for different off-axis angles and energies)
  char psf_filename[FILENAME_LENGTH]; // PSF input file

  char msg[MAXMSG];             // error output buffer
  int status=EXIT_SUCCESS;      // error status


  // register HEATOOL
  set_toolname("photon_imaging");
  set_toolversion("0.01");


  do {  // Beginning of the ERROR handling loop (will at most be run once)

    // --- Initialization ---

    // read parameters using PIL library
    if ((status=photon_imaging_getpar(photonlist_filename, orbit_filename, 
				      attitude_filename, psf_filename, 
				      &t0, &timespan, &telescope, &detector))) break;


    // Calculate the minimum cos-value for sources inside the FOV: 
    // (angle(x0,source) <= 1/2 * diameter)
    const double fov_min_align = cos(telescope.fov_diameter/2.); 
    

    // Initialize HEADAS random number generator and GSL generator for 
    // Gaussian distribution.
    HDmtInit(1);

    // Open the FITS file with the input photon list:
    if (fits_open_table(&photonlist_fptr, photonlist_filename, 
			READONLY, &status)) break;
    // Determine the number of rows in the photon list:
    if (fits_get_num_rows(photonlist_fptr, &photonlist_nrows, &status)) break;


    // Get the satellite catalog with the orbit and (telescope) attitude data:
    if ((status=get_satellite_catalog(&sat_catalog, &sat_nentries, t0, timespan, 
				      orbit_filename, attitude_filename))
	!=EXIT_SUCCESS) break;


    // get the PSF
    if ((status=get_psf(&psf_store, psf_filename, &status))!=EXIT_SUCCESS) break;

    // --- END of Initialization ---


    // --- Beginning of Imaging Process ---

    // LOOP over all timesteps given the specified timespan from t0 to t0+timespan
    long row=0;         // current row in the input list
    long sat_counter=0; // counter for orbit readout loop

    // Beginning of actual simulation (after loading required data):
    headas_chat(5, "start imaging process ...\n");


    // SCAN PHOTON LIST    
    for(row=0; (row<photonlist_nrows)&&(status==EXIT_SUCCESS); row++) {
      
      // Read an entry from the photon list:
      int anynul = 0;
      struct Photon photon = {0., 0., 0., 0.};
      fits_read_col(photonlist_fptr, TDOUBLE, 1, row+1, 1, 1, &photon.time,
		    &photon.time, &anynul, &status);
      fits_read_col(photonlist_fptr, TFLOAT, 2, row+1, 1, 1, &photon.energy,
		    &photon.energy, &anynul, &status);
      fits_read_col(photonlist_fptr, TDOUBLE, 3, row+1, 1, 1, &photon.ra,
		    &photon.ra, &anynul, &status);
      fits_read_col(photonlist_fptr, TDOUBLE, 4, row+1, 1, 1, &photon.dec,
		    &photon.dec, &anynul, &status);
      
      if (status!=EXIT_SUCCESS) break;
      photon.direction = unit_vector(photon.ra, photon.dec);


      // Get the last orbit entry before 'photon.time'
      // (in order to interpolate the position and velocity at this time  between 
      // the neighboring calculated orbit positions):
      for( ; sat_counter<sat_nentries; sat_counter++) {
	if(sat_catalog[sat_counter].time>photon.time) {
	  break;
	}
      }
      if(fabs(sat_catalog[--sat_counter].time-photon.time)>600.) { 
	// no entry within 10 minutes !!
	status = EXIT_FAILURE;
	sprintf(msg, "Error: no adequate orbit entry for time %lf!\n", photon.time);
	HD_ERROR_THROW(msg,status);
	break;
      }

      // Check whether the photon is inside the FOV:
      // First determine telescope pointing direction at the actual time.
      telescope.nz = 
	normalize_vector(interpolate_vec(sat_catalog[sat_counter].nz, 
					 sat_catalog[sat_counter].time, 
					 sat_catalog[sat_counter+1].nz, 
					 sat_catalog[sat_counter+1].time, 
					 photon.time));

      // Compare the photon direction to the unit vector specifiing the 
      // direction of the telescope axis:
      if(check_fov(photon.direction,telescope.nz,fov_min_align)==0) {
	// Photon is inside the FOV!
	
	// Determine telescope data like direction etc. (attitude):
	// Calculate nx: perpendicular to telescope axis and in the direction of
	// the satellite motion:
	telescope.nx = 
	  normalize_vector(interpolate_vec(sat_catalog[sat_counter].nx, 
					   sat_catalog[sat_counter].time, 
					   sat_catalog[sat_counter+1].nx, 
					   sat_catalog[sat_counter+1].time, 
					   photon.time));
	// Remove the component along the vertical direction nz 
	// (nx must be perpendicular to nz!):
	double scp = scalar_product(telescope.nz,telescope.nx);
	telescope.nx.x -= scp*telescope.nz.x;
	telescope.nx.y -= scp*telescope.nz.y;
	telescope.nx.z -= scp*telescope.nz.z;
	telescope.nx = normalize_vector(telescope.nx);

	// ny is perpendicular to telescope axis and nx:
	telescope.ny=normalize_vector(vector_product(telescope.nz, telescope.nx));
	
	// Determine the photon impact position on the detector (in [m]):
	struct Point2d position;  

	// Convolution with PSF:
	// Function returns 0, if the photon does not fall on the detector. 
	// If it hits a detector pixel, the return value is 1.
	if (get_psf_pos(&position, photon, telescope, psf_store)) {
	  // Check whether the photon hits the detector within the FOV. 
	  // (Due to the effects of the mirrors it might have been scattered over 
	  // the edge of the FOV, although the source is inside the FOV.)
	  if (sqrt(pow(position.x,2.)+pow(position.y,2.)) < 
	      tan(telescope.fov_diameter)*telescope.focal_length) {
	    printf("%lf %lf\n", position.x, position.y);
	  }
	} // END get_psf_pos(...)

      } // End of FOV check

    }  // END of scanning LOOP over the photon list.

  } while(0);  // END of the error handling loop.



  // --- cleaning up ---
  headas_chat(5, "cleaning up ...\n");

  // release HEADAS random number generator
  HDmtFree();

  // Close the event list FITS file.
  if (photonlist_fptr) fits_close_file(photonlist_fptr, &status);

  // release memory of orbit catalog
  if (sat_catalog) free(sat_catalog);

  // release memory of PSF:
  free_psf_store(psf_store);

  if (status == EXIT_SUCCESS) headas_chat(5, "finished\n\n");

  return(status);
}




////////////////////////////////////////////////////////////////
// This routine reads the program parameters using the PIL.
int photon_imaging_getpar(
			  // input: photon list file (FITS)
			  char photonlist_filename[],
			  // input: orbit file (FITS)
			  char orbit_filename[], 
			  // input: attitude file (FITS)
			  char attitude_filename[],
			  char psf_filename[],     // PSF FITS file
			  double *t0,              // start time for the simulation
			  double *timespan,        // time span for the simulation
			  struct Telescope *telescope,
			  struct Detector *detector
			  )
{
  char msg[MAXMSG];             // error output buffer
  int status=EXIT_SUCCESS;      // error status


  // get the filename of the input photon list (FITS file)
  if ((status = PILGetFname("photonlist_filename", photonlist_filename))) {
    sprintf(msg, "Error reading the filename of the photon list!\n");
    HD_ERROR_THROW(msg,status);
  }

  // get the filename of the orbit file (FITS file)
  else if ((status = PILGetFname("orbit_filename", orbit_filename))) {
    sprintf(msg, "Error reading the filename of the orbit file!\n");
    HD_ERROR_THROW(msg,status);
  }

  // get the filename of the attitude file (FITS file)
  else if ((status = PILGetFname("attitude_filename", attitude_filename))) {
    sprintf(msg, "Error reading the filename of the attitude file!\n");
    HD_ERROR_THROW(msg,status);
  }
  
  // get the filename of the PSF data file (FITS file)
  else if ((status = PILGetFname("psf_filename", psf_filename))) {
    sprintf(msg, "Error reading the filename of the PSF file!\n");
    HD_ERROR_THROW(msg,status);
  }

  // get the start time of the simulation
  else if ((status = PILGetReal("t0", t0))) {
    sprintf(msg, "Error reading the 't0' parameter!\n");
    HD_ERROR_THROW(msg,status);
  }

  // get the timespan for the simulation
  else if ((status = PILGetReal("timespan", timespan))) {
    sprintf(msg, "Error reading the 'timespan' parameter!\n");
    HD_ERROR_THROW(msg,status);
  }

  // read the diameter of the FOV (in arcmin)
  else if ((status = PILGetReal("fov_diameter", &telescope->fov_diameter))) {
    sprintf(msg, "Error reading the diameter of the FOV!\n");
    HD_ERROR_THROW(msg,status);
  }

  // read the focal length [m]
  else if ((status = PILGetReal("focal_length", &telescope->focal_length))) {
    sprintf(msg, "Error reading the focal length!\n");
    HD_ERROR_THROW(msg,status);
  }

  // convert angles from [arc min] to [rad]
  telescope->fov_diameter = telescope->fov_diameter*M_PI/(60.*180.); 

  return(status);
}



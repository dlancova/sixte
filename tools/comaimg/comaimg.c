#include "comaimg.h"


////////////////////////////////////
/** Main procedure. */
int comaimg_main() {
  struct Parameters par;

  AttitudeCatalog* ac=NULL;
  struct Telescope telescope; // Telescope data.
  PhotonListFile* plf=NULL;
  ImpactListFile* ilf=NULL;
  double refxcrvl=0., refycrvl=0.;
  CodedMask* mask=NULL;

  int status=EXIT_SUCCESS; // Error status.


  // Register HEATOOL:
  set_toolname("comaimg");
  set_toolversion("0.02");


  do {  // Beginning of the ERROR handling loop (will at most be run once)

    // --- Initialization ---

    // Read parameters using PIL library.
    if ((status=comaimg_getpar(&par))) break;

    float focal_length=par.mask_distance;

    // Calculate the minimum cos-value for sources inside the FOV: 
    // (angle(x0,source) <= 1/2 * diameter)
    const double fov_min_align=cos(M_PI/3.);
    
    // Initialize HEADAS random number generator and GSL generator for 
    // Gaussian distribution.
    HDmtInit(1);

    // Open the FITS file with the input photon list:
    plf=openPhotonListFile(par.photonlist_filename, READONLY, &status);
    CHECK_STATUS_BREAK(status);

    // Open the attitude file specified in the header keywords of the photon list.
    char comment[MAXMSG];
    fits_read_key(plf->fptr, TSTRING, "ATTITUDE", 
		  &par.attitude_filename, comment, &status);
    ac=loadAttitudeCatalog(par.attitude_filename, &status);
    CHECK_STATUS_BREAK(status);

    // Load the coded mask from the file.
    mask = getCodedMaskFromFile(par.mask_filename, &status);
    CHECK_STATUS_BREAK(status);

    // Create a new FITS file for the output of the impact list.
    ilf=openNewImpactListFile(par.impactlist_filename, 0, &status);
    CHECK_STATUS_BREAK(status);

    // Write WCS header keywords.
    fits_update_key(ilf->fptr, TDOUBLE, "REFXCRVL", 
		    &refxcrvl, "", &status);
    fits_update_key(ilf->fptr, TDOUBLE, "REFYCRVL", 
		    &refycrvl, "", &status);
    // Add attitude filename.
    fits_update_key(ilf->fptr, TSTRING, "ATTITUDE", 
		    par.attitude_filename,
		    "name of the attitude FITS file", &status);
    CHECK_STATUS_BREAK(status);
    
    // --- END of Initialization ---


    // --- Beginning of Imaging Process ---

    // Beginning of actual simulation (after loading required data):
    headas_chat(3, "start imaging process ...\n");

    // LOOP over all timesteps given the specified timespan from t0 to t0+timespan
    long attitude_counter=0;  // counter for AttitudeCatalog

    // SCAN PHOTON LIST    
    for(plf->row=0; plf->row<plf->nrows; plf->row++) {
      
      // Read an entry from the photon list:
      int anynul = 0;
      Photon photon = { .time=0., .energy=0., .ra=0., .dec=0. };
      fits_read_col(plf->fptr, TDOUBLE, plf->ctime, 
		    plf->row+1, 1, 1, &photon.time, &photon.time, 
		    &anynul, &status);
      fits_read_col(plf->fptr, TFLOAT, plf->cenergy, 
		    plf->row+1, 1, 1, &photon.energy, &photon.energy, 
		    &anynul, &status);
      fits_read_col(plf->fptr, TDOUBLE, plf->cra, 
		    plf->row+1, 1, 1, &photon.ra, &photon.ra, 
		    &anynul, &status);
      fits_read_col(plf->fptr, TDOUBLE, plf->cdec, 
		    plf->row+1, 1, 1, &photon.dec, &photon.dec, 
		    &anynul, &status);
      CHECK_STATUS_BREAK(status);

      // Rescale from [deg] -> [rad]
      photon.ra  = photon.ra *M_PI/180.;
      photon.dec = photon.dec*M_PI/180.;

      // Determine the unit vector pointing in the direction of the photon.
      Vector photon_direction = unit_vector(photon.ra, photon.dec);
   
      // Determine telescope pointing direction at the current time.
      telescope.nz=getTelescopeNz(ac, photon.time, &status);
      CHECK_STATUS_BREAK(status);

      // Check whether the photon is inside the FOV:
      // Compare the photon direction to the unit vector specifiing the 
      // direction of the telescope axis:
      if (check_fov(&photon_direction, &telescope.nz, fov_min_align)==0) {
	// Photon is inside the FOV!
	
	// Determine telescope data like direction etc. (attitude).
	// The telescope coordinate system consists of a nx, ny, and nz axis.
	// The nz axis is perpendicular to the detector plane and pointing along
	// the telescope direction. The nx axis is align along the detector 
	// x-direction, which is identical to the detector COLUMN.
	// The ny axis ix pointing along the y-direction of the detector,
	// which is also referred to as ROW.

	// Determine the current nx: perpendicular to telescope axis nz
	// and in the direction of the satellite motion.
	telescope.nx = 
	  normalize_vector(interpolate_vec(ac->entry[attitude_counter].nx, 
					   ac->entry[attitude_counter].time, 
					   ac->entry[attitude_counter+1].nx, 
					   ac->entry[attitude_counter+1].time, 
					   photon.time));
	
	// Remove the component along the vertical direction nz 
	// (nx must be perpendicular to nz!):
	double scp = scalar_product(&telescope.nz, &telescope.nx);
	telescope.nx.x -= scp*telescope.nz.x;
	telescope.nx.y -= scp*telescope.nz.y;
	telescope.nx.z -= scp*telescope.nz.z;
	telescope.nx = normalize_vector(telescope.nx);

	// The third axis of the coordinate system ny is perpendicular 
	// to telescope axis nz and nx:
	telescope.ny=normalize_vector(vector_product(telescope.nz, telescope.nx));
	
	// Determine the photon impact position on the detector (in [m]):
	struct Point2d position;  

	// Convolution with PSF:
	// Function returns 0, if the photon does not fall on the detector. 
	// If it hits the detector, the return value is 1.
	int retval=
	  getCodedMaskImpactPos(&position, &photon, mask, 
				&telescope, focal_length, &status);
	CHECK_STATUS_BREAK(status);
	if (1==retval) {
	  // Check whether the photon hits the detector within the FOV. 
	  // (Due to the effects of the mirrors it might have been scattered over 
	  // the edge of the FOV, although the source is inside the FOV.)
	  //if (sqrt(pow(position.x,2.)+pow(position.y,2.)) < 
	  //    tan(telescope.fov_diameter)*telescope.focal_length) {
	    
	  // Insert the impact position with the photon data into the impact list:
	  fits_insert_rows(ilf->fptr, ilf->row++, 1, &status);
	  fits_write_col(ilf->fptr, TDOUBLE, ilf->ctime, 
			 ilf->row, 1, 1, &photon.time, &status);
	  fits_write_col(ilf->fptr, TFLOAT, ilf->cenergy, 
			 ilf->row, 1, 1, &photon.energy, &status);
	  fits_write_col(ilf->fptr, TDOUBLE, ilf->cx, 
			 ilf->row, 1, 1, &position.x, &status);
	  fits_write_col(ilf->fptr, TDOUBLE, ilf->cy, 
			 ilf->row, 1, 1, &position.y, &status);
	  CHECK_STATUS_BREAK(status);
	  ilf->nrows++;
	  //}
	} // END getCodedMaskImpactPos(...)
      } // End of FOV check.
    } // END of scanning LOOP over the photon list.
    CHECK_STATUS_BREAK(status);
      
  } while(0);  // END of the error handling loop.


  // --- Cleaning up ---
  headas_chat(3, "cleaning up ...\n");

  // release HEADAS random number generator
  HDmtFree();

  // Close the FITS files.
  freeImpactListFile(&ilf, &status);
  freePhotonListFile(&plf, &status);

  freeAttitudeCatalog(&ac);
  destroyCodedMask(&mask);

  if (EXIT_SUCCESS==status) headas_chat(3, "finished successfully!\n\n");
  return(status);
}


int comaimg_getpar(struct Parameters* par)
{
  int status=EXIT_SUCCESS; // Error status.

  // Get the filename of the input photon list (FITS file).
  if ((status = PILGetFname("PhotonList", par->photonlist_filename))) {
    SIXT_ERROR("failed reading the filename of the photon list");
  }
  
  // Get the filename of the Coded Mask file (FITS image file).
  else if ((status = PILGetFname("Mask", par->mask_filename))) {
    SIXT_ERROR("failed reading the filename of the coded mask");
  }

  // Get the filename of the impact list file (FITS output file).
  else if ((status = PILGetFname("ImpactList", par->impactlist_filename))) {
    SIXT_ERROR("failed reading the filename of the impact list output file");
  }

  // Read the distance between the coded mask and the detector plane [m].
  else if ((status = PILGetReal("MaskDistance", &par->mask_distance))) {
    SIXT_ERROR("failed reading the distance between the mask and the detector");
  }
  CHECK_STATUS_RET(status, status);

  return(status);
}




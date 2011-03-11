#include "phgen.h"


void phgen(const GenDet* const det,
	   AttitudeCatalog* const ac,
	   XRaySourceCatalog* const srccat,
	   PhotonListFile* const plf,
	   const double t0, const double t1,
	   int* const status)
{
  // Step width of the time loop.
  const double dt = 1.0;

  // Loop over the specified time interval.
  double time;
  for (time=t0; time<t1; time+=dt) {
    // Display the program progress status.
    headas_chat(0, "\rtime: %.3lf s ", time);
    fflush(NULL);

    // Determine the telescope pointing at the current point of time.
    Vector pointing = getTelescopePointing(ac, time, status);
    CHECK_STATUS_BREAK(*status);
    
    // Get photons for all sources in the catalog.
    LinkedPhoListElement* pholist =
      genFoVXRayPhotons(srccat, &pointing, det->fov_diameter,
			time, time+dt, status);
    CHECK_STATUS_BREAK(*status);
      
    // Process the list of generated photons.
    while(pholist) {
      // Output to the FITS file.
      *status = addPhoton2File(plf, &(pholist->photon));
      CHECK_STATUS_BREAK(*status);
	
      // Delete the processed element.
      LinkedPhoListElement* next = pholist->next;
      free(pholist);
      pholist = next;
    }
    CHECK_STATUS_BREAK(*status);
  }
  // END of loop over the requested time interval.
}


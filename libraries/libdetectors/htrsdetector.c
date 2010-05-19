#include "htrsdetector.h"


int initHTRSDetector(HTRSDetector* hd, 
		     struct HTRSDetectorParameters* parameters)
{
  int status = EXIT_SUCCESS;

  // Set up the initial default HTRS configuration.
  hd->slow_shaping_time = parameters->slow_shaping_time;
  hd->fast_shaping_time = parameters->fast_shaping_time;
  hd->reset_time        = parameters->reset_time;
  hd->nevents = 0;
  hd->nsingles = 0;
  hd->ndoubles = 0;

  // Call the initialization routines of the underlying data structures.
  status = initGenericDetector(&hd->generic, &parameters->generic);
  if (EXIT_SUCCESS!=status) return(status);

#ifdef HTRS_HEXPIXELS
  status = initHexagonalPixels(&hd->pixels, &parameters->pixels);
  if (EXIT_SUCCESS!=status) return(status);
#endif
#ifdef HTRS_ARCPIXELS
  status = initArcPixels(&hd->pixels, &parameters->pixels);
  if (EXIT_SUCCESS!=status) return(status);
#endif

  // Create a new FITS event file and open it.
  status = openNewHTRSEventFile(&hd->eventlist, parameters->eventlist_filename,
				parameters->eventlist_template);
  if (EXIT_SUCCESS!=status) return(status);

  return(status);
}



int cleanupHTRSDetector(HTRSDetector* hd)
{
  int status=EXIT_SUCCESS;

  if (NULL!=hd) {
    // Print some statistical data:
    headas_printf("\nHTRS Detector statistics:\n");
    headas_printf(" number of events: %ld\n", hd->nevents);
    headas_printf(" number of single events: %ld\n", hd->nsingles);
    headas_printf(" number of double events: %ld\n", hd->ndoubles);
		  
    // Call the cleanup routines of the underlying data structures.
#ifdef HTRS_HEXPIXELS
    cleanupHexagonalPixels(&hd->pixels);
#endif
#ifdef HTRS_ARCPIXELS
    cleanupArcPixels(&hd->pixels);
#endif
    cleanupGenericDetector(&hd->generic);

    status = closeHTRSEventFile(&hd->eventlist);
  }

  return(status);
}



int addImpact2HTRSDetector(HTRSDetector* hd, Impact* impact)
{
  int status=EXIT_SUCCESS;

  // Determine a detector channel (PHA channel) according to the RMF.
  // The channel is obtained from the RMF using the corresponding
  // HEAdas routine which is based on drawing a random number.
  long channel;
  ReturnChannel(hd->generic.rmf, impact->energy, 1, &channel);

  // Check if the photon is really measured. If the
  // PHA channel returned by the HEAdas RMF function is equal to '-1', 
  // the photon is not detected.
  // This can happen as the RMF is actually an RSP including, e.g.,
  // the detector quantum efficiency and filter transmission.
  if (-1==channel) {
    return(status); // Break the function (and continue with the next photon).
  }
  assert(channel>=0);

  // Get the corresponding created charge (actually the corresponding
  // energy in [keV]).
  // NOTE: In this simulation the charge is represented by the nominal
  // photon energy which corresponds to the PHA channel according to the
  // EBOUNDS table.
  float charge = getEnergy(channel, hd->generic.rmf);
  
  if (charge > 0.) {

#ifdef HTRS_HEXPIXELS
    // Determine the affected pixel(s) and their corresponding charge fractions.
    int pixel[2];
    int npixels=0; // Number of VALID split partners.
    double fraction[2];
    int nsplits = getHexagonalPixelSplits(&hd->pixels, &hd->generic, 
					  impact->position, pixel, fraction);
    // Loop over all split partners.
    int pixel_counter;
    for (pixel_counter=0; pixel_counter<nsplits; pixel_counter++) {
      if (INVALID_PIXEL != pixel[pixel_counter]) {
	
	// Check if the pixel is currently active.
	if (impact->time - hd->pixels.array[pixel[pixel_counter]].last_impact 
	    < hd->shaping_time) {
	  // The photon cannot be detected in this pixel as it is still within 
	  // the shaping time after the previous event.

	  // TODO Although the current photon cannot be detected, the dead time
	  // of the affected pixel is extended due to its interaction with the
	  // detector (activate the following line to implement paralyzable dead 
	  // time).
	  //  hd->pixels.array[pixel[pixel_counter]].last_impact = impact->time;

	  continue;
	}

	// Add the charge created by the photon to the affected detector 
	// pixel(s).
	HTRSEvent event;

	// Determine the detector channel that corresponds to the charge 
	// fraction created by the incident photon in the regarded pixel.
	event.pha = getChannel(charge * fraction[pixel_counter], hd->generic.rmf);
	//                     |        |-> charge fraction due to split events
	//                     |-> charge created by incident photon
      
	// Check lower threshold (PHA and energy):
	if ((event.pha>=hd->generic.pha_threshold) && 
	    (charge*fraction[pixel_counter]>=hd->generic.energy_threshold)) { 

	  // Check if a valid channel is returned.
	  assert(event.pha >= 0);
	  
	  // The impact has generated an event in this pixel,
	  // so add it to the event file.
	  event.energy = charge * fraction[pixel_counter];
	  event.pixel = pixel[pixel_counter];
	  event.time = impact->time;
	  
	  // Store the time of this impact in the pixel in order to consider the
	  // shaping time.
	  hd->pixels.array[pixel[pixel_counter]].last_impact = impact->time;

	  // Add event to event file.
	  status = addHTRSEvent2File(&hd->eventlist, &event);
	  if (EXIT_SUCCESS!=status) return(status);

	  npixels++;

	} // END Check for thresholds.
      } // END if valid pixel
    } // END of loop over split partners

    // Count the number of events for statistical information.
    if (1==npixels) {
      hd->nsingles++;
      hd->nevents++;
    } else if (2==npixels) {
      hd->ndoubles++;
      hd->nevents++;
    }
#endif

#ifdef HTRS_ARCPIXELS

    // The event can be either a single or a double. Therefore we must be 
    // able to store up to 2 pairs of ring and pixel number.
    int ring[2], number[2]; 
    int npixels = getArcPixelSplits(&hd->pixels, &hd->generic, 
				    impact->position, ring, number);

    if (INVALID_PIXEL != ring[0]) {

      // Create a new event data structure.
      HTRSEvent event = { .grade1 = 0, .pileup = 0 };

      // Check if the pixel is currently active or whether it is in
      // a clearing (reset) phase.
      if (impact->time < 
	  hd->pixels.array[ring[0]][number[0]].reset_from + hd->reset_time) {
	event.grade1 = 4;
	event.grade2 = -1;
      } else {
	// The photon can be detected in this pixel.
	// The pixel is NOT within the clearing (reset) time.
	event.grade1 = 0;
	event.grade2 = 0;
      }

	
      // Determine the detector channel that corresponds to the charge fraction
      // created by the incident photon in the regarded pixel.
      event.pha = getChannel(charge, hd->generic.rmf);
      //                     |-> charge created by incident photon
      
      // Check lower thresholds (PHA and energy):
      if ((event.pha>=hd->generic.pha_threshold) && 
	  (charge>=hd->generic.energy_threshold)) { 
	
	assert(event.pha >= 0);
	
	// The impact has generated an event in this pixel,
	// so add it to the event file.
	event.energy = charge;
	event.pixel = getArcPixelIndex(&(hd->pixels), ring[0], number[0]);
	event.time = impact->time;
	
	// Store the exact impact position of the event for closer analysis.
	event.x = impact->position.x;
	event.y = impact->position.y;
	  
	// Add the newly generated charge to the pixel.
	hd->pixels.array[ring[0]][number[0]].charge += charge;
	// Store the time of this impact in the pixel in order to consider the
	// shaping time (OBSOLETE).
	hd->pixels.array[ring[0]][number[0]].last_impact = impact->time;
	// Check if the pixel has to be resetted (clear the charge).
	// The reset threshold 350 keV.
	if (hd->pixels.array[ring[0]][number[0]].charge > 350.) { 
	  hd->pixels.array[ring[0]][number[0]].charge = 0.;
	  hd->pixels.array[ring[0]][number[0]].reset_from = 
	    impact->time + hd->slow_shaping_time;
	}
	
	// Add event to the event file.
	status = addHTRSEvent2File(&hd->eventlist, &event);
	if (EXIT_SUCCESS!=status) return(status);
	
      } // END Check for thresholds.

      // Count the number of events for statistical information.
      if (1==npixels) {
	hd->nsingles++;
	hd->nevents++;
      } else if (2==npixels) {
	hd->ndoubles++;
	hd->nevents++;
      }
    } // END if valid pixel.
#endif

  } // END if(charge>0.)
 
  return(status);
}



int HTRSassignEventGrades1(HTRSDetector detector)
{
  int status = EXIT_SUCCESS;

  // Assign the right event grades to all events in the event list.
  HTRSEvent event, eventbuffer; // Event buffer.
  long row;
  int nbefore_slow, nbefore_fast, nafter_slow, nafter_fast;
  // Reset the event file row counter to the first line in the file.
  detector.eventlist.generic.row = 0;
  // Loop over all events in the event file.
  while((EXIT_SUCCESS==status) && (0==EventFileEOF(&detector.eventlist.generic))) {
    
    // Read the next event from the FITS file.
    status=HTRSEventFile_getNextRow(&detector.eventlist, &event);
    if(EXIT_SUCCESS!=status) break;

    // Check if the event has happend during a reset interval.
    // In that case a further analysis of the event grade is 
    // not necessary.
    if (4==event.grade1) continue;

    // Former events:
    nbefore_slow=0; nbefore_fast=0;
    row = detector.eventlist.generic.row - 1;
    while (1==EventFileRowIsValid(&detector.eventlist.generic, row)) {
      status = HTRSEventFile_getRow(&detector.eventlist, &eventbuffer, row);
      if (EXIT_SUCCESS!=status) break;
      if (event.time - eventbuffer.time > detector.slow_shaping_time) break;
      if (event.pixel == eventbuffer.pixel) {
	nbefore_slow++;
	if (event.time - eventbuffer.time < detector.fast_shaping_time) {
	  nbefore_fast++;
	}
	// Avoid too many unnecessary loop runs.
	break;
      }
      row--;
    }
    if (EXIT_SUCCESS!=status) break;
    // Subsequent events:
    nafter_slow=0; nafter_fast=0;
    row = detector.eventlist.generic.row + 1;
    while (1==EventFileRowIsValid(&detector.eventlist.generic, row)) {
      status = HTRSEventFile_getRow(&detector.eventlist, &eventbuffer, row);
      if (EXIT_SUCCESS!=status) break;
      if (eventbuffer.time - event.time > detector.slow_shaping_time) break;
      if (event.pixel == eventbuffer.pixel) {
	nafter_slow++;
	if (eventbuffer.time - event.time < detector.fast_shaping_time) {
	  nafter_fast++;
	}
	// Avoid too many unnecessary loop runs.
	break; 
      }
      row++;
    }
    if (EXIT_SUCCESS!=status) break;
    
    // Determine the grade of the event.
    if (nbefore_fast > 0) {
      // Event is not measured at all, because it cannot be distinguished 
      // from the previous event.
      event.grade1 = 3;
    } else if (nafter_fast > 0) {
      // Event is measured, but there is at least one subsequent event
      // that cannot be distinguished from this event.
      event.grade1 = 2;
    } else if (nbefore_slow + nafter_slow > 0) {
      // The event is detected as an individual event, but the energy 
      // cannot be measured with the nominal accuracy, because other
      // events are too close.
      event.grade1 = 1;
    } else {
      // The event is measured with the full nominal energy (and time) 
      // accuracy.
      event.grade1 = 0;
    }

    // Write the event information to the event file.
    status = HTRSEventFile_writeRow(&detector.eventlist, &event, 
				    detector.eventlist.generic.row);
    if (EXIT_SUCCESS!=status) break;
    
  } // END of loop over all events in the event list.

  return(status);
}



int HTRSassignEventGrades2(HTRSDetector detector)
{
  int status = EXIT_SUCCESS;
  
  // Assign the right event grades to all events in the event list.
  HTRSEvent event, eventbuffer; // Event buffer.
  long row;
  int nbefore_only_slow, nbefore_fast, nafter_only_slow, nafter_fast;
  // Reset the event file row counter to the first line in the file.
  detector.eventlist.generic.row = 0;
  // Loop over all events in the event file.
  while((EXIT_SUCCESS==status) && (0==EventFileEOF(&detector.eventlist.generic))) {
    
    // Read the next event from the FITS file.
    status=HTRSEventFile_getNextRow(&detector.eventlist, &event);
    if(EXIT_SUCCESS!=status) break;

    // Check if the event has happend during a reset interval.
    // In that case a further analysis of the event grade is 
    // not necessary.
    if (-1==event.grade2) continue;

    // Former events:
    nbefore_only_slow=0; nbefore_fast=0;
    row = detector.eventlist.generic.row - 1;
    while (1==EventFileRowIsValid(&detector.eventlist.generic, row)) {
      status = HTRSEventFile_getRow(&detector.eventlist, &eventbuffer, row);
      if (EXIT_SUCCESS!=status) break;
      if (event.time - eventbuffer.time > detector.slow_shaping_time) break;
      if (event.pixel == eventbuffer.pixel) {
	if (event.time - eventbuffer.time < detector.fast_shaping_time) {
	  nbefore_fast++;
	} else {
	  nbefore_only_slow++;
	  // Avoid too many unnecessary loop runs.
	  break;
	}
      }
      row--;
    }
    if (EXIT_SUCCESS!=status) break;
    // Subsequent events:
    nafter_only_slow=0; nafter_fast=0;
    row = detector.eventlist.generic.row + 1;
    while (1==EventFileRowIsValid(&detector.eventlist.generic, row)) {
      status = HTRSEventFile_getRow(&detector.eventlist, &eventbuffer, row);
      if (EXIT_SUCCESS!=status) break;
      if (eventbuffer.time - event.time > detector.slow_shaping_time) break;
      if (event.pixel == eventbuffer.pixel) {
	if (eventbuffer.time - event.time < detector.fast_shaping_time) {
	  nafter_fast++;
	} else {
	  nafter_only_slow++;
	  // Avoid too many unnecessary loop runs.
	  break; 
	}
      }
      row++;
    }
    if (EXIT_SUCCESS!=status) break;
    
    // Determine the grade of the event.
    if (nbefore_fast > 0) {
      // Event is not measured at all, because it cannot be distinguished 
      // from the previous event.
      event.grade2 = -2;
    } else if ((nbefore_only_slow>0)||(nafter_only_slow>0)) {
      // Event is detected as event with degraded energy information.
      event.grade2 = 1;
    } else {
      // Event is detected as nominal event.
      event.grade2 = 0;
    }

    // Check for pile-up.
    if ((nbefore_fast>0)||(nafter_fast>0)) {
      event.pileup = 1;
    } else {
      event.pileup = 0;
    }
    
    // Write the event information to the event file.
    status = HTRSEventFile_writeRow(&detector.eventlist, &event, 
				    detector.eventlist.generic.row);
    if (EXIT_SUCCESS!=status) break;
    
  } // END of loop over all events in the event list.
  
  return(status);
}



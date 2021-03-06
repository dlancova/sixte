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


   Copyright 2014 Jelle de Plaa, SRON, Thorsten Brand, FAU
   Copyright 2015-2019 Remeis-Sternwarte, Friedrich-Alexander-Universitaet
                       Erlangen-Nuernberg
*/

#include "tesdatastream.h"

TESDataStream* newTESDataStream(int* const status){

  TESDataStream* stream=(TESDataStream*)malloc(sizeof(TESDataStream));
  if(stream==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for TESDataStream failed");
    return(stream);
  }

  stream->Npix=0;
  stream->Ntime=0;
  stream->time=NULL;
  stream->adc_value=NULL;

  return stream;
}

void allocateTESDataStream(TESDataStream* stream,
			   long Nt,
			   int Npix,
			   int* const status)
{
  stream->Npix=Npix;
  stream->Ntime=Nt;

  stream->time=(double*)malloc(Nt*sizeof(double));
  if(stream->time==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for time array failed.");
    CHECK_STATUS_VOID(*status);
  }
  stream->adc_value=(uint16_t**)malloc(Nt*sizeof(uint16_t*));
  if(stream->adc_value==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for time adc_value failed.");
    CHECK_STATUS_VOID(*status);
  }
  int ii;
  for(ii=0; ii<Nt; ii++){
    stream->adc_value[ii]=(uint16_t*)malloc(Npix*sizeof(uint16_t));
    if(stream->adc_value[ii]==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for time adc_value failed.");
      CHECK_STATUS_VOID(*status);
    }
  }
}

void destroyTESDataStream(TESDataStream* stream){
  if(stream==NULL){
    return;
  }

  if(stream->time!=NULL){
    free(stream->time);
    stream->time=NULL;
  }
  if(stream->adc_value!=NULL){
    int ii;
    for(ii=0; ii<stream->Ntime; ii++){
      if(stream->adc_value[ii]!=NULL){
	free(stream->adc_value[ii]);
	stream->adc_value[ii]=NULL;
      }
    }
    free(stream->adc_value);
    stream->adc_value=NULL;
  }
  stream->Npix=0;
  stream->Ntime=0;
}

TESPulseProperties* newTESPulseProperties(int* const status){

  TESPulseProperties* prop=(TESPulseProperties*)malloc(sizeof(TESPulseProperties));
  if(prop==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for TESPulseProperties failed");
    return(prop);
  }

  prop->Npix=0;
  prop->pixID=NULL;
  prop->versionID=NULL;
  prop->templates=NULL;

  return prop;
}

void destroyTESPulseProperties(TESPulseProperties* prop){

  if(prop->pixID!=NULL){
    free(prop->pixID);
    prop->pixID=NULL;
  }
  if(prop->versionID!=NULL){
    free(prop->versionID);
    prop->versionID=NULL;
  }
  if(prop->templates!=NULL){
    destroyTESProfiles(prop->templates);
    prop->templates=NULL;
  }
}

TESFitsStream* newTESFitsStream(int* const status){

  TESFitsStream* stream=(TESFitsStream*)malloc(sizeof(TESFitsStream));
  if(stream==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for TESFitsStream failed");
    return(stream);
  }

  stream->Npix=0;
  stream->Ntime=0;
  stream->time=NULL;
  stream->pixID=NULL;
  stream->adc_value=NULL;
  stream->name[0]='\0';

  return stream;
}

void destroyTESFitsStream(TESFitsStream* stream){

  if(stream->pixID!=NULL){
    free(stream->pixID);
    stream->pixID=NULL;
  }

  if(stream->time!=NULL){
    free(stream->time);
    stream->time=NULL;
  }

  if(stream->adc_value!=NULL){
    int ii;
    for(ii=0; ii<stream->Npix; ii++){
      if(stream->adc_value[ii]!=NULL){
	free(stream->adc_value[ii]);
	stream->adc_value[ii]=NULL;
      }
    }
    free(stream->adc_value);
    stream->adc_value=NULL;
  }
  stream->Npix=0;
  stream->Ntime=0;
}

void createTESFitsStreamFile(fitsfile **fptr,
			     char *filename,
			     char* const telescop,
			     char* const instrume,
			     char* const filter,
			     char* const ancrfile,
			     char* const respfile,
			     char* const xmlfile,
			     char* const impactlist,
			     const double mjdref,
			     const double timezero,
			     const double tstart,
			     const double tstop,
			     const char clobber,
			     int* const status)
{
  fits_create_file_clobber(fptr,filename,clobber,status);
  CHECK_STATUS_VOID(*status);
  int logic=(int)'T';
  int bitpix=8;
  int naxis=0;
  fits_update_key(*fptr, TLOGICAL, "SIMPLE", &(logic), NULL, status);
  fits_update_key(*fptr, TINT, "BITPIX", &(bitpix), NULL, status);
  fits_update_key(*fptr, TINT, "NAXIS", &(naxis), NULL, status);

  sixt_add_fits_stdkeywords_obsolete(*fptr, 1, telescop, instrume, filter,
			    ancrfile, respfile, mjdref, timezero,
			    tstart, tstop, status);
  CHECK_STATUS_VOID(*status);

  //Write XML and pixel impact file info into header
  fits_update_key(*fptr,TSTRING,"XMLFILE",xmlfile,"detector XML description",status);
  fits_update_key(*fptr,TSTRING,"PIXFILE",impactlist,"Pixel impact file for this stream",status);

  CHECK_STATUS_VOID(*status);
}

void allocateTESFitsStream(TESFitsStream* stream,
			   long Nt,
			   int Npix,
			   int* const status)
{
  stream->Npix=Npix;
  stream->Ntime=Nt;

  stream->pixID=(int*)malloc(Npix*sizeof(int));
  CHECK_NULL_VOID(stream->pixID,*status,
		  "Memory allocation failed for TESFitsStream: pixID");

  stream->time=(double*)malloc(Nt*sizeof(double));
  CHECK_NULL_VOID(stream->time,*status,
		  "Memory allocation failed for TESFitsStream: time");

  stream->adc_value=(uint16_t**)malloc(Npix*sizeof(uint16_t*));
  CHECK_NULL_VOID(stream->adc_value,*status,
		  "Memory allocation failed for TESFitsStream: adc_value");

  int ii;
  for(ii=0; ii<Npix; ii++){
    stream->adc_value[ii]=(uint16_t*)malloc(Nt*sizeof(uint16_t));
    CHECK_NULL_VOID(stream->adc_value[ii],*status,
		    "Memory allocation failed for TESFitsStream: adc_value[i]");
  }
}

void writeTESFitsStream(fitsfile *fptr,
			TESFitsStream *stream,
			double tstart,
			double tstop,
			double timeres,
			long *Nevts,
			int ismonoen,
			float monoen,
			int* const status)
{

  // Create table
  long nrows=(long)stream->Ntime;
  int ncolumns=1+stream->Npix;
  int tlen=9;

  char *ttype[ncolumns];
  char *tform[ncolumns];
  char *tunit[ncolumns];

  int ii;

  for(ii=0; ii<ncolumns; ii++){
    ttype[ii]=(char*)malloc(tlen*sizeof(char));
    if(ttype[ii]==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for ttype failed");
      CHECK_STATUS_VOID(*status);
    }
    tform[ii]=(char*)malloc(tlen*sizeof(char));
    if(tform[ii]==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for tform failed");
      CHECK_STATUS_VOID(*status);
    }
    tunit[ii]=(char*)malloc(tlen*sizeof(char));
    if(tunit[ii]==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for tunit failed");
      CHECK_STATUS_VOID(*status);
    }
  }
  CHECK_STATUS_VOID(*status);

  ttype[0]="TIME";
  tform[0]="1D";
  tunit[0]="s";

  for(ii=0; ii<stream->Npix; ii++){
    sprintf(ttype[ii+1], "PXL%05d", stream->pixID[ii]+1);
    sprintf(tform[ii+1], "1U");
    sprintf(tunit[ii+1], "ADC");
  }

  fits_create_tbl(fptr, BINARY_TBL, 0, ncolumns,
	ttype, tform, tunit, "TESDATASTREAM", status);
  CHECK_STATUS_VOID(*status);

  // Write columns
  fits_write_col(fptr, TDOUBLE, 1, 1, 1, nrows, stream->time, status);
  CHECK_STATUS_VOID(*status);
  for(ii=0; ii<stream->Npix; ii++){
    fits_write_col(fptr, TUSHORT, 2+ii, 1, 1, nrows, stream->adc_value[ii], status);
    CHECK_STATUS_VOID(*status);

    char nev[tlen];
    sprintf(nev, "NES%05d", stream->pixID[ii]+1);
    fits_update_key(fptr, TLONG, nev, &Nevts[stream->pixID[ii]],
      "Number of simulated events in pixel stream", status);
    CHECK_STATUS_VOID(*status);
  }
  CHECK_STATUS_VOID(*status);

  int firstpix, lastpix;

  // Write header keywords
  fits_update_key(fptr, TDOUBLE, "TSTART",
        &tstart, "Start time of data stream", status);
  fits_update_key(fptr, TDOUBLE, "TSTOP",
        &tstop, "Stop time of data stream", status);
  fits_update_key(fptr, TDOUBLE, "DELTAT",
        &timeres, "Time resolution of data stream", status);
  fits_update_key(fptr, TINT, "NPIX",
        &(stream->Npix), "Number of pixel streams in extension", status);
  firstpix=stream->pixID[0]+1;
  fits_update_key(fptr, TINT, "FIRSTPIX",
        &(firstpix), "ID of first pixel in extension", status);
  lastpix=stream->pixID[stream->Npix-1]+1;
  fits_update_key(fptr, TINT, "LASTPIX",
        &(lastpix), "ID of last pixel in extension", status);
  CHECK_STATUS_VOID(*status);

  if(ismonoen==1){
    fits_update_key(fptr, TFLOAT, "MONOEN",
        &monoen, "Monochromatic energy of photons [keV]", status);
    CHECK_STATUS_VOID(*status);
  }
  CHECK_STATUS_VOID(*status);

  // free allocated memory
  for (ii=0; ii<ncolumns; ii++) {
    free(ttype[ii]);
    free(tform[ii]);
    free(tunit[ii]);
  }

}

void appendTESFitsStream(fitsfile *fptr,
			TESFitsStream *stream,
			double tstart,
			double tstop,
			long *Nevts,
			int* const status)
{

  // append to a table
  //
  // NOTE: THIS FUNCTION IS NOT WELL TESTED YET - USE WITH CAUTION
  //
  long nrows=(long)stream->Ntime;
  int ncolumns=1+stream->Npix;

  fits_movnam_hdu(fptr,BINARY_TBL,"TESDATASTREAM",0,status);
  CHECK_STATUS_VOID(*status);

  // update header keywords
  fits_update_key(fptr, TDOUBLE, "TSTART",
        &tstart, "Start time of data stream", status);
  CHECK_STATUS_VOID(*status);
  fits_update_key(fptr, TDOUBLE, "TSTOP",
        &tstop, "Stop time of data stream", status);
  CHECK_STATUS_VOID(*status);

  // get the required column numbers
  int timecol;
  fits_get_colnum(fptr,CASESEN,"TIME",&timecol,status);
  int ii;
  char ttype[9];
  int colnum[ncolumns];
  for(ii=0; ii<stream->Npix; ii++){
    sprintf(ttype, "PXL%05d", stream->pixID[ii]+1);
    fits_get_colnum(fptr,CASESEN,ttype,colnum+ii,status);
  }
  CHECK_STATUS_VOID(*status);

  // Update header keywords (doing this first avoid too many seeks in the
  // table)
  for(ii=0;ii<stream->Npix; ii++) {
    char nev[9];
    sprintf(nev, "NES%05d", stream->pixID[ii]+1);
    fits_update_key(fptr, TLONG, nev, &Nevts[stream->pixID[ii]],
      "Number of simulated events in pixel stream", status);
    CHECK_STATUS_VOID(*status);
  }

  // get number of rows in the table
  LONGLONG tablenrows;
  fits_get_num_rowsll(fptr, &tablenrows, status);
  CHECK_STATUS_VOID(*status);

  // append data to the table
  fits_write_col(fptr, TDOUBLE, timecol, tablenrows+1, 1, nrows, stream->time, status);
  CHECK_STATUS_VOID(*status);
  for(ii=0; ii<stream->Npix; ii++){
    fits_write_col(fptr, TUSHORT, colnum[ii], tablenrows+1, 1, nrows,
		   stream->adc_value[ii], status);
    CHECK_STATUS_VOID(*status);
  }

}

void getTESDataStream(TESDataStream* TESData,
		PixImpFile* PixFile,
		TESProfiles* TESProf,
		AdvDet* det,
		double tstart,
		double tstop,
		int Ndetpix,
		int Nactive,
		int* activearray,
		long* Nevts,
		int *ismonoc,
		float *monoen,
		unsigned long int seed,
		int* const status)
{

	/* Parameters that need to be obtained from elsewhere */

	double SampleFreq;     /* Sample Frequency / From XML 10^6 Hz */
	int Npix;              /* Number of active pixels*/
	AdvPix** simulated_pixels = getSimulatedPixelArray(det,activearray,Ndetpix,Nactive,status); /* Array containing pointers to the pixels actually simulated */

	/* Local variables */
	int inoise;            /* Position index in noise buffer */
	int ipix;              /* Pixel index */
	double t;              /* Time index */
	double PixVal;         /* Pixel value (double) */
	PixImpact impact;      /* Current impact */


	/* Status of getNextImpactFromPixImpFile */
	int piximpstatus=0;
	int evtpixid=-1; // PixID of event

	// total number of simulated events
	long ntot=0;

	/* Initialize running indices */
	t=tstart;
	inoise=NOISEBUFFERSIZE;

	/* Temporary definition of variables / Exposure of 10 ms */
	SampleFreq=det->SampleFreq;    	/* Hertz */
	Npix=Nactive;       		  	/* Number of pixels */

	long Nt=(tstop-tstart)*SampleFreq; // Number of time steps
	long tstep=0; // active timestep

	/* Initialize rng */
	gsl_rng *rng;
	setNoiseGSLSeed(&rng, seed);

	/* allocate output stream structure */
	allocateTESDataStream(TESData, Nt, Npix, status);
	CHECK_STATUS_VOID(*status);

	/* Initialize Noise buffer */
	NoiseBuffer* NBuffer=newNoiseBuffer(status, &Npix);
	CHECK_STATUS_VOID(*status);

	/* Initialize 1/F noise arrays */
	NoiseOoF** OFNoise=NULL;
	if (det->oof_activated){
		OFNoise = malloc(Nactive*sizeof(*OFNoise));
		if(OFNoise==NULL){
			*status=EXIT_FAILURE;
			SIXT_ERROR("Memory allocation for OFNoise failed");
			CHECK_STATUS_VOID(*status);
		}
		for(int i=0;i<Nactive;i++){
			if(simulated_pixels[i]->TESNoise->OoFRMS!=0.){
				OFNoise[i]=newNoiseOoF(status,&rng,SampleFreq,simulated_pixels[i]);
				CHECK_STATUS_VOID(*status);
			} else{
				OFNoise[i]=NULL;
			}
		}
	}

	/* Initialize array of linked lists containing active pulses */
	EvtNode** ActPulses=NULL;
	ActPulses=newEventNodes(&Npix,status);
	CHECK_STATUS_VOID(*status);
	if(ActPulses==NULL){
		*status=EXIT_FAILURE;
		SIXT_ERROR("ActPulses was NULL after memory allocation.");
	}
	CHECK_STATUS_VOID(*status);

	t=tstart;
	long t_long = 0;
	EvtNode *current;

	printf("Simulate %ld time steps for %d pixels.\n", Nt, Npix);

	*ismonoc=1;

	/* While loop over all time bins */
	while (tstep<Nt) {

		/* Write time stamp */
		TESData->time[tstep]=t;

		/* Fill Noise buffer */
		if (inoise==NOISEBUFFERSIZE) {
			genNoiseSpectrum(simulated_pixels,NBuffer,&SampleFreq,&rng,status);
			CHECK_STATUS_VOID(*status);
			inoise=0;
		}

		/* Calculate next state of 1/f noise base array */
		if(det->oof_activated){
			getNextOoFNoiseSumval(OFNoise, &rng,Nactive);
		}


		/* Get first event from the impact file */
		if (tstep==0) {
			piximpstatus=getNextImpactFromPixImpFile(PixFile,&impact,status);
			CHECK_STATUS_VOID(*status);
		}

		/* If the event occurs in this time bin and is in an active pixel, */
		/* add it to the node list */
		while ((piximpstatus>0) &&(impact.time>=t)&&(impact.time<t+(1.0/SampleFreq))) {
			evtpixid=checkPixIfActive(impact.pixID, Ndetpix, activearray);
			if(evtpixid>-1){
				int profver=det->pix[impact.pixID].profVersionID;
				int eindex=findTESProfileEnergyIndex(TESProf,
						profver,
						impact.energy);
				addEventToNode(ActPulses,TESProf,&impact,evtpixid,profver,eindex,status);
				CHECK_STATUS_VOID(*status);
				Nevts[impact.pixID]=Nevts[impact.pixID]+1;
				if(ActPulses[evtpixid]==NULL){
					*status=EXIT_FAILURE;
					SIXT_ERROR("Added impact but pointer is NULL.");
					CHECK_STATUS_VOID(*status);
				}
				CHECK_STATUS_VOID(*status);
				if(*ismonoc==1){
					if(ntot==0){
						*monoen=impact.energy;
					}else{
						if(impact.energy!=(*monoen)){
							*monoen=0.;
							*ismonoc=0;
						}
					}
				}
				ntot++;
			}
			CHECK_STATUS_VOID(*status);
			piximpstatus=getNextImpactFromPixImpFile(PixFile,&impact,status);
			CHECK_STATUS_VOID(*status);
		}

		for (ipix=0;ipix<Npix;ipix++) {
			/* Add the offset first */
			TESData->adc_value[tstep][ipix]=(uint16_t)simulated_pixels[ipix]->ADCOffset;
			PixVal=0.;

			/* Add 1/f noise to the pixel value (double) */
			CHECK_STATUS_VOID(*status);
			if(det->oof_activated && simulated_pixels[ipix]->TESNoise->OoFRMS!=0.){
				PixVal= PixVal + OFNoise[ipix]->Sumrval + gsl_ran_gaussian(rng,OFNoise[ipix]->Sigma);
			}
			CHECK_STATUS_VOID(*status);

			/* Add noise to the pixel value (double) */
			PixVal=PixVal + NBuffer->Buffer[inoise][ipix];

			/* Loop over linked list and add pulse values */
			current=ActPulses[ipix];
			while (current!=NULL) {
				PixVal=PixVal + simulated_pixels[ipix]->calfactor * current->adcpulse[(long)(current->count)];
				CHECK_STATUS_VOID(*status);
				current->count=current->count+(1./SampleFreq)/(current->time[1]-current->time[0]);
				current=current->next;
			}

			/* The digitization step */
			double tesdbl=TESData->adc_value[tstep][ipix] + PixVal;
			if(tesdbl<0.){
				tesdbl=0.;
			}
			if(tesdbl>65534.){//maximum coded value -1
				tesdbl=65534.;
			}
			TESData->adc_value[tstep][ipix]=(uint16_t)round(tesdbl); //TODO Noise buffer seems to contain repeated shapes. Needs to be investigated.
			if(TESData->adc_value[tstep][ipix]==65535){
				printf("tstep=%ld ipix=%d noise=%lf, inoise=%d, tesdbl=%le\n", tstep, ipix, PixVal, inoise, tesdbl);
			}

			/* If the end of the Pulse template is reached, remove the event */
			while (ActPulses[ipix]!=NULL && ActPulses[ipix]->count>=(double)(ActPulses[ipix]->Nt-1)) {
				removeEventFromNode(ActPulses,&ipix);
				CHECK_STATUS_VOID(*status);
			}
		}

		/* Go to next time step */
		inoise=inoise+1;
		t_long++;
		t=tstart+t_long/SampleFreq;
		tstep++;

	}

	/* Clean dynamic memory */
	for (ipix=0;ipix<Npix;ipix++) {
		destroyEventNode(ActPulses[ipix]);
	}
	destroyNoiseBuffer(NBuffer,status);
	free(simulated_pixels);
}


EvtNode** newEventNodes(int *NPixel, int* const status) {
  int i;
  EvtNode **ActPulses = NULL;
  ActPulses = (EvtNode**)malloc(*NPixel*sizeof(EvtNode*));
  if(ActPulses==NULL){
    *status=EXIT_FAILURE;
    SIXT_ERROR("memory allocation for ActPulses failed");
    return(ActPulses);
  }
  for (i=0;i<*NPixel;i++) {
    ActPulses[i]=NULL;
  }
  return ActPulses;
}

int addEventToNode(EvtNode** ActPulses,
		   TESProfiles* Pulses,
                   PixImpact* impact,
		   int pixno,
		   int versionID,
		   int EnID,
		   int* const status)
{
   int i;

   EvtNode *current=NULL, *end;

   /* Allocate space for the new node */
   current = (EvtNode*)malloc(sizeof(EvtNode));
   if(current==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for current failed");
      return(EXIT_FAILURE);
   }
   current->next=NULL;

   current->time=(double*)malloc(Pulses->profiles[versionID].Nt*sizeof(double));
   if(current->time==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for current->time failed");
      return(EXIT_FAILURE);
   }
   current->adcpulse=(double*)malloc(Pulses->profiles[versionID].Nt*sizeof(double));
   if(current->adcpulse==NULL){
      *status=EXIT_FAILURE;
      SIXT_ERROR("memory allocation for current->adcpulse failed");
      return(EXIT_FAILURE);
   }

   for (i=0;i<Pulses->profiles[versionID].Nt;i++) {
     current->time[i]=Pulses->profiles[versionID].time[i];
     current->adcpulse[i]=impact->energy * Pulses->profiles[versionID].adc_value[EnID][i];
   }
   current->count=0.;
   current->Nt=Pulses->profiles[versionID].Nt;

   if(ActPulses[pixno]==NULL){
     ActPulses[pixno]=current;
   }else{
     end=ActPulses[pixno];    /* Set pointer to the head of the list */
     while (end->next!=NULL) {  /* Go through the list to find the end */
       end=end->next;
     }
     end->next=current;
   }

   return EXIT_SUCCESS;
}


void removeEventFromNode(EvtNode** ActPulses,
			int* pixel)
{

   /* Set temporary pointers for pop-node and shift-node */
   EvtNode *pop, *shift;

   /* Pointer to node that needs to go (pop) and it's replacement (shift) */
   pop = ActPulses[*pixel];
   shift = ActPulses[*pixel]->next;
   ActPulses[*pixel] = shift;

   /* Release memory of the node */
   free(pop->time);
   free(pop->adcpulse);
   free(pop);
}

void destroyEventNode(EvtNode* node) {
  if (node!=NULL) {
    destroyEventNode(node->next);
    free(node->time);
    free(node->adcpulse);
    free(node);
  }
}

int checkPixIfActive(int pixID, int Npix, int* activearray){
  if(pixID<Npix && activearray[pixID]>-1){
    return activearray[pixID];
  } else {
    return -1;
  }
}

AdvPix** getSimulatedPixelArray(AdvDet* det,const int* const activearray,const int Ndetpix,const int Nactive,int* const status){
	int current_pixel=0;
	AdvPix** simulated_pixels = malloc(Nactive*sizeof(*simulated_pixels));
	if (NULL==simulated_pixels){
		*status=EXIT_FAILURE;
		SIXT_ERROR("Memory allocation for simulated_pixels failed");
		CHECK_STATUS_RET(*status,simulated_pixels);
	}
	for (int i=0;i<Ndetpix;i++){
		if(activearray[i]>-1){
			if (current_pixel>=Nactive){
				*status=EXIT_FAILURE;
				SIXT_ERROR("More active pixels than expected at begining of tesstream generation");
				CHECK_STATUS_RET(*status,simulated_pixels);
			}
			simulated_pixels[current_pixel]=&(det->pix[i]);
			current_pixel++;
		}
	}
	return(simulated_pixels);
}

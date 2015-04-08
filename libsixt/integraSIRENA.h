/************************************************************************************************

   This file is part of SIXTE/SIRENA software.

   SIXTE/SIRENA is free software: you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   any later version.

   SIXTE/SIRENA is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   For a copy of the GNU General Public License see
   <http://www.gnu.org/licenses/>.

   Copyright 2014:  This file has been developed by the INSTITUTO DE FISICA DE 
   CANTABRIA (CSIC-UC) with funding from the Spanish Ministry of Science and 
   Innovation (MICINN) under project  ESP2006-13608-C02-01, and Spanish 
   Ministry of Economy (MINECO) under projects AYA2012-39767-C02-01 and
   ESP2013-48637-C2-1-P.

************************************************************************************************/

#ifndef INTEGRASIRENA_H
#define INTEGRASIRENA_H 1

#include "tesrecord.h"
#include "teseventlist.h"



typedef struct {
  /** Duration of the pulse template in the library */
  int template_duration;

  /** Array containing the pulse template */
  double* ptemplate;

  /** Energy of the template */
  double energy;

  /** Pulse Height of the template */
  double pulse_height;

} PulseTemplate;

typedef struct {
  /** Duration of the matched filter in the library */
  int mfilter_duration;

  /** Array containing the matched filter */
  double* mfilter;

  /** Energy of the mfilter */
  double energy;

  /** Pulse Height of the mfilter */
  double pulse_height;

} MatchedFilter;

typedef struct {
  // Duration of the optimalfilter
  int ofilter_duration;

  // Array containing the optimal filter
  double* ofilter;

  // Normalization factor
  double nrmfctr;

} OptimalFilterSIRENA;

typedef struct {
  /** Number of templates & matched filters in the structure. */
  int ntemplates;

  /** Energies of the templates */
  double* energies;

  /** Pulse Heights of the templates */
  double* pulse_heights;

  /** Array containing all the pulse templates from the library */
  PulseTemplate* pulse_templates;

  /** Array containing all the pulse templates filtered & derivated from the library */
  PulseTemplate* pulse_templates_filder;

  /** Array containing all the pulse templates from the library */
  PulseTemplate* pulse_templates_B0;
  
  /** Array containing all the matched filters from the library */
  MatchedFilter* matched_filters;

  /** Array containing all the matched filters from the library */
  MatchedFilter* matched_filters_B0;

} LibraryCollection;

typedef struct {
  
  /** Duration of the noise spectrum */
  int noise_duration;
	
  /** Array containing the noise spectrum */
  double* noisespec;

  /** Array containing the frequecies of the noise spectrum */
  double* noisefreqs;

} NoiseSpec;

typedef struct {

  /** Pulse duration (maximum length) */
  int pulse_duration;

  /** Length of filter used during reconstruction (samples)*/
  int grade1;
 
  /** Distance to previous pulse in record (samples)*/
  int grade2;
  
  /** PIX_ID of the detected pulse*/
  int pixid;
  
  /** Array containing the pulse adc values */
  double* pulse_adc;

  /** Start time of the Pulse */
  double Tstart;

  /** End time of the Pulse */
  double Tend;
 
  /** Rise time of the Pulse */
  double riseTime;

  /** Fall time of the Pulse */
  double fallTime;

  /** Pulse height of the Pulse */
  double pulse_height;

  /** Uncalibrated Energy (eV) of the Pulse */
  double ucenergy;

  /** Calibrated Energy (eV) of the Pulse */
  double energy;

  /** Quality of the Pulse */
  double quality;

} PulseDetected;

typedef struct {
  /** Number of detected pulses in the structure. **/
  int ndetpulses;

  /** Array containing all the pulses detetected in record**/
  PulseDetected* pulses_detected;

} PulsesCollection;

/** Structure containing all the pointers and values to run the reconstruction stage */
typedef struct {
 	/**                   **/
	/** SIRENA parameters **/
	/**                   **/
        /** LibraryCollection structure (pulse templates and matched filters)*/		
	LibraryCollection* library_collection;
	
	/** Library file (to be used or to be created) **/
	char library_file[256];
	
	/** records file (if any) **/
	char record_file[256];
	
	/** Noise file (to be used) **/
	char noise_file[256];

	/** Pulse length */
	int pulse_length;
	
	/** Pulses Fall time (s)**/
    double tauFall;

	/** Detection scaleFactor (0.005 – no filtering) **/
	double scaleFactor;
	
	/** Detection samplesUp (samples to confirm threshold overcoming **/
    double samplesUp;

	/** Detection nSgms (sigmas to establish a threshold for detection) **/
	double nSgms;

    /** Library creation mode? (Y:1, N:0) **/
	int crtLib;

	/** Monochromatic energy for library creation **/
	double monoenergy;
	
	/** Running sum length for the RS raw energy estimation, in seconds (only in crtLib=0) **/
	double LrsT;
	
	/** Baseline averaging length for the RS raw energy estimation, in seconds (only in crtLib=0) **/
	double LbT;
	
    /** Run mode (0: calibration/lib creation  1:energy reconstruction) **/
	int mode;

    /** Noise spectrum **/
	NoiseSpec* noise_spectrum;

	/** Filtering Domain: T (Time) or F (Frequency) **/
    char FilterDomain[2];

    /** Filtering Method: F0 (deleting the zero frequency bin) or F0 (deleting the baseline) **/
    char FilterMethod[3];

    /** Energy Calibration type: Linear (1), Quadratic (2) **/
    int calibLQ;

    /** Linear Calibration Factor (externally input) **/
	double b_cF;

    /** Quadratic Calibration Factor (externally input) **/
    double c_cF;

	/** Write intermediate files **/
	int intermediate;
	
	/** File with the output detections **/
	char detectFile[256];
	
	/** File with the output detections **/
	char filterFile[256];
	
	/** Second records file (if mode=0 & crtLib=0) **/
	char record_file2[256];
	double monoenergy2;

	/** Overwrite files? **/
	int clobber;
	
} ReconstructInitSIRENA;

/** Destructor. */
#ifdef __cplusplus
extern "C"
#endif
void freeReconstructInitSIRENA(ReconstructInitSIRENA* reconstruct_init);

/** Constructor. Returns a pointer to an empty ReconstructInitSIRENA data structure. */
#ifdef __cplusplus
extern "C"
#endif
ReconstructInitSIRENA* newReconstructInitSIRENA(int* const status);

/** Initializes the structure ReconstructInitSIRENA with the variables required for SIRENA reconstruction */
#ifdef __cplusplus
extern "C"
#endif
/*void initializeReconstructionSIRENA(ReconstructInitSIRENA* reconstruct_init, char* const library_file, char* const record_file,
	double tauFall,	int pulse_length, double scaleFactor, double samplesUp, double nSgms, int crtLib, int mode, double LrsT, 
	double LbT,char* const noise_file, char* filter_domain, char* filter_method, int calibLQ,  double b_cF, double c_cF,
	double monoenergy, int interm, char* detectFile, char* filterFile, char clobber, int* const status);*/
void initializeReconstructionSIRENA(ReconstructInitSIRENA* reconstruct_init, char* const library_file, char* const record_file,
	double tauFall,	int pulse_length, double scaleFactor, double samplesUp, double nSgms, int crtLib, int mode, double LrsT,
	double LbT,char* const noise_file, char* filter_domain, char* filter_method, int calibLQ,  double b_cF, double c_cF,
	double monoenergy, int interm, char* detectFile, char* filterFile, char* record_file2, double monoenergy2, char clobber, int* const status);

/** Constructor. Returns a pointer to an empty PulsesCollection data structure. */
#ifdef __cplusplus
extern "C"
#endif
PulsesCollection* newPulsesCollection(int* const status);

/** Destructor. */
#ifdef __cplusplus
extern "C"
#endif
void freePulsesCollection(PulsesCollection* PulsesColl);

/** Constructor. Returns a pointer to an empty OptimalFilterSIRENA data structure. */
#ifdef __cplusplus
extern "C"
#endif
OptimalFilterSIRENA* newOptimalFilterSIRENA(int* const status);

/** Destructor. */
#ifdef __cplusplus
extern "C"
#endif
void freeOptimalFilterSIRENA(OptimalFilterSIRENA* PulsesColl);

#ifdef __cplusplus
extern "C"
#endif
//void writeCalibKeywords(outfile->fptr,b_cF, c_cF);
//void writeCalibKeywords(fitsfile* fptr, double const b_cF, double const c_cF, int* const status);
void writeCalibKeywords(fitsfile* fptr, double b_cF, double c_cF, int* const status);

/** Run reconstruction method with an option for SIRENA*/
#ifdef __cplusplus
extern "C"
#endif
void reconstructRecordSIRENA(TesRecord* record,TesEventList* event_list, ReconstructInitSIRENA* reconstruct_init, int lastRecord, int nRecord, PulsesCollection **pulsesAll, OptimalFilterSIRENA **optimalFilter, int* const status);






/** Create and retrieve a LibraryCollection from a file. */
LibraryCollection* getLibraryCollection(const char* const filename, int* const status);

/** LibraryCollection constructor. Returns a pointer to an empty LibraryCollection data structure. */
LibraryCollection* newLibraryCollection(int* const status);

/** LibraryCollection Destructor. */
void freeLibraryCollection(LibraryCollection** const library_collection);

/** Allocates memory for an LibraryCollection structure. */
void allocateLibraryCollection(LibraryCollection* library_collection,int ntemplates,int* const status);


/** Create and retrieve a NoiseSpec from a file. */
NoiseSpec* getNoiseSpec(const char* const filename,int* const status);

/** NoiseSpec constructor. Returns a pointer to an empty NoiseSpec data structure. */
NoiseSpec* newNoiseSpec(int* const status);

/** Allocates memory for a NoiseSpec structure. */
void allocateNoiseSpec(NoiseSpec* noise_spectrum,int noise_duration,int* const status);

/** Allocates memory for a Pulsetemplate structure. */
void allocatePulseTemplate(PulseTemplate* pulse_template, int template_duration,int* const status);
/** PulseTemplate destructor. */
void freePulsetemplate(PulseTemplate* pulse_template);


/** Allocates memory for a MatchedFilter structure. */
void allocateMatchedFilter(MatchedFilter* matched_filter,int mfilter_duration,int* const status);
/** MatchedFilter destructor. */
void freeMatchedFilter(MatchedFilter* matched_filter);

#ifdef __cplusplus
extern "C"
#endif
void runEnergyCalib(ReconstructInitSIRENA* reconstruct_init, PulsesCollection* pulsesAll_runEnergyCalib, PulsesCollection* pulsesAll2_runEnergyCalib, double *b_cF_runEnergyCalib, double *c_cF_runEnergyCalib);

#endif /* INTEGRASIRENA_H */




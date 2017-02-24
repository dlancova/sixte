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


   Copyright 2016 Thomas Dauser, FAU
*/

#ifndef CROSSTALK_H
#define CROSSTALK_H 1

#include "sixt.h"
#include "advdet.h"
#include "pixelimpact.h"

#define CROSSTALK_ID_NONE 0
#define CROSSTALK_ID_ALL 1
#define CROSSTALK_ID_THERM 2
#define CROSSTALK_ID_ELEC 3
#define CROSSTALK_ID_IMOD 4
#define INITXTALKNB 100
#define INITEVTPROXYNB 10

#define IMODCTK -31
#define THERCTK -32
#define ELECCTK -33

////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////

typedef struct {
	int* pixid;
	int* chan;
	double* freq;
	int len;
} channel_list;

typedef struct{
	PixImpact ** xtalk_impacts;
	int n_active_crosstalk;
	int* type;
	int* is_saved;
	int n_xtalk_pileup;
	long current_crosstalk_index;
	int xtalk_proxy_size;
}CrosstalkProxy;

typedef struct{
	double next,current,previous;
}gradingTimeStruct;

typedef struct{
	PixImpact *impact;
	CrosstalkProxy* xtalk_proxy;
	gradingTimeStruct * times;
	double crosstalk_energy;
	int nb_crosstalk_influence;
	long row;
	int is_first;
}GradeProxy;

typedef struct{
	PixImpact** impact;
	int event_proxy_size;
	int ind_grading_previous;
	int ind_grading_current;
	int ind_grading_next;
	int ind_event;
	int nb_active;
}EventProxy;


////////////////////////////////////////////////////////////////////////
// Function Declarations.
////////////////////////////////////////////////////////////////////////

/** Add crosstalk to proxy */
void addCrosstalk2Proxy(CrosstalkProxy* xtalk_proxy,PixImpact* impact, int type, double df, int* const status);

/**  Binary search for to find interpolation interval
 *   - return value is the bin [ind,ind+1]
 *   - assume list is sorted ascending */
int binary_search(double val, double* arr, int n);

/**Checks for pile-up or trigger of event*/
void checkTrigger(AdvDet* det, PixImpact* impact, CrosstalkProxy* xtalk_proxy, GradeProxy* grade_proxy, double* energies, TesEventFile* event_file,
		double next_time, double sample_length,int grade,int* is_trigger, int save_crosstalk,int* const status);

/** Compute total crosstalk influence */
void computeAllCrosstalkInfluence(AdvDet* det,PixImpact * impact,CrosstalkProxy* xtalk_proxy, GradeProxy* grade_proxy, TesEventFile* event_file,double* xtalk_energy,int* nb_influences,
		double next_time, double sample_length, int* is_trigger, int save_crosstalk, int grade, int ignore_influence,int full_proxy, int* const status);

/** Compute influence of the crosstalk event on an impact using the timedependence table */
int computeCrosstalkInfluence(CrosstalkTimedep* buffer,PixImpact* impact,PixImpact* crosstalk, double energy, double* influence);

/** Computes the time dependencies of the ctk events*/
void computeTimeDependency(AdvDet* det, CrosstalkProxy* xtalk_proxy,PixImpact * impact, double* energies, double* xtalk_energy,int* nb_influences,
		TesEventFile* event_file, int save_crosstalk, int grade, int ignore_influence,int full_proxy,int* const status);

/** Computes weights on a given impact given its grading */
void computeWeights(AdvDet* det, CrosstalkProxy* xtalk_proxy,PixImpact * impact, double* energies,
		int grade,int* const status);

/** Conversion energy/SQUID amplitude*/
double conv_ener2ampli(double ener);

/** free the channel list */
void free_channel_list(channel_list** chans);

/** Destructor of CrosstalkProxy structure */
void freeCrosstalkProxy(CrosstalkProxy** xtalk_proxy);

/** Frees event Proxy*/
void freeEventProxy(EventProxy** proxy);

/** Gets frequency differences*/
double get_imod_df(double f_sig, double f_per, int* status);

/** Computing intermodulation weight*/
double get_intermod_weight(AdvDet* det, int grading, double df, double dt,
		double ampli_perturber, double ampli_signal, int* const status);

/** Load the Channel-Frequency List */
ReadoutChannels* get_readout_channels(AdvDet* det, int* status);

CrosstalkTimedep* getTimeDep(AdvDet* det, CrosstalkProxy* xtalk_proxy, int ii, int grade, int* const status);

/** Destructor of the ARF library structure */
void init_crosstalk(AdvDet* det, int* const status);

/** Initializing readout*/
ReadoutChannels* init_newReadout(int num_chan, int* status);

/** Interpolation in multi-d&*/
double interp_lin_ndim(void* inp_arr, int* iarr, double* fac, int ndim);

/** Load the Channel-Frequency List */
channel_list* load_channel_list(char* fname, int* status);

// Loads electrical cross-talk for requested pixel
// Concretely, iterates over all the pixels of the channel
void load_electrical_cross_talk(AdvDet* det,int pixid,int* const status);

/** Electrical time dependence*/

void load_elec_timedep(AdvDet* det, int k, int* const status);

/** Electrical table*/
void load_elec_table(AdvDet* det, int k ,int* status);

/** Load intermodulation cross talk information into a single AdvPix*/
void load_intermod_cross_talk(AdvDet* det, int* status);

/** Load imod time dependency*/
void load_imod_timedep(AdvDet* det, int k, int* const status);

// Loads thermal cross-talk for requested pixel
// Concretely, iterates over all the pixels to find neighbours
void load_thermal_cross_talk(AdvDet* det,int pixid,int* const status);

/** Thermal time dependence*/
void load_thermal_timedep(AdvDet* det, int i, int k, int* const status);

/** Constructor of CrosstalkProxy structure */
CrosstalkProxy* newCrosstalkProxy(int* const status);

/** Constructor of Event Proxy type*/
EventProxy* newEventProxy(int* const status);

/** read one electrical crosstalk matrix from a fits file and store it in the structure (units given in keV) */
void read_elec_matrix(fitsfile* fptr,int n_freq_s, int n_freq_p, int n_ener_p,
		ElecTab* tab, char* extname, int* status);

/** read one intermodulation matrix from a fits file and store it in the structure */
void read_intermod_matrix(fitsfile* fptr,int n_ampl, int n_dt, int n_freq,
		ImodTab* tab, char* extname, int* status);

/**Reversing array*/
void reverse_array_dbl(double* arr, int n);

/**Stores event type*/
void storeEventtype(CrosstalkProxy* xtalk_proxy, int type, double df, int* const status);

/** Processes a graded event : update grading proxy and save previous event */
void processGradedEvent(GradeProxy* grade_proxy, const double sample_length,PixImpact* next_impact,
		AdvDet* det,TesEventFile* event_file, int is_crosstalk, int save_crosstalk, int grdcmp, int* const status);


#endif /* CROSSTALK_H */



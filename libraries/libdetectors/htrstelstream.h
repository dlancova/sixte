#ifndef HTRSTELSTREAM_H
#define HTRSTELSTREAM_H 1

#include "sixt.h"
#include "htrsevent.h"
#include "telemetrypacket.h"


////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////


typedef struct {

  /** TelemetryPacket containing the actual data. */
  TelemetryPacket* tp;

  /** Number of bits reserved for the packet header.
   * This number is included in value nbits. */
  int n_header_bits;
  /** Number of bits per spectrum bin. */
  int n_bin_bits;

  /** Number of RSP Channels. */
  int n_channels; 
  /** Number of bins for the spectrum. */
  int n_bins; 

  /** Maximum number of counts per spectral bin. Due to the limited
   * number of bits per spectral bin, the maximum number of counts per
   * bin is limited. The following variable represents this
   * maximum. */
  int max_counts;

  /** Binary output file. The data added to the HTRSTelemetryStream is
   * written to this binary output file. */
  FILE* output_file;

  /** Buffer for the binned spectrum. The number of elements is given by
   * n_bins. */
  int* spectrum;
  /** Start time of spectrum. This value is used to determine whether
   * events belong the current binned spectrum, or whether a new one
   * has to be started. */
  double spectrum_time;
  /** Integration time for the binned spectrum. If the time of an
   * event is bigger than the sum of spectrum_time and
   * integration_time, it does not belong to the current spectrum, but
   * a new one has to be started. */
  double integration_time;

  /** Lookup table, which RSP channels are binned to which spectrum bins. 
   * The array contains n_channels elements. */
  int* chans2bins; 
  
} HTRSTelStream;


struct HTRSTelStreamParameters {
  int n_packet_bits;
  int n_header_bits;
  int n_bin_bits;

  int n_channels; 
  int n_bins; 

  double integration_time;

  /** Filename of the binary output file. */
  char* output_filename;

  /** Lookup table, which RSP channels are binned to which spectrum bins. */
  int* chans2bins; 
};


/////////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////////


/** Constructor of the HTRSTelStream object. Allocate memory
 * and set up the initial configuration. */
HTRSTelStream* getHTRSTelStream
(struct HTRSTelStreamParameters* parameters, int* status);

/** Destructor of the HTRSTelStream object. Release the
 * allocated memory. */
void freeHTRSTelStream(HTRSTelStream* stream);

/** Add a new event to the HTRSTelStream. The function adds HTRSEvent
 * objects to the HTRSTelStream. The return values is the error
 * status. */
int addEvent2HTRSTelStream(HTRSTelStream* stream, HTRSEvent* event);

/** Start a new TelemetryPacket and insert the packet header at the
 * beginning. */
void newHTRSTelStreamPacket(HTRSTelStream* stream);


#endif /* HTRSTELSTREAM_H */

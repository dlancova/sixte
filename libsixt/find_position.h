#ifndef FIND_POSITION_H
#define FIND_POSITION_H 1

#include "sixt.h"
#include "sourceimage.h"
#include "squarepixels.h"

////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////
typedef struct {
  int midPixX, midPixY;     //position of original brightest pix (middle pix of SourceNeighbors)
  double posX, posY;        //position of source in pixel-coordinates
                            //(weighted position of all significant pixels)
  double posRA, posDEC;     //position of source in ra,dec
  double pixval;            //value of source-pixel
  /* double errorRA, errorDEC;*/ //error of found ra,dec compared to originally given value (source.simput)
} PixPosition;

typedef struct  {
  double** neighbor_list;    //array of neighboring pixels
  int neighborAmount;  //count for number of significant neighbors of current source
} SourceNeighbors;

typedef struct {
  PixPosition** entry;          //pointer to current entry of PixPosition
  long unsigned int entryCount; //count for all PixPosition-elements (all sources);
                                //and also all SourceNeighbors-elements (max 8 per source)
  SourceNeighbors** neighbors;  //pointer to current entry of SourceNeighbors
  int** found_pos;         //value of 'found_pos' equals (1D) index of sky image (zero else) to be 
                                   //able to compare both
} PixPositionList;


/////////////////////////////////////////////////////////////////////
// Function Declarations.
/////////////////////////////////////////////////////////////////////
PixPosition* getPixPosition(); //returns PixPosition structure

PixPositionList* getPixPositionList(SourceImage* sky_pixels); //returns PixPositionList structure

SourceNeighbors* getSourceNeighbors();


//returns value of current brightest source
double findBrightestPix(int threshold, SourceImage* sky_pixels, double pixval, PixPositionList* ppl,
		        struct wcsprm* wcs, int* const status);

void findNeighbors(int x, int y, PixPositionList* ppl, SourceImage* sky_pixels, struct wcsprm* wcs, int* const status);

double* getMedian_list(SourceImage* sky_pixels, int* const status);

//returns '1' for pix that is still 20% (arbitrary) above mean of all left pix, '2' else
int getThresholdForSources(double pix, PixPositionList* ppl, SourceImage* sky_pixels,
			   double* median_list, double factorOfSigma);

double getSdev(double* median_list,long unsigned int n);

//returns medianvalue of all remaining pix (all except already identified sources)
//to replace getMeanValue
double torben(double* m,long unsigned int n);
double getMean(double* median_list,long unsigned int n);

//saves PixPositionList to a FITS-file
void savePositionList(PixPositionList* ppl, char* filename, int* status);
void SaveSkyImage3Columns(SourceImage* si,char* filename,int* status);

void FreePixPositionList(PixPositionList* ppl);

void FreeLists(int** found_pos, double*  median_list, SourceImage* sky_pixels);

#endif /* FIND_POSITION_H */


#ifndef SIXT_H
#define SIXT_H 1

#if HAVE_CONFIG_H
#include <config.h>
#else
#error "Do not compile outside Autotools!"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <expat.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ape/ape_trad.h"
#include "fitsio.h"
#include "pil.h"
#include "headas.h"
#include "headas_error.h"
#include "headas_rand.h"



/////////////////////////////////////////////////////////////////
// Constants.
/////////////////////////////////////////////////////////////////


/** Maximum length of a filename. */
#define MAXFILENAME (512)
/** Maximum length of a message string. */
#define MAXMSG (512)

/** Not a Number. */
#define SIXT_NAN (0./0.)


/////////////////////////////////////////////////////////////////
// Macro definitions.
/////////////////////////////////////////////////////////////////


/** Macro returning the maximum of 2 values. */
#define MAX(a, b) ( (a)>(b) ? (a) : (b) )
/** Macro returning the minimum of 2 values. */
#define MIN(a, b) ( (a)<(b) ? (a) : (b) )


// Error reporting macros.
#define SIXT_ERROR(msg) (sixt_error(__func__, msg))

#define CHECK_STATUS_BREAK(status) \
  if (EXIT_SUCCESS!=status) break;

#define CHECK_STATUS_RET(status, retval) \
  if (EXIT_SUCCESS!=status) return(retval);

#define CHECK_STATUS_VOID(status) \
  if (EXIT_SUCCESS!=status) return;


#define CHECK_NULL_VOID(a,status,msg) \
  if (NULL==a) { \
    SIXT_ERROR(msg); \
    status=EXIT_FAILURE; \
    return;\
  }

#define CHECK_NULL_BREAK(a,status,msg) \
  if (NULL==a) { \
    SIXT_ERROR(msg); \
    status=EXIT_FAILURE; \
    break;\
  }

#define CHECK_NULL_RET(a,status,msg,ret) \
  if (NULL==a) { \
    SIXT_ERROR(msg); \
    status=EXIT_FAILURE; \
    return(ret);	 \
  }

#define CHECK_NULL(a,status,msg) CHECK_NULL_RET(a,status,msg,NULL);


/////////////////////////////////////////////////////////////////
// Function declarations.
/////////////////////////////////////////////////////////////////


/** This routine returns a random number created by the HEAdas random
    number generator. It is basically a wrapper around the HEAdas
    routine HDmtDrand(). The return values lie in the interval
    [0,1). The function requires HDmtInit() to be called once before
    usage in order to initialize the HEAdas random number
    generator. When the random number generator is not needed any
    more, it can be realeased with HDmtFree(). Information can be
    found in the HEAdas developer's guide or directly in the source
    files 'headas_rand.h' and 'headas_rand.c'. */
double sixt_get_random_number();


/** Returns a random value on the basis of an exponential distribution
    with a given average distance. In the simulation this function is
    used to calculate the temporal differences between individual
    photons from a source. The photons have Poisson statistics. */
double rndexp(const double avg);


/** Determine 2 (!) Gaussian distributed random numbers using the
    Box-Muller method (Gould & Tobochnik, p. 432). The standard
    deviation of the random numbers is 1. */
void get_gauss_random_numbers(double* const x, double* const y);


/** Convert a squence of chars into captial letters. The sequence has
    to be terminated by a '\0' mark. */
void strtoupper(char* string);


/** Print the given error message for an error occured in the
    specified function. The function name is also part of the
    output. */
void sixt_error(const char* const func, const char* const msg);


#endif /* SIXT_H */


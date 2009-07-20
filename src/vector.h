#ifndef VECTOR_H
#define VECTOR_H 1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>


/** 3-dimensional vector. Data structure with three double-valued components. */
typedef struct {
  double x;
  double y;
  double z;
} Vector;


/** Creates a unit vector for specified right ascension and declination.
 * Angles have to be given in [rad]. */
Vector unit_vector(const double ra, const double dec);

/** Returns a normalized vector of length 1.0 with the same direction as the original vector).*/
Vector normalize_vector(Vector);
/** Faster than normalize_vector. 
 * Deals with pointer instead of handling structures at function call. */
inline void normalize_vector_fast(Vector* v);

// calculates the scalar product of two vectors
inline double scalar_product(Vector* const, Vector* const);

// calculates the vector product of two vectors
Vector vector_product(Vector, Vector);

/** Calculates the difference between two vectors. */
Vector vector_difference(Vector x2, Vector x1);

/** Function interpolates between two vectors at time t1 and t2 for the specified time 
 * and returns the interpolated vector. */
Vector interpolate_vec(Vector v1, double t1, Vector v2, 
		       double t2, double time);
Vector interpolate_vec2(Vector v1, double t1, Vector v2, double t2, double time);

/** Function determines the equatorial coordinates of right ascension 
 * and declination for a given vector pointing in a specific direction. 
 * The angles are calculated in [rad].
 * The given vector doesn't have to be normalized. */
void calculate_ra_dec(Vector v, /**< Direction. Does not have to be normalized. */
		      double* ra, /**< Right ascension. Unit: [rad], Interval: [-pi;pi]. */ 
		      double* dec /**< Declination. Unit: [rad], Interval: [-pi/2;pi/2]. */);


#endif /* VECTOR_H */
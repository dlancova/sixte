#include "vector.h"


// creates a unit vector from given theta and phi (in astronomical coordiantes)
struct vector unit_vector(double rasc, double dec)
{
  struct vector x;
  double cos_dec;   // buffer variable

  cos_dec = cos(dec);

  x.x = cos_dec * cos(rasc);
  x.y = cos_dec * sin(rasc);
  x.z = sin(dec);

  return(x);
}



// returns a normalized vector (length 1.0, same direction)
struct vector normalize_vector(struct vector x) {
  double l;         // length of the vector x
  struct vector y;  // normalized vector

  l = sqrt(pow(x.x,2.0)+pow(x.y,2.0)+pow(x.z,2.0));

  y.x = x.x/l;
  y.y = x.y/l;
  y.z = x.z/l;

  return(y);
}



// calculates the scalar product of two vector structures
double scalar_product(struct vector x, struct vector y)
{
  return(x.x*y.x + x.y*y.y + x.z*y.z);
}



// calculates the vector product of two vectors
struct vector vector_product(struct vector x, struct vector y) {
  struct vector z;  // return vector

  z.x = x.y*y.z-x.z*y.y;
  z.y = x.z*y.x-x.x*y.z;
  z.z = x.x*y.y-x.y*y.x;

  return(z);
}

 

// Function interpolates between two vectors at time t1 and t2 for the specified time 
// and returns the interpolated vector.
struct vector interpolate_vec(struct vector v1, double t1, struct vector v2, double t2, double time) {
  struct vector pos;
  
  pos.x = v1.x + (time-t1)/(t2-t1)*(v2.x-v1.x);
  pos.y = v1.y + (time-t1)/(t2-t1)*(v2.y-v1.y);
  pos.z = v1.z + (time-t1)/(t2-t1)*(v2.z-v1.z);

  return(pos);
}

#ifndef KDTREE_H
#define KDTREE_H 1

#include "sixt.h"
#include "vector.h"
#include "sourcelist.h"

#ifndef HEASP_H
#define HEASP_H 1
#include "heasp.h"
#endif


////////////////////////////////////////////////////////////////////////
// Type Declarations.
////////////////////////////////////////////////////////////////////////


/** Node in the kdTree containing the X-ray sources in 3-dimensional
    space. */
struct structkdNode {
  /** Data structure representing the X-ray source. */
  struct SourceListEntry source;

  struct structkdNode* left; /**< Pointer to node on the left. */
  struct structkdNode* right; /**< Pointer to node on the right. */
};
typedef struct structkdNode kdNode;


////////////////////////////////////////////////////////////////////////
//   Function declarations
////////////////////////////////////////////////////////////////////////


/** Build up the kdTree from the given list of sources. As the
    function concept is based on recursive function calls, we also
    have to hand over the current depth of the recursion. */
kdNode* kdTreeBuild(SourceList* list, long nelements, int depth);

/** Perform a range search on the given kbTree, i.e., return all X-ray
    sources lying within a certain radius around the reference
    point. Note that the square of the search radius is required! New
    sources are appended at the end of the SourceList and the number
    of the returned X-ray sources is stored in the nelements
    parameter. The function return value is the error status. */
int kdTreeRangeSearch(kdNode* node, int depth,
		      Vector* ref, double radius2, 
		      SourceList** list, long *nelements);

/** Destructor. */
void freeKDTree(kdNode* tree);


#endif /* KDTREE_H */
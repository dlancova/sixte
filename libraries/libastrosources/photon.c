#include "photon.h"


float photon_energy(Spectrum* spectrum, struct RMF* rmf)
{
  // Get a random PHA channel according to the given PHA distribution.
  float rand = (float)sixt_get_random_number();
  long upper = spectrum->NumberChannels-1, lower=0, mid;
  
  if(rand > spectrum->rate[spectrum->NumberChannels-1]) {
    printf("PHA sum: %f < RAND: %f\n", 
	   spectrum->rate[spectrum->NumberChannels-1], rand);
  }

  // Determine the energy of the photon.
  while (upper-lower>1) {
    mid = (long)((lower+upper)/2);
    if (spectrum->rate[mid] < rand) {
      lower = mid;
    } else {
      upper = mid;
    }
  }
    
  if (spectrum->rate[lower] < rand) {
    lower = upper;
  }

  // Return an energy chosen randomly out of the determined PHA bin:
  return(rmf->ChannelLowEnergy[lower] + 
	 sixt_get_random_number()*(rmf->ChannelHighEnergy[lower]-
				   rmf->ChannelLowEnergy[lower]));
}



int create_photons(PointSource* ps /**< Source data. */,
		   double time /**< Current time. */, 
		   double dt /**< Time interval for photon generation. */,       
		   /** Address of pointer to time-ordered photon list.*/
		   struct PhotonOrderedListEntry** list_first,
		   struct RMF* rmf)
{
  // Second pointer to photon list, that can be moved along the list,   
  // without loosing the first entry.
  struct PhotonOrderedListEntry* list_current = *list_first;

  int status=EXIT_SUCCESS;

  // If there is no photon time stored so far, set the current time.
  if (ps->t_last_photon < 0.) {
    ps->t_last_photon = time;
  }


  // Create photons and insert them into the given time-ordered list:
  Photon new_photon; // Buffer for new Photon.
  while (ps->t_last_photon < time+dt) {
    new_photon.ra = ps->ra;
    new_photon.dec = ps->dec; 
    new_photon.direction = unit_vector(ps->ra, ps->dec); // REMOVE

    // Determine the energy of the new photon
    new_photon.energy = photon_energy(ps->spectrum, rmf);

    // Determine the impact time of the photon according to the light
    // curve of the source.
    if (NULL==ps->lc) {
      if (T_LC_CONSTANT==ps->lc_type) {
	ps->lc = getLinLightCurve(1, &status);
	if (EXIT_SUCCESS!=status) break;
	status = initConstantLinLightCurve(ps->lc, ps->rate, time, 1.e6);
	if (EXIT_SUCCESS!=status) break;
      } else if (T_LC_TIMMER_KOENIG==ps->lc_type) {
	ps->lc = getLinLightCurve(131072, &status);
	if (EXIT_SUCCESS!=status) break;
	status = initTimmerKoenigLinLightCurve(ps->lc, time, TK_LC_STEP_WIDTH, ps->rate,
					       ps->rate/3.);
	if (EXIT_SUCCESS!=status) break;
      } else {
	status=EXIT_FAILURE;
	HD_ERROR_THROW("Error: Unknown light curve type!\n", EXIT_FAILURE);
	break;
      }
    }

    if (time > ps->lc->t0 + ps->lc->nvalues*ps->lc->step_width) {
      if (T_LC_CONSTANT==ps->lc_type) {
	status = initConstantLinLightCurve(ps->lc, ps->rate, time, 1.e6);
	if (EXIT_SUCCESS!=status) break;
      } else if (T_LC_TIMMER_KOENIG==ps->lc_type) {
	status = initTimmerKoenigLinLightCurve(ps->lc, time, TK_LC_STEP_WIDTH, ps->rate,
					       ps->rate/3.);
	if (EXIT_SUCCESS!=status) break;
      }
    }

    new_photon.time = getPhotonTime(ps->lc, ps->t_last_photon);
    //assert(new_photon.time>ps->t_last_photon);
    if (new_photon.time<ps->t_last_photon) {
      printf("Error in subsequent photon times: %lf < %lf\n", 
	     new_photon.time, ps->t_last_photon);
      exit(-1);
    }
    ps->t_last_photon = new_photon.time;

    // Insert photon to the global photon list:
    if ((status=insert_Photon2TimeOrderedList(list_first, &list_current, &new_photon)) 
	!= EXIT_SUCCESS) break;  
    
  } // END of loop 'while(t_last_photon < time+dt)'

  return(status);
}



void clear_PhotonList(struct PhotonOrderedListEntry** pole)
{
  struct PhotonOrderedListEntry* buffer;

  while (NULL!=*pole) {
    buffer = (*pole)->next;
    free(*pole);
    *pole = buffer;
  }
}



int insert_Photon2TimeOrderedList(struct PhotonOrderedListEntry** first,
				  struct PhotonOrderedListEntry** current, 
				  Photon* ph) 
{
  // The iterator is shifted over the time-ordered list to find the right entry.
  struct PhotonOrderedListEntry** iterator = current;

  // Find the right entry where to insert the new photon.
  while ((NULL!=*iterator) && ((*iterator)->photon.time < ph->time)) {
    iterator = &((*iterator)->next);
  }
  // Now '*iterator' points to the entry before which the new photon has to be inserted.
  // I.e. 'iterator' is equivalent to '&[fromer_entry]->next'. 
  // Therefore changeing '*iterator' means redirecting the chain.
  // '*iterator' has to be redirected to the new entry.
    
  // Create a new PhotonOrderedListEntry and insert it before '**iterator'.
  struct PhotonOrderedListEntry* new_entry = 
    (struct PhotonOrderedListEntry*)malloc(sizeof(struct PhotonOrderedListEntry));
  if (NULL==new_entry) {
    HD_ERROR_THROW("Error: Could not allocate memory for new photon entry!\n", EXIT_FAILURE);
    return(EXIT_FAILURE);
  }

  // Set the values of the new entry:
  new_entry->photon = *ph;
  new_entry->next = *iterator;
  if (*iterator == *first) {
    // If the new photon was inserted as first entry of the list, the pointer to this 
    // first entry has to be redirected.
    *first = new_entry;
  }
  *iterator = new_entry;

  // The pointer '*current' should point to the new entry in the time-ordered list.
  *current = new_entry;
  
  return(EXIT_SUCCESS);
}



int insert_Photon2BinaryTree(struct PhotonBinaryTreeEntry** ptr, Photon* ph)
{
  int status = EXIT_SUCCESS;

  if (NULL==*ptr) {
    // The tree is completely empty so far. Therefore create the first
    // tree element and redirect the pointer from the calling routine to it.
    *ptr = (struct PhotonBinaryTreeEntry*)malloc(sizeof(struct PhotonBinaryTreeEntry));
    if (NULL==*ptr) { // Check if memory allocation was successfull.
      status = EXIT_FAILURE;
      HD_ERROR_THROW("Error: memory allocation for binary tree failed!\n", status);
      return(status);
    }
    (*ptr)->photon = *ph;
    (*ptr)->sptr = NULL;
    (*ptr)->gptr = NULL;

  } else { 
    // There is a first element (root) of the tree. So we have to find
    // the position where to insert the new element.
    struct PhotonBinaryTreeEntry** next=NULL;
    if ((*ptr)->photon.time > ph->time) {
      next = &(*ptr)->sptr;
    } else {
      next = &(*ptr)->gptr;
    }
    
    while (NULL!=*next) {
      // We have to go deeper into the tree. So decide whether the new photon
      // is earlier or later than the current entry.
      if ((*next)->photon.time > ph->time) {
	next=&(*next)->sptr;
      } else {
	next=&(*next)->gptr;
      }
    }

    // We have reached one of the ends of the tree. Insert the photon here.
    *next = (struct PhotonBinaryTreeEntry*)malloc(sizeof(struct PhotonBinaryTreeEntry));
    if (NULL==*next) { // Check if memory allocation was successfull.
      status = EXIT_FAILURE;
      HD_ERROR_THROW("Error: memory allocation for binary tree failed!\n", status);
      return(status);
    }
    (*next)->photon = *ph;
    (*next)->sptr = NULL;
    (*next)->gptr = NULL;

  }

  return(status);
}



int CreateOrderedPhotonList(struct PhotonBinaryTreeEntry** tree_ptr,
			    struct PhotonOrderedListEntry** list_first,
			    struct PhotonOrderedListEntry** list_current)
{
  int status=EXIT_SUCCESS;

  // Check if the current tree entry exists.
  if (NULL != *tree_ptr) {

    // Add the entries before the current one to the time-ordered list.
    status = CreateOrderedPhotonList(&(*tree_ptr)->sptr, list_first, list_current);
    if (EXIT_SUCCESS != status) return(status);

    // Insert the current entry into the time-ordered list at the right position.
    status = insert_Photon2TimeOrderedList(list_first, list_current, &(*tree_ptr)->photon);
    if (EXIT_SUCCESS != status) return(status);
    
    // Add the entries after the current one to the time-ordered list.
    status = CreateOrderedPhotonList(&(*tree_ptr)->gptr, list_first, list_current);
    if (EXIT_SUCCESS != status) return(status);

    // Delete the binary tree entry, as it is not required any more.
    free(*tree_ptr);
    *tree_ptr=NULL;

  } // END if current tree entry exists.

  return(status);
}
						       


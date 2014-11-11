#ifndef RAND_FUNCS_H
#define RAND_FUNCS_H

#include <vector>
#include <algorithm>
#include <time.h> 

#include <sim/core.h>

struct c_unique {
  int current;
  c_unique() {current=0;}
  int operator()() {return current++;}
} SeqFromZero;

uvec get_shuffled_indices(int n_elem) {
	std::vector<unsigned int> ids(n_elem);
    std::generate (ids.begin(), ids.end(), SeqFromZero);    
    std::time_t timer = std::time(NULL);
 	random_shuffle (ids.begin(), ids.end());
 	uvec ids_v(&ids[0], n_elem);    
 	return ids_v;
}

#endif
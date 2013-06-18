#ifndef DEBUG_FUNCS_H
#define DEBUG_FUNCS_H


#include <sim/core.h>

void print_m(mat v) {
	for(size_t i=0; i<v.n_cols;i++) {
		 	for(size_t j=0; j<v.n_rows;j++) {
				std::cout << v(i,j) << "|";
			}
		std::cout<< std::endl;
		}
	std::cin.ignore();
}

#endif
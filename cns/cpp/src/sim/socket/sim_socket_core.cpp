
#include "sim_client.h"

#include <string.h>

#include <sim/core.h>

#define HEAD_LEN 100
#define PORT 7778

void send_arma_mat(mat m, std::string name, long unsigned int* iter=NULL, bool verbose=false) {
	double *p = m.memptr();
	const char *name_c = name.c_str()	;
	char head_m[HEAD_LEN];
	if(iter == NULL) {
		snprintf(head_m, 100, "%s:%d:%d@",name_c, m.n_rows, m.n_cols);
	} else 	{
		snprintf(head_m, 100, "%s%d:%d:%d@",name_c,*iter, m.n_rows, m.n_cols);
	}
	
	init_socket(PORT);	
	send_message<char>(head_m, HEAD_LEN);	
	send_message<double>(p, sizeof(double)*m.n_rows*m.n_cols);
	if(verbose) {
		std::cout << "sending: " << head_m << std::endl;
	}		
}

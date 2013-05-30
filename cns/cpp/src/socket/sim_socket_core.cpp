
#include "sim_client.c"
#include <string.h>

#define HEAD_LEN 100

void send_arma_mat(mat m, std::string name) {
	double *p = m.memptr();
	const char *name_c = name.c_str()	;
	char head_m[HEAD_LEN];
	snprintf(head_m, 100, "%d:%d:%s@",m.n_rows, m.n_cols,name_c);
	std::cout << head_m << std::endl;
}
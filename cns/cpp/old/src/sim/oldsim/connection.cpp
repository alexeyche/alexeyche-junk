#include "connection.h"


Connection::Connection(SimElemCommon* pre, SimElemCommon* post) : connMat(pre->size.n_out, post->size.n_in), in(pre->getOutput()), out(post->getInput()) {
	connMat.ones();	
}

void Connection::computeMe(double dt) {
	uvec fired = find(in);
	if(fired.n_elem>0) {		
		vec out_to_all = in.t() * connMat;		
		srand ( time(NULL) );
		int out_i = rand() % (out_to_all.n_elem-5) + 0;
		out(out_i) = out_to_all(out_i);
		out(out_i+1) = out_to_all(out_i+1);
		out(out_i+2) = out_to_all(out_i+2);
		out(out_i+3) = out_to_all(out_i+3);
		out(out_i+4) = out_to_all(out_i+4);
	}		
}
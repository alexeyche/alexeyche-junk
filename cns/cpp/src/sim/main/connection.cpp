#include "connection.h"


Connection::Connection(SimElemCommon* pre, SimElemCommon* post) : connMat(pre->size.n_out, post->size.n_in), in(pre->getOutput()), out(post->getInput()) {
	connMat.ones();	
}

void Connection::computeMe(double dt) {
	uvec fired = find(in);
	if(fired.n_elem>0) {		
		out = in.t() * connMat;		
	}		
}
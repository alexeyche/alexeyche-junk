#include "connection.h"


Connection::Connection(SimElemCommon* pre, SimElemCommon* post) : connMat(pre->size.n_out, post->size.n_in), in(pre->getOutput()), out(post->getInput()) {
	connMat.ones();	
}

void Connection::computeMe(double dt) {
	mat in_post = in * connMat;
	out = in_post.row(0);	
}
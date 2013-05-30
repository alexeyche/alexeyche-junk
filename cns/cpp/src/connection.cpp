#include "connection.h"

Connection::Connection(SimElemCommon* pre, SimElemCommon* post) : pre(pre), post(post), connMat(pre->size.n_out, post->size.n_in) {//
	connMat.zeros();
}

void Connection::computeMe(double dt) {
	vec out_pre = pre->getOutput();
	mat in_post = out_pre * connMat;
	post->setInput(in_post.t());
}
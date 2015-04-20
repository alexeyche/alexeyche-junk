#pragma once

namespace dnn {

struct dnnException : public std::exception
{
	dnnException() {}
	dnnException(dnnException &exc) {
		ss << exc.ss.str();
	}

	template <typename T>
	dnnException& operator << (const T& s) {
		ss << s;
		return *this;
	}
 	const char * what () const throw () {
    	return ss.str().c_str();
  	}
  	stringstream ss;  	
};

struct dnnInterrupt : public std::exception {

};


}
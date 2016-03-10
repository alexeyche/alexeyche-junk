#pragma once

namespace NDnn {

	#ifndef PI
	    #define PI 3.1415926535897932384626433832795028841971693993751
	#endif


	double GetUnif();
	double GetUnifBetween(double low, double high);
	double GetExp(double rate);

	double SampleDelay(double gain, double rate);
	double GetNorm();
	double GetLogNorm(double logmean, double logsd);

} // namespace NDnn


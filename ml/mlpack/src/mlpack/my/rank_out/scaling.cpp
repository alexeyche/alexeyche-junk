
#include "scaling.hpp"

void ScaleMat(arma::mat& dataset, double down, double up) {
	Log::Info << "Perform scaling with lower bound: " << down << ", and upper bound: " << up << std::endl;
	size_t m = dataset.n_rows;
	size_t n = dataset.n_cols;
	arma::rowvec max_d(n);
	arma::rowvec min_d(n);
	double kan_span = up - down;
	for(size_t j=0; j<n; j++){
		min_d(j) = arma::min(dataset.col(j));
		max_d(j) = arma::max(dataset.col(j));
		double span = max_d(j) - min_d(j);
		for(size_t i=0; i<m; i++) {
			double value = dataset(i,j);
			if(value == min_d(j))
				value = down;
			else if(value == max_d(j))
				value = up;
			else
				value = down + kan_span * (value-min_d(j))/span;
			dataset(i,j) = value;
		}		
	}
	Log::Info << "Scaling finished" << std::endl;	
}
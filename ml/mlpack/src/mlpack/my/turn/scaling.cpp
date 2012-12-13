

void ScaleMat(arma::mat& dataset, double up, double down) {
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
			Log::Debug << i << "," << j << ": " << dataset(i,j) << " -> ";
			double k = (span - dataset(i,j))/span;
			dataset(i,j) = k * kan_span;
			Log::Debug << "k: " << k << ", " << dataset(i,j) << std::endl;
		}		
	}	
}
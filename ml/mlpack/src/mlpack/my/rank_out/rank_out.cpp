
#include "rank_out.hpp"
#include "turn.hpp"

arma::colvec rank_out(const arma::mat& dataset) {
	size_t dimensionality = dataset.n_cols;
	
	size_t d1 = math::RandInt(0,dimensionality);
	size_t d2_step = math::RandInt(0,dimensionality-1);
	size_t d2 = d1+d2_step;
	if(d2>dimensionality) {
		d2 -= dimensionality;
	}
	//std::pair<arma::uvec,arma::vec> clust_inf = calculate_clusters(dataset,r,d1,d2);	
}
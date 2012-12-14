#include "turn.hpp"

std::pair<arma::uvec,arma::vec> 
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2) 
{
	size_t m = dataset.n_rows;
	arma::colvec col1 = dataset.unsafe_col(d1);
	arma::colvec col2 = dataset.unsafe_col(d2);
	arma::uvec c1_s = arma::sort_index(col1, 0);
	arma::uvec c2_s = arma::sort_index(col2, 0);

	arma::uvec SNN(m);
	arma::vec dens(m);
	dens.zeros();
	SNN.zeros();
		
	for(size_t i=0; i<m; i++) {
		size_t cur = c1_s[i];
		size_t prev, next;
		if(i!=0) prev = c1_s[i-1];
		if(i!=(m-1)) next = c1_s[i+1];
		double dist_l = fabs(col1(cur) - col1(prev));
		double dist_r = fabs(col1(cur) - col1(next));
		double dist_l2 = fabs(col2(cur) - col2(prev));
		double dist_r2 = fabs(col2(cur) - col2(next));
		if((dist_l <= r) && (dist_l2<=r)) {	
			SNN[cur]++;					
		}
		if((dist_r <= r) && (dist_r2<=r)) {	
			SNN[cur]++;					
		} 
		if(SNN[cur] == 2) {
			dens[cur] = sqrt(pow(dist_l,2.0) + pow(dist_r,2.0));
		}
	}
	for(size_t i=1; i<(m-1); i++) {
		size_t cur = c2_s[i];
		size_t prev = c2_s[i-1];
		size_t next = c2_s[i+1];
		double dist_l = fabs(col2(cur) - col2(prev));
		double dist_r = fabs(col2(cur) - col2(next));
		double dist_l2 = fabs(col1(cur) - col1(prev));
		double dist_r2 = fabs(col1(cur) - col1(next));
		if((dist_l <= r) && (dist_l2<=r)) {	
			SNN[cur]++;					
		}
		if((dist_r <= r) && (dist_r2<=r)) {	
			SNN[cur]++;					
		}
		if(SNN[cur] == 4) {
			dens[cur] = 1/( dens[cur] + sqrt(pow(dist_l,2.0) + pow(dist_r,2.0))); // calculate density for out SNN
		} else if(dens[cur] != 0) {  // it is mean that we calculate this for nothing, but had to try 
			dens[cur] = 0;
		}

	}
	std::pair<arma::uvec,arma::vec> clust_inf(SNN,dens);
	return clust_inf;
}
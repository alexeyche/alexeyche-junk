
#include "rank_out.hpp"
#include "turn.hpp"

void rank_out(const arma::mat& dataset, double r_given) { 
	size_t dimensionality = dataset.n_cols;
	size_t m = dataset.n_rows;
	Log::Debug << "dim: " << dimensionality << std::endl;
	size_t d1 = math::RandInt(0,dimensionality);
	size_t d2_step = math::RandInt(0,dimensionality-1);
	size_t d2 = d1+d2_step;
	if(d2>dimensionality) {
		d2 -= dimensionality;
	}
	d1 = 0;
	d2 = 1;
	size_t counter=0;	
	arma::vec resolutions("0.005  0.01 0.02 0.03 0.04 0.05 0.06 0.07 0.1"); // 0.5 0.75 1");
	std::vector<Clusters*> all_clusters;
	for(size_t r_i=0; r_i<resolutions.n_elem; r_i++) {
		double r = resolutions(r_i);
		Clusters *clusts = calculate_clusters(dataset,r,d1,d2);	
		
		all_clusters.push_back(clusts);
	}
	arma::vec rank(m);
	rank.zeros();
	
	for(size_t i=0; i<m; i++) {
		for(size_t cl_i=0; cl_i<(all_clusters.size()-1); cl_i++) {
			Clusters* cl = all_clusters[cl_i];
			Clusters* cl_next = all_clusters[cl_i+1];		
			int cur_cl = cl->clust_ind[i];
			int next_cl = cl_next->clust_ind[i];
			int cur_size, next_size;
			if (cur_cl == 0) {
				cur_size = 1;
			} else 
				cur_size = cl->clust_size(cur_cl);			
			if(next_cl == 0) {				
				next_size = 1;
			} else 
				next_size = cl_next->clust_size(next_cl);			
			
			rank(i) += (double)(cur_size-1)/(next_size);
			if(rank(i)>100) {
				Log::Debug << std::endl;
			}
		}
	}
	string output = "out.csv";
	data::Save(output.c_str(),rank,false,false);

	for(size_t cl_i; cl_i<all_clusters.size(); cl_i++) {
		Clusters* cl = all_clusters[cl_i];
		delete cl;
	}
}
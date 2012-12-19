#include "turn.hpp"

Clusters
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2) 
{
	Log::Info << "Starting clustering with parameters - " << "dimension1: " << d1 << ", dimension2: " << d2 << ", resolution: " << r << endl;
	size_t m = dataset.n_rows;
	size_t dim = 2; // 2 axis
	
	std::vector<arma::uvec*> col_sort_ind;
	for(size_t ax=0; ax<dim; ax++) {
		arma::colvec col_cur = dataset.unsafe_col(ax);			
		col_sort_ind.push_back(new arma::uvec(arma::sort_index(col_cur, 0)));
	}
	
	arma::umat SNN(m,dim); //SNN for dim independent axis 
	arma::vec dens(m); // density for each point
	arma::uvec clusts(m); //id clust for each point
	dens.zeros();
	SNN.zeros();
	clusts.zeros();
	
	size_t clust_id=1;	// start clusters count from 1, 0 means no cluster
	for(int ax=0; ax<1; ax++) {		
		bool in_clust=false;
		for(size_t i=1; i<(m-1); i++) {
			size_t cur = col_sort_ind[ax]->at(i);
			size_t prev = col_sort_ind[ax]->at(i-1);
			size_t next = col_sort_ind[ax]->at(i+1);

			double dist_l = fabs(dataset(cur,ax) - dataset(prev,ax));
			double dist_r = fabs(dataset(cur,ax) - dataset(next,ax));
			double dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
			double dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
			if((dist_l2 > r) && (dist_l<=r)) {				
				for(int j=i-2; j>=0; j--) {
					prev = col_sort_ind[ax]->at(j);
					dist_l = fabs(dataset(cur,ax) - dataset(prev,ax));
					dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
					if(dist_l > r) {
						break; // break bec. we out of bounds
					}
					if(dist_l2 <= r) {
						break; // break bec. we found good prev point
					}
				}
			}
			if((dist_r2 > r) && (dist_r<=r)) {				
				for(int j=i+2; j<m; j++) {
					next = col_sort_ind[ax]->at(j);
					dist_r = fabs(dataset(cur,ax) - dataset(next,ax));
					dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
					if(dist_r > r) {
						break; // break bec. we out of bounds
					}
					if(dist_r2 <= r) {
						break; // break bec. we found good next point
					}
				}
			}
			if((dist_l <= r) && (dist_r <= r) && (dist_l2 <= r) && (dist_r2 <= r)) { 					
				SNN(cur,ax) += 2;								
				//SNN(prev,ax) += 1;				
				//SNN(next,ax) += 1;
				
				if(clusts(cur)>0) {										
					clusts(prev) = clusts(cur);
					clusts(next) = clusts(cur);
				} else if (clusts(prev)>0) {
					clusts(cur) = clusts(prev);
					clusts(next) = clusts(cur);
				} else {
					clusts(cur) = clust_id;;	
					clusts(next) = clust_id;
					clusts(prev) = clust_id;
					clust_id++;
				}		
			}		
		}
	}

	//Log::Info << "Clusters for each axis perfomed. Axis with max clusters choosing for clustering (";
	//Log::Info << clust_axis << ": " << max_clust << " clusters)";
	//Log::Info << std::endl;

	Clusters clusts0(m);
	
	size_t cur_cl_id = 0;
	
	string clust_out = "clusts.csv";
	data::Save(clust_out.c_str(),clusts,false,false);	
	
	string snn_out = "snn.csv";
	data::Save(snn_out.c_str(),SNN,false,false);	
	// bool in_clust = false;
	// for(size_t i=0; i<m; i++) {
	// 	size_t cur = col_sort_ind[clust_axis]->at(i);				
	// 	if ((SNN(cur,0)>0) || (SNN(cur,1)>0))  {
	// 		if(!in_clust) { 
	// 			clusts.clust_num++; 
	// 			in_clust = true; 
	// 		}				
	// 		clusts.clust_ind(cur) = SNN(cur,0) + SNN(cur,1);			
	// 	} else {
	// 		in_clust = false;
	// 	}
	// }	
	return clusts0;	
}
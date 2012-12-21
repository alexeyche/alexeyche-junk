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
		for(size_t i=0; i<(m-1); i++) {			
			turn_iteration(i,col_sort_ind,dataset,ax,r,clusts,clust_id);									
			// string clust_out = "clusts.csv";
			// data::Save(clust_out.c_str(),clusts,false,false);	
			// std::cin.get();
		}
		for(size_t i=(m-2); i>0; i--) {			
			//if(i==9) { 
			//	Log::Debug << std::endl; 
			//}
		//	turn_iteration(i,col_sort_ind,dataset,ax,r,clusts,clust_id,true);
		}
		//for(size_t i=(m-2); i>0; i--) {
		//	turn_iteration(i,col_sort_ind,dataset,ax,r,clusts,clust_id);
		//}
	}
	string clust_out = "clusts.csv";
	data::Save(clust_out.c_str(),clusts,false,false);	
	//Log::Info << "Clusters for each axis perfomed. Axis with max clusters choosing for clustering (";
	//Log::Info << clust_axis << ": " << max_clust << " clusters)";
	//Log::Info << std::endl;

	Clusters clusts0(m);
	
	size_t cur_cl_id = 0;
	

	
	string snn_out = "snn.csv";
	data::Save(snn_out.c_str(),SNN,false,false);		
	return clusts0;	
}


std::vector<int> find_near_left(int i, const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r)
{
	size_t m = dataset.n_rows;
	double cur = col_sort_ind[ax]->at(i);
	std::vector<int> out;
	for(int j=(i-1); j>=0; j--) {
		double prev = col_sort_ind[ax]->at(j);
		double dist_l = fabs(dataset(cur,ax) - dataset(prev,ax));
		if(dist_l > r) {
			return out;
		}
		double dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
		if(dist_l2 <= r) {
			out.push_back(prev);
			Log::Debug << "Found for " << cur << " from the left " << prev << std::endl;
		}
	}
	return out;
}

std::vector<int> find_near_right(int i, const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
	const int ax,const double r) 
{
	size_t m = dataset.n_rows;
	double cur = col_sort_ind[ax]->at(i);
	std::vector<int> out;
	for(int j=(i+1); j<m; j++) {
		double next = col_sort_ind[ax]->at(j);
		double dist_r = fabs(dataset(cur,ax) - dataset(next,ax));
		if(dist_r > r) {
			return out;
		}
		double dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
		if(dist_r2 <= r) {
			out.push_back(next);
			Log::Debug << "Found for " << cur << " from the right " << next << std::endl;
		}
	}
	return out;
}
void turn_iteration(const int i,const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r, arma::uvec &clusts, size_t &clust_id, bool back) 
{
	Log::Debug << "iteration num " << i << std::endl;
	size_t cur;
	
	cur = col_sort_ind[ax]->at(i);	
	if(clusts(cur)>0) {
		return;
	}
	Log::Debug << "Current " << cur << std::endl;
	int choosed_clust = 0;
	std::vector<int> left_side = find_near_left(i,col_sort_ind,dataset,ax,r);
	int left_count = left_side.size();
	std::vector<int> right_side = find_near_right(i,col_sort_ind,dataset,ax,r);
	int right_count = right_side.size();
	if((left_count>=2)&&(right_count>=2)) {
		for(size_t j=0; j<left_count; j++) {
			if(clusts(left_side[j]) > 0) {				
				if((choosed_clust != 0) &&(clusts(left_side[j]) != choosed_clust)) {
					Log::Debug << "!!!: equals clusters: " << clusts(left_side[j]) << " == " << choosed_clust << std::endl;
				} 
				choosed_clust = clusts(left_side[j]);
				Log::Debug << "Left was found, his clust taken - " << choosed_clust << std::endl;
			}
		}
		if(choosed_clust == 0) { 
			choosed_clust = clust_id;
			Log::Debug << "Left was not found, clust_id taken - " << clust_id << std::endl;
			clust_id++;
		}
		Log::Debug << "Left side was clustered (" << choosed_clust << "): "; 
		for(size_t j=0; j<left_count; j++) {
			clusts(left_side[j]) = choosed_clust;
			Log::Debug << left_side[j] << ", ";
		}
		Log::Debug << std::endl;
		Log::Debug << "Right side was clustered (" << choosed_clust << "): "; 
		clusts(cur) = choosed_clust; // don't forget yourself
		Log::Debug << cur << ", ";
		for(size_t j=0; j<right_count; j++) {
			clusts(right_side[j]) = choosed_clust;
			Log::Debug << right_side[j] << ", ";
		}
		Log::Debug << std::endl;
	}
			
	
}	

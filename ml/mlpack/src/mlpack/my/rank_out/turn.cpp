#include "turn.hpp"

std::vector<Cluster*>
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2) 
{
	Log::Info << "Starting clustering with parameters - " << "dimension1: " << d1 << ", dimension2: " << d2 << ", resolution: " << r << endl;
	size_t m = dataset.n_rows;
	arma::colvec col1 = dataset.unsafe_col(d1);
	arma::colvec col2 = dataset.unsafe_col(d2);
	arma::uvec c1_s = arma::sort_index(col1, 0);
	arma::uvec c2_s = arma::sort_index(col2, 0);

	arma::uvec SNN(m);
	arma::uvec SNN2(m);
	arma::vec dens(m);
	dens.zeros();
	SNN.zeros();
	SNN2.zeros();
	
	size_t clust_id=0;	
	bool in_clust=false;
	for(size_t i=1; i<(m-1); i++) {
		size_t cur = c1_s[i];
		size_t prev = c1_s[i-1];
		size_t next = c1_s[i+1];
		double dist_l = fabs(col1(cur) - col1(prev));
		double dist_r = fabs(col1(cur) - col1(next));
		if((dist_l <= r) && (dist_r <= r)) { 	
			if(!in_clust) { clust_id++; in_clust = true; }
			SNN[cur] = clust_id;					
		} else {
			in_clust = false;
		}		
	}
	size_t num_clust1 = clust_id;
	clust_id=0;
	in_clust=false;
	for(size_t i=1; i<(m-1); i++) {
		size_t cur = c2_s[i];
		size_t prev = c2_s[i-1];
		size_t next = c2_s[i+1];
		double dist_l = fabs(col2(cur) - col2(prev));
		double dist_r = fabs(col2(cur) - col2(next));
		if((dist_l <= r) && (dist_r <= r)) { 	
			if(!in_clust) { clust_id++; in_clust = true; }
			SNN2[cur] = clust_id;					
		} else {
			in_clust = false;
		}				

	}
	size_t num_clust2 = clust_id;

	std::vector<Cluster*> clusters;
	Cluster *cur_cl = NULL;
	size_t cur_cl_id = 0;
	if(num_clust1 >= num_clust2) {
		for(size_t i=0; i<m; i++) {
			size_t cur = c1_s[i];
			if ((SNN(cur)>0) && (SNN2(cur)>0))  {
				if((cur_cl_id == 0) || (SNN(cur) != cur_cl_id)) {
					cur_cl = new Cluster();
					clusters.push_back(cur_cl);
					cur_cl_id = SNN(cur);
				}							
				cur_cl->point_id.push_back(cur);
				cur_cl->size++;
			} 
		}	
	} else {
		for(size_t i=0; i<m; i++) {
			size_t cur = c2_s[i];
			if ((SNN(cur)>0) && (SNN2(cur)>0))  {
				if((cur_cl_id == 0) || (SNN2(cur) != cur_cl_id)) {
					cur_cl = new Cluster();	
					clusters.push_back(cur_cl);
					cur_cl_id = SNN2(cur);
				}							
				cur_cl->point_id.push_back(cur);
				cur_cl->size++;
			} 
		}	
	}
	
	string outputFile1 = "output1.csv";
	data::Save(outputFile1.c_str(), SNN,false,false);
	string outputFile2 = "output2.csv";
	data::Save(outputFile2.c_str(), SNN2,false,false);	

	for(size_t i=0; i<clusters.size(); i++) {
		Cluster *cl = clusters[i];
		Log::Debug << "Cluster number " << i << ", size:" << cl->size << std::endl;
	}
	// Log::Info << "Clustering count finished" << std::endl;
	// Log::Info << "Clustering analising" << std::endl;
	
	// Cluster *cur_cl = NULL;
	// for(size_t i=1; i<m; i++) {
	// 	int ind = c1_s[i];
	// 	int ind_prev = c1_s[i-1];
	// 	if(SNN(ind) == 4) {
	// 		if(!cur_cl) {
	// 			cur_cl = new Cluster();   
	// //			cur_cl->point_id.push_back(i-1); // adding prev point in respect to including boundary points
	// //			cur_cl->density += dens(i-1);					
	// //			cur_cl->size++;
	// 		}
	// 		cur_cl->point_id.push_back(ind);
	// 		cur_cl->density += dens(ind);					
	// 		cur_cl->size++;
	// 	} else {
	// 		if(cur_cl) {
	// //			cur_cl->point_id.push_back(i); // adding point in respect to including boundary points
	// //			cur_cl->density += dens(i);					
	// //			cur_cl->size++;
	// 			clusters.push_back(cur_cl);				
	// 			cur_cl = NULL;
	// 		}
	// 	}
	// }	
	// Log::Info << "Clustering finished, " << clusters.size() << " clusters was found" << std::endl;
	// string outputFile = "output.csv";
 //  	data::Save(outputFile.c_str(), SNN,false,false);	
	return clusters;	
}
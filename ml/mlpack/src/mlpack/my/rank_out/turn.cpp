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
	
	Cluster *cur_cl = NULL;	
	std::vector<Cluster*> clusters1;
	for(size_t i=1; i<(m-1); i++) {
		size_t cur = c1_s[i];
		size_t prev = c1_s[i-1];
		size_t next = c1_s[i+1];
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
			if(!cur_cl) {
				cur_cl = new Cluster();
				clusters1.push_back(cur_cl);
			}
			cur_cl->point_id.push_back(cur);			
			cur_cl->size++;
		} else {
			//if(cur_cl) {
			//	clusters1.push_back(cur_cl);
			//}
			cur_cl = NULL;
		}
	}
	cur_cl = NULL;
	std::vector<Cluster*> clusters2;
	for(size_t i=1; i<(m-1); i++) {
		size_t cur = c2_s[i];
		size_t prev = c2_s[i-1];
		size_t next = c2_s[i+1];
		double dist_l = fabs(col2(cur) - col2(prev));
		double dist_r = fabs(col2(cur) - col2(next));
		double dist_l2 = fabs(col1(cur) - col1(prev));
		double dist_r2 = fabs(col1(cur) - col1(next));
		if((dist_l <= r) && (dist_l2<=r)) {	
			SNN2[cur]++;					
		}
		if((dist_r <= r) && (dist_r2<=r)) {	
			SNN2[cur]++;					
		}
		if(SNN2[cur] == 2) {
			dens[cur] = 1/( dens[cur] + sqrt(pow(dist_l,2.0) + pow(dist_r,2.0))); // calculate density for out SNN			
			if(!cur_cl) {
				cur_cl = new Cluster();
				clusters2.push_back(cur_cl);
			}
			cur_cl->point_id.push_back(cur);			
			cur_cl->size++;
		} else {
			if(dens[cur] != 0) {  // it is mean that we calculate this for nothing, but had to try 
				dens[cur] = 0;
			}			
			cur_cl = NULL;
		}			

	}
	for(size_t i=0; i<clusters1.size(); i++) {
		Cluster *cl = clusters1[i];
		Log::Debug << "c1: Cluster number " << i << ", size:" << cl->size << std::endl;
 		//Log::Debug << "elements: ";
		
 		//for(size_t p_i=0; p_i<cl->size; p_i++) {
 		//	Log::Debug << cl->point_id[p_i] << ": " << dataset(cl->point_id[p_i],d1) << ", " << dataset(cl->point_id[p_i],d2) << "; ";
  		//}
	}
	for(size_t i=0; i<clusters2.size(); i++) {
		Cluster *cl = clusters2[i];
		Log::Debug << "c2: Cluster number " << i << ", size:" << cl->size << std::endl;
 		//Log::Debug << "elements: ";
		
 		//for(size_t p_i=0; p_i<cl->size; p_i++) {
 		//	Log::Debug << cl->point_id[p_i] << ": " << dataset(cl->point_id[p_i],d1) << ", " << dataset(cl->point_id[p_i],d2) << "; ";
  		//}
	}
	// Log::Info << "Clustering count finished" << std::endl;
	// Log::Info << "Clustering analising" << std::endl;
	std::vector<Cluster*> clusters;
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
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
	
	size_t clust_id=0;	// start clusters count from 1, 0 means no cluster
	for(int ax=0; ax<1; ax++) {		
		bool in_clust=false;
		for(size_t i=1; i<(m-1); i++) {			
			turn_iteration(i,col_sort_ind,dataset,ax,r,clusts,clust_id);			
			break;
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
	
	//Log::Info << "Clusters for each axis perfomed. Axis with max clusters choosing for clustering (";
	//Log::Info << clust_axis << ": " << max_clust << " clusters)";
	//Log::Info << std::endl;

	Clusters clusts0(m);
	
	size_t cur_cl_id = 0;
	
	string clust_out = "clusts.csv";
	data::Save(clust_out.c_str(),clusts,false,false);	
	
	string snn_out = "snn.csv";
	data::Save(snn_out.c_str(),SNN,false,false);		
	return clusts0;	
}

void turn_iteration(const int i,const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r, arma::uvec &clusts, size_t &clust_id, bool back) {
	Log::Debug << "iteration num " << i << std::endl;
	size_t m = dataset.n_rows;
	size_t prev,cur,next;
	cur = col_sort_ind[ax]->at(i);
	
	// if(!back) {
	// 	prev = col_sort_ind[ax]->at(i-1);
	next = col_sort_ind[ax]->at(i+1);
	// } else {
	// 	next = col_sort_ind[ax]->at(i-1);
	// 	prev = col_sort_ind[ax]->at(i+1);
	// }
	double dist_l, dist_r, dist_l2, dist_r2;
	// if(!back) {
	// 	dist_l = fabs(dataset(cur,ax) - dataset(prev,ax));
	dist_r = fabs(dataset(cur,ax) - dataset(next,ax));
	// 	dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
	// 	dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
	// } else {
	// 	dist_l = fabs(dataset(cur,ax) - dataset(next,ax));
	// 	dist_r = fabs(dataset(cur,ax) - dataset(prev,ax));
	// 	dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
	// 	dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
	// }
	// if(dist_l<=r) {				
	// 	for(int j=i-1; j>=0; j--) {
	// 		if(!back) {
	// 			prev = col_sort_ind[ax]->at(j);
	// 			dist_l = fabs(dataset(cur,ax) - dataset(prev,ax));
	// 			dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
	// 		} else {
	// 			next = col_sort_ind[ax]->at(j);
	// 			dist_l = fabs(dataset(cur,ax) - dataset(next,ax));
	// 			dist_l2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
	// 		}				
	// 		if(dist_l > r) {
	// 			break; // break bec. we out of bounds
	// 		}
	// 		if(dist_l2 <= r) {
	// 			if(clusts(cur) == 0 ) {
	// 				clusts(cur) = clust_id;
	// 				clust_id++;
	// 			}
	// 			Log::Debug << "Found prev point to " << cur << ", " << prev << "; propagate cluster: " << clusts(cur) << std::endl; 
	// 			clusts(prev) = clusts(cur);
	// 			cur = prev;
	// 		}
	// 	}
	// }
	std::vector<int> cluster_points;
	int cluster_found = 0;
	if(dist_r<=r) {				
		for(int j=i+1; j<m; j++) {
			if(!back) {	
				next = col_sort_ind[ax]->at(j);			
				dist_r = fabs(dataset(cur,ax) - dataset(next,ax));
				dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(next,abs(ax-1)));
			} else {				
				prev = col_sort_ind[ax]->at(j);
				dist_r = fabs(dataset(cur,ax) - dataset(prev,ax));
				dist_r2 = fabs(dataset(cur,abs(ax-1)) - dataset(prev,abs(ax-1)));
			}
			if(dist_r > r) {
				cluster_points.push_back(cur);
				break; // break bec. we out of bounds
			}
			if(dist_r2 <= r) {
				cluster_points.push_back(cur);
				Log::Debug << "Saving " << cur << " to  buffer" << std::endl;
				cur = next;
				if(clusts(next)>0) {
					cluster_found = clusts(next);
					Log::Debug << "Cluster was found: " << cluster_found << std::endl;
				}
			}
		}
	}
	if(cluster_found != 0) {
		clust_id = cluster_found;		
	} else {
		clust_id++;
	}

	for(size_t cl_i=0; cl_i<cluster_points.size(); cl_i++ ) {
		if(cl_i == 0) {
			Log::Debug << "Saving clust information (Num " << clust_id << ") for points : ";
		}
		Log::Debug << cluster_points[cl_i] << ",";
		clusts(cluster_points[cl_i]) = clust_id;
	}
	Log::Debug << std::endl;

	// if((dist_l <= r) && (dist_r <= r) && (dist_l2 <= r) && (dist_r2 <= r)) { 					
	// 	Log::Debug << "Neighboors: " << prev << "," << cur << ","  << next << std::endl;
	// 	if(clusts(cur)>0) {													
	// 		Log::Debug << "cur clust grow than 0: " << clusts(cur) << std::endl; 
	// 		if((clusts(prev)>0)&& (clusts(cur) != clusts(prev))) {
	// 			Log::Debug << "cur != prev : " << dataset(clusts(cur),0) << "," << dataset(clusts(cur),1) << " cl(" << clusts(cur) << ")";
	// 			Log::Debug << " != " << dataset(clusts(prev),0) << "," << dataset(clusts(prev),1) << " cl(" << clusts(prev) << ")" << std::endl;

	// 			if (back) { //|| (ax == 2)) {
	// 			  	Log::Debug << "clust changed: " << clusts(cur);
	// 			  	clusts(cur) = clusts(prev);
	// 			  	Log::Debug << " -> " << clusts(cur) << std::endl;
	// 			}				
	// 		} else
	// 		if((clusts(next)>0)&&(clusts(cur) != clusts(next))) {
	// 			Log::Debug << "cur != next : " << dataset(clusts(cur),0) << "," << dataset(clusts(cur),1) << " cl(" << clusts(cur) << ")";
	// 			Log::Debug << " != " << dataset(clusts(next),0) << "," << dataset(clusts(next),1) << " cl(" << clusts(next) << ")" << std::endl;					
	// 			if(back) {				  	
	// 			  	Log::Debug << "clust changed: " << clusts(next);
	// 			  	clusts(next) = clusts(cur);
	// 			  	Log::Debug << " -> " << clusts(next) << std::endl;
	// 			}									
	// 		} else {
	// 			clusts(next) = clusts(cur);
	// 		}			
	// 	} else
	// 	if(clusts(prev)>0) {
	// 		Log::Debug << "found prev clust grow than 0: " << clusts(prev) << std::endl;
	// 		clusts(cur) = clusts(prev);
	// 		clusts(next) = clusts(cur);
	// 	} else 
	// 	if(clusts(cur) == 0) {
	// 		clusts(cur) = clust_id;;	
	// 		clusts(next) = clust_id;
	// 		clusts(prev) = clust_id;
	// 		clust_id++;
	// 	}		
	// }					
}
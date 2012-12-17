
#include "rank_out.hpp"
#include "turn.hpp"

void rank_out(const arma::mat& dataset) {
	size_t dimensionality = dataset.n_cols;
	Log::Debug << "dim: " << dimensionality << std::endl;
	size_t d1 = math::RandInt(0,dimensionality);
	size_t d2_step = math::RandInt(0,dimensionality-1);
	size_t d2 = d1+d2_step;
	if(d2>dimensionality) {
		d2 -= dimensionality;
	}
	double r = 1.5;
	d1 = 0;
	d2 = 1;
	std::vector<Cluster*> clusters = calculate_clusters(dataset,r,d1,d2);	
	// arma::mat m(dataset.n_rows,3);
	// int count = 0;
	// for(size_t i=0; i<clusters.size(); i++) {
	// 	Cluster *cl = clusters[i];
	// 	Log::Debug << "Cluster number " << i << ", size:" << cl->size << std::endl;
 // 		Log::Debug << "elements: ";

 		
 // 		for(size_t p_i=0; p_i<cl->size; p_i++) {
 // 			//Log::Debug << cl->point_id[p_i] << ": " << dataset(cl->point_id[p_i],d1) << ", " << dataset(cl->point_id[p_i],d2) << "; ";
 // 			m(count,d1) = dataset(cl->point_id[p_i],d1);
 // 			m(count,d2) = dataset(cl->point_id[p_i],d2);
 // 			m(count,2) = i;
 // 			count++;
 //  		// 
 //  		}  		
		
  		
 //  		Log::Debug << std::endl;
 // 		delete cl;
 // 	} 	
 // 	string outputFile = "out.csv";
 // 	data::Save(outputFile.c_str(), m,false,false);
}
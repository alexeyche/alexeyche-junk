
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
	d1 = 0;
	d2 = 1;
	size_t counter=0;
//	for(double r=0.005; r<=2; r*=2) {
		double r=0.1;
		Clusters clusts = calculate_clusters(dataset,r,d1,d2);	
		string outputFile = boost::str( boost::format("output%d.csv") % counter );
		data::Save(outputFile.c_str(),clusts.clust_ind,false,false);
//		counter++;
//	}
}
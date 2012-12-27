#include <mlpack/core.hpp>

using namespace std;
using namespace mlpack;

// struct Cluster {
// 	Cluster(size_t cl_id = 1) : density(0), size(0), id(cl_id) {}
// 	std::vector<int> point_id;
// 	double density;
// 	size_t size;
// 	size_t id;
// };

struct Clusters {
	Clusters(arma::uvec clust_ind_given, size_t inst_num_given, size_t clust_num_given) : 
		clust_num(clust_num_given), inst_num(inst_num_given), clust_ind(clust_ind_given), clust_size(clust_num+1) 
	{ 
		clust_size.zeros();
		for(size_t i=0; i<inst_num; i++) {
			if(clust_ind(i)>0) {
				clust_size(clust_ind(i))++;
			}
		}
		for(int i=1; i<=this->clust_num; i++) {
			Log::Debug << "Size of cluster " << i << " is " << clust_size(i) << std::endl;
		}
		if(this->clust_num>3) {
			string out = boost::str(boost::format("out_%d.csv") % clust_num);
			string out2 = boost::str(boost::format("out_size_%d.csv") % clust_num);
			data::Save(out.c_str(),this->clust_ind,false,false);
			data::Save(out2.c_str(),this->clust_size,false,false);
		}
	}
	
	size_t clust_num;
	size_t inst_num;
	arma::uvec clust_ind;
	arma::uvec clust_size;
};


Clusters*
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2);

void
turn_iteration(const int i,const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r, arma::uvec &clusts, size_t &clust_id);

std::vector<int> find_near_left(int i, const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r);
std::vector<int> find_near_right(int i, const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
	const int ax,const double r);
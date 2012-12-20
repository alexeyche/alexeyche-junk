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
	Clusters(size_t inst_num_given) : clust_num(0), inst_num(inst_num_given), clust_ind(inst_num) { clust_ind.zeros(); }
	size_t clust_num;
	size_t inst_num;
	arma::uvec clust_ind;
};


Clusters
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2);

void
turn_iteration(const int i,const std::vector<arma::uvec*> &col_sort_ind, const arma::mat &dataset, 
					const int ax,const double r, arma::uvec &clusts, size_t &clust_id, bool back = false);
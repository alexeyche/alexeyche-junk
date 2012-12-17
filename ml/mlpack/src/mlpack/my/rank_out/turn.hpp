#include <mlpack/core.hpp>

using namespace std;
using namespace mlpack;

struct Cluster {
	Cluster() : density(0), size(0){}
	std::vector<int> point_id;
	double density;
	size_t size;
};


std::vector<Cluster*>
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2);
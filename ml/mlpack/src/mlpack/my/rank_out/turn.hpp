#include <mlpack/core.hpp>

using namespace std;
using namespace mlpack;

std::pair<arma::uvec,arma::vec> 
calculate_clusters(const arma::mat& dataset, double r, size_t d1, size_t d2);
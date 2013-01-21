


#include <mlpack/core.hpp>

using namespace mlpack;
using namespace std;

#include "rank_out.hpp"
#include "scaling.hpp"

PROGRAM_INFO("Stochastic gradient descent", "");
PARAM_STRING_REQ("inputFile", "Input dataset to perform sgd.", "i");

int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);
}  
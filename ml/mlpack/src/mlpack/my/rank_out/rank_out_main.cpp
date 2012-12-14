
#include <mlpack/core.hpp>

using namespace mlpack;
using namespace std;

#include "rank_out.hpp"
#include "scaling.hpp"

//PROGRAM_INFO("TURN* clustering", "Just perform turn clustering for 2d space");

//PARAM_STRING_REQ("inputFile","poum",""); // not working, I don't know why
//PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);
// Define parameters for the executable.
PROGRAM_INFO("TURN* Clustering", "Just TURN clustering");
PARAM_STRING_REQ("inputFile", "Input dataset to perform clustering on.", "i");
PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);
PARAM_DOUBLE("resolution", "resolution", "r", 0.05);
int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);
    // Initialize random seed.
  if (CLI::GetParam<int>("seed") != 0)
    math::RandomSeed((size_t) CLI::GetParam<int>("seed"));
  else
    math::RandomSeed((size_t) std::time(NULL));

  string inputFile = CLI::GetParam<string>("inputFile");
  double r = CLI::GetParam<double>("resolution");
  Log::Debug << inputFile << std::endl;
  arma::mat dataset;
  data::Load(inputFile.c_str(), dataset,true,false);
  //perform scaling
  ScaleMat(dataset,-1,1);
  string outputFile = "output.csv";
  data::Save(outputFile.c_str(), dataset,false,false);
  //turn(dataset,r);  
  
}
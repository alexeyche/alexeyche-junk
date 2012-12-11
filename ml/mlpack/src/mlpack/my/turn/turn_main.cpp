
#include <mlpack/core.hpp>

using namespace mlpack;
using namespace std;

//PROGRAM_INFO("TURN* clustering", "Just perform turn clustering for 2d space");

//PARAM_STRING_REQ("inputFile","poum",""); // not working, I don't know why
//PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);
// Define parameters for the executable.
PROGRAM_INFO("TURN* Clustering", "Just TURN clustering");
PARAM_STRING_REQ("inputFile", "Input dataset to perform clustering on.", "i");
PARAM_INT("seed", "Random seed.  If 0, 'std::time(NULL)' is used.", "s", 0);

int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);
    // Initialize random seed.
  if (CLI::GetParam<int>("seed") != 0)
    math::RandomSeed((size_t) CLI::GetParam<int>("seed"));
  else
    math::RandomSeed((size_t) std::time(NULL));

  string inputFile = CLI::GetParam<string>("inputFile");
  Log::Debug << inputFile << std::endl;
  arma::mat dataset;
  data::Load(inputFile.c_str(), dataset,true,false);
  
  //Lets choos 2 dims
  size_t dimensionality = dataset.n_cols;
  size_t d1 = math::RandInt(0,dimensionality);
  size_t d2_step = math::RandInt(0,dimensionality-1);
  size_t d2 = d1+d2_step;
  if(d2>dimensionality) {
    d2 -= dimensionality;
  }

  Log::Debug << d1 << "," << d2 << " from " << dimensionality << std::endl;

  for(int i=0; i<dimensionality; i++) {
    dataset()
    arma::rowvec      
  }
}
#include <mlpack/core.hpp>
using namespace mlpack;
using namespace std;

PROGRAM_INFO("TURN* clustering");

PARAM_STRING("inputFile", "input file","i");

int main(int argc, char** argv)
{
  CLI::ParseCommandLine(argc, argv);
  string inputFile = CLI::GetParam<string>("inputFile");
  arma::mat dataset;
  data::Load(inputFile.c_str(), dataset,false);
}
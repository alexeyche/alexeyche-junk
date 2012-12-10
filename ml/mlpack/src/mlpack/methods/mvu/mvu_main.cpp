/**
 * @file mvu_main.cpp
 * @author Ryan Curtin
 *
 * Executable for MVU.
 *
 * This file is part of MLPACK 1.0.3.
 *
 * MLPACK is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * MLPACK is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details (LICENSE.txt).
 *
 * You should have received a copy of the GNU General Public License along with
 * MLPACK.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <mlpack/core.hpp>
#include "mvu.hpp"

PROGRAM_INFO("Maximum Variance Unfolding (MVU)", "This program implements "
    "Maximum Variance Unfolding, a nonlinear dimensionality reduction "
    "technique.  The method minimizes dimensionality by unfolding a manifold "
    "such that the distances to the nearest neighbors of each point are held "
    "constant.");

PARAM_STRING_REQ("input_file", "Filename of input dataset.", "i");
PARAM_INT_REQ("new_dim", "New dimensionality of dataset.", "d");

PARAM_STRING("output_file", "Filename to save unfolded dataset to.", "o",
    "output.csv");
PARAM_INT("num_neighbors", "Number of nearest neighbors to consider while "
    "unfolding.", "k", 5);

using namespace mlpack;
using namespace mlpack::mvu;
using namespace mlpack::math;
using namespace arma;
using namespace std;

int main(int argc, char **argv)
{
  // Read from command line.
  CLI::ParseCommandLine(argc, argv);

  RandomSeed(time(NULL));

  // Load input dataset.
  const string inputFile = CLI::GetParam<string>("input_file");
  mat data;
  data::Load(inputFile, data, true);

  // Verify that the requested dimensionality is valid.
  const int newDim = CLI::GetParam<int>("new_dim");
  if (newDim <= 0 || newDim > (int) data.n_rows)
  {
    Log::Fatal << "Invalid new dimensionality (" << newDim << ").  Must be "
      << "between 1 and the input dataset dimensionality (" << data.n_rows
      << ")." << std::endl;
  }

  // Verify that the number of neighbors is valid.
  const int numNeighbors = CLI::GetParam<int>("num_neighbors");
  if (numNeighbors <= 0 || numNeighbors > (int) data.n_cols)
  {
    Log::Fatal << "Invalid number of neighbors (" << numNeighbors << ").  Must "
        << "be between 1 and the number of points in the input dataset ("
        << data.n_cols << ")." << std::endl;
  }

  // Now run MVU.
  MVU mvu(data);

  mat output;
  mvu.Unfold(newDim, numNeighbors, output);

  // Save results to file.
  const string outputFile = CLI::GetParam<string>("output_file");
  data::Save(outputFile, output, true);
}

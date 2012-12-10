/**
 * @file em_fit_impl.hpp
 * @author Ryan Curtin
 *
 * Implementation of EM algorithm for fitting GMMs.
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
#ifndef __MLPACK_METHODS_GMM_EM_FIT_IMPL_HPP
#define __MLPACK_METHODS_GMM_EM_FIT_IMPL_HPP

// In case it hasn't been included yet.
#include "em_fit.hpp"

// Definition of phi().
#include "phi.hpp"

namespace mlpack {
namespace gmm {

template<typename InitialClusteringType>
void EMFit<InitialClusteringType>::Estimate(const arma::mat& observations,
                                            std::vector<arma::vec>& means,
                                            std::vector<arma::mat>& covariances,
                                            arma::vec& weights)
{
  InitialClustering(observations, means, covariances, weights);

  double l = LogLikelihood(observations, means, covariances, weights);

  Log::Debug << "EMFit::Estimate(): initial clustering log-likelihood: "
      << l << std::endl;

  double lOld = -DBL_MAX;
  arma::mat condProb(observations.n_cols, means.size());

  // Iterate to update the model until no more improvement is found.
  size_t maxIterations = 300;
  size_t iteration = 0;
  while (std::abs(l - lOld) > 1e-10 && iteration < maxIterations)
  {
    // Calculate the conditional probabilities of choosing a particular
    // Gaussian given the observations and the present theta value.
    for (size_t i = 0; i < means.size(); i++)
    {
      // Store conditional probabilities into condProb vector for each
      // Gaussian.  First we make an alias of the condProb vector.
      arma::vec condProbAlias = condProb.unsafe_col(i);
      phi(observations, means[i], covariances[i], condProbAlias);
      condProbAlias *= weights[i];
    }

    // Normalize row-wise.
    for (size_t i = 0; i < condProb.n_rows; i++)
      condProb.row(i) /= accu(condProb.row(i));

    // Store the sum of the probability of each state over all the observations.
    arma::vec probRowSums = trans(arma::sum(condProb, 0 /* columnwise */));

    // Calculate the new value of the means using the updated conditional
    // probabilities.
    for (size_t i = 0; i < means.size(); i++)
    {
      means[i] = (observations * condProb.col(i)) / probRowSums[i];

      // Calculate the new value of the covariances using the updated
      // conditional probabilities and the updated means.
      arma::mat tmp = observations - (means[i] *
          arma::ones<arma::rowvec>(observations.n_cols));
      arma::mat tmp_b = tmp % (arma::ones<arma::vec>(observations.n_rows) *
          trans(condProb.col(i)));

      covariances[i] = (tmp * trans(tmp_b)) / probRowSums[i];
    }

    // Calculate the new values for omega using the updated conditional
    // probabilities.
    weights = probRowSums / observations.n_cols;

    // Update values of l; calculate new log-likelihood.
    lOld = l;
    l = LogLikelihood(observations, means, covariances, weights);

    iteration++;
  }
}

template<typename InitialClusteringType>
void EMFit<InitialClusteringType>::Estimate(const arma::mat& observations,
                                            const arma::vec& probabilities,
                                            std::vector<arma::vec>& means,
                                            std::vector<arma::mat>& covariances,
                                            arma::vec& weights)
{
  InitialClustering(observations, means, covariances, weights);

  double l = LogLikelihood(observations, means, covariances, weights);

  Log::Debug << "EMFit::Estimate(): initial clustering log-likelihood: "
      << l << std::endl;

  double lOld = -DBL_MAX;
  arma::mat condProb(observations.n_cols, means.size());

  // Iterate to update the model until no more improvement is found.
  size_t maxIterations = 300;
  size_t iteration = 0;
  while (std::abs(l - lOld) > 1e-10 && iteration < maxIterations)
  {
    // Calculate the conditional probabilities of choosing a particular
    // Gaussian given the observations and the present theta value.
    for (size_t i = 0; i < means.size(); i++)
    {
      // Store conditional probabilities into condProb vector for each
      // Gaussian.  First we make an alias of the condProb vector.
      arma::vec condProbAlias = condProb.unsafe_col(i);
      phi(observations, means[i], covariances[i], condProbAlias);
      condProbAlias *= weights[i];
    }

    // Normalize row-wise.
    for (size_t i = 0; i < condProb.n_rows; i++)
      condProb.row(i) /= accu(condProb.row(i));

    // This will store the sum of probabilities of each state over all the
    // observations.
    arma::vec probRowSums(means.size());

    // Calculate the new value of the means using the updated conditional
    // probabilities.
    for (size_t i = 0; i < means.size(); i++)
    {
      // Calculate the sum of probabilities of points, which is the
      // conditional probability of each point being from Gaussian i
      // multiplied by the probability of the point being from this mixture
      // model.
      probRowSums[i] = accu(condProb.col(i) % probabilities);

      means[i] = (observations * (condProb.col(i) % probabilities)) /
        probRowSums[i];

      // Calculate the new value of the covariances using the updated
      // conditional probabilities and the updated means.
      arma::mat tmp = observations - (means[i] *
          arma::ones<arma::rowvec>(observations.n_cols));
      arma::mat tmp_b = tmp % (arma::ones<arma::vec>(observations.n_rows) *
          trans(condProb.col(i) % probabilities));

      covariances[i] = (tmp * trans(tmp_b)) / probRowSums[i];
    }

    // Calculate the new values for omega using the updated conditional
    // probabilities.
    weights = probRowSums / accu(probabilities);

    // Update values of l; calculate new log-likelihood.
    lOld = l;
    l = LogLikelihood(observations, means, covariances, weights);

    iteration++;
  }
}

template<typename InitialClusteringType>
void EMFit<InitialClusteringType>::InitialClustering(
    const arma::mat& observations,
    std::vector<arma::vec>& means,
    std::vector<arma::mat>& covariances,
    arma::vec& weights)
{
  // Assignments from clustering.
  arma::Col<size_t> assignments;

  // Run clustering algorithm.
  clusterer.Cluster(observations, means.size(), assignments);

  // Now calculate the means, covariances, and weights.
  weights.zeros();
  for (size_t i = 0; i < means.size(); ++i)
  {
    means[i].zeros();
    covariances[i].zeros();
  }

  // From the assignments, generate our means, covariances, and weights.
  for (size_t i = 0; i < observations.n_cols; ++i)
  {
    size_t cluster = assignments[i];

    // Add this to the relevant mean.
    means[cluster] += observations.col(i);

    // Add this to the relevant covariance.
    covariances[cluster] += observations.col(i) * trans(observations.col(i));

    // Now add one to the weights (we will normalize).
    weights[cluster]++;
  }

  // Now normalize the mean and covariance.
  for (size_t i = 0; i < means.size(); ++i)
  {
    covariances[i] -= means[i] * trans(means[i]) / weights[i];

    means[i] /= weights[i];
    covariances[i] /= (weights[i] > 1) ? weights[i] : 1;
  }

  // Finally, normalize weights.
  weights /= accu(weights);
}

template<typename InitialClusteringType>
double EMFit<InitialClusteringType>::LogLikelihood(
    const arma::mat& observations,
    const std::vector<arma::vec>& means,
    const std::vector<arma::mat>& covariances,
    const arma::vec& weights) const
{
  double logLikelihood = 0;

  arma::vec phis;
  arma::mat likelihoods(means.size(), observations.n_cols);
  for (size_t i = 0; i < means.size(); ++i)
  {
    phi(observations, means[i], covariances[i], phis);
    likelihoods.row(i) = weights(i) * trans(phis);
  }

  // Now sum over every point.
  for (size_t j = 0; j < observations.n_cols; ++j)
    logLikelihood += log(accu(likelihoods.col(j)));

  return logLikelihood;
}

}; // namespace gmm
}; // namespace mlpack

#endif

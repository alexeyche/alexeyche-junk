
#include <vector>
#include <sstream>
#include <exception>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include "lbfgs.h"


using TFloat = double;
using TLong = long;

template <typename T>
using TVec = std::vector<T>;

using TInt = int;
using TVecF = std::vector<TFloat>;
using TVecL = std::vector<TLong>;
using TSize = size_t;

#define ENSURE(cond, message) \
    if(!(cond)) { \
        std::stringstream ss; \
        ss << message; \
        throw std::runtime_error(ss.str()); \
    }\


TVecF zeros(TSize size) {
    return TVecF(size, 0.0);
}

TVec<TVecL> getBinomials(TInt m) {
    TVec<TVecL> binoms(m);
    for (TVecL& b: binoms) b.resize(m);

    for (TInt i = 0; i<m; i++) {
        binoms[i][0] = 1;
        for (int j=1; j<=i; j++) {
            binoms[i][j] = binoms[i-1][j-1] + binoms[i-1][j];
        }
    }
    return binoms;
}

TVec<TVecL> getChebyCoefficients(TInt k) {
    TVec<TVecL> chebyCoeffs(k);
    for (TVecL& c: chebyCoeffs) c.resize(k);

    chebyCoeffs[0][0] = 1;
    if (k == 1) {
        return chebyCoeffs;
    }

    chebyCoeffs[1][1] = 1;
    for (TInt i = 2; i < k; i++) {
        TVecL c1 = chebyCoeffs[i-1];
        TVecL c2 = chebyCoeffs[i-2];
        chebyCoeffs[i][0] = -c2[0];
        for (TInt j = 1; j <= i; j++) {
            chebyCoeffs[i][j] = 2*c1[j-1] - c2[j];
        }
    }
    return chebyCoeffs;
}

TVecF shiftPowerSum(TVecF powerSums, TFloat r, TFloat xc) {
    ENSURE(powerSums.size() > 0, "Got empty powerSums");
    TVecF scaledPowerSums(powerSums.size());
    TVecF nxcPowers = zeros(powerSums.size());
    TVecF rNegPowers = zeros(powerSums.size());
    nxcPowers[0] = 1.0;
    rNegPowers[0] = 1.0;
    for (TSize i=1; i<powerSums.size(); ++i) {
        nxcPowers[i] = nxcPowers[i-1] * (-xc);
        rNegPowers[i] = rNegPowers[i-1] / r;
    }
    TVec<TVecL> mBinoms = getBinomials(powerSums.size());
    for (TSize m = 0; m < powerSums.size(); m++) {
        double sum = 0.0;
        for (int j = 0; j<=m; j++) {
            sum += mBinoms[m][j]*nxcPowers[m-j]*powerSums[j];
        }
        scaledPowerSums[m] = rNegPowers[m] * sum;
    }
    return scaledPowerSums;
}

TVecF powerSumsToZerodMoments(TVecF powerSums, TFloat min, TFloat max) {
    ENSURE(powerSums.size() > 0, "Got empty powerSums");

    double r = (max - min) / 2;
    double xc = (max + min) / 2;
    TVecF scaledPowerSums = shiftPowerSum(powerSums, r, xc);
    double count = scaledPowerSums[0];
    for (TSize i = 0; i < powerSums.size(); ++i) {
        scaledPowerSums[i] /= count;
    }
    return scaledPowerSums;
}

TVecF powerSumsToChebyMoments(TVecF powerSums, TFloat min, TFloat max) {
    ENSURE(powerSums.size() > 0, "Got empty powerSums");

    double r = (max - min) / 2;
    double xc = (max + min) / 2;
    // First rescale the variables so that they lie in [-1,1]
    TVecF scaledPowerSums = shiftPowerSum(powerSums,r,xc);

    double count = powerSums[0];
    TVec<TVecL> cCoeffs = getChebyCoefficients(powerSums.size());
    // Then convert from power sums to chebyshev moments
    TVecF scaledChebyMoments(powerSums.size());
    for (int i = 0; i <powerSums.size(); i++) {
        double sum = 0.0;
        for (int j = 0; j <= i; j++) {
            sum += cCoeffs[i][j]*scaledPowerSums[j];
        }
        scaledChebyMoments[i] = sum / count;
    }
    return scaledChebyMoments;
}





struct DMaxentLoss {
    TVecF cMoments;
    TInt nGrid;
    TInt dim;

    // Cached chebyshev polynomial values at grid points
    TVec<TVecF> cpVals;

    // Coefficients for expression weights using chebyshev polynomials
    TVecF weights;
    TVecF mus;
    TVecF grad;

    DMaxentLoss(TVecF _cMoments, TInt _nGrid) {
        cMoments = _cMoments;
        nGrid = _nGrid;
        dim = cMoments.size();

        TVecF xs(nGrid);
        for (TInt i = 0; i < nGrid; ++i) {
            xs[i] = i*2.0/(nGrid-1)-1.0;
        }

        cpVals.resize(2*dim);
        for (auto& v: cpVals) v.resize(nGrid);

        for (TInt i = 0; i<nGrid; ++i) {
            cpVals[0][i] = 1.0;
            cpVals[1][i] = xs[i];
        }
        for (TInt j = 2; j < 2*dim; ++j) {
            for (TInt i = 0; i < nGrid; ++i) {
                cpVals[j][i] = 2*xs[i]*cpVals[j-1][i] - cpVals[j-2][i];
            }
        }

        weights = TVecF(nGrid, 0.0);
        mus = TVecF(2*dim, 0.0);
        grad = TVecF(dim, 0.0);
    }

    double compute(TVecF point) {
        for (TInt i = 0; i < nGrid; ++i){
            TFloat sum = 0.0;
            for (TInt j = 0; j < dim; j++){
                sum += point[j]*cpVals[j][i];
            }
            weights[i] = std::exp(sum);
        }
        for (TInt i = 0; i < 2*dim; ++i){
            TFloat sum = 0.0;
            for (TInt j = 0; j < nGrid; ++j) {
                sum += cpVals[i][j]*weights[j];
            }
            mus[i] = sum;
        }
        for (TInt i = 0; i < dim; i++) {
            grad[i] = mus[i] - cMoments[i];
        }

        TFloat sum = 0.0;
        for (TInt i = 0; i < dim; i++) {
            sum += point[i] * cMoments[i];
        }
        return mus[0] - sum;
    }
};



void getBinomials_test() {
    auto b = getBinomials(6);
    ENSURE(b[5][2] == 10L, "getBinomials_test failed");
}


void shiftPowerSum_test() {
    auto r = shiftPowerSum({1.0, 2.0, 3.0}, 2.0, 0.5);
    auto exp = TVecF{1.0,0.75,0.3125};
    ENSURE(r == exp, "shiftPowerSum_Test failed");
}

void powerSumsToChebyMoments_test() {
    TVecF uniformPowerSums = {1001,500500,333833500,250500250000};
    TVecF r = powerSumsToChebyMoments(uniformPowerSums, 0, 1000);
    TVecF exp = {1.0, 0, -.332, 0};
    ENSURE(r == exp, "powerSumsToChebyMoments_test failed");
}


void DMaxentLoss0_test() {
    TVecF m_values = {1.0, 0, -1.0/3, 0, -1.0/15, 0, -1.0/35};
    TVecF l_values = {0.0, 0, 0, 0, 0, 0, 0};
    double tol = 1e-10;

    DMaxentLoss loss(m_values, 21);
    double v = loss.compute(l_values);
    ENSURE(v == 21.0, "DMaxentLoss0_test failed");
}

void DMaxentLoss1_test() {
    TVecF m_values = {
        1., -0.06277785, -0.4079572, -0.26898716, -0.1348974,
        0.34909762, 0.28072058, -0.00188146, -0.27568291, -0.17049106
    };

    TVecF l0(m_values.size(), 0.0);
    l0[0] = std::log(1.0 / 20.0);

    DMaxentLoss loss(m_values, 21);

    double v = loss.compute(l0);
    double exp = 4.045732273553991;

    ENSURE(
        std::abs(v - exp) < std::numeric_limits<TFloat>::epsilon(),
        "DMaxentLoss0_test failed"
    );
}


void DMaxentLoss2_test() {
    TVecF sums = {1000,101172.864051531,10240137.609583,1036877504.896428,105034307931.022,
                  10644348066181.16, 1079178179395168.0,109459991792955584.0,
                  11107280307688728576.0};
    TFloat min = 97.59915093074763;
    TFloat max = 106.9324489018609;

    TVecF powerChebyMoments = powerSumsToChebyMoments(sums, min, max);

    TVecF l0(powerChebyMoments.size(), 0.0);
    l0[0] = std::log(1.0 / sums[0]);

    DMaxentLoss loss(powerChebyMoments, 1024);

    // std::cerr << loss.compute(l0) << "\n";
    // for (const auto& g: loss.grad) { std:: cout << g << ","; }
    // std::cerr << "\n";
    // double exp = 4.045732273553991;

    // ENSURE(
    //     std::abs(v - exp) < std::numeric_limits<TFloat>::epsilon(),
    //     "DMaxentLoss0_test failed"
    // );
}


struct Parameters {
    TSize gridSize = 1024;
    TSize maxIter = 100;
    TFloat tolerance = 1e-08;
    TFloat learningRate = 0.01;
    TFloat beta1 = 0.9;
    TFloat beta2 = 0.999;
    TFloat epsilon = 0.001;
    TSize progressIteration = 1000;
};


void optimize_adam(TVecF& l0, DMaxentLoss& loss, Parameters p) {
    TVecF m = TVecF(l0.size(), 0.0);
    TVecF v = TVecF(l0.size(), 0.0);

    double value = std::numeric_limits<TFloat>::max();
    for (TSize iter=0; iter<p.maxIter; ++iter) {
        double newValue = loss.compute(l0);
        if (std::abs(newValue - value) < p.tolerance) {
            std::cerr << "Converged on iteration #" << iter-1 << "\n";
            break;
        }
        value = newValue;

        for (TSize xid=0; xid < l0.size(); ++xid) {
            m[xid] = p.beta1 * m[xid] + (1.0 - p.beta1) * loss.grad[xid];
            v[xid] = p.beta2 * v[xid] + (1.0 - p.beta2) * loss.grad[xid] * loss.grad[xid];

            TFloat weightGrad = m[xid] / (sqrt(v[xid]) + p.epsilon);
            // std::cerr << weightGrad << ", ";

            // std::cerr << loss.grad[xid] << ", ";
            l0[xid] += - p.learningRate * weightGrad;
        }

        if ((p.progressIteration > 0) && (iter % p.progressIteration == 0)) {
            std::cerr << "Iteration #" << iter << ": " << value << "\n";
        }
    }

}


double _evaluate(void *instance, const lbfgsfloatval_t *x, lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t) {
    DMaxentLoss* loss = reinterpret_cast<DMaxentLoss*>(instance);
    TVecF l0(x, x+n);

    double value = loss->compute(l0);

    for (TSize gid=0; gid<n; ++gid) {
        g[gid] = loss->grad[gid];
    }
    return value;
}

int _progress(
    void *instance,
    const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g,
    const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm,
    const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step,
    int n,
    int k,
    int ls)
{
    if (k % 1 == 0) {
        std::cerr << "Iteration #" <<  k << ": " << fx
            << ", xnorm = " << xnorm
            << ", gnorm =" << gnorm
            << ", step = " << step << "\n";
    }
    return 0;
}

void optimize_lbfgs(TVecF& l0, DMaxentLoss& loss, Parameters p) {
    TSize N = loss.dim;

    lbfgs_parameter_t _p;
    lbfgs_parameter_init(&_p);

    _p.delta = p.tolerance;
    _p.max_iterations = p.maxIter;
    _p.past = 5;
    _p.m = 20;

    TFloat fx;
    TFloat *m_x = lbfgs_malloc(N);

    int ret = lbfgs(N, m_x, &fx, _evaluate, _progress, &loss, &_p);
    std::cerr << "L-BFGS optimization terminated with status code = " << ret << "\n";
}

TVecF get_quantiles(
    TVecF quantiles,
    DMaxentLoss loss,
    TFloat min,
    TFloat max,
    Parameters p)
{
    TFloat center = (max + min)/2.0;
    TFloat scale = (max - min)/2.0;

    TVecF xs(p.gridSize);
    for (TSize xs_id = 0; xs_id<p.gridSize; ++xs_id) {
        double scaledX = xs_id * 2.0 / (p.gridSize-1.0) - 1.0;
        xs[xs_id] = scaledX * scale + center;
    }
    TVecF cdf(p.gridSize);
    cdf[0] = 0.0;
    for (TSize cdf_id = 1 ; cdf_id < p.gridSize; ++cdf_id) {
        cdf[cdf_id] = cdf[cdf_id-1] + loss.weights[cdf_id];
    }

    TVecF ans(quantiles.size());
    for (TSize q_id=0; q_id < quantiles.size(); ++q_id) {

        TFloat lastRank;
        TFloat curRank = 0.0;
        TSize targetIdx = p.gridSize - 1;
        for (TSize curIdx = 0; curIdx<p.gridSize; ++curIdx) {
            lastRank = curRank;
            curRank = cdf[curIdx];
            if (curRank >= quantiles[q_id]) {
                targetIdx = curIdx;
                break;
            }
        }
        ans[q_id] = xs[targetIdx];
    }
    return ans;
}

TVecF run(TFloat min, TFloat max, TVecF powerSums, Parameters p, TVecF quantiles) {
    TVecF powerChebyMoments = powerSumsToChebyMoments(powerSums, min, max);

    DMaxentLoss loss(powerChebyMoments, p.gridSize);

    TVecF l0(powerSums.size(), 0.0);
    l0[0] = std::log(1.0 / p.gridSize);
    optimize_adam(l0, loss, p);
    // optimize_lbfgs(l0, loss, p);
    return get_quantiles(quantiles, loss, min, max, p);
}


#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdlib>



int main(int argc, const char** argv) {
    getBinomials_test();
    shiftPowerSum_test();
    powerSumsToChebyMoments_test();
    DMaxentLoss0_test();
    DMaxentLoss1_test();
    DMaxentLoss2_test();

    const char* gridSize = std::getenv("GRID_SIZE");
    const char* maxIter = std::getenv("MAX_ITER");
    const char* tolerance = std::getenv("TOLERANCE");
    const char* learningRate = std::getenv("LEARNING_RATE");
    const char* beta1 = std::getenv("BETA1");
    const char* beta2 = std::getenv("BETA2");
    const char* epsilon = std::getenv("EPSILON");
    const char* progressIteration = std::getenv("PROGRESS_ITERATION");

    Parameters p;
    if (gridSize != NULL) { p.gridSize = std::stof(gridSize); }
    if (maxIter != NULL) { p.maxIter = std::stoi(maxIter); }
    if (tolerance != NULL) { p.tolerance = std::stof(tolerance); }
    if (learningRate != NULL) { p.learningRate = std::stof(learningRate); }
    if (beta1 != NULL) { p.beta1 = std::stof(beta1); }
    if (beta2 != NULL) { p.beta2 = std::stof(beta2); }
    if (epsilon != NULL) { p.epsilon = std::stof(epsilon); }
    if (progressIteration != NULL) { p.progressIteration = std::stoi(progressIteration); }

    TFloat min;
    TFloat max;
    TVecF powerSums;

    bool firstLine = true;
    TVecF quantiles;
    for (std::string line; std::getline(std::cin, line);) {
        if (firstLine) {
            firstLine = false;

            std::vector<std::string> cont;

            std::istringstream iss(line);
            std::copy(
                std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                std::back_inserter(cont)
            );

            min = std::stof(cont[0]);
            max = std::stof(cont[1]);

            for (TSize i=2; i<cont.size(); ++i) {
                powerSums.push_back(std::stof(cont[i]));
            }
        } else {
            quantiles.push_back(std::stof(line));
        }
    }

    TVecF x = run(min, max, powerSums, p, quantiles);

    for(int i = 1; i < x.size() - 1; ++i) {
        TFloat pdf = (quantiles[i+1] - quantiles[i-1]) / (x[i+1] - x[i-1]);

        std::cout << x[i] << " " << quantiles[i] << " " << pdf << "\n";
    }
}

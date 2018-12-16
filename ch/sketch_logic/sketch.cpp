
#include <vector>
#include <sstream>
#include <exception>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>



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




void vec_op(TVecF &x, const TVecF& y, std::function<void(TFloat&, TFloat)> op) {
    ENSURE(x.size() == y.size(), "sizes should correspond");
    for (TSize xid=0; xid < x.size(); ++xid) {
        op(x[xid], y[xid]);
    }
}

void optimize(TVecF& l0, DMaxentLoss& loss, TFloat tol, TSize maxIter) {
    TFloat beta1 = 0.9;
    TFloat beta2 = 0.999;
    TFloat learningRate = 0.01;
    TFloat epsilon = 0.001;

    TVecF m = TVecF(l0.size(), 0.0);
    TVecF v = TVecF(l0.size(), 0.0);

    for (TSize iter=0; iter<maxIter; ++iter) {
        double value = loss.compute(l0);

        for (TSize xid=0; xid < l0.size(); ++xid) {
            m[xid] = beta1 * m[xid] + (1.0 - beta1) * loss.grad[xid];
            v[xid] = beta2 * v[xid] + (1.0 - beta2) * loss.grad[xid] * loss.grad[xid];

            TFloat weightGrad = m[xid] / (sqrt(v[xid]) + epsilon);
            // std::cout << weightGrad << ", ";

            // std::cout << loss.grad[xid] << ", ";
            l0[xid] += - learningRate * weightGrad;
        }

        // std::cout << "\n";

        // vec_op(l0, loss.grad, [](double& x, double y) {
        //     x += - 0.0001 * y;
        // });

        std::cout << "Iteration #" << iter << ": " << value << "\n";
    }

}

void run(TVecF powerSums, TFloat min, TFloat max) {
    TVecF powerChebyMoments = powerSumsToChebyMoments(powerSums, min, max);

    TInt gridSize = 1024;
    DMaxentLoss loss(powerChebyMoments, gridSize);

    TVecF l0(powerSums.size(), 0.0);
    l0[0] = std::log(1.0 / gridSize);
    std::cout << loss.compute(l0) << "\n";
    // optimize(l0, loss, 1e-06, 1000);
}




int main(int argc, const char** argv) {
    getBinomials_test();
    shiftPowerSum_test();
    powerSumsToChebyMoments_test();
    DMaxentLoss0_test();
    DMaxentLoss1_test();

    // TVecF sums = {1000,4616.627711851067,21313.65542468823,98400.94770643908,
    //                  454306.4464091641,2097523.412883016,9684407.540596521,
    //                  44714424.09630082,206457465.9479744,953283183.8847713,
    //                  4401712592.228215,20324967717.45339,93852641598.89296,
    //                  433382741898.2874,2001267865937.076};
    // TFloat min = 4.580868793901161;
    // TFloat max = 4.672197316422162;

    TVecF sums = {1000,101172.864051531,10240137.609583,1036877504.896428,105034307931.022,
                  10644348066181.16, 1079178179395168.0,109459991792955584.0,
                  11107280307688728576.0};

                                // 1.127591878416511e+21,1.145222240347376e+23,1.163651385521531e+25,
                                // 1.182913146584308e+27,1.203042997951893e+29,1.224078137992483e+31};
    TFloat min = 97.59915093074763;
    TFloat max = 106.9324489018609;
    run(sums, min, max);
}

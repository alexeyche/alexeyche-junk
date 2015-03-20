#pragma once


#include <dnn/base/base.h>
#include <dnn/util/util.h>

namespace dnn {


template <typename T = double>
class Distribution : public Printable {
public:
    virtual T getSample() = 0;
};


class NormalDistribution : public Distribution<double> {
public:
    NormalDistribution(double _mu, double _sigma) : mu(_mu), sigma(_sigma) {}
    double getSample() {
        return mu + sigma * getNorm();
    }
    void print(std::ostream& str) const {
        str << "Norm(" << mu << ", " << sigma << ")";
    }
    double mu;
    double sigma;
};

class UniformDistribution : public Distribution<double> {
public:
    UniformDistribution(double _from, double _to) : from(_from), to(_to) {}
    double getSample() {
        return from + getUnif() * (to - from);
    }
    void print(std::ostream& str) const {
        str << "Unif(" << from << ", " << to << ")";
    }

    double from;
    double to;
};


class ExpDistribution : public Distribution<double> {
public:
    ExpDistribution(double _gain, double _rate) : gain(_gain), rate(_rate) {}
    double getSample() {
        if (fabs(rate) < 0.00001) return 0.0;
        return gain * getExp(rate);
    }
    void print(std::ostream& str) const {
        str << "Exp(" << gain << ", " << rate << ")";
    }

    double gain;
    double rate;
};

template <typename T>
uptr<Distribution<T>> parseDistribution(const string &str_init) {
    if (strStartsWith(str_init, "Exp")) {
        vector<double> params = parseParenthesis(str_init);
        if (params.size() != 2) {
            throw dnnException()<< "Bad parameters to Exp distribution: " << str_init << "\n";
        }
        return uptr<ExpDistribution>(new ExpDistribution(params[0], params[1]));
    }
    if (strStartsWith(str_init, "Norm")) {
        vector<double> params = parseParenthesis(str_init);
        if (params.size() != 2) {
            throw dnnException()<< "Bad parameters to Norm distribution: " << str_init << "\n";
        }
        return uptr<NormalDistribution>(new NormalDistribution(params[0], params[1]));
    }
    if (strStartsWith(str_init, "Unif")) {
        vector<double> params = parseParenthesis(str_init);
        if (params.size() != 2) {
            throw dnnException()<< "Bad parameters to Unif distribution: " << str_init << "\n";
        }
        return uptr<UniformDistribution>(new UniformDistribution(params[0], params[1]));
    }
    throw dnnException()<< "Unknown Distribution " << str_init << "\n";
}


}

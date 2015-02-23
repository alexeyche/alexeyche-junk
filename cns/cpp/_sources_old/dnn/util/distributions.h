#pragma once


#include <snnlib/base.h>
#include <snnlib/util/util.h>

template <typename T>
class Distribution : public Printable {
public:
    virtual T getSample() = 0;
};


class NormalDistribution : public Distribution<double> {
public:
    NormalDistribution(double _mu, double _sigma) : mu(_mu), sigma(_sigma) {}
    double getSample() {
        return mu + sigma*getNorm();
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
        return from + getUnif()*(to-from);
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
       if(fabs(rate) < 0.00001) return 0.0;
        return gain*getExp(rate);
    }
    void print(std::ostream& str) const {
        str << "Exp(" << gain << ", " << rate << ")";
    }

    double gain;
    double rate;
};

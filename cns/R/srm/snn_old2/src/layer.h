#ifndef LAYER_H
#define LAYER_H

class NetSim;

class Layer {
public:
    Layer(const int &N_) : N(N_), saveStat(false), learn(true), determ(false) {}
    virtual void simdt(const double &t, const double &dt, const List &c, NetSim &n) = 0;
    virtual void prepare(const List &c) = 0;
    virtual const arma::uvec& getConns(size_t ni) const = 0;
    virtual const arma::uvec& getIds() const = 0;
    bool saveStat;
    bool learn;
    bool determ;
    int N;
};


#endif

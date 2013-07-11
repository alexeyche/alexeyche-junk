

#include <boost/numeric/odeint.hpp>



#include "lookup_table.h"



struct push_back_state_and_time
{
    std::vector< state_type >& m_states;
    std::vector< double >& m_times;

    push_back_state_and_time( std::vector< state_type > &states , std::vector< double > &times )
    : m_states( states ) , m_times( times ) { }

    void operator()( const state_type &x , double t )
    {
        m_states.push_back( x );
        m_times.push_back( t );
    }
};


int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
    LookupTableIzh lut;
    double Vs = -105.0;
    double us = -15.5;
    double Is = -72;
    NeuronIzh n;
    std::vector<double> vals;
    vals.push_back(Vs); vals.push_back(us); vals.push_back(Is); vals.push_back(0.5);
    float int_val = lut.bilineal_interpolation(vals);
    Log::Info << "interp: " << int_val << " real: " << lut.getValue(0,0,0,1) << std::endl;
    // for(size_t icyc=0; icyc<50; icyc++) {
    //     for(double t=0; t<50; t+=1) {
    //         lut.getValue(Vs, us, Is, t);
    //     }
    //     double u = lut.getLastU(Vs, us, Is);
    //     //std::cout << "last u: " << u << "\n";
    //     double V = lut.getValue(Vs, us, Is, 49.0);
    //     //std::cout << "last V: " << V << "\n";
    //     Vs=V;
    //     us=u;
    // }

    return 0;
}
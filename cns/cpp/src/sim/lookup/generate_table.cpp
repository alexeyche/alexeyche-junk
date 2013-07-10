

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
    double Vs = -103.0;
    double us = -11.0;
    double Is = 10;
    NeuronIzh n;
    lut.bilineal_interpolation(Vs, us, Is, 5.1);
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
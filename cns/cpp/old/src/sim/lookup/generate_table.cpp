

#include <boost/numeric/odeint.hpp>



#include "neuron_models.h"



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
    NeuronLIF *nlif = new NeuronLIF();
    state_type x(2);
    x[0] = -70;
    x[1] = 10;
    for(double t=0; t<1000; t++) {
        nlif->do_step(x,t);
        printf("t:%f\tv:%f\n",t,x[0]);
        if(t>500) {
           x[1] = 0;
        }
    }
    delete nlif;
    return 0;
}



#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>
#include <sim/int/DEIntegrator.h>
#include <sim/int/simple_int.h>

#include "srm.h"
#include "srm_neurons.h"

void epsp_test(bool just_print = false) {
    double Tmax = 100;
    double dt = 0.1;
    vec t = linspace<vec>(0.0, Tmax, (int)Tmax/dt);

    double fi = 0;
    double fj = 10;
    mat epsp(t.n_elem, 2);
    for(size_t ti=0; ti<t.n_elem; ti++) {
        if(t[ti] >= 15) {
            fi = 15; 
        }
        epsp(ti, 0) = t[ti];
        epsp(ti, 1) = srm::epsp(t[ti], fj, fi);
    }
    if(just_print) {
        epsp.print();
    } else {        
        send_arma_mat(epsp, "epsp");
    }        
}

void nu_test(bool just_print = false) {
    double Tmax = 100;
    double dt = 0.1;
    vec t = linspace<vec>(0.0, Tmax, (int)Tmax/dt);

    double fi = datum::inf;
    mat nupot(t.n_elem, 2);
    for(size_t ti=0; ti<t.n_elem; ti++) {
        if(t[ti] >= 15) {
            fi = 15; 
        }
        nupot(ti, 0) = t[ti];
        nupot(ti, 1) = srm::nu(t[ti], fi);
    }
    if(just_print) {
        nupot.print();
    } else {        
        send_arma_mat(nupot, "nupot");
    }        
}

void ttime_test() {
    srm::TTime in;
    std::srand(time(NULL));
//    vec  q       = randu<vec>(20);
    vec  q("1 2 3 4 5 6 7");
    uvec indices = sort_index(q);
    for(size_t iq=0; iq<indices.n_elem; iq++) {
        in.push_back(q(indices(iq)));
    }
    Log::Info << "TTime: \n";
    for(size_t ind=0; ind<in.size(); ind++) {
        Log::Info << "i:" << ind << " v: " << in[ind] << "\n";
    }
//    vec bs = randu<vec>(1);
    vec bs("3.1");
    size_t i = in.binary_search(bs(0));
    Log::Info << "binary search " << bs(0) << " : " << i << "\n";
    size_t test_i;
    if(in[0] > bs(0)) { 
        test_i = 0;
    } else {
        for(test_i=0; test_i<in.size(); test_i++) {
            if(in[test_i] > bs(0)) {
                test_i--;
                break;
            }
        }
        if(test_i == in.size()) {
            test_i--;
        }
    }
    Log::Info << "binary search (test) " << bs(0) << " : " << test_i << "\n";
    if(test_i == i) {
        Log::Info << "test passed\n";
    } else {
        Log::Info << "test failed\n";
    }
}


void srm_test() {
    srm::SrmNeuron n;
    for(size_t i=0; i<10; i++) {
        srm::SrmNeuron* inp_n = new srm::SrmNeuron();
        n.add_input(inp_n, 30);
    }
    n.in[5]->y.push_back(15);
    n.in[5]->y.push_back(16);
    n.in[5]->y.push_back(17);
    n.in[6]->y.push_back(16);
    n.in[6]->y.push_back(17);
    n.in[6]->y.push_back(18);
    n.in[7]->y.push_back(17);
    n.in[7]->y.push_back(18);
    n.in[7]->y.push_back(19);

    double Tmax = 100;
    double dt = 0.1;
    vec t = linspace<vec>(0.0, Tmax, (int)Tmax/dt);
    mat unif(t.n_elem, 1, fill::randu);
    mat stat(t.n_elem, 4);
    for(size_t ti=0; ti<t.n_elem; ti++) {
        stat(ti, 0) = t(ti);
        stat(ti, 1) = n.u(t(ti));     
        stat(ti, 2) = n.p(stat(ti,1));
        if(stat(ti, 2)*dt > unif(ti, 0)) {
            n.y.push_back(t(ti));
            Log::Info << "We had a spike at " << t(ti) << "(" << stat(ti, 2)*dt  << "<" << unif(ti,0)  << ")\n";
        } 
        stat(ti, 3) = n.y.last(t(ti));
    }
    send_arma_mat(stat, "stat");
    for(size_t ni=0; ni<n.in.size(); ni++) {
        delete n.in[ni];
    }
}

double prob(const double &t, srm::SrmNeuron *n) {
   return n->p(t);
}




void srm_lh_test() {
    srm::SrmNeuron n;
    for(size_t i=0; i<2; i++) {
        srm::SrmNeuron* inp_n = new srm::SrmNeuron();
        n.add_input(inp_n, 70);
    }
    n.in[0]->y.push_back(15);
    n.in[0]->y.push_back(16);
    n.in[0]->y.push_back(17);
    n.in[1]->y.push_back(19);
    n.in[1]->y.push_back(20);
    n.in[1]->y.push_back(21);
    double spike_time = 0;
    n.y.push_back(spike_time);
    Log::Info << prob(spike_time, &n) << "\n";
    double dt=0.1;
    vec w1 = linspace<vec>(-10, 500, (int)5/dt);
    vec w2 = linspace<vec>(-10, 500, (int)5/dt);
    n.w(0) = w1(0);
    n.w(1) = w2(0);
    n.w.print();
    
    double Tmax=100;
    mat w_de(w1.n_elem, w2.n_elem);
    mat w_trap(w1.n_elem, w2.n_elem);
    mat w_brute(w1.n_elem, w2.n_elem);
    int s_eval;
    double s_err;
    for(spike_time=0; spike_time<100; spike_time++) {
        for(size_t wi1=0; wi1<w1.n_elem; wi1++) {
            for(size_t wi2=0; wi2<w2.n_elem; wi2++) {
                int eval;
                double err;           
                n.w(0) = wi1;
                n.w(1) = wi2;
                w_de(wi1, wi2) = DEIntegrator<double>::Integrate(&n, &prob, 0, Tmax, 1e-02, eval, err);
    //            w_trap(wi1, wi2) = int_trapezium(0, Tmax, 300, n, &srm::SrmNeuron::p);
    //            w_brute(wi1, wi2) = int_brute(0, Tmax, 0.005, n, &srm::SrmNeuron::p);
                w_de(wi1, wi2) = n.p(spike_time)*exp(-w_de(wi1,wi2));
                s_eval += eval;
                s_err += err;
            }
        }        
        const unsigned int spike_time_int = spike_time;
        send_arma_mat(w_de, "wz_de", &spike_time_int);
    }
    Log::Info << "int eval(med.) " << s_eval/(w1.n_elem*w2.n_elem) << "\n";
    Log::Info << "int err(med.) " << s_err/(w1.n_elem*w2.n_elem) << "\n";
    send_arma_mat(w1, "w1");
    send_arma_mat(w2, "w2");
//    send_arma_mat(w_trap, "wz_trap");
//    send_arma_mat(w_brute, "wz_brute");
}


PROGRAM_INFO("SIM TEST", "sim tests"); 
PARAM_STRING("test", "name for test. default \"all\"", "t", "all");
PARAM_FLAG("nosend", "if it true. no sending to R", "n");

int main(int argc, char** argv) {
    CLI::ParseCommandLine(argc, argv);
	const std::string test_name = CLI::GetParam<std::string>("test");   
    bool nosend = CLI::HasParam("nosend");
    Log::Info << "Tests started" << std::endl;
    if((test_name == "all") || (test_name == "epsp")) {
        Log::Info << "epsp test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        epsp_test(nosend);
        Log::Info << "===============================================================" << std::endl;
    }        
    if((test_name == "all") || (test_name == "nu")) {
        Log::Info << "nu test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        nu_test(nosend);
        Log::Info << "===============================================================" << std::endl;
    }        
    if((test_name == "all") || (test_name == "srm")) {
        Log::Info << "srm test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        srm_test();
        Log::Info << "===============================================================" << std::endl;
    }        
    if((test_name == "all") || (test_name == "ttime")) {
        Log::Info << "ttime test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        ttime_test();
        Log::Info << "===============================================================" << std::endl;
    }        
    if((test_name == "all") || (test_name == "srm_lh")) {
        Log::Info << "srm likelyhood test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        srm_lh_test();
        Log::Info << "===============================================================" << std::endl;
    }
    return 0;
}




#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>
#include <sim/int/DEIntegrator.h>
#include <sim/int/simple_int.h>

#include "sim.h"
#include "neurons.h"
#include "research.h"
#include "entropy.h"
#include "groups.h"
#include "connections.h"

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
        epsp(ti, 1) = srm::SrmNeuron::epsp(t[ti], fj, fi);
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
        nupot(ti, 1) = srm::SrmNeuron::nu(t[ti], fi);
    }
    if(just_print) {
        nupot.print();
    } else {        
        send_arma_mat(nupot, "nupot");
    }        
}

void pfun_test() {
    vec u = linspace<vec>(-75, 0, 500);
    double beta = 1;
    double alpha = 1; 
    double tresh = -50;
    mat pfun(500,2);
    for(size_t ui=0; ui<u.n_elem; ui++) {
        pfun(ui,0) = u(ui);
        pfun(ui,1) = (beta/alpha)*(log(1+exp(alpha*(tresh-u(ui)))) - alpha*(tresh-u(ui)));
    }
    send_arma_mat(pfun, "pfun");
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
    vec bs("0.5");
    int i = in.binary_search(bs(0));
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
        n.add_input(inp_n, 2);
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
        stat(ti, 2) = n.p(t(ti));
        //if(stat(ti, 2)*dt > unif(ti, 0)) {
        //    n.y.push_back(t(ti));
        //    Log::Info << "We had a spike at " << t(ti) << "(" << stat(ti, 2)*dt  << "<" << unif(ti,0)  << ")\n";
        //} 
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
    n.in[0]->y.push_back(6);
    n.in[1]->y.push_back(15);
    n.in[1]->y.push_back(38);

    double dt=0.1;
    vec w1 = linspace<vec>(0, 500, (int)5/dt);
    vec w2 = linspace<vec>(0, 500, (int)5/dt);
    n.w(0) = w1(0);
    n.w(1) = w2(0);
    n.w.print();
    
    double Tmax=100;
    mat w_de(w1.n_elem, w2.n_elem);
    mat w_trap(w1.n_elem, w2.n_elem);
    mat w_brute(w1.n_elem, w2.n_elem);
    int s_eval;
    double s_err;
    size_t wmax1i=0;
    size_t wmax2i=0;
    double wmax=0;
    mat w_de_stat1(w1.n_elem, w2.n_elem);
    mat w_de_stat2(w1.n_elem, w2.n_elem);
    mat w_de_stat3(w1.n_elem, w2.n_elem);
    for(double spike_time=0; spike_time<100; spike_time++) {
        for(size_t wi1=0; wi1<w1.n_elem; wi1++) {
            for(size_t wi2=0; wi2<w2.n_elem; wi2++) {
                int eval;
                double err;           
                n.w(0) = wi1;
                n.w(1) = wi2;
                w_de_stat1(wi1, wi2) = DEIntegrator<double>::Integrate(&n, &prob, 0, Tmax, 1e-02, eval, err);
                w_de_stat2(wi1, wi2) = n.p(spike_time)*n.p(spike_time+25);
                w_de_stat3(wi1, wi2) = exp(-w_de_stat1(wi1,wi2));
                w_de(wi1, wi2) = w_de_stat2(wi1, wi2) * w_de_stat3(wi1, wi2);
                if(w_de(wi1, wi2)>wmax) {
                    Log::Info << wmax << "\n";
                    wmax = w_de(wi1, wi2);
                    wmax1i = wi1;
                    wmax2i = wi2;
                }
                s_eval += eval;
                s_err += err;
            }
        }        
        const unsigned int spike_time_int = spike_time;
        send_arma_mat(w_de, "wz_de", &spike_time_int);
        send_arma_mat(w_de_stat1, "wz_de_stat1", &spike_time_int);
        send_arma_mat(w_de_stat2, "wz_de_stat2", &spike_time_int);
        send_arma_mat(w_de_stat3, "wz_de_stat3", &spike_time_int);
    }
    Log::Info << "int eval(med.) " << s_eval/(w1.n_elem*w2.n_elem) << "\n";
    Log::Info << "int err(med.) " << s_err/(w1.n_elem*w2.n_elem) << "\n";
    Log::Info << "max p value " << wmax << " for weights " << w1(wmax1i) << ", " << w2(wmax2i) << " (ind: " << wmax1i << ", " << wmax2i << ")\n";
    send_arma_mat(w1, "w1");
    send_arma_mat(w2, "w2");
//    send_arma_mat(w_trap, "wz_trap");
//    send_arma_mat(w_brute, "wz_brute");
}

void testf1() {
    srm::SrmNeuron n;
    for(size_t i=0; i<2; i++) {
        srm::SrmNeuron* inp_n = new srm::SrmNeuron();
        n.add_input(inp_n, 70);
    }
    n.in[0]->y.push_back(6);
    n.in[1]->y.push_back(15);
    double Tmax = 100;
    int eval;
    double err;
    double pint = DEIntegrator<double>::Integrate(&n, &prob, 0, Tmax, 1e-02, eval, err);
    Log::Info << "pint: " << pint << "\n";

    for(size_t ni=0; ni<n.in.size(); ni++) {
        delete n.in[ni];
    }
}

void test_integral_calc() {
    srm::SrmNeuron n;

    double w_start = 3;
    n.add_input(new srm::DetermenisticNeuron("3  10 11 12"), w_start);
    n.add_input(new srm::DetermenisticNeuron("4  13 14 15"), w_start);
    n.add_input(new srm::DetermenisticNeuron("5  15 16 17"), w_start);
    n.add_input(new srm::DetermenisticNeuron("6 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("7 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("8 "), w_start);
    n.y.clean();

    double pfull = survFunction(&n, 0 , 100);
    Log::Info << "full: p[0,100] = " << pfull << "    H[0,100] = " << pfull*log(pfull) << "\n";
    double ppart = 1;
    for(double tr=0; tr<100; tr+=20) {
        double p = survFunction(&n, tr, tr+20);
        Log::Info << "p[" << tr << "," << tr+20 << "] = " << p << "    H[" << tr << "," << tr+20 << "] = " << p*log(p) << "\n";
        ppart *= p;
    }
    Log::Info << "part: p[0,100] = " << ppart <<  "    Hpart[0,100] = " << ppart*log(ppart) <<"\n";
    Log::Info << "delta: " << pfull - ppart << "\n";
}

void test_entropy_surface() {
    srm::SrmNeuron n;
    for(size_t i=0; i<2; i++) {
        srm::SrmNeuron* inp_n = new srm::SrmNeuron();
        n.add_input(inp_n, 70);
    }
//    srm::TimeSeriesGroup g(2, 0, 100); 
//    g.loadPatternFromFile("/var/tmp/d1.csv", 100, 100);
//    srm::connectFeedForward(&g, &n, 0.3);

    n.in[0]->y.push_back(6);
    n.in[1]->y.push_back(15);
    n.in[1]->y.push_back(38);

    double dt=1;
    vec w1 = linspace<vec>(0, 30, (int)5/dt);
    vec w2 = linspace<vec>(0, 30, (int)5/dt);
   
    double Tmax_all=50;
    unsigned int i = 0;
    for(double Tmax=10; Tmax<=Tmax_all; Tmax+=10) {
        mat H_stat(w1.n_elem, w2.n_elem);
        for(size_t wi1=0; wi1<w1.n_elem; wi1++) {
           for(size_t wi2=0; wi2<w2.n_elem; wi2++) {
                n.w(0) =  w1(wi1); 
                n.w(1) =  w2(wi2); 
                srm::EntropyCalc ec(&n, Tmax-10, Tmax);
                double H = ec.run(4);
                Log::Info << "w1: " << n.w(0) << " w2: " << n.w(1) << " H = " << H << "\n";
                H_stat(wi1, wi2) = H;
           }
        }       
        send_arma_mat(H_stat, "H_stat", &i);
        i++;
    }        
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
    if((test_name == "all") || (test_name == "pfun")) {
        Log::Info << "pfun test: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        pfun_test();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "testf1")) {
        Log::Info << "testf1: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        testf1();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "integral_calc")) {
        Log::Info << "integral_calc: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_integral_calc();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "entropy_surface")) {
        Log::Info << "entropy_surface: " << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_entropy_surface();
        Log::Info << "===============================================================" << std::endl;
    }
    
    return 0;
}


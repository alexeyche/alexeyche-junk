

#include <sim/core.h>
#include <sim/socket/sim_socket_core.h>
#include <sim/int/DEIntegrator.h>
#include <sim/int/simple_int.h>
#include <sim/int/gauss_legendre.c>

#include "sim.h"
#include "neurons.h"
#include "research.h"
#include "entropy.h"
#include "groups.h"
#include "connections.h"
#include "entropy_grad.h"

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

//double prob(const double &t, srm::SrmNeuron *n) {
//   return n->p(t);
//}


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
    n.w[0] = w1[0];
    n.w[1] = w2[0];
    //n.w.print();
    
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
                n.w[0] = wi1;
                n.w[1] = wi2;
                w_de_stat1(wi1, wi2) = DEIntegrator<double,srm::SrmNeuron*>::Integrate(&n, &srm::prob, 0, Tmax, 1e-02, eval, err);
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
    double pint = DEIntegrator<double, srm::SrmNeuron*>::Integrate(&n, &srm::prob, 0, Tmax, 1e-02, eval, err);
    Log::Info << "pint: " << pint << "\n";

    for(size_t ni=0; ni<n.in.size(); ni++) {
        delete n.in[ni];
    }
}

void test_integral_calc() {
    int seed = time(NULL);
    seed = 1385387986; 
    Log::Info << "seed: " << seed << "\n";
    std::srand(seed);
    srm::SrmNeuron n;
    srm::Sim s;
    double w_start = 3;
    n.add_input(new srm::DetermenisticNeuron("3 4 10 11 12"), w_start);
    n.add_input(new srm::DetermenisticNeuron("4 5 13 14 15"), w_start);
    n.add_input(new srm::DetermenisticNeuron("5 6 15 16 17"), w_start);
    n.add_input(new srm::DetermenisticNeuron("6 7 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("7 8"), w_start);
    n.add_input(new srm::DetermenisticNeuron("8 9"), w_start);
    
    s.addNeuron(&n);
    s.run(50*srm::ms, 0.1); 
    
    double pfull = srm::survFunction(&n, 0 , 20);
    double pfull_test = srm::survFunction__test(&n, 0 , 20);
    Log::Info << "full: p[0,20] = " << pfull << "    H[0,20] = " << pfull*log(pfull) << "\n";
    Log::Info << "full test: p[0,20] = " << pfull_test << "    H[0,20] = " << pfull_test*log(pfull_test) << "\n";
    return;
    double ppart = 0;
    double ppart_test = 0;
    for(double tr=0; tr<100; tr+=20) {
        double p = survFunction(&n, tr, tr+20);
        double p_test = survFunction__test(&n, tr, tr+20);
        Log::Info << "p[" << tr << "," << tr+20 << "] = " << p << "    H[" << tr << "," << tr+20 << "] = " << p*log(p) << "\n";
        Log::Info << "p_test[" << tr << "," << tr+20 << "] = " << p_test << "    H_test[" << tr << "," << tr+20 << "] = " << p_test*log(p_test) << "\n";
        ppart += p;
        ppart_test += p_test;
    }
    Log::Info << "part: p[0,100] = " << ppart <<  "    Hpart[0,100] = " << ppart*log(ppart) <<"\n";
    Log::Info << "part_test: p[0,100] = " << ppart_test <<  "    Hpart_test[0,100] = " << ppart_test*log(ppart_test) <<"\n";
    Log::Info << "delta: " << pfull - ppart << "\n";
    Log::Info << "delta_test: " << pfull_test - ppart_test << "\n";

}

double survFunction_brute(srm::SrmNeuron *n, double T0, double Tmax) {
    double t_right=Tmax;
    double p = 1;
    Log::Info << "Surv.function of " << n->id() << " (from behind)\n";
    Log::Info << t_right << "|      ";
    for(int yi = n->y.n_elem(Tmax)-1; yi>=0; yi--) {
        if(n->y[yi]<T0) { break; } 
        double t_left = n->y[yi];
        double no_spike = exp(-int_brute<srm::SrmNeuron>(t_left+1e-12, t_right, 0.0001, n, &srm::prob));
        double spike = 1-exp(-srm::prob(t_left, n));
        Log::Info << "p:" << no_spike << "     |spike t:" << t_left << " p:" << spike << "|     ";
        p = p*no_spike*spike;
        t_right = t_left-1e-12;
    }
    double no_spike = exp(-int_brute<srm::SrmNeuron>(T0, t_right, 0.0001, n, &srm::prob));
    p = p*no_spike;
    Log::Info << no_spike << "|" << T0 << "\n";
    Log::Info << "p = " << p << "\n";
    return p;
}

double integrand_gl(double x, void* data) {
    srm::SrmNeuron *n = (srm::SrmNeuron*)data;
    return n->p(x);
}


void test_int_calc2() {
    std::srand(1);
    srm::SrmNeuron n;
    srm::Sim s;
    double w_start = 4;
    n.add_input(new srm::DetermenisticNeuron("3 4 10 11 12"), w_start);
    n.add_input(new srm::DetermenisticNeuron("4 5 13 14 15"), w_start);
    n.add_input(new srm::DetermenisticNeuron("5 6 15 16 17"), w_start);
    n.add_input(new srm::DetermenisticNeuron("6 7 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("7 8"), w_start);
    n.add_input(new srm::DetermenisticNeuron("8 9"), w_start);
    
    s.addNeuron(&n);
    s.run(50*srm::ms, 0.1); 
    
    double p_br = int_brute<srm::SrmNeuron>(0, 50, 0.00001, &n, &srm::prob);
    double p_de = DEIntegrator<double, srm::SrmNeuron*>::Integrate(&n, &srm::prob, 0, 50, 1e-12);
    double p_gl = 0;
    double delta_p_gl = datum::inf;
    int delta_n = -1;
    double p_gl_best;
    for(size_t i=2; i<1024; i++) {
        p_gl = gauss_legendre(i, integrand_gl, (void*)&n, 0, 50);
        if(p_gl > 0) {
            Log::Info << "p_gl (" << i << ") delta: " << fabs(p_gl - p_br) << " int: " << p_gl << "\n";
            if( fabs(p_gl - p_br) < delta_p_gl) { delta_p_gl = fabs(p_gl-p_br); delta_n = i; p_gl_best = p_gl; }
        }            
    }        
    Log::Info << "p_gl (" << delta_n << ") delta: " << delta_p_gl << " int: " <<  p_gl_best << "\n";
    Log::Info << "p_br " << p_br << "\n";
    Log::Info << "p_de " << p_de << "\n";
    double p_br_p = 0;
    double p_de_p = 0;
    double p_gl_p = 0;
    Log::Info << "brute part: \n";
    for(double t=0; t<50; t+=5) {
        double pp = int_brute<srm::SrmNeuron>(t, t+5, 0.0001, &n, &srm::prob);
        p_br_p = p_br_p+pp;
        Log::Info << t << ":" << t+5 << " = " << pp << "\n";
    }        
    Log::Info << "DE part: \n";
    for(double t=0; t<50; t+=5) {
        double pp = DEIntegrator<double, srm::SrmNeuron*>::Integrate(&n, &srm::prob, t, t+5, 1e-12);
        p_de_p = p_de_p+pp;
        Log::Info << t << ":" << t+5 << " = " << pp  << "\n";
    }        
    Log::Info << "GL part: \n";
    for(double t=0; t<50; t+=5) {
        double pp = gauss_legendre(delta_n, integrand_gl, (void*)&n, t, t+5);
        p_gl_p = p_gl_p+pp;
        Log::Info << t << ":" << t+5 << " = " << pp  << "\n";
    }    
    Log::Info << "DE delta = " << p_de_p - p_de << "\n"; 
    Log::Info << "brute delta = " << p_br_p - p_br << "\n"; 
    Log::Info << "GL delta = " << p_gl_p - p_gl << "\n"; 
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
                n.w[0] =  w1(wi1); 
                n.w[1] =  w2(wi2); 
                srm::TEntropyCalc ec(&n, Tmax-10, Tmax);
                double H = ec.run(4);
                Log::Info << "w1: " << n.w[0] << " w2: " << n.w[1] << " H = " << H << "\n";
                H_stat(wi1, wi2) = H;
           }
        }       
        send_arma_mat(H_stat, "H_stat", &i);
        i++;
    }        
}

double break_apart_int_test(srm::SrmNeuron *n, double T0, double Tmax, double dt) {
    double t;
    double integral = 0;
    for(t=T0; t<Tmax; t+=dt) {
        double t_right = t+dt;
        double t_left = t;
        if(t_right>Tmax) { t_right = Tmax; t=datum::inf; } 
            
//        double integral_cur = int_brute<srm::SrmNeuron>(t_left, t_right, 0.000001, n, &srm::prob);
        double integral_cur = gauss_legendre(1024, integrand_gl, (void*)n, t_left, t_right);

        Log::Info << "integral at " << t_left << ":" << t_right << " = " << integral_cur << "\n";
        integral += integral_cur;
    }
    return integral;
}


double surv0(srm::SrmNeuron *n, double T0, double Tmax) {
    double integral = 0;
    double t_left = T0;
    for(size_t yi=0; yi<n->y.size(); yi++) {
        double t_right = n->y[yi];
        double integral_cur = break_apart_int_test(n, t_left, t_right, 3.); //int_brute<srm::SrmNeuron>(t_left, t_right, 0.00001, n, &srm::prob);
//        Log::Info << "integral at " << t_left << ":" << t_right << " = " << integral_cur << "\n";
        integral += integral_cur;
        t_left = t_right;
    }
    double integral_cur = break_apart_int_test(n, t_left, Tmax, 3.); //int_brute<srm::SrmNeuron>(t_left, Tmax, 0.00001, n, &srm::prob);
    integral += integral_cur;
//    Log::Info << "integral at " << t_left << ":" << Tmax << " = " << integral_cur << "\n";
    Log::Info << "integral whole = " << integral << "\n";
}

double surv1(srm::SrmNeuron *n, double T0, double Tmax) {
    double no_spike = break_apart_int_test(n, T0, Tmax, 50.0); //int_brute<srm::SrmNeuron>(T0, Tmax, 0.00001, n, &srm::prob);
    Log::Info << "integral at whole = " << no_spike << "\n";
//    for(size_t yi=0; yi<n->y.size(); yi++) {
//        double t_right = n->y[yi];
//        Log::Info << "integral at " << t_left << ":" << t_right << " = " << integral_cur << "\n";
//        integral += integral_cur;
//        t_left = t_right;
//    }
//    double integral_cur = int_brute<srm::SrmNeuron>(t_left, Tmax, 0.00001, n, &srm::prob);
//    Log::Info << "integral at " << t_left << ":" << Tmax << " = " << integral_cur << "\n";
//    Log::Info << "integral whole = " << integral << "\n";
}
void test_surv_func() {
    std::srand(time(NULL));
    srm::SrmNeuron n;
    srm::Sim s;
    double w_start = 4;
    n.add_input(new srm::DetermenisticNeuron("3 4 10 11 12"), w_start);
    n.add_input(new srm::DetermenisticNeuron("4 5 13 14 15"), w_start);
    n.add_input(new srm::DetermenisticNeuron("5 6 15 16 17"), w_start);
    n.add_input(new srm::DetermenisticNeuron("6 7 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("7 8"), w_start);
    n.add_input(new srm::DetermenisticNeuron("8 9"), w_start);
    
    s.addNeuron(&n);
    s.run(50*srm::ms, 0.1); 
    vec n_prob(50/0.1);
    size_t i=0;
    for(double t=0; t<50; t+=0.1) {
        n_prob(i) = n.p(t);
        i++;
    }
    send_arma_mat(n_prob, "n_prob");
    Log::Info << "///////////////////////////////////////////\n";
    surv0(&n, 0, 50); 
    Log::Info << "///////////////////////////////////////////\n";
    surv1(&n, 0, 50); 
    Log::Info << "///////////////////////////////////////////\n";

}

void test_p_stroke() {
    std::srand(time(NULL));
    srm::SrmNeuron n;
    srm::Sim s;
    double w_start = 4;
    n.add_input(new srm::DetermenisticNeuron("3 4 10 11 12"), w_start);
    n.add_input(new srm::DetermenisticNeuron("4 5 13 14 15"), w_start);
    n.add_input(new srm::DetermenisticNeuron("5 6 15 16 17"), w_start);
    n.add_input(new srm::DetermenisticNeuron("6 7 "), w_start);
    n.add_input(new srm::DetermenisticNeuron("7 8"), w_start);
    n.add_input(new srm::DetermenisticNeuron("8 9"), w_start);
    s.addNeuron(&n);
    s.run(50, 0.1);
    srm::TEntropyGrad eg(&n, 0, 25);
    vec t = linspace<vec>(0, 50, 150);
    vec ps(150, fill::zeros);
    for(size_t ti=0; ti<t.n_elem; ti++) {
  //      ps(ti) = eg.p_stroke(t(ti), &n);
    }
    send_arma_mat(ps, "ps");
}

mat H_surface(srm::SrmNeuron *n, double f1, double f2) {
    n->in[0]->y[0] = f1;
    n->in[1]->y[0] = f2;
    double dw=5;
    double wmin=0;
    double wmax=50;
    vec w1=linspace<vec>(wmin, wmax, (wmax-wmin)/dw);
    vec w2=linspace<vec>(wmin, wmax, (wmax-wmin)/dw);
    mat Hsurf(w1.n_elem, w2.n_elem);
    srm::TEntropyCalc ec(n, 0, 50);
    for(size_t wi1=0; wi1<w1.n_elem; wi1++) {
        for(size_t wi2=0; wi2<w2.n_elem; wi2++) {
            n->w[0] = w1(wi1);
            n->w[1] = w2(wi2);
            Hsurf(wi1, wi2) = ec.run(3);
            Log::Info << "wi1: " << wi1 << " wi2: " << wi2 << " w1: " << w1(wi1) << " w2: " << w2(wi2)  << " H: " << Hsurf(wi1,wi2) << "\n"; 
        }
    }
    return Hsurf;
}

vec get_mean_firetimes(srm::SrmNeuron *n, srm::Sim *s, double simtime = 50, double ntrials = 20, int max_fires=10) {
    vec fmean(max_fires, fill::zeros);
    uvec fire_times(max_fires, fill::zeros);

    for(size_t tr_i=0; tr_i<ntrials; tr_i++) {
        n->y.clean();
        s->run(simtime*srm::ms,0.5, false);
        for(size_t yi=0; yi<std::min((double)max_fires,(double)n->y.size()); yi++) {
            fmean(yi) += n->y[yi];
            fire_times(yi) += 1; 
        }
    }
    return fmean/fire_times;
}

void test_stdp() {
    std::srand(time(NULL));
    srm::Sim s;
    srm::SrmNeuron n;

    n.add_input(new srm::DetermenisticNeuron("25"), 50); 
    n.add_input(new srm::DetermenisticNeuron("25"), 10);

    s.addRecNeuron(&n);
    for(size_t wi=0; wi<n.w.size(); wi++) { Log::Info << n.w[wi] << ", "; } Log::Info << "\n";
    double t2_start = 0;
    double t2_end = 50;
    double dt = 1;
    mat stdp1( (t2_end-t2_start)/dt+1, 2);
    mat stdp2( (t2_end-t2_start)/dt+1, 2);
    size_t ti = 0;
    mat probs((t2_end-t2_start)/dt+1, 50/0.5, fill::zeros);
    mat pots((t2_end-t2_start)/dt+1, 50/0.5, fill::zeros);
    for(double t2=t2_start; t2<=t2_end; t2 += dt) { 
        n.w[0] = 25; // 55, 5 - classic stdp, alpha=1 beta=1 ; 25, 5, beta=0.25, alpha=0.35 - More ltd
        n.w[1] = 7.5;  // 
        n.in[1]->y[0] = t2;
        vec fmeanv = get_mean_firetimes(&n, &s, 50, 20, 1);
        double fmean = fmeanv(0);
        vec ttest = linspace<vec>(0,50, 50/0.5);
        for(size_t tti=0; tti<ttest.n_elem; tti++) { srm::TTime ytest(1); ytest[0] = fmean; probs(ti, tti) = n.p(ttest(tti), ytest); pots(ti, tti) = n.u(ttest(tti), ytest);}
        Log::Info << "=================================\n";
        Log::Info << "delta t = " << fmean - n.in[1]->y[0] <<  "\n";
        srm::TEntropyGrad eg(&n, 0, 50);
        srm::TEntropyCalc ec(&n, 0, 50);
        vec dHdw_mean(n.w.size(), fill::zeros);
        size_t epoch = 1;
        for(size_t i=0; i<epoch; i++) {  // epoch
            vec dHdw = eg.grad();
            dHdw_mean += dHdw;
            for(size_t wi=0; wi<n.w.size(); wi++) { n.w[wi] -= 3 * dHdw(wi); }
            Log::Info << "Epoch(" << i << "): ";
            Log::Info << "Grad: ";
            for(size_t wi=0; wi<n.w.size(); wi++) { Log::Info << dHdw(wi) << ", "; }
            if(i % 1 == 0) {
                Log::Info << " H: ";
                double H = ec.run(3);
                Log::Info << H;
            }            
            Log::Info << "\n";
        }
        dHdw_mean = -dHdw_mean/epoch;
        stdp1(ti,0) = fmean - n.in[0]->y[0];
        stdp1(ti,1) = dHdw_mean(0);
        stdp2(ti,0) = fmean - n.in[1]->y[0];
        stdp2(ti,1) = dHdw_mean(1);
        ti+=1;
    }        
    send_arma_mat(stdp1, "stdp1", NULL, true);
    send_arma_mat(stdp2, "stdp2", NULL, true);
    send_arma_mat(probs, "probs");
    send_arma_mat(pots, "pots");
}

void test_stdp_many() {
    std::srand(time(NULL));
    srm::Sim s;
    srm::SrmNeuron n;

    n.add_input(new srm::DetermenisticNeuron("1"), 7); 
    n.add_input(new srm::DetermenisticNeuron("1"), 7);
    n.add_input(new srm::DetermenisticNeuron("1"), 7); 
    n.add_input(new srm::DetermenisticNeuron("1"), 7);
    n.add_input(new srm::DetermenisticNeuron("1"), 7); 
    n.add_input(new srm::DetermenisticNeuron("1"), 1);
    n.add_input(new srm::DetermenisticNeuron("1"), 1); 
    n.add_input(new srm::DetermenisticNeuron("1"), 1);
    n.add_input(new srm::DetermenisticNeuron("1"), 1); 
    n.add_input(new srm::DetermenisticNeuron("1"), 1);
    s.addRecNeuron(&n);

//    s.addStatListener(&n, srm::TStatListener::Spike);
//    s.addStatListener(&n, srm::TStatListener::Pot);
//    s.addStatListener(&n, srm::TStatListener::Prob);

    s.run(100*srm::ms, 0.5, true);
    double tstart=0;
    double tend=8;
    double dt=4;
    mat grads((tend-tstart)/dt, n.w.size());
    size_t gi=0;
    for(double tshift=tstart; tshift<tend; tshift+=dt) {
        Log::Info << "=================================\n";
        Log::Info << "=================================\n";
        double cur_tshift = tshift/5;
        for(size_t ni=5; ni<n.w.size(); ni++) {
          n.in[ni]->y[0] = 1+cur_tshift*(ni-4);
          Log::Info << "ni("<< ni << "): " << n.in[ni]->y[0] << ", ";
        }
        Log::Info << "\n";
        Log::Info << "=================================\n";
        vec meanf = get_mean_firetimes(&n, &s, 100, 20, 5);
        for(size_t mfi=0; mfi<meanf.n_elem; mfi++) { if(meanf[mfi]>0.001) { Log::Info << meanf[mfi] << ", "; } }
        Log::Info << "\n";
        for(size_t ni=5; ni<n.w.size(); ni++) { Log::Info << "dt = " << meanf[0]- n.in[ni]->y[0] << ", "; } 
        Log::Info <<  "\n";
        vec dHdw(n.w.size(), fill::zeros);
        for(double tg=0; tg<100; tg+=10) {
            srm::TEntropyGrad eg(&n, tg, tg+20);
            dHdw += eg.grad();
        }            
        Log::Info << "Grad: ";
        for(size_t wi=0; wi<n.w.size(); wi++) { grads(gi,wi) = -dHdw(wi); Log::Info << dHdw(wi) << ", "; }
        gi++;
        Log::Info << "\n";
    }
    send_arma_mat(grads,"grads",NULL, true);

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
    if((test_name == "all") || (test_name == "int_calc2")) {
        Log::Info << "int_calc2:" << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_int_calc2();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "surv_func")) {
        Log::Info << "int_calc2:" << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_surv_func();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "p_stroke")) {
        Log::Info << "p_stroke:" << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_p_stroke();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "stdp")) {
        Log::Info << "stdp:" << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_stdp();
        Log::Info << "===============================================================" << std::endl;
    }
    if((test_name == "all") || (test_name == "stdp_many")) {
        Log::Info << "stdp_many:" << std::endl;
        Log::Info << "===============================================================" << std::endl;
        test_stdp_many();
        Log::Info << "===============================================================" << std::endl;
    }
  
    return 0;
}


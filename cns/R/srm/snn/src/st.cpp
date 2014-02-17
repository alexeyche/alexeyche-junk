
#include <RcppArmadillo.h>

using namespace Rcpp;

#include "gauss_legendre.h"

// [[Rcpp::depends("RcppArmadillo")]]

double erf_int(double x, void *data) {
    return exp(-x*x);
}

double erf(double x) {
    return (2/sqrt(PI)) *gauss_legendre(256, erf_int, NULL, 0, x);
}

void rectifiedGaussian(double mu,double sigma, double *mur,double *sigmar) {
	// Calculate the mean and standard deviation for a rectified
	// Gaussian distribution
	// mu, sigma: parameters of the original distribution
	// *mur, *sigmar: parameters of the rectified distribution
	//
	// CHECKED	
	double a=1.+erf(mu/(sqrt(2)*sigma));
	
	*mur=(sigma/sqrt(2.*M_PI))*exp(-0.5*std::pow(mu/sigma,2))+.5*mu*a;
	*sigmar=sqrt((mu-*mur)*(*mur)+.5*std::pow(sigma,2)*a);
}

void invRectifiedGaussian(double mur,double sigmar, double *mu,double *sigma) {
	// Inverse of function rectifiedGaussian
	//
	// CHECKED
	double m,s;
	double x0,x1,fx0,fx1,x2;
	int n;

	// Use the secant method to find the root
	// Initialization
	x0=mur/sigmar;
	x1=1.1*(mur/sigmar);
	rectifiedGaussian(x0,1.,&m,&s);
	fx1=m/s-mur/sigmar;
	// Iterations
	n=0;
	while ((fabs(fx1)>1e-4) && (n<1000)) {
		rectifiedGaussian(x1,1.,&m,&s);
		fx0=fx1;
		fx1=m/s-mur/sigmar;
		x2=x1;
		x1=x1-((x1-x0)/(fx1-fx0))*fx1;
		x0=x2;
		n++;
	}
	
	*sigma=mur/(exp(-0.5*x1*x1)/(sqrt(2.*M_PI))+.5*x1*(1.+erf(x1/sqrt(2.))));
	*mu=x1*(*sigma);
}

List homogeneousPoolCoxCD1(double r, double c, double tauc, double dt, int ndt, int n) {
	// Clock-driven generation of homogeneously correlated spike trains
	// (Cox processes)
	// r = rate (Hz)
	// c = total correlation strength (in [0,1])
	// tauc = correlation time constant (ms)
	// dt = timestep (ms)
	// ndt = number of timesteps
	// n = number of neurons
	// spike = array of 0/1 integers, each row is a timestep, each column is a neuron
    List net(n);
    for(size_t ni=0; ni<n; ni++) {
        net[ni] = NumericVector();
    }
	double sigmar,sigma,s,lambda,x,mu;
	int i,j;
	
	// Correction of mu and sigma
	sigmar=sqrt(c*r/(2.*tauc*0.001));
	invRectifiedGaussian(r,sigmar,&mu,&sigma);
	
	// Simulation
	x=0.;
	lambda=exp(-dt/tauc);
	s=sigma*sqrt(1-exp(-2.*dt/tauc));
	mu=mu*dt*0.001;
	s=s*dt*0.001;
    NumericVector nrm = rnorm(ndt, 0, s);
	for(j=0;j<ndt;j++) {
		x=x*lambda+nrm[j];
        NumericVector coins = runif(n);
		for(i=0;i<n;i++)
			if (coins[i]<x+mu) {
                NumericVector sp = as<NumericVector>(net[i]);
                sp.push_back(j);
                net[i] = sp;
			}
	}
    return net;
}

// [[Rcpp::export]]
SEXP generateCorrPoiss(int M, double r, double corr, double T0, double Tmax, double dt, double tauc) {
    return homogeneousPoolCoxCD1(r, corr, tauc, dt, (Tmax-T0)/dt, M);
}
	

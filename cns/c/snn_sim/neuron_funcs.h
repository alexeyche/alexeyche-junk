#ifndef NEURON_FUNCS_H
#define NEURON_FUNCS_H

#include <math.h>

#include <constants.h>
double exp5(double x);
double probf( const double *u, const Constants *c);
double pstroke(const double *u, const Constants *c);
double B_calc(const unsigned char *Yspike, const double *p, const double *pmean, const Constants *c);
double C_calc(const unsigned char *Yspike, const double *p, const double *u, const double *M, const double *syn, const Constants *c);
double rate_calc(const double *w, const Constants *c);
double bound_grad(const double *w, const double *dw, const double *wmax, const Constants *c);

#endif

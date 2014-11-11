#ifndef NEURON_FUNCS_H
#define NEURON_FUNCS_H

#include <math.h>

#include <core/constants.h>
double exp5(double x);
double probf( const double *u, const Constants *c);
double pstroke(const double *u, const Constants *c);
double B_calc(const unsigned char *Yspike, const double *p, const double *pmean, const Constants *c);
double C_calc(const unsigned char *Yspike, const double *p, const double *p_stroke, const double *u, const double *denominator_p, const double *syn, const Constants *c);
double rate_calc(const double *w, const Constants *c);
double bound_grad(const double *w, const double *dw, const double *wmax, const Constants *c);
double fast_exp (double x);

double prob_fun_Exp( const double *u, const Constants *c);
double prob_fun_stroke_Exp( const double *u, const Constants *c);

double prob_fun_ExpHennequin( const double *u, const Constants *c);
double prob_fun_stroke_ExpHennequin( const double *u, const Constants *c);

double prob_fun_ExpBohte( const double *u, const Constants *c);
double prob_fun_stroke_ExpBohte( const double *u, const Constants *c);

double prob_fun_LinToyoizumi( const double *u, const Constants *c);
double prob_fun_stroke_LinToyoizumi( const double *u, const Constants *c);

double prob_fun_Determ( const double *u, const Constants *c);
double prob_fun_stroke_Determ( const double *u, const Constants *c);

#endif


#include "neuron_funcs.h"

#define LINEAR 1
#define EXP 2
#define EXP2 3
#define SIMP_EXP 4

#define B_HARD 1
#define B_SOFT 2

#define PROB_FUNC SIMP_EXP
#define WEIGHT_BOUND B_HARD

static double old_arg = -999999999, old_result;

double exp_cached(double arg, double* old_arg, double* old_result){
  if (arg== *old_arg) return *old_result;
  *old_arg = arg;
  *old_result = exp(arg);
  return *old_result;
}

/* max. rel. error <= 1.73e-3 on [-87,88] */
double fast_exp (double x)
 {
   volatile union {
     double f;
     unsigned int i;
   } cvt;

   /* exp(x) = 2^i * 2^f; i = floor (log2(e) * x), 0 <= f <= 1 */
   double t = x * 1.442695041f;
   double fi = floorf (t);
   double f = t - fi;
   int i = (int)fi;
   cvt.f = (0.3371894346f * f + 0.657636276f) * f + 1.00172476f; /* compute 2^f */
   cvt.i += (i << 23);                                          /* scale by 2^i */
   return cvt.f;
 }

inline double probf( const double *u, const Constants *c) {
#if PROB_FUNC == LINEAR 
    double p = (c->pr + (*u - c->u_rest)*c->gain_factor)/c->sim_dim;
    if(p < c->pr/c->sim_dim) return(c->pr/c->sim_dim);
#elif PROB_FUNC == EXP
    if( *u  <= c->u_rest ) return(c->__pr);    
    double p = ( (c->pr + (c->beta/c->alpha) * (log(1 + exp( c->alpha*(c->u_tr - *u))) - c->alpha * (c->u_tr - *u)))/c->sim_dim );
#elif PROB_FUNC == EXP2
    if( *u  <= c->u_rest ) return(c->__pr);    
    double p = (c->pr + c->r0 * log(1 + exp( c->beta*(*u - c->u_tr) )))/c->sim_dim;
    if(p>1) return(1);
    return(p);
#elif PROB_FUNC == SIMP_EXP
    return( exp(*u) );
#endif
}


inline double pstroke(const double *u, const Constants *c) {
#if PROB_FUNC == LINEAR 
    return( c->gain_factor / c->sim_dim );
#elif PROB_FUNC == EXP
    return( (c->beta/(1+exp(c->alpha*(c->u_tr - *u))))/c->sim_dim );
#elif PROB_FUNC == EXP2
//    printf("%3.10f\n", c->beta*(*u - c->u_tr));
//    double part = exp_cached(c->beta*(*u - c->u_tr), &old_arg, &old_result);
    double part = exp(c->beta*(*u - c->u_tr));
//    double part = exp(c->beta*(*u - c->u_tr));

    return( ( (part*c->r0*c->beta) / ( 1+part))/c->sim_dim );
#elif PROB_FUNC == SIMP_EXP
    return( exp(*u) );
#endif
}

// new versions:
double prob_fun_Exp( const double *u, const Constants *c) {
    return( exp(*u)/c->sim_dim );
}
double prob_fun_stroke_Exp( const double *u, const Constants *c) {
    return( exp(*u)/c->sim_dim );
}


double prob_fun_ExpHennequin( const double *u, const Constants *c) {
    if( *u  <= c->u_rest ) return(c->__pr);    
    double p = (c->pr + c->r0 * log(1 + exp( c->beta*(*u - c->u_tr) )))/c->sim_dim;
    if(p>1) return(1);
    return(p);
}
double prob_fun_stroke_ExpHennequin( const double *u, const Constants *c) {
    double part = exp(c->beta*(*u - c->u_tr));
//    double part = exp(c->beta*(*u - c->u_tr));
    return( ( (part*c->r0*c->beta) / ( 1+part))/c->sim_dim );
}


double prob_fun_ExpBohte( const double *u, const Constants *c) {
    if( *u  <= c->u_rest ) return(c->__pr);    
    double p = ( (c->pr + (c->beta/c->alpha) * (log(1 + exp( c->alpha*(c->u_tr - *u))) - c->alpha * (c->u_tr - *u)))/c->sim_dim );
    return( p );
}
double prob_fun_stroke_ExpBohte( const double *u, const Constants *c) {
    return( (c->beta/(1+exp(c->alpha*(c->u_tr - *u))))/c->sim_dim );
}


double prob_fun_LinToyoizumi( const double *u, const Constants *c) {
    double p = (c->pr + (*u - c->u_rest)*c->gain_factor)/c->sim_dim;
    if(p < c->pr/c->sim_dim) return(c->pr/c->sim_dim);
    return( p );
}
double prob_fun_stroke_LinToyoizumi( const double *u, const Constants *c) {
    return( c->gain_factor / c->sim_dim );
}


double bound_grad(const double *w, const double *dw, const double *wmax, const Constants *c) {
#if WEIGHT_BOUND == B_SOFT   
    if(*dw > 0) return(*dw);
    return( (1 - 1/(1+c->aw*(*w/c->ws)) + (1/(1+c->aw))*(*w/c->ws)) * *dw );
#elif WEIGHT_BOUND == B_HARD
    if( *w > *wmax) return(0);
    if( *w < 0 ) return(0);      
    return(*dw);
#endif   
}


//inline double rate_calc(const double *w, const Constants *c) {
//    double norm_w = *w/(2*c->ws);
//    double w4 = pow(*w, 4);
//    return( 0.04*(w4/(w4+0.0016)) );
//    if( norm_w > 1) {
//        return(0.04);
//    } else
//    if( norm_w < 0) {
//        return(0.0);
//    } else {
//        double k,b;
//        if(norm_w > 0.5) {
//            k = 0.06; b = -0.020;
//        } else
//        if(norm_w > 0.25) {
//            k = 0.03; b = -0.005;
//        } else {
//            k = 0.01; b = 0;
//        }
////        printf("x: %f y: %f\n", norm_w, norm_w*k+b);
//        return(norm_w*k+b);
//    }        
//}


#include "neuron_funcs.h"

#define LINEAR 1
#define EXP 2

#define PROB_FUNC LINEAR


double probf( const double *u, const Constants *c) {
#if PROB_FUNC == LINEAR 
    double p = (c->pr + (*u - c->u_rest)*c->gain_factor)/c->sim_dim;
    if(p < c->pr/c->sim_dim) return(c->pr/c->sim_dim);
#elif PROB_FUNC == EXP
    if( *u  <= c->u_rest ) return(c->__pr);    
    double p = ( (c->pr + (c->beta/c->alpha) * (log(1 + exp( c->alpha*(c->tr - *u))) - c->alpha * (c->tr - *u)))/c->sim_dim );
#endif
    return(p);
}


double pstroke(const double *u, const Constants *c) {
#if PROB_FUNC == LINEAR 
    return( c->gain_factor / c->sim_dim );
#elif PROB_FUNC == EXP
    return( (c->beta/(1+exp(c->alpha*(c->tr - *u))))/c->sim_dim );
#endif
}


double B_calc(const unsigned char *Yspike, const double *p, const double *pmean, const Constants *c) {
    if( *pmean == 0 ) return(0);
    double pmean_w = *pmean/c->mean_p_dur;
//    printf("pmean_w %f, 1part: %f, 2part: %f\n", pmean_w, ( *Yspike * log( *p/pmean_w) - (*p - pmean_w)), c->target_rate_factor * ( *Yspike * log( pmean_w/c->__target_rate) - (pmean_w - c->__target_rate) ));
    return (( *Yspike * log( *p/pmean_w) - (*p - pmean_w)) - c->target_rate_factor * ( *Yspike * log( pmean_w/c->__target_rate) - (pmean_w - c->__target_rate) ));

}


double C_calc(const unsigned char *Yspike, const double *p, const double *u, const double *syn, const Constants *c) {
    double pstr = pstroke(u, c);
    return ( pstr/(*p) ) * ( *Yspike - *p ) * (*syn);
}


double rate_calc(const double *w, const Constants *c) {
    double norm_w = *w/c->ws;
    if( norm_w > 1) {
        return(0.04);
    } else {
        double k,b;
        if(norm_w > 0.5) {
            k = 0.06; b = -0.020;
        } else
        if(norm_w > 0.25) {
            k = 0.03; b = -0.005;
        } else {
            k = 0.01; b = 0;
        }
//        printf("x: %f y: %f\n", norm_w, norm_w*k+b);
        return(norm_w*k+b);
    }        
}

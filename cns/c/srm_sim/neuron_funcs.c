
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


double rate_calc(const double *w) {
    double w4 = *w * *w * *w * *w;
    return( 0.04 * w4/(w4+0.0016));
//    if(*w > 0.5) {
//        return(0.04);
//    } else
//    if(*w > 0.1) {
//        return(0.04);
//    } else
//    if(*w > 0.01) {
//        return(0.01);
//    } else
//    return(0);
}

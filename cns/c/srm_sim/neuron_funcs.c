
#include "neuron_funcs.h"

double probf( const double *u, const Constants *c) {
    if( *u  <= c->u_rest ) return(c->__pr);    
    return( (c->pr + (c->beta/c->alpha) * (log(1 + exp( c->alpha*(c->tr - *u))) - c->alpha * (c->tr - *u)))/c->sim_dim );
}


double pstroke(const double *u, const Constants *c) {
    return( (c->beta/(1+exp(c->alpha*(c->tr - *u))))/c->sim_dim );
}


double B_calc(const unsigned char *Yspike, const double *p, const double *pmean, const Constants *c) {
    double pmean_w = *pmean/c->mean_p_dur;
    return (( *Yspike * log( *p/pmean_w) - (*p - pmean_w)) - c->target_rate_factor * ( *Yspike * log( pmean_w/c->__target_rate) - (pmean_w - c->__target_rate) ));

}


double C_calc(const unsigned char *Yspike, const double *p, const double *u, const double *syn, const Constants *c) {
    double pstr = pstroke(u, c);
    return ( pstr/(*p) ) * ( *Yspike - *p ) * (*syn);

}

#pragma once


#define BW_CORRECTION      1.0190
#define VERY_SMALL_NUMBER  1e-200
#ifndef M_PI
#define M_PI               3.14159265358979323846
#endif

/*=======================
 * Utility functions
 *=======================
 */
#define myMax(x,y)     ( ( x ) > ( y ) ? ( x ) : ( y ) )
#define myMod(x,y)     ( ( x ) - ( y ) * floor ( ( x ) / ( y ) ) )
#define erb(x)         ( 24.7 * ( 4.37e-3 * ( x ) + 1.0 ) )


#include <snnlib/core.h>

class GammatoneFilter {
public:
    enum class Options { OnlyMembrane, Full };
    GammatoneFilter(GammatoneFilter::Options _o) : o(_o) { }

    void calc(const vector<double> &x,  int fs, double cf, int hrect) {
        int i, j, t, nsamples;
        double a, tpt, tptbw, gain;
        double p0r, p1r, p2r, p3r, p4r, p0i, p1i, p2i, p3i, p4i;
        double a1, a2, a3, a4, a5, u0r, u0i; /*, u1r, u1i;*/
        double qcos, qsin, oldcs, coscf, sincf, oldphase, dp, dps;

        nsamples = x.size();
        membrane.resize(nsamples);
        if(o == Options::Full) {
            hilbert_envelope.resize(nsamples);
            inst_freq.resize(nsamples);
            inst_phase.resize(nsamples);
        }

        oldphase = 0.0;
        tpt = ( M_PI + M_PI ) / fs;
        tptbw = tpt * erb ( cf ) * BW_CORRECTION;
        a = exp ( -tptbw );

        /* based on integral of impulse response */
        gain = ( tptbw*tptbw*tptbw*tptbw ) / 3;

        /* Update filter coefficients */
        a1 = 4.0*a; a2 = -6.0*a*a; a3 = 4.0*a*a*a; a4 = -a*a*a*a; a5 = a*a;
        p0r = 0.0; p1r = 0.0; p2r = 0.0; p3r = 0.0; p4r = 0.0;
        p0i = 0.0; p1i = 0.0; p2i = 0.0; p3i = 0.0; p4i = 0.0;

        /*===========================================================
        * exp(a+i*b) = exp(a)*(cos(b)+i*sin(b))
        * q = exp(-i*tpt*cf*t) = cos(tpt*cf*t) + i*(-sin(tpt*cf*t))
        * qcos = cos(tpt*cf*t)
        * qsin = -sin(tpt*cf*t)
        *===========================================================
        */
        coscf = cos ( tpt * cf );
        sincf = sin ( tpt * cf );
        qcos = 1; qsin = 0;   /* t=0 & q = exp(-i*tpt*t*cf)*/
        for ( t=0; t<nsamples; t++ )
        {
           /* Filter part 1 & shift down to d.c. */
           p0r = qcos*x[t] + a1*p1r + a2*p2r + a3*p3r + a4*p4r;
           p0i = qsin*x[t] + a1*p1i + a2*p2i + a3*p3i + a4*p4i;

           /* Clip coefficients to stop them from becoming too close to zero */
           if (fabs(p0r) < VERY_SMALL_NUMBER)
             p0r = 0.0F;
           if (fabs(p0i) < VERY_SMALL_NUMBER)
             p0i = 0.0F;

           /* Filter part 2 */
           u0r = p0r + a1*p1r + a5*p2r;
           u0i = p0i + a1*p1i + a5*p2i;

           /* Update filter results */
           p4r = p3r; p3r = p2r; p2r = p1r; p1r = p0r;
           p4i = p3i; p3i = p2i; p2i = p1i; p1i = p0i;

          /*==========================================
           * Basilar membrane response
           * 1/ shift up in frequency first: (u0r+i*u0i) * exp(i*tpt*cf*t) = (u0r+i*u0i) * (qcos + i*(-qsin))
           * 2/ take the real part only: membrane = real(exp(j*wcf*kT).*u) * gain;
           *==========================================
           */
           membrane[t] = ( u0r * qcos + u0i * qsin ) * gain;
           cout << membrane[t] << "\n";
           if ( 1 == hrect && membrane[t] < 0 ) {
              membrane[t] = 0;  /* half-wave rectifying */
           }
          /*==========================================
           * Instantaneous Hilbert envelope
           * env = abs(u) * gain;
           *==========================================
           */
           if (o == Options::Full) {
              hilbert_envelope[t] = sqrt ( u0r * u0r + u0i * u0i ) * gain;
           }
          /*==========================================
           * Instantaneous phase
           * instp = unwrap(angle(u));
           *==========================================
           */
           if (o == Options::Full) {
              inst_phase[t] = atan2 ( u0i, u0r );
              /* unwrap it */
              dp = inst_phase[t] - oldphase;
              if ( abs ( dp ) > M_PI ) {
                 dps = myMod ( dp + M_PI, 2 * M_PI) - M_PI;
                 if ( dps == -M_PI && dp > 0 ) {
                    dps = M_PI;
                 }
                 inst_phase[t] = inst_phase[t] + dps - dp;
              }
              oldphase = inst_phase[t];
           }
          /*==========================================
           * Instantaneous frequency
           * instf = cf + [diff(inst_phase) 0]./tpt;
           *==========================================
           */
           if ( o == Options::Full && t > 0 ) {
              inst_freq[t-1] = cf + ( inst_phase[t] - inst_phase[t-1] ) / tpt;
           }

          /*====================================================
           * The basic idea of saving computational load:
           * cos(a+b) = cos(a)*cos(b) - sin(a)*sin(b)
           * sin(a+b) = sin(a)*cos(b) + cos(a)*sin(b)
           * qcos = cos(tpt*cf*t) = cos(tpt*cf + tpt*cf*(t-1))
           * qsin = -sin(tpt*cf*t) = -sin(tpt*cf + tpt*cf*(t-1))
           *====================================================
           */
           qcos = coscf * ( oldcs = qcos ) + sincf * qsin;
           qsin = coscf * qsin - sincf * oldcs;
        }
        if ( o == Options::Full ) {
           inst_freq[nsamples-1] = cf;
        }
    }

    Options o;

    vector<double> membrane;
    vector<double> hilbert_envelope;
    vector<double> inst_phase;
    vector<double> inst_freq;
};

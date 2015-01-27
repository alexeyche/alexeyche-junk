#ifndef RGAMMATONEFB_H
#define RGAMMATONEFB_H

#define STRICT_R_HEADERS
#include <Rcpp.h>

#include "common.h"

#include <snnlib/util/gammatone_fb.h>


class RGammatoneFB : public GammatoneFilter {
public:
    RGammatoneFB(size_t verbose = 0) : GammatoneFilter(GammatoneFilter::Options::OnlyMembrane)  {
        if(verbose > 0) {
            o = GammatoneFilter::Options::Full;
        }
    }
    Rcpp::List calc(Rcpp::NumericVector x, Rcpp::NumericVector freqs, double sampling_rate, int hrect) {
        vector<vector<double>> out;
        vector<double> xv = Rcpp::as<vector<double>>(x);
        for(auto it = freqs.begin(); it != freqs.end(); ++it) {
            vector<double> out_f;
            GammatoneFilter::calc(xv, sampling_rate, *it, hrect);
            out.push_back(out_f);
        }
        Rcpp::List list_out;
        list_out["membrane"] = Rcpp::wrap(out);
        if(o == GammatoneFilter::Options::Full) {
            list_out["hilbert_envelope"] = Rcpp::wrap(hilbert_envelope);
            list_out["inst_phase"] = Rcpp::wrap(inst_phase);
            list_out["inst_freq"] = Rcpp::wrap(inst_freq);
        }
        return list_out;
    }

    
};

#endif

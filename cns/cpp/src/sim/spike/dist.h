#ifndef DIST_H
#define DIST_H


#include <sim/int/gauss_legendre.h>
#include <sim/core.h>

namespace srm {
    
    struct kern_data {
        kern_data(const vec &st1c, const vec &st2c, double sigmac) : st1(st1c), st2(st2c), sigma(sigmac)  {}
        const vec & st1;
        const vec & st2;
        double sigma;
    };

    double Kexp(double t, double sigma) {
        return exp(-abs(t)/sigma);
    }
    
    double exp_dist(double t, void *s) {
        kern_data *kd = (kern_data*)s;
        double sum1 = 0, sum2 = 0;
        for(size_t ei=0; ei<std::max(kd->st1.n_elem, kd->st2.n_elem); ei++) {
            if(ei<kd->st1.n_elem) {
                if(kd->st1(ei)>=0) {
                    sum1 += Kexp(t - kd->st1(ei), kd->sigma);
                }
            }
            if(ei<kd->st2.n_elem) {
                if(kd->st2(ei)>=0) {
                    sum2 += Kexp(t - kd->st2(ei), kd->sigma);
                }
            }
        }
        return sum1*sum2;
    }
    double dist(mat &raster1, mat &raster2, double sigma = 20) {
        assert(raster1.n_rows == raster2.n_rows);        
        double T0 = std::min( raster1.min(), raster2.min());
        double Tmax = std::max( raster1.max(), raster2.max());
        vec dist(raster1.n_rows, fill::zeros);
        Log::Info << "Distance (sigma = " << sigma << ") = \n";
        for(size_t ri=0; ri < raster1.n_rows; ri++) {
            kern_data kd(raster1.row(ri), raster2.row(ri), sigma);
            dist(ri) = gauss_legendre(256, exp_dist, (void*)&kd, T0, Tmax);
            Log::Info << "  | ni(" << ri << ") = " << dist(ri) << "\n";
        }
        Log::Info << "sum: " << sum(dist) << "\n";
        return sum(dist);            
    }

};


#endif

#pragma once

#include <dnn/connections/connection.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DifferenceOfGaussiansC : public Serializable<Protos::DifferenceOfGaussiansC> {
    DifferenceOfGaussiansC() : sigma(5.0), k(2) {}

    void serial_process() {
        begin() << "sigma: " << sigma << ", "
        		<< "k: " 	 << k 	  << Self::end;
    }

    double sigma;
    double k;
};


class DifferenceOfGaussians : public Connection<DifferenceOfGaussiansC> {
public:
    const string name() const {
        return "DifferenceOfGaussians";
    }
    static double gaussFunction(double x, double y, double xc, double yc, double sigma) {
    	return exp( - ( (x-xc)*(x-xc) + (y-yc)*(y-yc) )/(2*sigma*sigma) ) ;
    }
    ConnectionRecipe getConnectionRecipe(const SpikeNeuronBase &left, const SpikeNeuronBase &right) {
    	ConnectionRecipe recipe;
    	double v = 2*gaussFunction(right.xi(), right.yi(), left.xi(), left.yi(), c.sigma) - \
    				 gaussFunction(right.xi(), right.yi(), left.xi(), left.yi(), c.k*c.sigma);

    	recipe.exists = true;
    	if(v<0) {
    		recipe.inhibitory = true;
    	}
    	recipe.amplitude = fabs(v);
    	return recipe;
    }
};





};
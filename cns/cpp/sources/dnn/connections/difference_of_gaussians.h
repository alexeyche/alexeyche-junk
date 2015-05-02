#pragma once

#include <dnn/connections/connection.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DifferenceOfGaussiansC : public Serializable<Protos::DifferenceOfGaussiansC> {
    DifferenceOfGaussiansC() : a(3.0), b(3.0), r(1) {}

    void serial_process() {
        begin() << "a: " << a << ", "
                << "b: " << b << ", "
        		<< "r: " << r << Self::end;
    }

    double a;
    double b;
    double r;
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
    	double v = (1+c.a) * gaussFunction(right.xi(), right.yi(), left.xi(), left.yi(), c.r) - \
    				  c.a * gaussFunction(right.xi(), right.yi(), left.xi(), left.yi(), c.b*c.r);

    	recipe.exists = true;
    	if(v<0) {
    		recipe.inhibitory = true;
    	}
    	recipe.amplitude = fabs(v);
    	return recipe;
    }
};





};
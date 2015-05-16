#pragma once

#include <dnn/connections/connection.h>

namespace dnn {

/*@GENERATE_PROTO@*/
struct DifferenceOfGaussiansC : public Serializable<Protos::DifferenceOfGaussiansC> {
    DifferenceOfGaussiansC() : a(3.0), b(3.0), r(1), dimension(2) {}

    void serial_process() {
        begin() << "a: " << a << ", "
                << "b: " << b << ", "
        		<< "r: " << r << ", "
                << "dimension: " << dimension <<  Self::end;
    }
    int dimension;
    double a;
    double b;
    double r;
};


class DifferenceOfGaussians : public Connection<DifferenceOfGaussiansC> {
public:
    const string name() const {
        return "DifferenceOfGaussians";
    }
    static double gaussFunction2d(double x, double y, double xc, double yc, double sigma) {
    	return exp( - ( (x-xc)*(x-xc) + (y-yc)*(y-yc) )/(2*sigma*sigma) ) ;
    }
    static double gaussFunction(double x, double xc, double sigma) {
        return exp( - ( (x-xc)*(x-xc) )/(2*sigma*sigma) ) ;
    }
    ConnectionRecipe getConnectionRecipe(const SpikeNeuronBase &left, const SpikeNeuronBase &right) {
    	ConnectionRecipe recipe;
    	double v;
        if(c.dimension == 1) {
            v = (1+c.a) * gaussFunction(right.localId(), left.localId(), c.r) - \
    				  c.a * gaussFunction(right.localId(), left.localId(), c.b*c.r);
        } else
        if(c.dimension == 2) {
            v = (1+c.a) * gaussFunction2d(right.xi(), right.yi(), left.xi(), left.yi(), c.r) - \
                      c.a * gaussFunction2d(right.xi(), right.yi(), left.xi(), left.yi(), c.b*c.r);
        } else {
            throw dnnException() << "Can't build DifferenceOfGaussians with dimension like this: " << c.dimension << "\n" 
                                 << "Only 1 or 2 dimension supported\n";
        }        
    	recipe.exists = true;
    	if(v<0) {
    		recipe.inhibitory = true;
    	}
    	//recipe.amplitude = fabs(v);
    	return recipe;
    }
};





};
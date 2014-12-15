#pragma once

#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

typedef double (*lt_mod)(const double &);

class WeightNormalization : public Serializable<Protos::BlankModel> {
public:
	static double defaultLtMod(const double &w) {
		return 1.0;
	}

    WeightNormalization() : Serializable(EBlankModel) {}
	virtual void init(const ConstObj *_c, Neuron *_n) = 0;
	virtual void preModifyMeasure() = 0;
    virtual void modifyWeightDerivative(double &dw, const size_t &syn_id) = 0;
	virtual lt_mod getLtpMod() { return defaultLtMod; }    
	virtual lt_mod getLtdMod() { return defaultLtMod; }    
protected:
    Neuron *n;
};


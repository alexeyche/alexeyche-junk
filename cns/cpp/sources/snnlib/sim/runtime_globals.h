#pragma once



class RuntimeGlobals {
public:
	RuntimeGlobals() {}
	void setDt(double _dt) {
		dt = _dt;
	}
	void setC(Constants &_c) {
		c = &_c;
	}
	
	inline const double& Dt() const {
		return dt;
	}
    inline const Constants& C() const {
        return *c;
    }
    inline Constants& mut_C() const {
        return *c;
    }
private:
	double dt;
    Constants *c;
};

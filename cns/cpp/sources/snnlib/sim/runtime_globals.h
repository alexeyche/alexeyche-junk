#pragma once



class RuntimeGlobals {
public:
	RuntimeGlobals(const Constants &_c) : c(_c) {}
	void setDt(double _dt) {
		dt = _dt;
	}
	inline const double& Dt() const {
		return dt;
	}
    inline const Constants& C() const {
        return c;
    }
private:
	double dt;
    const Constants &c;
};

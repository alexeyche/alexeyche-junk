#pragma once



class RuntimeGlobals {
public:
	RuntimeGlobals() {}
	void setDt(double _dt) {
		dt = _dt;
	}
	inline const double& Dt() const {
		return dt;
	}
private:	
	double dt;
};
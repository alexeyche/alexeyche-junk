#pragma once


class Sim {
public:
	Sim() {

	}
	void run() {
		for(double t=0; t < 1000; t+= 1.0) {
			System::state_type dsdt
			s.step()
		}
	}
private:
	vector< unique_ptr<DynamicObject> > system;
};
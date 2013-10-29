
#include <sim/core.h>
#include "DEIntegrator.h"

double DemoFunction1 (double x) 
	{
		return exp(-x/5.0)*(2.0 + sin(2.0*x));
	}

double DemoFunction2(double x)
	{
		return pow(1.0 - x, 5.0)*pow(x, -1.0/3.0);
	}

int main()
{
	int evaluations;
	double errorEstimate;
	std::cout << std::setprecision(15);
	double integral = DEIntegrator<double>::Integrate(&DemoFunction1, 0, 10, 1e-6, evaluations, errorEstimate);
	std::cout << integral << ", " << errorEstimate << ", " << evaluations << "\n";

	integral = DEIntegrator<double>::Integrate(&DemoFunction2, 0, 1, 1e-6, evaluations, errorEstimate);
	std::cout << integral << ", " << errorEstimate << ", " << evaluations << "\n";

	return 0;
}

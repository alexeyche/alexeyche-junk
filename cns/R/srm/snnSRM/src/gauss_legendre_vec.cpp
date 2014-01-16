#include "gauss_legendre.h"
#include <RcppArmadillo.h>

arma::vec gauss_legendre_vec(int n, arma::vec (*f)(arma::vec,void*), int numDim, void* data, double a, double b) {
    double* x = NULL;
	double* w = NULL;
	arma::vec A(numDim), B(numDim), Ax(numDim), s(numDim);
	int i, dtbl, m;

	m = (n+1)>>1;

	/* Load appropriate predefined table */
	dtbl = 0;
	for (i = 0; i<GLAWSIZE;i++)
	{
		if(n==glaw[i].n)
		{
			x = glaw[i].x;
			w = glaw[i].w;
			break;
		}
	}
	
	/* Generate new if non-predefined table is required */
	/* with precision of 1e-10 */
	if(NULL==x)
	{
		dtbl = 1;

		x = (double*)malloc(m*sizeof(double));
		w = (double*)malloc(m*sizeof(double));

		gauss_legendre_tbl(n,x,w,1e-10);
	}
	
    A.fill( 0.5*(b-a) );
	B.fill( 0.5*(b+a) );

	if(n&1) /* n - odd */
	{
		s = w[0]*((*f)(B,data));
		for (i=1;i<m;i++)
		{
			Ax = A*x[i];
			s += w[i]*((*f)(B+Ax,data)+(*f)(B-Ax,data));
		}

	}else{ /* n - even */
		
		s = 0.0;
		for (i=0;i<m;i++)
		{
			Ax = A*x[i];
			s += w[i]*((*f)(B+Ax,data)+(*f)(B-Ax,data));			
		}
	}

	if (dtbl)
	{
		free(x);
		free(w);
	}
	return A*s;

}


#include "backprop.h"
// NeuralNet.cpp : Defines the entry point for the console application.
//
#include <cstdio>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{


	// prepare XOR traing data
	double data[][4]={
				{0,	0,	0,	0},
			    {0,	0,	1,	1},
				{0,	1,	0,	1},
				{0,	1,	1,	0},
				{1,	0,	0,	1},
				{1,	0,	1,	0},
				{1,	1,	0,	0},
				{1,	1,	1,	1}};

	// prepare test data	
	double testData[][3]={
                            {0,      0,      0},
                            {0,      0,      1},
                            {0,      1,      0},
                            {0,      1,      1},
                            {1,      0,      0},
                            {1,      0,      1},
                            {1,      1,      0},
                            {1,      1,      1}};

	
	// defining a net with 4 layers having 3,3,3, and 1 neuron respectively,
	// the first layer is input layer i.e. simply holder for the input parameters
	// and has to be the same size as the no of input parameters, in out example 3
	int numLayers = 4, lSz[4] = {3,3,3,1};

	
	// Learing rate - beta
	// momentum - alpha
	// Threshhold - thresh (value of target mse, training stops once it is achieved)
	double beta = 0.3, alpha = 0.1, MinErr =  0.00001;

	
	// maximum no of iterations during training
	long num_iter = 2000000;

	
	// Creating the net
	CBackProp *bp = new CBackProp(numLayers, lSz, beta, alpha);
	
	cout << endl <<  "Now training the network...." << endl;	
	double MSE;
    int ep;
    int ntr = 8;
    long nep = num_iter;
    for (ep=0;ep<nep;ep++)
    {
        MSE=0.0;
        for(int i=0;i<ntr;i++)
        {
            bp->bpgt(data[i], &data[i][lSz[0]]);
            MSE+=bp->mse(&data[i][lSz[0]]);
        }
        MSE/=ntr;
        if(MSE < MinErr) 
        {
            cout << "Network trained in " << ep << " epochs. MSE: " << MSE << endl;
            break;
        }
    }
//    long i;
//    for ( i=0; i<num_iter ; i++)
//	{
//		
//		bp->bpgt(data[i%8], &data[i%8][3]);
//
//		if( bp->mse(&data[i%8][3]) < Thresh) {
//			cout << endl << "Network Trained. Threshold value achieved in " << i << " iterations." << endl;
//			cout << "MSE:  " << bp->mse(&data[i%8][3]) 
//				 <<  endl <<  endl;
//			break;
//		}
//		if ( i%(num_iter/10) == 0 )
//			cout <<  endl <<  "MSE:  " << bp->mse(&data[i%8][3]) 
//				<< "... Training..." << endl;
//
//	}
	
//	if ( ep == num_iter )
//		cout << endl << i << " iterations completed..." 
//		<< "MSE: " << bp->mse(&data[(i-1)%8][3]) << endl;  	

	cout << "Now using the trained network to make predctions on test data...." << endl << endl;	
	for (int i = 0 ; i < 8 ; i++ )
	{
		bp->ffwd(testData[i]);
		cout << testData[i][0]<< "  " << testData[i][1]<< "  "  << testData[i][2]<< "  " << bp->Out(0) << endl;
	}

	return 0;
}




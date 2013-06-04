// SPNET: Spiking neural network with axonal conduction delays and STDP
// Created by Eugene M. Izhikevich, May 17, 2004, San Diego, CA
// Saves spiking data each second in file spikes.dat
// To plot spikes, use MATLAB code: load spikes.dat;plot(spikes(:,1),spikes(:,2),'.');
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 

using namespace std;

#define getrandom(max1) ((rand()%(int)((max1)))) // random integer between 0 and max-1
												
const	int		Ne = 800;		// excitatory neurons			
const	int		Ni = 200;		// inhibitory neurons				 
const	int		N  = Ne+Ni;		// total number of neurons	
const	int		M  = 100;		// the number of synapses per neuron 
const	int		D  = 20;		// maximal axonal conduction delay
		float	sm = 10.0;		// maximal synaptic strength		
int		post[N][M];				// indeces of postsynaptic neurons
float	s[N][M], sd[N][M];		// matrix of synaptic weights and their derivatives
short	delays_length[N][D];	// distribution of delays
short	delays[N][D][M];		// arrangement of delays   
int		N_pre[N], I_pre[N][3*M], D_pre[N][3*M];	// presynaptic information
float	*s_pre[N][3*M], *sd_pre[N][3*M];		// presynaptic weights
float	LTP[N][1001+D], LTD[N];	// STDP functions 
float	a[N], d[N];				// neuronal dynamics parameters
float	v[N], u[N];				// activity variables
int		N_firings;				// the number of fired neurons 
const int N_firings_max=100*N;	// upper limit on the number of fired neurons per sec
int		firings[N_firings_max][2]; // indeces and timings of spikes

void initialize()
{	int i,j,k,jj,dd, exists, r;
	for (i=0;i<Ne;i++) a[i]=0.02;// RS type
	for (i=Ne;i<N;i++) a[i]=0.1; // FS type

	for (i=0;i<Ne;i++) d[i]=8.0; // RS type
	for (i=Ne;i<N;i++) d[i]=2.0; // FS type

	for (i=0;i<N;i++) for (j=0;j<M;j++) 
	{
		do{
			exists = 0;		// avoid multiple synapses
			if (i<Ne) r = getrandom(N);
			else	  r = getrandom(Ne);// inh -> exc only
			if (r==i) exists=1;									// no self-synapses 
			for (k=0;k<j;k++) if (post[i][k]==r) exists = 1;	// synapse already exists  
		}while (exists == 1);
		post[i][j]=r;
	}
	for (i=0;i<Ne;i++)	for (j=0;j<M;j++) s[i][j]=6.0;  // initial exc. synaptic weights
	for (i=Ne;i<N;i++)	for (j=0;j<M;j++) s[i][j]=-5.0; // inhibitory synaptic weights
  	for (i=0;i<N;i++)	for (j=0;j<M;j++) sd[i][j]=0.0; // synaptic derivatives 
  	for (i=0;i<N;i++)
	{
		short ind=0;
		if (i<Ne)
		{
			for (j=0;j<D;j++) 
			{	delays_length[i][j]=M/D;	// uniform distribution of exc. synaptic delays
				for (k=0;k<delays_length[i][j];k++)
					delays[i][j][k]=ind++;
			}
		}
		else
		{
			for (j=0;j<D;j++) delays_length[i][j]=0;
			delays_length[i][0]=M;			// all inhibitory delays are 1 ms
			for (k=0;k<delays_length[i][0];k++)
					delays[i][0][k]=ind++;
		}
	}
	
  	for (i=0;i<N;i++)
	{
		N_pre[i]=0;
		for (j=0;j<Ne;j++)
		for (k=0;k<M;k++)
		if (post[j][k] == i)		// find all presynaptic neurons 
		{
			I_pre[i][N_pre[i]]=j;	// add this neuron to the list
			for (dd=0;dd<D;dd++)	// find the delay
				for (jj=0;jj<delays_length[j][dd];jj++)
					if (post[j][delays[j][dd][jj]]==i) D_pre[i][N_pre[i]]=dd;
			s_pre[i][N_pre[i]]=&s[j][k];	// pointer to the synaptic weight	
			sd_pre[i][N_pre[i]++]=&sd[j][k];// pointer to the derivative
		}
	}

	for (i=0;i<N;i++)	for (j=0;j<1+D;j++) LTP[i][j]=0.0;
	for (i=0;i<N;i++)	LTD[i]=0.0;
	for (i=0;i<N;i++)	v[i]=-65.0;		// initial values for v
	for (i=0;i<N;i++)	u[i]=0.2*v[i];	// initial values for u

	N_firings=1;		// spike timings
	firings[0][0]=-D;	// put a dummy spike at -D for simulation efficiency 
	firings[0][1]=0;	// index of the dummy spike
}

int main()
{
	int		i, j, k, sec, t;
	float	I[N];
	FILE	*fs;

	initialize();	// assign connections, weights, etc. 

	for (sec=0; sec<60*60*24; sec++)		// simulation of 1 day
	{
		for (t=0;t<1000;t++)				// simulation of 1 sec
		{
			for (i=0;i<N;i++) I[i] = 0.0;	// reset the input 
			for (k=0;k<N/1000;k++)
				I[getrandom(N)]=20.0;		// random thalamic input 
			for (i=0;i<N;i++) 
			if (v[i]>=30)					// did it fire?
			{
				v[i] = -65.0;					// voltage reset
				u[i]+=d[i];					// recovery variable reset
				LTP[i][t+D]= 0.1;		
				LTD[i]=0.12;
				for (j=0;j<N_pre[i];j++) *sd_pre[i][j]+=LTP[I_pre[i][j]][t+D-D_pre[i][j]-1];// this spike was after pre-synaptic spikes
				firings[N_firings  ][0]=t;
				firings[N_firings++][1]=i;
				if (N_firings == N_firings_max) {cout << "Two many spikes at t=" << t << " (ignoring all)";N_firings=1;}
			}
			k=N_firings;
			while (t-firings[--k][0] <D)
			{
				for (j=0; j< delays_length[firings[k][1]][t-firings[k][0]]; j++)
				{
					i=post[firings[k][1]][delays[firings[k][1]][t-firings[k][0]][j]]; 
					I[i]+=s[firings[k][1]][delays[firings[k][1]][t-firings[k][0]][j]];
					if (firings[k][1] <Ne) // this spike is before postsynaptic spikes
						sd[firings[k][1]][delays[firings[k][1]][t-firings[k][0]][j]]-=LTD[i];
				}
			}
			for (i=0;i<N;i++)
			{
				v[i]+=0.5*((0.04*v[i]+5)*v[i]+140-u[i]+I[i]); // for numerical stability
				v[i]+=0.5*((0.04*v[i]+5)*v[i]+140-u[i]+I[i]); // time step is 0.5 ms
				u[i]+=a[i]*(0.2*v[i]-u[i]);
				LTP[i][t+D+1]=0.95*LTP[i][t+D];
				LTD[i]*=0.95;
			}
		}
		cout << "sec=" << sec << ", firing rate=" << float(N_firings)/N << "\n";
   		fs = fopen("spikes.dat","w");
		for (i=1;i<N_firings;i++)
			if (firings[i][0] >=0)
				fprintf(fs, "%d  %d\n", firings[i][0], firings[i][1]);
		fclose(fs);

		for (i=0;i<N;i++)		// prepare for the next sec
			for (j=0;j<D+1;j++)
			LTP[i][j]=LTP[i][1000+j];
		k=N_firings-1;
		while (1000-firings[k][0]<D) k--;
		for (i=1;i<N_firings-k;i++)
		{
			firings[i][0]=firings[k+i][0]-1000;
			firings[i][1]=firings[k+i][1];
		}
		N_firings = N_firings-k;

		for (i=0;i<Ne;i++)	// modify only exc connections
		for (j=0;j<M;j++)
		{
			s[i][j]+=0.01+sd[i][j];
			sd[i][j]*=0.9;			
			if (s[i][j]>sm) s[i][j]=sm;
			if (s[i][j]<0) s[i][j]=0.0;
		}
	}
}

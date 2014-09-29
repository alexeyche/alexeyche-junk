/******************************************************************************
*******************************************************************************
******                                                                  *******
******     This code is written by Abdullah Al Mueen at the department  *******
******     of Computer Science and Engineering of University of         *******
******     California - RIverside.                                      *******
******                                                                  *******
*******************************************************************************
******************************************************************************/

/*#############################################################################
######                                                                  #######
######     This code is open to use, reuse and distribute at the user's #######
######     own risk and as long as the above credit is ON. The complete #######
######     description of the algorithm and methods applied can be      #######
######     found in the paper                                           #######
######                                                                  #######
#############################################################################*/


#include <iostream>
#include <fstream>
#include <sstream>


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
using namespace std;

#define MAXREF 5
#define INF 9999999999999.99
#define DEBUG 1
#define RESULT 1
#define AND 1
#define OR -1


#include "timeSeries.h"
#include "xyMatrix.h"
#include "logicOrderLine.h"

int start = 10 , end;	
int stepSize = 10;
int samplingRate = 1;
int maxLen,minLen;
char inFname[100];


void error(int id)
{
    if(id==1)
        printf("ERROR : Memory can't be allocated!!!\n\n");
    else if ( id == 2 )
        printf("ERROR : File not Found!!!\n\n");
    else if ( id == 3 )
        printf("ERROR : Can't create Output File!!!\n\n");
    else if ( id == 4 )
        printf("ERROR : Invalid Number of Arguments!!!\n\n");
    system("PAUSE");
    exit(1);

    }
  
struct logicalShapelet
{

	int * shpIDs;
	int * shpPos;
	int * shpLen;
	double * dist;
	double * gap;
	double * infoGain;
	int  * d;
	int type;

};


orderLine * Logic_Shapelet(timeSeries ** T, int N , int C , struct logicalShapelet * history , orderLine * pLine , int opt)
{
    int i , j , k , len;	//search iterators
    double t1,t2;			//timers
	long long count = 0, pruneCount = 0;   //performance counters

	double t3, boundTime = 0;
	
	if(opt == -1)
		opt=-1;
	int refCount = 0;
    t1 = clock();
    
    int * clsHist = (int *) malloc( sizeof(int) * C );
	for( i = 0 ; i < C ; i++ )
		clsHist[i] = 0;
	for( i = 0 ; i < N ; i++ )
		clsHist[T[i]->c]++;

    xyMatrix ** XY = (xyMatrix **)malloc(sizeof(xyMatrix *)*N);
  
    double bsf = 0;

	orderLine * line = new orderLine(N,C,clsHist);
    orderLine * bestSplit = new orderLine();
    orderLine ** references = new orderLine * [MAXREF];

	 for( j = 0 ; j < MAXREF ; j++ )
		references[j] = new orderLine();

	 for( j = 0 ; j < N ; j++ )
		  XY[j] = new xyMatrix(maxLen,maxLen);


	//For all time series 
	 for( k = 0 ; k < N;/*pLine->splitPos ; */k++ )
	{

		 if( k == 8 )
			 k = 8;

		 //Generate the matrices self matrix is also included when j == k
		 for( j = 0 ; j < N ; j++ )
			 XY[j]->computeXY(T[k]->x,T[k]->length,T[j]->x,T[j]->length);

		 //For all possible lengths  
		 for(len = start ; len <= end ; len += stepSize )
		 {
			     if( len == 30 )
					 len = 30;
				  refCount = 0;

				 //For all possible positions
				 for( i = 0; i < T[k]->length - len + 1 ; i++ )
				 {
					 if( i == 16 )
						 i = 16;


					   count++;

					   t3 = clock();
					   //printf("Doing %dth time series at position %d with length %d\n",k,i,len);
					   //We have a candidate. So create an order line. then sort it. then try different split point and record it.

					   if( !T[k]->checkPos(i,len) )
										   continue;
					   double mux , muy , sgx , sgy , smxy;
					   mux = T[k]->mean(i,len);
					   sgx = T[k]->stdv(i,len);
						if(sgx == 0 )
							continue;
					/*   int yy = 0;
					   while( yy < refCount )
					   {
							   int r = references[yy]->shapeletPos;
				   			   muy = T[k]->mean(r,len);
							   sgy = T[k]->stdv(r,len);
							   smxy = XY[k]->sumXY(i,r,len);

							   if( sgy == 0 )
								   error(1);

								double t = smxy - len*mux*muy;
								t = t / (len*sgx*sgy);
								t = 2*(1-t);
								if( t <= 0 )
									t = 0;
								else
									t = sqrt(t);

								double tt = references[yy]->shiftEntropy(t, pLine , opt);
								if( references[yy]->informationGain < bestSplit->informationGain )
											break;
								else if( references[yy]->informationGain == bestSplit->informationGain  && bestSplit->gap > references[yy]->gap )  ///breaking ties
										    break;
								/*
								if( tt < bestSplit->informationGain )
								{
									//printf("tt %lf, best info gain : %lf\n",tt, bestSplit->informationGain );
									break;
								}
								yy++;
					   }

					   t2 = clock();
					   boundTime += (t2-t3)/CLOCKS_PER_SEC;
					   if( yy < refCount )
					   {
						   pruneCount++;
						   continue;
					   }
*/

					   line->reset(k,i,len,sgx);
                       
					   for( j = 0 ; j < N ; j++ )
					   {
						   if( j == 8 )
							   j = 8;
						   struct projection p;
						   p.cls = T[j]->c;
						   p.length = len;
						   p.tID = j;

                           
						   double bsf = INF , t;
						   int bestU = -1;
						   for( int u = 0 ; u < T[j]->length-len+1 ; u++ )
						   {
							  if( !T[j]->checkPos(u,len) )
								   continue;
							   muy = T[j]->mean(u,len);
							   sgy = T[j]->stdv(u,len);

							   if(sgy == 0 )
								   sgy = 1;

							   smxy = XY[j]->sumXY(i,u,len);
								t = smxy - len*mux*muy;
								t = t / (len*sgx*sgy);
								t = 2*(1-t);
								if( t <= 0 )
									t = 0;
								else
									t = sqrt(t);

								if ( t < bsf )
								{
								   bsf = t;
								   bestU = u;
								}
						   } 
						   if( bsf == INF )
						        break;
						   p.pos = bestU;
						   p.distance = bsf;
						   p.depth = *history->d;
						   line->insert(j , p);
						   //double bE = line->bestGain();
						   //if ( bE < bestSplit->informationGain )
							 //  break;
                           
					   }

					   if ( j < N )
						   continue;
					   line->mergeTwoLines(pLine,opt);
					   line->findBestSplit();
					   //printf("Best Gain %lf\n", bestSplit->informationGain );
					   //if( refCount < MAXREF - 1)
					   //refCount = 0;

						if( line->informationGain > bestSplit->informationGain )
								bestSplit->copy(line);
						else if( line->informationGain == bestSplit->informationGain  && bestSplit->gap < line->gap )  ///breaking ties
						//else if( line->informationGain == bestSplit->informationGain  && bestSplit->shapeletSTD < line->shapeletSTD )  ///breaking ties
							    bestSplit->copy(line);

							references[refCount%MAXREF]->copy(line);
							refCount = (refCount+1)%MAXREF;
					   //else
					  // {
						//   int qq = rand()%MAXREF;
					//	   references[qq]->copy(line);
					  // }
				  }
		   }
		   t2 = clock();
#if !RESULT
		   printf("\nTime for %d th time series : %lf seconds\n",k,(t2-t3)/CLOCKS_PER_SEC);
		   printf("\nBest information gain : %lf \n",bestSplit->informationGain);
#endif



	  }               
    
	t2 = clock();
  
#if !RESULT
	printf("Shpelet ID : %d , Start Position : %d , Shapelet Length : %d\n\n", bestSplit->shapeletID , bestSplit->shapeletPos , bestSplit->shapeletLength );
	printf("Split informationGain : %lf , split position %d , split distance %lf and the separation gap %lf\n\n",bestSplit->informationGain,bestSplit->splitPos,bestSplit->splitDist,bestSplit->gap);
	printf("Total candidates : %lld , ",count);
	printf("Number of Pruned candidates : %lld\n\n", pruneCount );
    printf("Time for current shapelet : %lf seconds and Bound Computation Time %lf\n",(t2-t1)/CLOCKS_PER_SEC,boundTime);
#endif



	if( bestSplit->splitPos == -1 )
		return bestSplit;
	int sP = bestSplit->splitPos+1;

	bestSplit->findEntropies();


	
	cout << "In "<< history->type  << " Divided classes : " << endl;
	
	int * divd = (int *)malloc(sizeof(int)*N);
	int * runLengths = (int *)malloc(sizeof(int)*N);
	
	int runId = 0;
	int runCount  = 1;

	for( i = 0 ; i < N ; i++)
		divd[i] = -1,runLengths[i] = 0;
	divd[runId] = T[bestSplit->line[0].tID]->c;
	for( i = 1 ; i < sP  ; i++ )
		if ( T[bestSplit->line[i].tID]->c == T[bestSplit->line[i-1].tID]->c )
			runCount++;
		else
		{
			runLengths[runId] = runCount;
			divd[runId++] = T[bestSplit->line[i].tID]->c;
			runCount = 1;
		}
	runLengths[runId] = runCount;
	cout << "The left one " << endl;
	for( i = 0 ; i <= runId ; i++ )
		{
			cout << runLengths[i] << "(" << divd[i] << ") ";
		}
	cout << endl;
	runId = 0,runCount  = 1;
	for( i = 0 ; i < N ; i++)
		divd[i] = -1,runLengths[i] = 0;
	divd[runId++] = T[bestSplit->line[sP].tID]->c;
	for( i = sP+1; i < N  ; i++ )
		if ( T[bestSplit->line[i].tID]->c == T[bestSplit->line[i-1].tID]->c )
			runCount++;
		else
		{
			runLengths[runId] = runCount;
			divd[runId++] = T[bestSplit->line[i].tID]->c;
			runCount = 1;
		}

	cout << "The right one " <<endl;
	runLengths[runId] = runCount;
	for( i = 0 ; i <= runId ; i++ )
		{
			cout << runLengths[i] << "(" << divd[i] << ") ";
		}

	cout << endl << "gap : " << bestSplit->gap << "  dist : " << bestSplit->splitDist << " InfoGain : " << bestSplit->informationGain << endl << endl;

	for( i = 0; i < N  ; i++ )
		T[bestSplit->line[i].tID]->insertPos(bestSplit->line[i].pos,bestSplit->line[i].length);

	
	if( pLine->informationGain < bestSplit->informationGain || (history->type ==  AND && pLine->informationGain == bestSplit->informationGain  && pLine->gap < bestSplit->gap ) )
	//if( pLine->informationGain < bestSplit->informationGain || (history->type ==  AND && pLine->informationGain == bestSplit->informationGain  && pLine->shapeletSTD < bestSplit->shapeletSTD ) )
	{
			int ao = *history->d;
			int sZ = ++(*history->d);
			

			history->shpIDs = (int *)realloc ( history->shpIDs , sizeof(int)*sZ );
			history->shpPos = (int *)realloc (  history->shpPos , sizeof(int)*sZ );
			history->shpLen = (int *)realloc (  history->shpLen , sizeof(int)*sZ );
			history->dist = (double *)realloc (  history->dist , sizeof(double)*sZ );
			history->gap = (double *)realloc (  history->gap , sizeof(double)*sZ );
			history->infoGain = (double *)realloc (  history->infoGain , sizeof(double)*sZ );


			history->shpIDs[ao] = bestSplit->shapeletID;
			history->shpPos[ao] = bestSplit->shapeletPos;
			history->shpLen[ao] = bestSplit->shapeletLength;
			history->dist[ao] = bestSplit->splitDist;
			history->gap[ao] = bestSplit->gap;
			history->infoGain[ao] = bestSplit->informationGain;

			

	 for( j = 0 ; j < N ; j++ )
		delete XY[j];

  free(XY);

			if( sZ < 4 )
			{
					
				orderLine * tLine = Logic_Shapelet(T,N,C,history,bestSplit, opt);
				if( tLine->informationGain > bestSplit->informationGain )
				{
					delete bestSplit;
					bestSplit = tLine;
				}
				else
					delete tLine;
			}
	}	





	 for( j = 0 ; j < MAXREF ; j++ )
		delete references[j];

	// for( j = 0 ; j < N ; j++ )
	//	delete XY[j];

	 delete line;
	 free(clsHist);
	// free(XY);
 	return bestSplit;
}

   

int recursiveShapelet(timeSeries ** T, int N , int C , int nodeId)
{
    int i , j , k , len;	//search iterators
    double t1,t2;			//timers
	long long count = 0, pruneCount = 0;   //performance counters

	double t3, boundTime = 0;
	
	int refCount = 0;
    t1 = clock();
    


    int * clsHist = (int *) malloc( sizeof(int) * C );
	for( i = 0 ; i < C ; i++ )
		clsHist[i] = 0;
	for( i = 0 ; i < N ; i++ )
		clsHist[T[i]->c]++;

    xyMatrix ** XY = (xyMatrix **)malloc(sizeof(xyMatrix *)*N);
  
    double bsf = 0;

	orderLine * line = new orderLine(N,C,clsHist);
    orderLine * bestSplit = new orderLine();
    orderLine ** references = new orderLine * [MAXREF];

	 for( j = 0 ; j < MAXREF ; j++ )
		references[j] = new orderLine();

	 for( j = 0 ; j < N ; j++ )
		  XY[j] = new xyMatrix(maxLen,maxLen);


	//For all time series 
	for( k = 0 ; k < N ; k++ )
	{
		 if( k == 34 )
			 k = 34;

		 //Generate the matrices self matrix is also included when j == k
		 for( j = 0 ; j < N ; j++ )

			 XY[j]->computeXY(T[k]->x,T[k]->length,T[j]->x,T[j]->length);



		 //For all possible lengths    
		 for(len = start ; len <= end ; len += stepSize )
		 {
				  refCount = 0;

				 //For all possible positions
				 for( i = 0; i < T[k]->length - len + 1 ; i++ )
				 {

					   count++;

					   t3 = clock();
					   //printf("Doing %dth time series at position %d with length %d\n",k,i,len);
					   //We have a candidate. So create an order line. then sort it. then try different split point and record it.


					   if( !T[k]->checkPos(i,len) )
										   continue;
					   double mux , muy , sgx , sgy , smxy;
					   mux = T[k]->mean(i,len);
					   sgx = T[k]->stdv(i,len);
					   if( sgx == 0 )
						  continue;
/*
					   int yy = 0;
					   while( yy < refCount )
					   {
							   int r = references[yy]->shapeletPos;
				   			   muy = T[k]->mean(r,len);
							   sgy = T[k]->stdv(r,len);
							   smxy = XY[k]->sumXY(i,r,len);

							   if( sgy == 0 )
								   error(1);

								double t = smxy - len*mux*muy;
								t = t / (len*sgx*sgy);
								t = 2*(1-t);
								if( t <= 0 )
									t = 0;
								else
									t = sqrt(t);


								double tt = references[yy]->shiftEntropy(t);
								if( references[yy]->informationGain < bestSplit->informationGain )
											break;
								//else if( references[yy]->informationGain == bestSplit->informationGain  && bestSplit->gap > references[yy]->gap )  ///breaking ties
								else if( references[yy]->informationGain == bestSplit->informationGain  && bestSplit->shapeletSTD > references[yy]->shapeletSTD )  ///breaking ties		    
									break;
			
								yy++;
					   }

					   t2 = clock();
					   boundTime += (t2-t3)/CLOCKS_PER_SEC;
					   if( yy < refCount )
					   {
						   pruneCount++;
						   continue;
					   }
*/

					   line->reset(k,i,len , sgx);
                       
					   for( j = 0 ; j < N ; j++ )
					   {

						   struct projection p;
						   p.cls = T[j]->c;
						   p.length = len;
						   p.tID = j;

                           
						   double bsf = INF , t;
						   int bestU = -1;
						   for( int u = 0 ; u < T[j]->length-len+1 ; u++ )
						   {
							 //  if( !T[j]->checkPos(u,len) )
								   //continue;
							   muy = T[j]->mean(u,len);
							   sgy = T[j]->stdv(u,len);

							   if( sgy == 0 )
								   sgy = 1;

							   smxy = XY[j]->sumXY(i,u,len);
								t = smxy - len*mux*muy;
								t = t / (len*sgx*sgy);
								t = 2*(1-t);
								if( t <= 0 )
									t = 0;
								else
									t = sqrt(t);

								if ( t < bsf )
								{
								   bsf = t;
								   bestU = u;
								}
						   } 
						   if( bsf == INF )
							   break;
						   p.pos = bestU;
						   p.distance = bsf;
						   p.depth = 0;
						   line->insert(j ,  p);
						   //double bE = line->bestGain();
						   //if ( bE < bestSplit->informationGain )
							 //  break;
                           
					   }

					   if( j < N )
						   continue;
					   line->findBestSplit();
					   //printf("Best Gain %lf\n", bestSplit->informationGain );
					   //if( refCount < MAXREF - 1)
					   //refCount = 0;

						if( line->informationGain > bestSplit->informationGain )
								bestSplit->copy(line);
						else if( line->informationGain == bestSplit->informationGain  && bestSplit->gap < line->gap )  ///breaking ties
						//else if( line->informationGain == bestSplit->informationGain  && bestSplit->shapeletSTD < line->shapeletSTD )  ///breaking ties
						{
							//cout << bestSplit->shapeletSTD << endl;
							//cout << "  " << line->shapeletSTD << endl;

							bestSplit->copy(line);
						}
							references[refCount%MAXREF]->copy(line);
							refCount = (refCount+1)%MAXREF;
					   //else
					  // {
						//   int qq = rand()%MAXREF;
					//	   references[qq]->copy(line);
					  // }
				  }
		   }
		   t2 = clock();
#if !RESULT
		   printf("\nTime for %d th time series : %lf seconds\n",k,(t2-t3)/CLOCKS_PER_SEC);
		   printf("\nBest information gain : %lf \n",bestSplit->informationGain);
#endif



	  }               
    
	t2 = clock();
  
#if RESULT
	printf("Shpelet ID : %d , Start Position : %d , Shapelet Length : %d\n\n", bestSplit->shapeletID , bestSplit->shapeletPos , bestSplit->shapeletLength );
	printf("Split informationGain : %lf , split position %d , split distance %lf and the separation gap %lf\n\n",bestSplit->informationGain,bestSplit->splitPos,bestSplit->splitDist,bestSplit->gap);
	printf("Total candidates : %lld , ",count);
	printf("Number of Pruned candidates : %lld\n\n", pruneCount );
    printf("Time for current shapelet : %lf seconds and Bound Computation Time %lf\n",(t2-t1)/CLOCKS_PER_SEC,boundTime);
#endif


  

	bestSplit->findEntropies();

	
	for( i = 1 ; i < N  ; i++ )
	{
		T[i]->clearPos();
		T[i]->init();
	}
	for( i = 0 ; i < N  ; i++ )
		T[bestSplit->line[i].tID]->insertPos(bestSplit->line[i].pos,bestSplit->line[i].length);


	struct logicalShapelet history ,  * pH = NULL;

	history.shpIDs = (int *)malloc ( sizeof(int) );
	history.shpPos = (int *)malloc ( sizeof(int) );
	history.shpLen = (int *)malloc ( sizeof(int) );
	history.dist = (double *)malloc ( sizeof(double) );
	history.gap = (double *)malloc ( sizeof(double) );
	history.infoGain = (double *)malloc ( sizeof(double) );

	history.shpIDs[0] = bestSplit->shapeletID;
	history.shpPos[0] = bestSplit->shapeletPos;
	history.shpLen[0] = bestSplit->shapeletLength;
	history.dist[0] = bestSplit->splitDist;
	history.gap[0] = bestSplit->gap;
	history.infoGain[0] = bestSplit->informationGain;

	history.d = (int *)malloc ( sizeof(int) );
	*history.d = 1;
	history.type = AND;

	orderLine * bestBest = bestSplit;
	orderLine * tLine = Logic_Shapelet(T,N,C,&history,bestSplit, AND);
	if( tLine->informationGain > bestBest->informationGain || (tLine->informationGain == bestBest->informationGain &&  tLine->gap > bestBest->gap ) )
	//if( tLine->informationGain > bestBest->informationGain || (tLine->informationGain == bestBest->informationGain &&  tLine->shapeletSTD > bestBest->shapeletSTD ) )
	{
		bestBest = tLine;
		pH = &history;
		bestBest->findEntropies();
	}
	else
		delete tLine;

	cout << endl << endl << "AND FINISHED" << endl << endl;

	struct logicalShapelet history1;
	history1.shpIDs = (int *)malloc ( sizeof(int) );
	history1.shpPos = (int *)malloc ( sizeof(int) );
	history1.shpLen = (int *)malloc ( sizeof(int) );
	history1.dist = (double *)malloc ( sizeof(double) );
	history1.gap = (double *)malloc ( sizeof(double) );
	history1.infoGain = (double *)malloc ( sizeof(double) );

	history1.shpIDs[0] = bestSplit->shapeletID;
	history1.shpPos[0] = bestSplit->shapeletPos;
	history1.shpLen[0] = bestSplit->shapeletLength;
	history1.dist[0] = bestSplit->splitDist;
	history1.gap[0] = bestSplit->gap;
	history1.infoGain[0] = bestSplit->informationGain;

	history1.d = (int *)malloc ( sizeof(int) );
	*history1.d = 1;
	history1.type = OR; 


	for( i = 1 ; i < N  ; i++ )
	{
		T[i]->clearPos();
		T[i]->init();
	}
	for( i = 0 ; i < N  ; i++ )
		T[bestSplit->line[i].tID]->insertPos(bestSplit->line[i].pos,bestSplit->line[i].length);



	tLine = Logic_Shapelet(T,N,C,&history1,bestSplit, OR);

	if( tLine->informationGain > bestBest->informationGain )
	{
		bestBest = tLine;
		pH = &history1;
		bestBest->findEntropies();
	}
	else
		delete tLine;



	for( i = 1 ; i < N  ; i++ )
	{
		T[i]->clearPos();
		T[i]->init();
	}



	if( bestSplit != bestBest )
	{
		delete bestSplit;
		bestSplit = bestBest;
	}
	int sP = bestSplit->splitPos+1;
	int cc;

	
	cout << "In Regular Divided classes : " << endl;
	
	int * divd = (int *)malloc(sizeof(int)*N);
	int * runLengths = (int *)malloc(sizeof(int)*N);
	
	int runId = 0;
	int runCount  = 1;

	for( i = 0 ; i < N ; i++)
		divd[i] = -1,runLengths[i] = 0;
	divd[runId] = T[bestSplit->line[0].tID]->c;
	for( i = 1 ; i < sP  ; i++ )
		if ( T[bestSplit->line[i].tID]->c == T[bestSplit->line[i-1].tID]->c )
			runCount++;
		else
		{
			runLengths[runId] = runCount;
			divd[runId++] = T[bestSplit->line[i].tID]->c;
			runCount = 1;
		}

	runLengths[runId] = runCount;
	
	cout << "The left one " << endl;
	for( i = 0 ; i <= runId ; i++ )
		{
			cout << runLengths[i] << "(" << divd[i] << ") ";
		}
	cout << endl;
	runId = 0,runCount  = 1;
	for( i = 0 ; i < N ; i++)
		divd[i] = -1,runLengths[i] = 0;
	divd[runId++] = T[bestSplit->line[sP].tID]->c;
	for( i = sP+1; i < N  ; i++ )
		if ( T[bestSplit->line[i].tID]->c == T[bestSplit->line[i-1].tID]->c )
			runCount++;
		else
		{
			runLengths[runId] = runCount;
			divd[runId++] = T[bestSplit->line[i].tID]->c;
			runCount = 1;
		}

	runLengths[runId] = runCount;
	cout << "The right one " <<endl;

	for( i = 0 ; i <= runId ; i++ )
		{
			cout << runLengths[i] << "(" << divd[i] << ") ";
		}

	cout << endl << endl << endl;
	
	   ofstream fout(inFname, ios::app);
   if ( pH !=  NULL )
   {
	   for( i = 0 ; i < *(pH->d) ; i++ )
			cout << pH->shpIDs[i] << " " << pH->shpPos[i] << " " << pH->shpLen[i] << " " << pH->dist[i]  << " " << pH->gap[i] << "  "<< pH->infoGain[i] << endl;
	   cout << endl;


		fout << nodeId << endl;
		fout  << *(pH->d) << endl << pH->type << endl << pH->dist[*pH->d-1]*sqrt((double)samplingRate) << endl;

		for( int k = 0 ; k < *pH->d ; k++ )
		{
			int sId = pH->shpIDs[k], s = pH->shpPos[k];

			fout << pH->shpLen[k]*samplingRate << endl;
			for( i = 0 ; i < pH->shpLen[k] ; i++ )
				for( j = 0 ; j < T[sId]->samplingRate ; j++ )
					fout << T[sId]->original_x[(s+i)*T[sId]->samplingRate+j] << " ";
			fout << endl;
		}
   }
   else
   {
	

		fout << nodeId << endl;
		fout  << 1 << endl << AND << endl << bestSplit->splitDist*sqrt((double)samplingRate) << endl;

		int sId = bestSplit->shapeletID, s = bestSplit->shapeletPos;

		fout << bestSplit->shapeletLength*samplingRate << endl;
		for( i = 0 ; i < bestSplit->shapeletLength ; i++ )
				for( j = 0 ; j < T[sId]->samplingRate ; j++ )
					fout << T[sId]->original_x[(s+i)*T[sId]->samplingRate+j] << " ";
		fout << endl;

   }


	 for( j = 0 ; j < N ; j++ )
		  delete XY[j];


	 free(XY);

	if( bestSplit->leftEntropy != 0)
	{
		timeSeries ** LL = (timeSeries **)malloc(sizeof(timeSeries *)*sP);
		for( i = 0 ; i < C ; i++)
			clsHist[i] = 0;

		for( i = 0 ; i < sP  ; i++ )
		{
			LL[i] = T[bestSplit->line[i].tID];
			clsHist[LL[i]->c] = 1;
			//printf("%d ",bestSplit->line[i].tID);
		}


		cc = 0;
		k = 0;
		for( j = 0 ; j < C ; j++ )
		{
			cc += clsHist[j];
			if(clsHist[j] == 1)
			{
				for( i = 0 ; i < sP ; i++ )
					if( LL[i]->c == j )
						LL[i]->c = k;
				k++;
			}
		}

		recursiveShapelet(LL,sP,cc,2*nodeId);
	}
	else
	{
		fout << 2*nodeId << endl;
		fout << "0" << endl << T[bestSplit->line[sP-1].tID]->originalClass << endl;
	}


	if( bestSplit->rightEntropy != 0 )
	{
		cc = 0;

		timeSeries ** RR = (timeSeries **)malloc(sizeof(timeSeries *)*(N-sP));

    	for( i = 0 ; i < C ; i++)
	    	clsHist[i] = 0;

		for( i = 0 ; i < (N-sP)  ; i++ )
		{
			RR[i] = T[bestSplit->line[i+sP].tID];
			clsHist[RR[i]->c] = 1;
			//printf("%d ",bestSplit->line[i+sP].tID);
		}

		cc = 0;
		k = 0;
		for( j = 0 ; j < C ; j++ )
		{
			cc += clsHist[j];
			if(clsHist[j] == 1)
			{
				for( i = 0 ; i < (N-sP) ; i++ )
					if( RR[i]->c == j )
						RR[i]->c = k;
				k++;
			}
		}

		recursiveShapelet(RR,N-sP,cc,2*nodeId+1);
	}
	else
	{
		fout << 2*nodeId+1 << endl;
		fout << "0" << endl << T[bestSplit->line[sP].tID]->originalClass << endl;
	}


	 for( j = 0 ; j < MAXREF ; j++ )
		delete references[j];

	 //for( j = 0 ; j < N ; j++ )
	//	  delete XY[j];

	 delete line;
	 free(clsHist);
	// free(XY);
	 delete bestSplit;

	fout.close();
	return 1;
}







int main(int argc , char ** argv )
{
    int i , j ;
    double t1,t2;
    t1 = clock();

	if( argc < 4 )
	{
		printf("ERROR!!! usage: mueen_shapelet.exe train_file N C maxln minln stepsize\n");
		exit(1);
	}

	int N = atoi(argv[2]);
    int C = atoi(argv[3]);
	ifstream ifs ( argv[1] , ifstream::in );

    if( !ifs.is_open() )
        error(2);
    

    strcpy(inFname,argv[1]);

    int * clsHist = (int *) malloc( sizeof(int) * C );
	for( i = 0 ; i < C ; i++ )
		clsHist[i] = 0;

	maxLen = 0;
	minLen = 999999999 ;
	
	//Read the input file
    timeSeries ** T = (timeSeries **)malloc(sizeof(timeSeries *)*N);

    for( j = 0 ; j < N && !ifs.eof() ; j++ )
    {
         char s[100000];
		 ifs.getline(s,100000);
		
		 stringstream ss (stringstream::in | stringstream::out);
		 ss << s;	  
		 double x[10000];
         double ccc;
		 ss >> ccc;
		 int c = (int)ccc;
		 if( c < 0 )
			 c = 0;
		 c = c%C;

		 for( i = 0 ; !ss.eof() ; i++ )
  			ss >> x[i];
  
		 //int n = (int)x[i-1];
		 int n = i;
		 if( n > maxLen ) maxLen = n;
		 if( n < minLen ) minLen = n;
         T[j] = new timeSeries(n , x , c , samplingRate);
		 T[j]->init();
		 clsHist[c]++;
    }
	ifs.close();

	t1 = clock();

	start = 10;
	end = maxLen;
	stepSize = 10;

	if( argc > 4 )	end = atoi(argv[4]);
	if( argc > 5 )	start = atoi(argv[5]);
	if( argc > 6 )	stepSize = atoi(argv[6]);


	#if RESULT
		printf("maxlen %d minLen %d\n",maxLen,minLen);
		for( i = 0 ; i < C ; i++ )
			cout << clsHist[i] << " ";
		cout << endl;
	#endif



	strcat(inFname,"_logictree");
	ofstream fout(inFname);
	fout.close();
	recursiveShapelet(T,N,C,1);

	t2 = clock();
    printf("Total Execution Time : %lf\n\n",(t2-t1)/CLOCKS_PER_SEC);

	free(clsHist);
	for( i = 0 ; i < N ; i++ )
		delete T[i];
	free(T);


    return 1;   
}

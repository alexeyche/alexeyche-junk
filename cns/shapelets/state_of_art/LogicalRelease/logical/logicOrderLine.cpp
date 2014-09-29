
#include "timeSeries.h"
#include "xyMatrix.h"
#include "logicOrderLine.h"
#include <stdio.h>
#include <string.h>
#define INF 9999999999999.99
      
orderLine::orderLine()
{
     N = 0;
     nCls = 0;
     splitPos = -1;
	 splitDist = -1;
	 curN = -1;
	 shapeletSTD = 0;
	
	 entropy = 0;
     line = NULL;
     clsHist = leftClsHist = rightClsHist = NULL;
	 informationGain = 0;
	 gap = 0;
                       
};      
orderLine::orderLine(int n , int nC , int * cH)
{
     N = n;
     nCls = nC;
     splitPos = -1;
	 splitDist = -1;
     curN = 0;
	 shapeletSTD = 0;

     line = (projection *)malloc(sizeof(projection)*N);
     leftClsHist = (int *)malloc(sizeof(int)*nCls);
     rightClsHist = (int *)malloc(sizeof(int)*nCls);
     clsHist = (int *)malloc(sizeof(int)*nCls);
	 for( int i = 0 ; i < nCls ; i++ )
		 clsHist[i] = cH[i];

	 entropy = 0;	
	 double ratio;
     for( int i = 0 ; i < nCls ; i++ )
     {
          ratio = (double)(clsHist[i])/N;
          entropy += -(log(ratio)*ratio);
     }

	 printf("entropy is : %lf\n\n",entropy);

     };

void orderLine::copy(orderLine * src )
{
	      shapeletID = src->shapeletID;
	      shapeletPos = src->shapeletPos;
	      shapeletLength = src->shapeletLength;
		  shapeletSTD = src->shapeletSTD;

	      N = src->N;
		  curN = src->curN;
	      splitPos = src->splitPos;
		  splitDist = src->splitDist;
	      nCls = src->nCls;
	      informationGain = src->informationGain;
	      gap = src->gap;


		  if( line == NULL )
			line = (projection *)malloc(sizeof(projection)*N);

	      for( int i = 0 ; i < N ; i++ )
	 		 line[i] = src->line[i];


		  if( leftClsHist == NULL )
		      leftClsHist = (int *)malloc(sizeof(int)*nCls);
		  if( rightClsHist == NULL )	
			  rightClsHist = (int *)malloc(sizeof(int)*nCls);
	      if( clsHist == NULL )
			  clsHist = (int *)malloc(sizeof(int)*nCls);

	      for( int i = 0 ; i < nCls ; i++ )
		  {
			  leftClsHist[i] = src->leftClsHist[i];
			  rightClsHist[i] = src->rightClsHist[i];
 	 		  clsHist[i] = src->clsHist[i];
		  }
	  	  entropy = 0;
	 	  double ratio;
	      for( int i = 0 ; i < nCls ; i++ )
	      {
	           ratio = (double)(clsHist[i])/N;
	           entropy += -(log(ratio)*ratio);
	      }
		  //printf("in copy\n");
};


void orderLine::reset(int sId, int sPos , int sLen, double stdv )
{
	 shapeletID = sId;
     shapeletPos = sPos;
     shapeletLength = sLen;
	 shapeletSTD = stdv;
     splitPos = -1;
	 splitDist = -1;
	 curN = 0;
	 rightTotal = leftTotal = 0;

	 projection p;
     p.cls = -1;
     p.length = 0;
     p.pos = -1;
     p.tID = -1;
     p.distance = 9999999.99;
	 

     for(int i = 0 ; i < N ; i++ )
           line[i] = p;

	 for( int i = 0 ; i < nCls ; i++ )
	 {
		rightClsHist[i] = 0;
	    leftClsHist[i] = 0;
	 }
	 //printf("LEft RESET\n");
	 informationGain = 0;
	 gap = 0;

};


void orderLine::insert(int i , projection p)
{

	if( curN == N )
	{
		printf("ERROR!!! line is full.\n");
	    exit(1);
	}

	//ordered insertion
	int j , k;
	for(  j = 0 ; j < curN ; j++ )
	{
		if( line[j].distance > p.distance )
		{
			for( k = curN-1 ; k >= j ; k-- )
				line[k+1] = line[k];
			line[j] = p;
			break;
		}
	}
	if( j == curN )
		line[j] = p;

	rightClsHist[p.cls]++;
	rightTotal++;

	if (curN == i) curN++;
	else printf("ERROR!!! insertion order missmatch\n");
};

double orderLine::minGap(int j)
{

	double meanLeft = 0 , meanRight = 0;
	for(int i = 0 ; i <= j ; i++)
		meanLeft += line[i].distance;

	meanLeft /= (j+1);

	for(int i = j+1 ; i < N ; i++)
		meanRight += line[i].distance;

	meanRight /= (N-j);

	return (meanRight-meanLeft);
	/*
	if( j < curN )
	{
		//printf("%d %lf %lf\n",j , line[j+1].distance , line[j].distance );
		return line[j+1].distance - line[j].distance;
	}
	else
		return 0;
*/
};


double orderLine::gapDist(int j)
{
	if( j < curN )
	{
		//printf("%d %lf %lf\n",j , line[j+1].distance , line[j].distance );
		return (line[j+1].distance + line[j].distance)/2.0;
	}
	else
		return 0;

};


int comp(const void *a,const void *b)
{
    projection *x=(projection *)a;
    projection *y=(projection *)b;

    if (x->distance > y->distance )
        return 1;
    else if (x->distance < y->distance )
        return -1;
    else
        return 0;
}


void orderLine::mergeTwoLines(orderLine * pLine , int opt)
{
	int i , j , ind;

	for(int i = 0 ; i < N ; i++ )
	{
		for ( j = 0 ; j < N ; j++ )
			if( pLine->line[j].tID == line[i].tID )
				break;

		if( opt > 0 && line[i].distance < pLine->line[j].distance && pLine->line[j].distance < INF  )
			line[i].distance =  pLine->line[j].distance;

		else if( opt < 0 && line[i].distance > pLine->line[j].distance )
			line[i].distance  = pLine->line[j].distance;

		//line[i].pos = pLine->line[j].pos;
		//line[i].length = pLine->line[j].length;
	}
	qsort(line,N,sizeof(projection),comp);
}


double orderLine::shiftEntropy(double shiftAmount, orderLine * pLine, int opt)
{
	projection * tempLine;
	tempLine = (projection *)malloc(sizeof(projection)*N);
	memcpy(tempLine,line,sizeof(projection)*N);

	double maxInf = 0 , maxGap = 0 , maxDistance = 0;
	int maxi = -1;


	for( int j = 1 ; j < pow(2.0,nCls)-2 ; j++ )
	{
		//printf("Original Split Point %d ",splitPos);
		for(int i = 0 ; i < N ; i++ )
		{
			int k = j&(int)pow(2.0,line[i].cls);
			if( k == 0 )
				line[i].distance -= shiftAmount;
			else
				line[i].distance += shiftAmount;
		}
		qsort(line,N,sizeof(projection),comp);
		if( pLine != NULL )
			mergeTwoLines(pLine,opt);
		findBestSplit();
        if( informationGain > maxInf )
		{
			maxi = splitPos;
			maxInf = informationGain;
			maxGap = gap;
			maxDistance = splitDist;
			//printf("splitPos : %d\n",splitPos);
		}
        else if(informationGain == maxInf && gap > maxGap )
		{
			maxi = splitPos;
			maxInf = informationGain;
			maxGap = gap;
			maxDistance = splitDist;
			//printf("splitPos : %d\n",splitPos);
		}
	}


	splitPos = maxi;
	informationGain = maxInf;
	gap = maxGap;
	splitDist = maxDistance;


	memcpy(line,tempLine,sizeof(projection)*N);
	free(tempLine);

	return maxInf;
};



double orderLine::findEntropies()
{
       int i;
       for( i = 0; i < nCls ; i++ )
	   {
	      leftClsHist[i] = 0;
		  rightClsHist[i] = clsHist[i];
	   }
	   leftTotal = 0;
	   rightTotal = N;

       for( i = 0 ; i <= splitPos ; i++ )
       {
            int c = line[i].cls;
            leftClsHist[c]++;
			leftTotal++;
            rightClsHist[c]--;
			rightTotal--;
	   }
       informationGain = computeInformationGain();            

       return informationGain;
};
     


double orderLine::findBestSplit()
{
       int i;
       for( i = 0; i < nCls ; i++ )
	   {
	      leftClsHist[i] = 0;
		  rightClsHist[i] = clsHist[i];
	   }
	   leftTotal = 0;
	   rightTotal = N;

	   double maxInf = 0;
	   double maxGap = 0;
	   int maxi = -1;

       for( i = 0 ; i < N ; i++ )
       {
            int c = line[i].cls;
            leftClsHist[c]++;
			//printf("c : %d, lefthist[c] %d\n",c,leftClsHist[c]);
			leftTotal++;
            rightClsHist[c]--;
			rightTotal--;
            informationGain = computeInformationGain();            
			//printf("information Gain : %lf\n",informationGain);
            double mG = minGap(i);
            if( informationGain > maxInf )
			{ 
				maxi = i;
				maxInf = informationGain;
				maxGap = mG;
				//printf("splitPos : %d\n",splitPos);
			}
            else if(informationGain == maxInf && mG > maxGap )
			{
				maxi = i;
				maxInf = informationGain;
				maxGap = mG;
				//printf("splitPos : %d\n",splitPos);
			}

       }
      // printf("Shifted Split Point %d and informationGain %lf\n",maxi,maxInf);
       gap = maxGap;
       splitPos = maxi;
	   splitDist = gapDist(splitPos);
	   informationGain = maxInf;

       return maxInf;
};
     


double orderLine::computeInformationGain()
{
     double ratio;

     leftEntropy = 0;
     for( int i = 0 ; i < nCls ; i++ )
     {
          ratio = (double)leftClsHist[i]/leftTotal;
		  if(ratio>0)
			leftEntropy += -(log(ratio)*ratio);
	      //printf("%d\n", leftClsHist[i]);

     }

     rightEntropy = 0;
     for( int i = 0 ; i < nCls ; i++ )
     {
          ratio = (double)rightClsHist[i]/rightTotal;
		  if(ratio>0)
			rightEntropy += -(log(ratio)*ratio);
     }

	// printf("Left Entropy : %lf , Right Entropy : %lf , N %d \n",leftEntropy, rightEntropy,N);
     return entropy - (leftTotal/N)*leftEntropy - (rightTotal/N)*rightEntropy;
};

orderLine::~orderLine()
{
     if( line != NULL )
         free(line);
     if( leftClsHist != NULL )
         free(leftClsHist);
     if( rightClsHist != NULL )
         free(rightClsHist);

};

/********************************/
/* Update at 4/6/2012   (Minor) */
/* Create at 3/12/2012 			*/
/********************************/

#include "FastShapelet.h"

#define INF 1e20                                                                //Pseudo Infitinte number for this code

int m,M;                                                                        //Length of each time series
double bsf;

typedef struct Index
    {   double value;
        int    index;
    } Index;


int comp(const void *a, const void* b)
{   Index* x = (Index*)a;
    Index* y = (Index*)b;
    return abs((int)(y->value)) - abs((int)(x->value));  // high to low
}


double distance(double* Q, int* order, double * T , int j , int m , double mean , double std , double best_so_far = INF )
{
    int i;
    double sum = 0;
    double bsf2 = best_so_far*best_so_far;
    for ( i = 0 ; i < m && sum < bsf2 ; i++ )
    {
        double x = (T[(order[i]+j)]-mean)/std;
        sum += (x-Q[i])*(x-Q[i]);
    }
    return sqrt(sum);
}

double NearestNeighborSearch(vector<double> const& query, vector<double> const& data, int obj_id, double* Q, int* order)
{
    double d;
    long long i;
	int	j ;
    double ex , ex2 , mean, std;
    long long loc = 0;

    m = query.size();
    M = data.size();

    bsf = INF;
    i = 0;
    j = 0;
    ex = ex2 = 0;

    if (obj_id == 0)
    {
        for(i=0; i<m; i++)
        {	d = query[i];
            ex += d;
            ex2 += d*d;
            Q[i] = d;
        }

        mean = ex/m;
        std = ex2/m;
        std = sqrt(std-mean*mean);

        for( i = 0 ; i < m ; i++ )
             Q[i] = (Q[i] - mean)/std;

        Index *Q_tmp = (Index *)malloc(sizeof(Index)*m);
        if( Q_tmp == NULL )
            error(3);
        for( i = 0 ; i < m ; i++ )
        {
            Q_tmp[i].value = Q[i];
            Q_tmp[i].index = i;
        }
        qsort(Q_tmp, m, sizeof(Index),comp);
        for( i=0; i<m; i++)
        {   Q[i] = Q_tmp[i].value;
            order[i] = Q_tmp[i].index;
        }
        free(Q_tmp);
    }

    i = 0;
    j = 0;
    ex = ex2 = 0;


    double* T = (double *)malloc(sizeof(double)*2*m);
    if( T == NULL )      error(3);

    double dist = 0;
    while(i<M)
    {
        d = data[i];
        ex += d;
        ex2 += d*d;
        T[i%m] = d;
        T[(i%m)+m] = d;


        if( i >= m-1 )
        {
            mean = ex/m;
            std = ex2/m;
            std = sqrt(std-mean*mean);

            j = (i+1)%m;
            dist = distance(Q,order,T,j,m,mean,std,bsf);
            if( dist < bsf )
            {
                bsf = dist;
                loc = i-m+1;
            }
            ex -= T[j];
            ex2 -= T[j]*T[j];
        }
        i++;
    }
    free(T);
    return bsf;
}


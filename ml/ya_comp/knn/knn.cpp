
#include <math.h>
#include "../read_csv.cpp"
#include "../count_lines.cpp"


#define PI 3.14159265358979


double euclidean_dist(double *x1, double *x2,  int n, int grad = 2) {
    double unp_dist = 0;
    for(int i=0; i<n; i++) {
        unp_dist = unp_dist + pow(x1[i]-x2[i],grad);
    }
    return pow(unp_dist,(double)1/grad);
}

template <typename T>
void shift_and_insert(T *arr, T value, int i, int n) {
    for(int it=n-1; it>i; it--) {
        arr[it] = arr[it-1];            
    }
    arr[i] = value;
}

void insert_in_min_seq(double *min_seq, double value, int n) {
    for(int i=0; i<n; i++) {
        if(value < min_seq[i]) {
            shift_and_insert<double>(min_seq, value, i, n);             
            break;
        }
    }
}

long fact(int n)
{
    long f; 
    if((n==0)||(n==1))
     f = 1;
    else    
        for(int i=1;i<=n;i++)
            f*=i;
    return f;       
    
}

int gamma(int n) {
    return(fact(n-1));
}

#define C_8 4.058712

double hypersphere_volume(double r, int n) {
    if(n==8) {
        return( pow(r,n) * C_8 );
    }
}

template <typename T>
int compare(const void* a, const void* b)
{
    if (*(T*)a < *(T*)b)
        return -1;
    else if (*(T*)a > *(T*)b)
        return 1;
    else
        return 0;
}

double* knn(double *x, int m, int n, int k) {
    double *y = new double[m];
    double *dists = new double[m];
    for(int cur=0; cur<m; cur++) {
        if(cur%10000 == 0) {
            printf("%d lines processed\n", cur);
        }
        double *x_et = splice(x,cur*n,cur*n+n);  // first element 
        for(int i=0; i<m; i++) {
            if(i != cur) {    // exclude myself
                double *x_cur = splice(x,i*n, i*n+n);
                dists[i] = euclidean_dist(x_et,x_cur,n);
                free(x_cur);
            } else {
                dists[i] = -1;
            }
        }
        qsort(dists,m,sizeof(double),compare<double>);
        double rad = dists[k];
        double Vhs = hypersphere_volume(rad, n);
        double y_cur = ((double)(k-1)/(m-1))/Vhs;
        y[cur] = y_cur;
        printf("%f %f %f\n", rad, Vhs, y[cur]);
        free(x_et);
    }
    delete []dists;
    return y;
}

void test_euclidean_dist() {
    double *x1 = new double[3];
    double *x2 = new double[3];
    x1[0] = 0.1;    x1[1] = 0.2;    x1[2] = 0.3;
    x2[0] = 0.1;    x2[1] = 0.2;    x2[2] = 0.4;

    printf("%f\n", euclidean_dist(x1,x2,3));
}


int main(int argc, char *argv[]) {
    char *input_f;
    if(argc>1) {
        input_f = argv[1];
    } else {
        printf("Need input file\n");
        return 1;
    }
    int nrow = count_rows(input_f);
    int ncol = count_cols(input_f, '\t');
    double *x = read_csv_file(input_f,'\t',nrow,ncol);
    double k1 = 100;
    int kn_1 = k1 * sqrt(nrow-1);
    double *y = knn(x, nrow, ncol, kn_1);
    write_csv_file(y,"responce",'\t',nrow,1);
    return 0;
}



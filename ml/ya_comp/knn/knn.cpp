
#include <math.h>
#include "../read_csv.cpp"
#include "../count_lines.cpp"


#define PI 3.14159265358979


double euclidean_dist(double *x1, double *x2, int n) {
    double sq_dist = 0;
    for(int i=0; i<n; i++) {
        sq_dist = sq_dist + pow(x1[i]-x2[i],2);
    }
    return sqrt(sq_dist);
}

template <typename T>
void shift_and_insert(T *arr, T value, int i, int n) {
    for(int it=n-1; it>i; it--) {
        arr[it] = arr[it-1];            
    }
    arr[i] = value;
}

void insert_in_min_seq(double *min_seq, int *min_index, double value, int index, int n) {
    for(int i=0; i<n; i++) {
        if(value < min_seq[i]) {
            shift_and_insert<double>(min_seq, value, i, n);             
            shift_and_insert<int>(min_index, index, i, n);
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

double* knn(double *x, int m, int n, int k) {
    for(int cur=0; cur<7; cur++) {
        double *x_et = splice(x,cur*n,cur*n+n);  // first element 
        double *min_dist = new double[k];
        int *min_index = new int[k];
        //init min_dist
        for(int it=0; it<k; it++) {
            min_dist[it] = 999;
            min_index[it] = -1;
        }
        for(int i=0; i<m; i++) {
            if(i != cur) {    // exclude myself
                double *x_cur = splice(x,i*n, i*n+n);
                double dist = euclidean_dist(x_et,x_cur,n);
                free(x_cur);
                insert_in_min_seq(min_dist, min_index, dist, i, k);
            }
        }
//        printf("%f %f %f %f\n", min_dist[0],min_dist[1],min_dist[2],min_dist[3]);
//        printf("%d %d %d %d\n", min_index[0],min_index[1],min_index[2],min_index[3]);
//        printf("//------------------------------------------\n");
        double rad = min_dist[k-1];
        double Vhs = hypersphere_volume(rad, n);
        double y = (k/(m-1))/Vhs;
        printf("%f %f %f \n", rad, Vhs, y);
        delete []min_dist;
        delete []min_index;
        free(x_et);
    }
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
    double k1 = 1;
    int kn_1 = k1 * sqrt(nrow-1);
    double *y = knn(x, nrow, ncol, kn_1);
    return 0;
}



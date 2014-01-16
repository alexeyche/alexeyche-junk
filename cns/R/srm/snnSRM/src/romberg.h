
#include<iostream>
#include<cmath>

#define I_MAX 10
#define J_MAX I_MAX
using namespace std;

double integrateRomberg(double (*f)(double,void*), void *data, double a, double b, unsigned char order_i, unsigned char order_j) {
    long double h0,h;
    long double I[2][order_i];
    long double temp_sum;
    unsigned int i,j,j_1;
    unsigned long exp_2=1;
    unsigned long exp_4=1;
    unsigned long exp_4_1;
    h0=b-a;
    h=h0;
    cout.precision(40);
    I[0][0]=h*(f(a, data)+f(b, data))/2.0;
    if(I[0][0] == 0) return 0;
    
    cout<<endl<<I[0][0]*4.0;
    for(i=1; i< order_i; i++)
        {
        temp_sum=0.0;
        exp_2<<=1;
        h=h0/((long double)exp_2);
        for(j=1; j< exp_2; j+=2)
            temp_sum+=f( a + j*h, data);
        I[0][i] = I[0][i-1] * 0.5 + temp_sum * h;
        cout<<endl<<I[0][i]*4.0;
        }   
    for(j=1; j<order_j; j++)
        {
        exp_4<<=2;
        exp_4_1=exp_4-1;
        j_1=j-1;
        cout<<"\nNext order";
        for(i=0; i< order_j-j; i++)
            {
            I[j%2][i]=(exp_4*I[j_1%2][i+1]-I[j_1%2][i])/(exp_4_1);      
            cout<<endl<<4.0*I[j%2][i];
            }
        }
   cout << "\n";
   cout<<endl<<4.0*I[j%2][i];
   cout << "\n";
   return(4.0*I[j%2][i]);
}


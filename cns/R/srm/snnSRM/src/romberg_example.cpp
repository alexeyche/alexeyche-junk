#include<iostream>
#include<cmath>
using namespace std;
const int i_max=50;
const int j_max=i_max;
long double f(long double x)
    {
    return 1.0/(1.0+x*x);
    }
int main()
    {
    long double a,b,h0,h;
    long double I[2][i_max];
    long double temp_sum;
    unsigned int i,j,j_1;
    unsigned long exp_2=1;
    unsigned long exp_4=1;
    unsigned long exp_4_1;
    cout<<"\nEnter a and b ";
    cin>>a>>b;
    cout<<a<<endl<<b;
    cout.precision(40);
    h0=b-a;
    h=h0;
    I[0][0]=h*(f(a)+f(b))/2.0;
    cout<<endl<<I[0][0]*4.0;
    for(i=1; i< i_max; i++)
        {
        temp_sum=0.0;
        exp_2<<=1;
        h=h0/((long double)exp_2);
        for(j=1; j< exp_2; j+=2)
            temp_sum+=f( a + j*h);
        I[0][i] = I[0][i-1] * 0.5 + temp_sum * h;
        cout<<endl<<I[0][i]*4.0;
        }   
    for(j=1; j<j_max; j++)
        {
        exp_4<<=2;
        exp_4_1=exp_4-1;
        j_1=j-1;
        cout<<"\nNext order";
        for(i=0; i< j_max-j; i++)
            {
            I[j%2][i]=(exp_4*I[j_1%2][i+1]-I[j_1%2][i])/(exp_4_1);      
            cout<<endl<<4.0*I[j%2][i];
            }
        }
    return 0;
    }   

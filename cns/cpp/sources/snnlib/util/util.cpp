
#include "util.h"

vector<string> split_into(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split_into(s, delim, elems);
    return elems;
}

void trim(string &str) {
    size_t endpos = str.find_last_not_of(" \t");
    if( string::npos != endpos )
    {
        str = str.substr( 0, endpos+1 );
    }
    size_t startpos = str.find_first_not_of(" \t");
    if( string::npos != startpos )
    {
        str = str.substr( startpos );
    }
}

double normal_distr_var = -1;

double getUnif() {
    return (double)rand()/(double)RAND_MAX;
}


double getExp(double rate) {
    double u = getUnif();
    return -log(u)/rate;
}

double getNorm() {
    if(normal_distr_var<0) {
        double U = getUnif();
        double V = getUnif();
        normal_distr_var = sqrt(-2*log(U)) * cos(2*PI*V);
        return(sqrt(-2*log(U)) * sin(2*PI*V));
    } else {
        double ret = normal_distr_var;
        normal_distr_var=-1;
        return(ret);
    }
}

double getLogNorm(double logmean, double logsd) {
    return( exp(logmean + logsd*getNorm()) );
}

void swapInd(size_t *l, size_t *r) {
    size_t temp = *r;
    *r = *l;
    *l = temp;
}

double getUnifBetween(double low, double high) {
    return( low + getUnif()*(high-low) );
}

double sampleDelay(double gain, double rate) {
    if(fabs(rate) < 0.00001) return 0.0;
    return gain*getExp(rate);
}

string strip_white(const string& input)
{
   size_t b = input.find_first_not_of(' ');
   if (b == string::npos) b = 0;
   return input.substr(b, input.find_last_not_of(' ') + 1 - b);
}

string strip_comments(const string& input, const string& delimiters)
{
   return strip_white(input.substr(0, input.find_first_of(delimiters)));
}

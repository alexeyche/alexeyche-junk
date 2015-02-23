#pragma once

#include <snnlib/base.h>
#include <sys/stat.h>

vector<string> split_into(const string &s, char delim, vector<string> &elems);
vector<string> split(const string &s, char delim);
vector<string> splitBySubstr(const string &s_inp, const string &delimiter);

void trim(string &str);
string trimC(const string &str);

#ifndef PI
    #define PI 3.1415926535897932384626433832795028841971693993751
#endif

double getUnif();
double getUnifBetween(double low, double high);
double getExp(double rate);

double sampleDelay(double gain, double rate);
double getNorm();
string strip_white(const string& input);
string strip_comments(const string& input, const string& delimiters);
long getFileSize(string filename);

bool strStartsWith(const string &s, const string &prefix);

struct IndexSlice {
    IndexSlice(size_t _from, size_t _to) : from(_from), to(_to) {}
    size_t from;
    size_t to;
};

vector<IndexSlice> dispatchOnThreads(size_t elements_size, size_t jobs);


vector<double> parseParenthesis(const string &s);

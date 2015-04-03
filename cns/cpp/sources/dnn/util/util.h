#pragma once


#include <dnn/base/base.h>
#include <sys/stat.h>
#include <dnn/contrib/rapidjson/document.h>

namespace dnn {

using namespace rapidjson;

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
bool fileExists(const std::string& name);
bool strStartsWith(const string &s, const string &prefix);

struct IndexSlice {
    IndexSlice(size_t _from, size_t _to) : from(_from), to(_to) {}
    size_t from;
    size_t to;
};
void replaceAll( string &s, const string &search, const string &replace );

vector<IndexSlice> dispatchOnThreads(size_t elements_size, size_t jobs);

#define TRY(X) \
	try {	\
		X;	\
	} catch {	\
		throw dnnException()<< "Error!\n"; \
	}\


vector<double> parseParenthesis(const string &s);
map<string, string> parseArgOptionsPairs(const vector<string> &opts);


}




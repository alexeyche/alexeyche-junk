#include "ClassShapelet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>
//#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <time.h>

//#define min(x,y) ((x)<(y)?(x):(y))
//#define max(x,y) ((x)>(y)?(x):(y))
#define abs(x)   ((x) >0 ?(x):-(x))

using namespace std;


/**************************************/
/*********  Type  Declaration  ********/
/**************************************/
typedef int SAX_word_type;
typedef vector< vector<double> > Data_type;
typedef unordered_set<int>      Obj_set_type;
typedef unordered_map<int,int>  Obj_count_type;
typedef vector< pair<int,int> > SAX_id_type;

struct USAX_elm_type
    {   Obj_set_type                obj_set;
        SAX_id_type                 sax_id;
        Obj_count_type              obj_count;
    };
typedef unordered_map<SAX_word_type, USAX_elm_type>     USAX_Map_type;
typedef unordered_map<SAX_word_type, Obj_set_type>      Hash_Mark_type;

typedef pair<SAX_word_type, double> Pair_score_type;
typedef vector< Pair_score_type >   Score_List_type;

typedef pair<int,double>            Pair_dist_type;
typedef vector<Pair_dist_type>      Dist_List_type;

//typedef tuple<double,double,double,int,int,SAX_word_type, SAX_id_type, int> Tuple;

struct Shapelet_type
    {   double  gain;
        int     obj_id;
        int     pos;
        int     len;
        double  dist_th;
        double  gap;
        int     num_diff;       // Difference between number of in_obj and out_obj.
    };

struct Rule_elm_type
    {   Shapelet_type   shapelet;
        Obj_set_type    obj_list;
    };
typedef vector<Rule_elm_type>   Rule_type;

typedef vector<int>             Obj_list_type;
typedef vector<int>             Classify_type;
typedef vector< Obj_list_type > Node_Obj_set_type;

/**************************************/
/*******  Function  Prototype *********/
/**************************************/

void error(int);
void PrintData();
SAX_word_type CreateSAXWord(const double*, const int*, double, double, int);
SAX_word_type CreateMaskWord(int, int);
void RandomProjection(int, double, int);
double CalScore(USAX_elm_type &, int);
int sortScore(Pair_score_type, Pair_score_type);
int sortDist(Pair_dist_type, Pair_dist_type);
double NearestNeighbor(int,  int, int);
double NearestNeighborSearch(vector<double> const&, vector<double> const&, int, double*, int*);

/// old function but work
double Entropy(vector<int>, int);
double CalInfoGain1(vector<int> const&, vector<int> const&, int const&, int const&);

///// new function still in doubt
double EntropyArray(vector<int> const&, int const&);
double CalInfoGain2(vector<int> const&, vector<int> const&, int const&, int const&);

Shapelet FindBestSAX(int);


void ReadTrainData(char*);
void CreateSAXList(int, int, int);
//void ReadTrainData(char* filename, Data_type &Data, vector<int> &Label, vector<int> &Class_Freq);
//void CreateSAXList(int subseq_len, int sax_len, int w, Data_type const& Data);

void printNodeSetList();
void printResult(FILE*,int,char**);

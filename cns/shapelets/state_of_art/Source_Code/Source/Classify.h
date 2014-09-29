#include "ClassShapelet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <functional>

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define abs(x)   ((x) >0 ?(x):-(x))

using namespace std;




/**************************************/
/*******  Function  Prototype *********/
/**************************************/

void error(int);
double NearestNeighbor(int,  int, int);
double NearestNeighborSearch(vector<double> const&, vector<double> const&, int, double*, int*);

void ReadData(char*);

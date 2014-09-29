#include "ClassShapelet.h"
#include <stdio.h>
#define INF                 1e8

Shapelet::Shapelet()
{
    this->gain = -INF;
    this->gap = -INF;
    this->dist_th = INF;
    this->obj = -1;
    this->pos = -1;
    this->len = -1;
    this->num_diff = -1;
}

Shapelet::Shapelet(double gain=-1, double gap=0, double dist_th=0, int obj=0, int pos=0, int len=0, int num_diff=0)
{
    this->gain = gain;
    this->gap = gap;
    this->dist_th = dist_th;
    this->obj = obj;
    this->pos = pos;
    this->len = len;
    this->num_diff = num_diff;
}

void Shapelet::setValueFew(double gain, double gap, double dist_th)
{   this->gain = gain;
    this->gap = gap;
    this->dist_th = dist_th;
}

void Shapelet::setValueAll(double gain, double gap, double dist_th, int obj, int pos, int len, int num_diff, vector<int> c_in, vector<int> c_out)
{   this->gain = gain;
    this->gap = gap;
    this->dist_th = dist_th;
    this->obj = obj;
    this->pos = pos;
    this->len = len;
    this->num_diff = num_diff;
    this->c_in = c_in;
    this->c_out = c_out;
}

void Shapelet::setValueTree(int obj, int pos, int len, double dist_th)
{   this->dist_th = dist_th;
    this->obj = obj;
    this->pos = pos;
    this->len = len;
}

void Shapelet::setTS(vector<double> ts)
{
    this->ts = ts;
}

/// max gain,  min num_diff,  max gap
bool Shapelet::operator<(const Shapelet& other) const
{
    if (gain > other.gain) return false;
    return ( (gain < other.gain) || \
             ((gain==other.gain)&&(num_diff>other.num_diff)) || \
             ((gain==other.gain)&&(num_diff==other.num_diff)&&(gap<other.gap)) );
}

bool Shapelet::operator>(const Shapelet& other) const
{   return (&other < this);
}


void Shapelet::printShort()
{
    printf("\n");
    printf("obj pos len dist_th   gain\n");
    printf("%3d %3d %3d %7.4f %6.4f\n", obj,pos,len,dist_th,gain);
}

void Shapelet::printLong()
{
    printf("\nLen=%-3d  @[%3d,%3d]  d_th=%5.2f gap=%.6f gain=%8.5f (%-3d)",len, obj, pos, dist_th, gap, gain, num_diff);
    printf("==> ");
    for (unsigned int i=0; i<c_in.size(); i++)     printf("%3d ", c_in[i]);
    printf(" / ");
    for (unsigned int i=0; i<c_in.size(); i++)     printf("%3d ", c_out[i]);
    printf("\n");
}


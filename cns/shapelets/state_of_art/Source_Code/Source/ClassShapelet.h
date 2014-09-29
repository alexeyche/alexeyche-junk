#include <vector>
using namespace std;

/**************************************/
/***********  Shapelet Class **********/
/**************************************/
class Shapelet
{   public:
        double  gain;
        double  gap;
        double  dist_th;
        int     obj;
        int     pos;
        int     len;
        int     num_diff;
        vector<int> c_in;
        vector<int> c_out;
        vector<double> ts;

        Shapelet();
        Shapelet(double gain, double gap, double dist_th, int obj, int pos, int len, int num_diff);
        bool operator<(const Shapelet& other) const;
        bool operator>(const Shapelet& other) const;
        void setValueFew(double gain, double gap, double dist_th);
        void setValueAll(double gain, double gap, double dist_th, int obj, int pos, int len, int num_diff, vector<int> c_in, vector<int> c_out);
        void setValueTree(int obj, int pos, int len, double dist_th);
        void setTS(vector<double>);

        void printLong();
        void printShort();
};

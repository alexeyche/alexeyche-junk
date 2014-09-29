#include <stdlib.h>
#include <math.h>

struct projection{
       
       int tID;
       int pos;
       int length;
       double distance;
       int cls;
	   int depth;
       };

class orderLine{
    
    public:  
      int shapeletID;
      int shapeletPos;
      int shapeletLength;
	  double shapeletSTD;

      
      int N;
	  int curN;
      int nCls;
	  double entropy;
	  int * clsHist;


      int * leftClsHist;
      int * rightClsHist;
      
      double leftTotal;
      double rightTotal;
      double leftEntropy;
      double rightEntropy;

	  double informationGain;
	  double gap;
	  double splitDist;		
      int splitPos;

       

      struct projection * line;
      
      orderLine();
      //orderLine(orderLine & p);
      orderLine(int n , int nC , int * cH);
	  ~orderLine();
      double findBestSplit();


	  void copy(orderLine * src );
	  
	  void reset(int sId, int sPos, int sLen , double);
      void insert(int i , projection p);
      double computeInformationGain();
	  double minGap(int j);
	  double gapDist(int j);
	  double shiftEntropy(double shftAmount, orderLine * pLine = NULL , int opt = 0);
	  double findEntropies();

	  void mergeTwoLines( orderLine * pLine , int opt);
      };

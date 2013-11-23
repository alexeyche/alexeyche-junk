/*
 * =============================================================================
 *
 *       Filename:  LZ76.c
 *
 *    Description:  Compute the Lempel-Ziv complexity of binary strings. Take a file as input. 
 *    One string/line. See On the complexity of finite sequences by Lempel and Ziv.
 *    Strings must be < 1000 characters
 *    Original implementation in MATLAB by S. Faul 
 *    available here : http://www.mathworks.co.uk/matlabcentral/fileexchange/6886-kolmogorov-complexity
 *
 *        Version:  1.0
 *        Created:  03/09/13 22:51:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  B.Frot (), benjamin.frot@dtc.ox.ac.uk
 *        Company:  
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define WRAPPER identityWrapper //Return LZ76 of a string
//#define WRAPPER kolmogorovWrapper //Return LZ76*log2(n) of a string
//#define WRAPPER meanWrapper //Return the mean of the K complexity of a string and its reverse.


void pr(char *s, int l, int r) {
    for(int i=l; i<=r; i++) {
        printf("%c", s[i]);
    }
    printf("\n");
}

int LZ(char * s, int n) {
  int c=1,l=1,i=0,k=1,kmax = 1,stop=0; 
  while(stop ==0) {
    pr(s, 0, l);
    printf("c: %d\n", c);
    if (s[i+k-1] != s[l+k-1]) {
      if (k > kmax) {
        kmax=k;
      }
      i++;

      if (i==l) {
        printf("is it a word? \n");
        pr(s, k, l);
        printf("----------\n");
        c++;
        l += kmax;
        if (l+1>n)
          stop =1;
        else {
          i=0;
          k=1;
          kmax=1;
        }
      } else {
        k=1;
      }
    } else {
     
      k++;
      if (l+k > n) {
        c++;
        stop =1;
      }
    }
  }
  return c;
}

float identityWrapper(char * s, int n) {
  return (float) LZ(s,n);
}

//Approximate the K complexity by multiplying by log_2(n)
float kolmogorovWrapper(char *s, int n){
  return LZ(s,n)*log2(n);
}

//Return the mean K-Complexity of s and rev(s).
float meanWrapper(char * s, int n) {
  char * revs = (char *) malloc(n*sizeof(char));
  float v;
  int j;
  for (j = 0 ; j < n ; j++)
    revs[j] = s[n-1-j];
  v = (kolmogorovWrapper(s,n) + kolmogorovWrapper(revs,n))/2.0;
  free(revs);
  return v;
}

int main(int argc, char** argv) {

  if (argc != 2) {
    printf("Usage : ./LZ76.c filename.\n Three complexity flavours are available. See #define WRAPPER in the souce code.\n");
    return 0;
  }

  FILE * f = fopen(argv[1],"r");
  if (f == NULL) {
    printf("This file does not exist or cannot be read.\n");
    return 0;
  }
  char line[1000]; //Strings cannot be longer than that.
  while (fscanf(f,"%s\n",line) != EOF) {
    printf("%f\n",WRAPPER(line,strlen(line))); 
  }
  fclose(f);
  return 0;
}


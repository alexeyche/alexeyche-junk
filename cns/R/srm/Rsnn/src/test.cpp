
#include "test.h"


// [[Rcpp::export]]
List testFun(List d1, List d2) {
    List data1 = d1["data"];
    List data2 = d2["data"];

    if(data1.size() != data2.size()) {
        printf("Two input lists must have identical size!\n");
        return R_NilValue;
    }
    
    return List::create(Named("data") = 0.0);
}    

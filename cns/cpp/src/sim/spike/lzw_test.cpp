
#include <sim/core.h>

#include "lzw.cpp"

#include <iostream>
#include <iterator>
#include <vector>
 
int main(int argc, char **argv) {
  CLI::ParseCommandLine(argc, argv);
  std::vector<int> compressed;
//  vec in("0 0 1 1 0 1 1 0 0 0 1 1 0 1 0 1 0 1 0 0 1 0 0 1 0 0 1 1 0 1 0 0 0 0 0 1 0 1 0 0 1 0 1 1 0 0 1 0 1 1 0");
  vec in("0 1 0 1 1 0 1 0 0 0 1 1 0 1 1 1 0 0 1 0"); // 0.1.011.0100.011011.1001.0
//  vec in("0 1 1 0 1 0 0 1"); // 0.1.10.100.1
  //in.print();
  Log::Info << "=====================================\n";
//  test_compress();
  vec out = compress(in);
  Log::Info << "=====================================\n";
//  out.t().print();
 
  return 0;
}

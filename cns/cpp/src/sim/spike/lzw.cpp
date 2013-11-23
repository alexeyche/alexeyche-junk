
#include <string>
#include <map>
 
#include <sim/core.h>

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.

void print_dict(const std::map<std::string,int> &d) {
    std::map<std::string,int>::const_iterator cur = d.begin();
    while(cur != d.end()) {
        Log::Info << cur->first << " -> " << cur->second << "\n";
        cur++;
    }
}

std::string str(char bit) {
    if(bit == 0) { return std::string("0"); }
    if(bit == 1) { return std::string("1"); }
    Log::Fatal << "Error in bit string: " << bit <<"\n"; 
}

std::string test_str(char bit) {
    if(bit == 'a') { return std::string("a"); }
    if(bit == 'b') { return std::string("b"); }
    if(bit == 'c') { return std::string("c"); }
    Log::Fatal << "Error in bit string: " << bit <<"\n"; 
}

void test_compress() { // from lz76.ps
    std::vector<char> result;
    int dictSize = 0;
    std::map<std::string,int> dictionary;

    std::string w = "";
    char seq[25] = { 'a', 'b', 'c', 'a', 'c', 'b', 
                     'c', 'a', 'b', 'c', 'a', 'b',
                     'c', 'a', 'b', 'c', 'a', 'b',
                     'c', 'a', 'b', 'c', 'a', 'b',
                     'c' };


    for(size_t it =0; it < 25; it++) {
        char c = seq[it];
        std::string wc = w + test_str(c);
        if (dictionary.count(wc)) {
            w = wc; 
        } else {
            dictionary[wc] = dictSize++;
            Log::Info << wc << "\n";
            w = "";
        }
    }
    
    Log::Info << "dict:\n"; 
    print_dict(dictionary);
    // Output the code for w.
    if (!w.empty())
      result.push_back(dictionary[w]);
    return; //vec(result);
}
 
vec compress(const vec &uncompressed) {
    // Build the dictionary.
    std::vector<double> result;
    int dictSize = 2;
    std::map<std::string,int> dictionary;
//    dictionary["0"] = 0;
//    dictionary["1"] = 1;
      
    uncompressed.t().print();

    std::string w = "";
    for (size_t it = 0; it !=uncompressed.n_elem; ++it) {
      char c = uncompressed[it];
      std::string wc = w + str(c);
      if (dictionary.count(wc)) {
          w = wc; 
      } else {
          result.push_back( dictionary[w] );
          // Add wc to the dictionary.
          dictionary[wc] = dictSize++;
          Log::Info << wc << "\n";
          w = "";
      }
    }
    Log::Info << "dict:\n"; 
    print_dict(dictionary);
    // Output the code for w.
    if (!w.empty())
      result.push_back(dictionary[w]);
    return vec(result);
}
//// Decompress a list of output ks to a string.
//// "begin" and "end" must form a valid range of ints
//template <typename Iterator>
//std::string decompress(Iterator begin, Iterator end) {
//  // Build the dictionary.
//  int dictSize = 256;
//  std::map<int,std::string> dictionary;
//  for (int i = 0; i < 256; i++)
//    dictionary[i] = std::string(1, i);
// 
//  std::string w(1, *begin++);
//  std::string result = w;
//  std::string entry;
//  for ( ; begin != end; begin++) {
//    int k = *begin;
//    if (dictionary.count(k))
//      entry = dictionary[k];
//    else if (k == dictSize)
//      entry = w + w[0];
//    else
//      throw "Bad compressed k";
// 
//    result += entry;
// 
//    // Add w+entry[0] to the dictionary.
//    dictionary[dictSize++] = w + entry[0];
// 
//    w = entry;
//  }
//  return result;
//}
// 


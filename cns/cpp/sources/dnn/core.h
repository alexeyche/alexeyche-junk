#pragma once


#include <iostream>
#include <memory>
#include <vector>
#include <numeric>
#include <functional>
#include <sstream>
#include <string>
#include <map>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <google/protobuf/message.h>
typedef google::protobuf::Message* ProtoMessage;

using std::map;
using std::cout;
using std::move;
using std::vector;
using std::accumulate;
using std::stringstream;
using std::istringstream;
using std::string;
using std::ostream;
using std::istream;
using std::cerr;
using std::terminate;
using std::cin;
using std::pair;
using std::min;
using std::ofstream;
using std::ifstream;
using std::multimap;

template <typename T>
using uptr = std::unique_ptr<T>;

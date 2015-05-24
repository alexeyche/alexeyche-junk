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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <queue>
#include <atomic>
#include <stdexcept>
#include <exception>
#include <unordered_set>
#include <set>
#include <future>

typedef google::protobuf::Message* ProtoMessage;

using std::map;
using std::set;
using std::cout;
using std::move;
using std::vector;
using std::accumulate;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::string;
using std::ostream;
using std::istream;
using std::cin;
using std::pair;
using std::min;
using std::ofstream;
using std::ifstream;
using std::multimap;
using std::priority_queue;
using std::unordered_set;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using sptr = std::shared_ptr<T>;


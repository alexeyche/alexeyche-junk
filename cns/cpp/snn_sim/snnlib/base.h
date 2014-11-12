#pragma once

#include <snnlib/core.h>

class Printable {
protected:
    virtual void print(std::ostream& str) const = 0;
public:    
    friend std::ostream& operator<<(std::ostream& str, Printable const& data) {
        data.print(str);
        return str;
    }
};


template <typename T>
void print_vector(vector<T> v, ostream &str) {
    std::copy(v.cbegin(), v.cend(), ostream_iterator<T>(str, ", "));
    str << "\n";
}

#pragma once


class Synapse : public Printable {
public:
    size_t id_pre;
    
    double x;
    double w;
    void print(std::ostream& str) const {
        str << "Synapse(id_pre: " << id_pre << ", x:" << x << ", w: )" << w;
    }
};

#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H

struct dim {
	size_t dim_size;
	double min;
	double max;
	double dx;
};


typedef std::vector<dim> dims;

class LookupTable {
public:
	LookupTable(double *t, dims d) : t(t), d(d) {}
	double getValue(size_t dim_num, size_t i) {
		dim_num*d[dim_num].dim_size
	}
	double *t;
	dims d;
};



#endif
#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H


#include "neuron_models.h"

struct dim {    
    size_t dim_size;
    double min;
    double max;
    double dx;
};

class TableIzh {
public:    
    TableIzh() : whole_dim(1) {}
    ~TableIzh() { free(v); free(u_fin); std::cout << "table finished" << std::endl; }
    void addDim(const std::string &name, size_t dim_size, double min, double max) {
        dim d;
        d.dim_size = dim_size;
        d.min = min;
        d.max = max;
        d.dx = (max-min)/dim_size;
        dims[name] = d;
        whole_dim *= d.dim_size;
    }    
    double *v;
    double *u_fin;
    size_t whole_dim;
    std::map<std::string, dim> dims;
};



class LookupTableIzh {
public:
	LookupTableIzh() : curr_time(0) {
		NeuronIzh n;
		generate_table(&n);
	}
	~LookupTableIzh() { delete table; }
	
	double getValue(double V, double u, double t) {
		size_t vi = getIndex("V", V);
		size_t ui = getIndex("u", u);
		size_t ti = getIndex("t", t);
		
		size_t vi_it = vi * table->dims["t"].dim_size * table->dims["u"].dim_size;
		size_t ui_it = ui * table->dims["t"].dim_size;

		//std::cout << "vi: " << vi << "\t" << "ui: " << ui << "\t" << "ti: " << ti << "\n";
		//std::cout << "vi_it: " << vi_it << "\t" << "ui_it: " << ui_it << "\n";
		std::cout << table->v[vi_it+ui_it+ti] << "\n";
		return table->v[vi_it+ui_it+ti];
	}

	double getLastU(double V, double u) {
		size_t vi = getIndex("V", V);
		size_t ui = getIndex("u", u);
		size_t vi_it = vi * table->dims["u"].dim_size;
		return table->u_fin[vi_it+ui];
	}

	void generate_table(Model *m) {    	
        
        table = new TableIzh();
        table->addDim("V", 100, -100.0, 30.0); // V dim
        table->addDim("u", 50, -20.0, 10.0); // u dim
        table->addDim("t", 70, 0.0, 50.0); // t dim

        Log::Info << "Generating table with " << table->whole_dim << " elements ... ";
        table->v = (double*)malloc(sizeof(double)*table->whole_dim);
        table->u_fin = (double*)malloc(sizeof(double) * table->dims["V"].dim_size * table->dims["u"].dim_size);
        
        vec V_space = linspace<vec>(table->dims["V"].min, table->dims["V"].max, table->dims["V"].dim_size);
        vec t_space = linspace<vec>(table->dims["t"].min, table->dims["t"].max, table->dims["t"].dim_size);
        vec u_space = linspace<vec>(table->dims["u"].min, table->dims["u"].max, table->dims["u"].dim_size);

        for(size_t vi=0; vi<V_space.n_elem; vi++) {
    		size_t vi_it = vi * table->dims["t"].dim_size * table->dims["u"].dim_size;
            
            for(size_t ui=0; ui<u_space.n_elem; ui++) {
            	
                size_t ui_it = ui * table->dims["t"].dim_size;
                
                state_type x(3);
                x[0] = V_space(vi);
                x[1] = u_space(ui);
                x[2] = 10;
                
                for(size_t ti=0; ti<t_space.n_elem; ti++) {
                    m->do_step(x, t_space(ti));
                    table->v[vi_it+ui_it+ti] = x[0];
                }                
                
                table->u_fin[vi*table->dims["u"].dim_size+ui] = x[1];
    	    }
        	
        }
        Log::Info << "done" << std::endl;
    }
private:
	size_t getIndex(const std::string &axis_name, double value) {		
		return (size_t)(value - table->dims[axis_name].min)/table->dims[axis_name].dx;  // -100 -70
	}	
	TableIzh *table;
	double curr_time;
};



#endif
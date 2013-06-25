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
    float *v;
    float *u_fin;
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
	
	float getValue(double V, double u, double t) {
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

	float getLastU(double V, double u) {
		size_t vi = getIndex("V", V);
		size_t ui = getIndex("u", u);
		size_t vi_it = vi * table->dims["u"].dim_size;
		return table->u_fin[vi_it+ui];
	}

	void generate_table(Model *m) {    	
        
        table = new TableIzh();
        table->addDim("V", 100, -105.0, 30.0); // V dim
        table->addDim("u", 100, -15.5, 36.5); // u dim
        table->addDim("I", 100, -70, 70); // I dim
        table->addDim("t", 100, 0.0, 50.0); // t dim


        Log::Info << "Generating table with " << table->whole_dim << " elements ... ";
        table->v = (float*)malloc(sizeof(float)*table->whole_dim);
        table->u_fin = (float*)malloc(sizeof(float) * table->dims["V"].dim_size * table->dims["u"].dim_size);
        
        vec V_space = linspace<vec>(table->dims["V"].min, table->dims["V"].max, table->dims["V"].dim_size);
        vec u_space = linspace<vec>(table->dims["u"].min, table->dims["u"].max, table->dims["u"].dim_size);
        vec I_space = linspace<vec>(table->dims["I"].min, table->dims["I"].max, table->dims["I"].dim_size);
        vec t_space = linspace<vec>(table->dims["t"].min, table->dims["t"].max, table->dims["t"].dim_size);

        for(size_t vi=0; vi<V_space.n_elem; vi++) {
    		size_t vi_it = vi * table->dims["u"].dim_size * table->dims["t"].dim_size;
            
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
    void check_bounds(Model *m) {
        float V0 = -70;
        float u0 = -14;
        float V_min=100; float V_max=-100; float u_min=100;  float u_max=-100;

        vec V_space = linspace<vec>(-105,30.0,100);
        vec u_space = linspace<vec>(-15.5, 36.5,100);
        vec I_space = linspace<vec>(-70,70,100);
        vec t_space = linspace<vec>(0,50,100);
        for(size_t vi=0; vi<V_space.n_elem; vi++) {
            for(size_t ui=0; ui<u_space.n_elem; ui++) {
                for(size_t ii=0; ii<I_space.n_elem; ii++) {
                    state_type x(3);
                    x[0] = V_space(vi);
                    x[1] = u_space(ui);
                    x[2] = I_space(ii);
                    for(size_t ti=0; ti<t_space.n_elem; ti++) {
                            m->do_step(x, t_space(ti)); 
                            if (V_min > x[0]) {
                                V_min = x[0];
                            }         
                            if(V_max < x[0]) {
                                V_max = x[0];
                            }     
                            if(u_min > x[1]) {
                                u_min = x[1];
                            }         
                            if(u_max < x[1]) {
                                u_max = x[1];
                            }                                
                    }
                }
            }
        }   
        std::cout << "V_min: " << V_min << " V_max: " << V_max << " u_min: " << u_min << " u_max: " << u_max << std::endl;        
    }

private:
	size_t getIndex(const std::string &axis_name, double value) {		
		return (size_t)(value - table->dims[axis_name].min)/table->dims[axis_name].dx;  // -100 -70
	}	
	TableIzh *table;
	double curr_time;
};



#endif
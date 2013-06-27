#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H

#include <fstream>
#include "neuron_models.h"

struct dim {    
    size_t dim_size;
    double min;
    double max;
    double dx;
    size_t it_size;
    size_t it_size_no_t;
};


class TableIzh {
public:    
    TableIzh() : dims_size(1), dims_size_no_t(1) {}
    ~TableIzh() { free(v); free(u_fin); std::cout << "table finished" << std::endl; }
    void addDim(const std::string &name, size_t dim_size, double min, double max) {
        dims_ins_order.push_back(name);
        dim d;
        d.dim_size = dim_size;
        d.min = min;
        d.max = max;
        d.dx = (max-min)/dim_size;
        dims[name] = d;       
        dims_size *= d.dim_size;

        if( name != "t" ) {
            dims_size_no_t *= d.dim_size;
        }
        size_t dim_acc=1;
        size_t dim_acc_no_t=1;
        for(size_t ) {
            iterator--;
            Log::Info << "iter " << dims[*iterator] << std::endl;
            dims[*iterator].it_size = dim_acc;
            dims[*iterator].it_size_no_t = dim_acc_no_t;
            dim_acc *= dims[*iterator].dim_size;
            if( *iterator != "t") {
                dim_acc_no_t *= dims[*iterator].dim_size;
            }                
        }
    }

    float *v;
    float *u_fin;
    size_t dims_size;
    size_t dims_size_no_t;
    std::map<std::string, dim> dims;
//private:
    std::vector<const std::string> dims_ins_order;    
};


#define BUFF_LEN 1000
#define FILENAME "NeuronIzhData.bin"

class LookupTableIzh {
public:
	LookupTableIzh() : curr_time(0)  {		
        NeuronIzh n;

		generate_table(&n);
	}
	~LookupTableIzh() { delete table; }
	
	float getValue(double V, double u, double I, double t) {
		size_t vi = getIndex("V", V);
		size_t ui = getIndex("u", u);
		size_t ii = getIndex("I", I);
        size_t ti = getIndex("t", t);
		
		size_t vi_it = vi  * table->dims["V"].it_size;
		size_t ui_it = ui * table->dims["u"].it_size;
        size_t ii_it = ii * table->dims["I"].it_size;

		//std::cout << "vi: " << vi << "\t" << "ui: " << ui << "\t" << "ti: " << ti << "\n";
		//std::cout << "vi_it: " << vi_it << "\t" << "ui_it: " << ui_it << "\n";
		std::cout << table->v[vi_it+ui_it+ii_it+ti] << "\n";
		return table->v[vi_it+ui_it+ii_it+ti];
	}

	float getLastU(double V, double u, double I) {
		size_t vi = getIndex("V", V);
		size_t ui = getIndex("u", u);
        size_t ii = getIndex("I", I);

		size_t vi_it = vi * table->dims["u"].dim_size * table->dims["I"].dim_size;
        size_t ui_it = ui * table->dims["I"].dim_size;

		return table->u_fin[vi_it+ui_it+ii];
	}

	void generate_table(Model *m) {    	
        
        table = new TableIzh();
        table->addDim("V", 100, -105.0, 30.0); // V dim
        table->addDim("u", 50, -15.5, 36.5); // u dim
        table->addDim("I", 100, -70, 70); // I dim
        table->addDim("t", 100, 0.0, 50.0); // t dim
        for(it_type iterator = table->dims_ins_order.begin(); iterator != table->dims_ins_order.end(); iterator++) {
            std::cout << *iterator << " it_size: " << table->dims[*iterator].it_size  
                                   << " it_size_no_t: " << table->dims[*iterator].it_size_no_t << std::endl;     

        }

        Log::Info << "Generating table with " << table->dims_size << " elements ... ";        
        table->v = (float*)malloc(sizeof(float)*table->dims_size);
        table->u_fin = (float*)malloc(sizeof(float) * table->dims_size_no_t);
        Log::Info << "Checking file " << FILENAME;
        std::ifstream bin_data(FILENAME, std::ios::binary);
        if (bin_data.good()) {
            Log::Info << "... file found. Reading data ... ";
            // float buff[BUFF_LEN];
            // int count = 0;
            // float *dest = table->v;
            // while(!bin_data.eof()) {
            //     bin_data.read(buff, sizeof(float)*BUFF_LEN);
            //     if(count >= table->dims_size) {
            //         dest = table->u_fin;
            //     }
            //     memcpy(dest[count], buff, sizeof(float)*BUFF_LEN)
            //     count += BUFF_LEN;
            // }
            Log::Info << "done" << std::endl;
        }
        Log::Info << "... file not found. New table will be generated." << std::endl;
        return;
        Timer::Start("Table generation");
        
        vec V_space = linspace<vec>(table->dims["V"].min, table->dims["V"].max, table->dims["V"].dim_size);
        vec u_space = linspace<vec>(table->dims["u"].min, table->dims["u"].max, table->dims["u"].dim_size);
        vec I_space = linspace<vec>(table->dims["I"].min, table->dims["I"].max, table->dims["I"].dim_size);
        vec t_space = linspace<vec>(table->dims["t"].min, table->dims["t"].max, table->dims["t"].dim_size);

        for(size_t vi=0; vi<V_space.n_elem; vi++) {
    		size_t vi_it = vi  * table->dims["V"].it_size;
            for(size_t ui=0; ui<u_space.n_elem; ui++) {
                size_t ui_it = ui * table->dims["u"].it_size;
                for(size_t ii=0; ii<I_space.n_elem; ii++) {
                    size_t ii_it = ii * table->dims["I"].it_size;
                    
                    state_type x(3);
                    x[0] = V_space(vi);
                    x[1] = u_space(ui);
                    x[2] = I_space(ii);
                
                    for(size_t ti=0; ti<t_space.n_elem; ti++) {
                        m->do_step(x, t_space(ti));
                        table->v[vi_it+ui_it+ii_it+ti] = x[0];
                    }                
                    table->u_fin[vi * table->dims["V"].it_size_no_t +
                                 ui * table->dims["u"].it_size_no_t + ii] = x[1];
                }                
    	    }
        	
        }
        Timer::Stop("Table generation");
        Log::Info << "Table generated. Saving data to file " << filename << std::endl;
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
    const std::string filename;
};



#endif
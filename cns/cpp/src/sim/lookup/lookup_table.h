#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H

#include <fstream>
#include <algorithm>
#include "neuron_models.h"

struct dim {    
    size_t dim_size;
    double min;
    double max;
    double dx;
    size_t it_size;
    size_t it_size_no_t;
    
    std::string name;
    vec space;

    bool equals(const dim &other_dim) {
        if(dim_size != other_dim.dim_size) { return false; }
        if(min != other_dim.min) { return false; }
        if(max != other_dim.max) { return false; }
        if(dx != other_dim.dx) { return false; }
        if(it_size != other_dim.it_size) { return false; }
        if(it_size_no_t != other_dim.it_size_no_t) { return false; }
        //if(name != other_dim.name) { return false; }        

        return true;
    }
};

struct pointIzh {
    pointIzh(size_t Vi,  size_t ui, size_t Ii, size_t ti = -1) : Vi(Vi), ui(ui), Ii(Ii), ti(ti) {}
    size_t Vi;
    size_t ui;
    size_t Ii;
    size_t ti;
};

struct fileHead {
        unsigned int dims_num;
        dim *dims; 
        
        void writeToFile(FILE *fp) {
            fwrite(&dims_num, sizeof(dims_num), 1, fp);
            fwrite(dims, sizeof(dim), dims_num, fp);
        }
        void readFromFile(FILE *fp) {
            size_t rb = fread(&dims_num, sizeof(dims_num), 1, fp);
            dims = new dim[dims_num];
            rb = fread(dims, sizeof(dim), dims_num, fp);            
        }
        bool equals(const fileHead &other_fileHead) {
            if (other_fileHead.dims_num != dims_num) { return false; }
            
            for(size_t i=0; i<dims_num; i++) {
                if(!dims[i].equals(other_fileHead.dims[i])) {
                    return false;
                }
            }
            return true;
        }
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
        d.name = name.c_str();
        vec space(d.dim_size);
        double val=d.min;
        for(size_t i=0; i<d.dim_size; i++) {
            space(i) = val;
            val += d.dx;
        }
        d.space = space;
        dims[name] = d;       
        dims_size *= d.dim_size;

        if( name != "t" ) {
            dims_size_no_t *= d.dim_size;
        }
        
        size_t dim_acc=1;
        size_t dim_acc_no_t=1;
        for(int i=dims_ins_order.size()-1; i>=0; --i) {
            const std::string cur_name = dims_ins_order[i];            
            dims[cur_name].it_size = dim_acc;
            dims[cur_name].it_size_no_t = dim_acc_no_t;
            
            dim_acc *= dims[cur_name].dim_size;
            if( cur_name != "t") {                
                dim_acc_no_t *= dims[cur_name].dim_size;
            }   
        }
    }
    vec& getSpaceVec(const std::string axis_name) {
        return dims[axis_name].space;
    }
    void placeState(pointIzh p, state_type &state) {
        size_t Vi_it, ui_it, Ii_it, ti_it;
        Vi_it = p.Vi * dims["V"].it_size;
        ui_it = p.ui * dims["u"].it_size;
        Ii_it = p.Ii * dims["I"].it_size;
        ti_it = p.ti * dims["t"].it_size;
        v[Vi_it + ui_it + Ii_it + ti_it] = state[0]; // putting V
        
        if(ti_it == dims["t"].dim_size-1)  { // very end of time
            Vi_it = p.Vi * dims["V"].it_size_no_t;
            ui_it = p.ui * dims["u"].it_size_no_t;
            Ii_it = p.Ii * dims["I"].it_size_no_t;
            u_fin[Vi_it + ui_it + Ii_it] = state[1]; // putting final u
        }            
    }
    void allocMemory() {
        Log::Info << "Allocating " << sizeof(float) * dims_size << " bytes" << std::endl;
        v = (float*)malloc(sizeof(float) * dims_size);
        Log::Info << "Allocating " << sizeof(float) * dims_size_no_t << " bytes" << std::endl;
        u_fin = (float*)malloc(sizeof(float) * dims_size_no_t);
    }
    fileHead* generateFileHeader() {
        fileHead* fh = new fileHead();
        fh->dims_num = dims_ins_order.size();
        fh->dims = new dim[fh->dims_num];
        for(size_t i=0; i<dims_ins_order.size(); i++) {
            const std::string cur_name = dims_ins_order[i];     
            memcpy((void*)&fh->dims[i], (void*)&dims[cur_name], sizeof(dim));
        }
        return fh;
    }


    float *v;
    float *u_fin;
    size_t dims_size;
    size_t dims_size_no_t;
    std::map<std::string, dim> dims;
//private:
    std::vector<std::string> dims_ins_order;    
};


#define BUFF_LEN 10000
#define FILENAME "NeuronIzhData.bin"

class LookupTableIzh {

struct LinealInterpGrid {
    float low;
    float high;
    int low_i;
    int high_i;
};
public:
	LookupTableIzh() : curr_time(0)  {		
        NeuronIzh n;

		generate_table(&n);
	}
	~LookupTableIzh() { delete table; }
	
	float getValue(size_t vi, size_t ui, size_t ii, size_t ti) {
		size_t vi_it = vi  * table->dims["V"].it_size;
		size_t ui_it = ui * table->dims["u"].it_size;
        size_t ii_it = ii * table->dims["I"].it_size;

		return table->v[vi_it+ui_it+ii_it+ti];
    }
    float bilineal_interpolation(std::vector<double> vals) {
        vec& t_space = table->getSpaceVec("t");
        t_space.print();
        std::vector<LinealInterpGrid> grid_values;
        for(size_t i=0; i<table->dims_ins_order.size(); i++) {
            grid_values.push_back(getGridValue(table->dims_ins_order[i],vals[i]));        
        }

        float denom = 1.0;
        for(size_t ax=0; ax<grid_values.size(); ax++) {
            denom *= grid_values[ax].high - grid_values[ax].low;
        }
        
        int calc_len = pow(2,grid_values.size());
        char digits[4] = {8, 4, 2, 1};
        float final_val = 0;
        for(size_t i=0; i<calc_len; i++) {            
            size_t work_ind[4];
            double deltas = 1.0;
            for(size_t j=0; j<4; j++) {
                if(i & digits[j]) {
                    work_ind[j] = grid_values[j].high_i;
                    Log::Info << "i:" << i << " " << "j:" << j  << " - " << vals[j] << ", " << grid_values[j].low << " = " << vals[j] - grid_values[j].low << std::endl;
                    deltas *= vals[j] - grid_values[j].low;
                } else {
                    work_ind[j] = grid_values[j].low_i;
                    Log::Info << "i:" << i << " " << "j:" << j  << " - " << grid_values[j].high << ", " << vals[j] << " = " << grid_values[j].high - vals[j] << std::endl;
                    deltas *=  grid_values[j].high - vals[j];
                }
            }       
            float val = getValue(work_ind[0],  work_ind[1],  work_ind[2],  work_ind[3]);
            final_val += (val/denom)*deltas;
            Log::Info <<  work_ind[0] << ":" << work_ind[1] << ":" << work_ind[2] << ":" << work_ind[3] << " " << val << "    | deltas: " << deltas << std::endl;
        }
        
        Log::Info << "Final value: " << final_val << std::endl;
        return final_val;
    }

    LinealInterpGrid getGridValue(const std::string &axis_name, double value) {
        vec space = table->getSpaceVec(axis_name);
        LinealInterpGrid gr;
        gr.low_i = floor((value - table->dims[axis_name].min)/table->dims[axis_name].dx);  // -100 -70
        gr.high_i = gr.low_i +1;
        gr.low = space(gr.low_i);
        gr.high = space(gr.high_i);
        Log::Info << axis_name << ": " << table->dims[axis_name].dx << std::endl;
        Log::Info << axis_name << ": " << (value - table->dims[axis_name].min)/table->dims[axis_name].dx << std::endl;
        Log::Info << axis_name << ": " << "low_i: " << gr.low_i << " high_i: " << gr.high_i << " low_v: " << gr.low << " high_v: " << gr.high << std::endl; 
         
        return gr;
     }

	// float getLastU(double V, double u, double I) {
	// 	size_t vi = getIndex("V", V);
	// 	size_t ui = getIndex("u", u);
 //        size_t ii = getIndex("I", I);

	// 	size_t vi_it = vi * table->dims["u"].dim_size * table->dims["I"].dim_size;
 //        size_t ui_it = ui * table->dims["I"].dim_size;

	// 	return table->u_fin[vi_it+ui_it+ii];
	// }

	void generate_table(Model *m) {    	
        table = new TableIzh();
        
        table->addDim("V", 100, -105.0, 30.0); // V dim
        table->addDim("u", 100, -15.5, 36.5); // u dim
        table->addDim("I", 50, -70, 70); // I dim
        table->addDim("t", 100, 0.0, 50.0); // t dim
        
        Log::Info << "Generating table with " << table->dims_size << " elements ... " << std::endl;        
        table->allocMemory(); // allocate mem
           
        
        if(readFromFile()) {
           return;
        }

        Timer::Start("Table generation");
        
        vec& V_space = table->getSpaceVec("V");
        vec& u_space = table->getSpaceVec("u");
        vec& I_space = table->getSpaceVec("I");
        vec& t_space = table->getSpaceVec("t");

        for(size_t vi=0; vi<V_space.n_elem; vi++) {
            for(size_t ui=0; ui<u_space.n_elem; ui++) {
                for(size_t ii=0; ii<I_space.n_elem; ii++) {
                   
                    state_type x(3);
                    x[0] = V_space(vi);
                    x[1] = u_space(ui);
                    x[2] = I_space(ii);
                
                    for(size_t ti=0; ti<t_space.n_elem; ti++) {
                        m->do_step(x, t_space(ti));
                        table->placeState( pointIzh(vi, ui, ii, ti), x);                        
                    }        
                }                
    	    }
        }
        Timer::Stop("Table generation");
        Log::Info << "Table generated. Saving data to file " << filename << std::endl;
        saveToFile();                       
    }

    void saveToFile() {
         FILE *bin_data = fopen(FILENAME,"wb");
         fileHead * fh = table->generateFileHeader();
         fh->writeToFile(bin_data);
         // writing table         
         float *buff = new float[BUFF_LEN];
         unsigned int download_b = BUFF_LEN;
         // first one for V
         int count = table->dims_size;
         int p = 0;
         while (p <= table->dims_size) {
            count -= BUFF_LEN;
            if(count<0) {
                download_b = BUFF_LEN + count;
            }
            memcpy(buff, table->v+p, sizeof(float)*download_b);
            fwrite((void*)buff, sizeof(float), download_b, bin_data);
            
            if(count<0) break;
            
            p += download_b;
         }
         //second for u_fin
         download_b = BUFF_LEN;
         count = table->dims_size_no_t;
         p = 0;
         while (p <= table->dims_size_no_t) {
            count -= BUFF_LEN;
            if(count<0) {
                download_b = BUFF_LEN + count;
            }
            memcpy(buff, table->u_fin+p, sizeof(float)*download_b);
            
            fwrite((void*)buff, sizeof(float), download_b, bin_data);
            if(count<0) break;

            p += download_b;
         }
         fclose(bin_data);
    }
    bool readFromFile() {
         FILE* bin_data;
         Log::Info << "Checking file " << FILENAME;
         if(bin_data = fopen(FILENAME,"rb")) {
             Log::Info << "... file found. Reading data ... " << std::endl;

             fileHead * fh_file = new fileHead();
             fh_file->readFromFile(bin_data);
             fileHead * fh_cur = table->generateFileHeader();
             if(fh_cur->equals(*fh_file)) {
                Log::Info << "Head file is proper. Loading table ... ";
                Timer::Start("Table read");
                float *buff = new float[BUFF_LEN];
                
                float *dest = table->v;
                int count = table->dims_size;   
                int p = 0;         
                unsigned int download_b = BUFF_LEN;
                
                while(p <= table->dims_size) {
                    count -= BUFF_LEN;                                      
                    if(count<0) {
                        download_b = BUFF_LEN + count;                        
                    }
                    size_t rb = fread((void*)buff, sizeof(float), download_b, bin_data);
                    memcpy(table->v+p, buff, sizeof(float)*download_b);   
                    if(count<0) break;
                    
                    p += download_b;
                }
                download_b = BUFF_LEN;
                count = table->dims_size_no_t;
                p = 0;           
                while(p <= table->dims_size_no_t) {
                    count -= BUFF_LEN;                                      
                    if(count<0) {
                        download_b = BUFF_LEN + count;                        
                    }
                    size_t rb = fread((void*)buff, sizeof(float), download_b, bin_data);
                    memcpy(table->u_fin+p, buff, sizeof(float)*download_b);   
                    if(count<0) break;
                                 
                    p += download_b;
                }
                fclose(bin_data);
                Timer::Stop("Table read");
                Log::Info << "done" << std::endl;      
                return true;
             } else {
                Log::Info << "Head of file " << FILENAME << " don't proper for current configs. File will be overwritten with new version" << std::endl;
                return false;
             }
         } else {
            Log::Info << "... file not found. New table will be generated." << std::endl;
            return false;
         }         
    }

private:

	TableIzh *table;
	double curr_time;
    const std::string filename;
};



#endif
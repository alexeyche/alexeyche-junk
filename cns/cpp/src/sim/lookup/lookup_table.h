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
    
    void writeToFile(std::ofstream &fs) {
        fs.write(reinterpret_cast<const char*>(&dim_size), sizeof(dim_size));
        fs.write(reinterpret_cast<const char*>(&min), sizeof(min));
        fs.write(reinterpret_cast<const char*>(&max), sizeof(max));
        fs.write(reinterpret_cast<const char*>(&dx), sizeof(dx));
        fs.write(reinterpret_cast<const char*>(&it_size), sizeof(it_size));
        fs.write(reinterpret_cast<const char*>(&it_size_no_t), sizeof(it_size_no_t));
        fs.write(reinterpret_cast<const char*>(&name), sizeof(name));
    }
    void readFromFile(std::ifstream &fs) {
        fs.read(reinterpret_cast<char*>(&dim_size), sizeof(dim_size));
        fs.read(reinterpret_cast<char*>(&min), sizeof(min));
        fs.read(reinterpret_cast<char*>(&max), sizeof(max));
        fs.read(reinterpret_cast<char*>(&dx), sizeof(dx));
        fs.read(reinterpret_cast<char*>(&it_size), sizeof(it_size));
        fs.read(reinterpret_cast<char*>(&it_size_no_t), sizeof(it_size_no_t));
        fs.read(reinterpret_cast<char*>(&name), sizeof(name));
    }
    bool equals(const dim &other_dim) {
        if(dim_size != other_dim.dim_size) { return false; }
        if(min != other_dim.min) { return false; }
        if(max != other_dim.max) { return false; }
        if(dx != other_dim.dx) { return false; }
        if(it_size != other_dim.it_size) { return false; }
        if(it_size_no_t != other_dim.it_size_no_t) { return false; }
        if(name != other_dim.name) { return false; }
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
        unsigned int dims_size;
        dim *dims; 
        
        void writeToFile(std::ofstream &fs) {
            fs.write(reinterpret_cast<const char*>(&dims_size), sizeof(dims_size));
            for(size_t i=0; i<dims_size; i++) {
                dims[i].writeToFile(fs);
            }
        }
        void readFromFile(std::ifstream &fs) {
            fs.read(reinterpret_cast<char*>(&dims_size), sizeof(dims_size));
            dims = new dim[dims_size];
            for(size_t i=0; i<dims_size; i++) {
                dims[i].readFromFile(fs);
            }
        }
        bool equals(const fileHead &other_fileHead) {
            if (other_fileHead.dims_size != dims_size) { return false; }
            for(size_t i=0; i<dims_size; i++) {
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
    vec getSpaceVec(const std::string axis_name) {
        return linspace<vec>(dims[axis_name].min, dims[axis_name].max, dims[axis_name].dim_size);
    }
    void placeState(pointIzh p, state_type &state) {
        size_t Vi_it, ui_it, Ii_it, ti_it;
        //Log::Info << p.Vi << ":" << p.ui << ":" << p.Ii << ":" << p.ti << " = " << state[0] << std::endl;
        Vi_it = p.Vi * dims["V"].it_size;
        ui_it = p.ui * dims["u"].it_size;
        Ii_it = p.Ii * dims["I"].it_size;
        ti_it = p.ti * dims["t"].it_size;
        v[Vi_it + ui_it + Ii_it + ti_it] = state[0]; // putting V
        
        if(ti_it == dims["t"].dim_size-1)  { // very end of time
            Vi_it = p.Vi * dims["V"].it_size_no_t;
            ui_it = p.ui * dims["u"].it_size_no_t;
            Ii_it = p.Ii * dims["I"].it_size_no_t;
            //Log::Info << p.Vi << ":" << p.ui << ":" << p.Ii << " = " << state[1] << std::endl;
            u_fin[Vi_it + ui_it + Ii_it] = state[1]; // putting final u
        }            
    }
    void allocMemory() {
        v = (float*)malloc(sizeof(float) * dims_size);
        u_fin = (float*)malloc(sizeof(float) * dims_size_no_t);
    }
    fileHead* generateFileHeader() {
        fileHead* fh = new fileHead();
        fh->dims_size = dims_size;
        fh->dims = new dim[dims_size];
        for(size_t i=0; i>dims_ins_order.size(); i++) {
            const std::string cur_name = dims_ins_order[i];     
            fh->dims[i] = dims[cur_name];
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
        
        // table->addDim("V", 100, -105.0, 30.0); // V dim
        // table->addDim("u", 50, -15.5, 36.5); // u dim
        // table->addDim("I", 100, -70, 70); // I dim
        // table->addDim("t", 100, 0.0, 50.0); // t dim
        
        table->addDim("V", 100, -105.0, 30.0); // V dim
        table->addDim("u", 100, -15.5, 36.5); // u dim
        table->addDim("I", 50, -70, 70); // I dim
        table->addDim("t", 100, 0.0, 50.0); // t dim
        
        Log::Info << "Generating table with " << table->dims_size << " elements ... ";        
        table->allocMemory(); // allocate mem
           
        if(readFromFile()) {
            return;
        }
        
        Timer::Start("Table generation");
        
        vec V_space = table->getSpaceVec("V");
        vec u_space = table->getSpaceVec("u");
        vec I_space = table->getSpaceVec("I");
        vec t_space = table->getSpaceVec("t");

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
    void saveToFile() {
         std::ofstream bin_data(FILENAME, std::ios::out | std::ios::binary);
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
            memcpy(buff, &table->v[p], sizeof(float)*download_b);
            bin_data.write(reinterpret_cast<const char*>(buff),sizeof(float)*download_b);
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
            memcpy(buff, &table->u_fin[p], sizeof(float)*download_b);
            bin_data.write(reinterpret_cast<const char*>(buff),sizeof(float)*download_b);
            if(count<0) break;

            p += download_b;
         }
         bin_data.close();
    }
    bool readFromFile() {
         std::ifstream bin_data(FILENAME, std::ios::in | std::ios::binary);
         Log::Info << "Checking file " << FILENAME;
         if(bin_data.good()) {
             Log::Info << "... file found. Reading data ... " << std::endl;

             fileHead * fh_file = new fileHead();
             fh_file->readFromFile(bin_data);
             fileHead * fh_cur = table->generateFileHeader();
             if(fh_cur->equals(*fh_file)) {
                Log::Info << "Head file is proper. Loading table ... ";
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
                    bin_data.read(reinterpret_cast<char*>(buff), sizeof(float)*download_b);
                    memcpy(&table->v[p], buff, sizeof(float)*download_b);   
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
                    bin_data.read(reinterpret_cast<char*>(buff), sizeof(float)*download_b);
                    memcpy(&table->u_fin[p], buff, sizeof(float)*download_b);   
                    if(count<0) break;
                                 
                    p += download_b;
                }
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
	size_t getIndex(const std::string &axis_name, double value) {		
		return (size_t)(value - table->dims[axis_name].min)/table->dims[axis_name].dx;  // -100 -70
	}	
	TableIzh *table;
	double curr_time;
    const std::string filename;
};



#endif
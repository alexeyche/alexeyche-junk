#ifndef CONF_PARSER_H
#define CONF_PARSER_H

namespace srm {
    typedef std::map<std::string, std::vector<std::string> > TClassSet;
    class TStimSet {
    public:        
        TStimSet(std::string namec) : name(namec) {}
        void addClassSet(std::string class_name, std::string fname) {
            if(cs.find(class_name) == cs.end()) {
                cs[class_name] = std::vector<std::string>();
            }
            cs[class_name].push_back(fname);
        }
        std::string name;
        TClassSet cs;
    };

    std::string parseString(std::string &s, char delim, int num=1) {
        std::string out;
        for(size_t si=0; si<s.size(); si++) {
            if((s[si] == delim)||(si == s.size()-1)) {
                num--;
                if(num == 0) {
                    if((s[si] != delim) && (out.size() == s.size()-1)) {
                        return std::string();
                    }
                    if(s[si] != delim) { 
                        out.push_back(s[si]);
                    }                        
                    out.erase(std::remove(out.begin(),out.end(),' '),out.end());
                    return out;
                }                    
            } else {
                if(num == 1) {
                    out.push_back(s[si]);
                }                    
            }
        }
        return std::string();
    }
    struct TStimSets {
        double duration;    
        std::map<std::string, TStimSet> ss;
        int size() {
            return ss.size();
        }
    };
    struct TConfig {
        int nneurons_in;
        int nneurons_out;
        double neurons_in_start_weight;
        double neurons_out_start_weight;
        std::string stimuli_dir;
        std::string responces_dir;
        std::string models_dir;
        TStimSets sets;
        void print() {
            Log::Info << "========= config =========\n";
            Log::Info << "nneurons_in = " << nneurons_in << "\n";
            Log::Info << "nneurons_out = " << nneurons_out << "\n";
            Log::Info << "neurons_in_start_weight = " << neurons_in_start_weight << "\n";
            Log::Info << "neurons_out = " << neurons_out_start_weight << "\n";
            Log::Info << "stimuli_dir = " << stimuli_dir << "\n";
            Log::Info << "responces_dir = " << responces_dir << "\n";
            Log::Info << "models_dir = " << models_dir << "\n";
        }
    };
    std::string parseParam(std::string line, std::string paramName) {
        std::string param = parseString(line,'=');
        if(param == paramName) {
            return parseString(line, '=', 2);
        }
        return std::string();
    }
    void clearQuotes(std::string &s) {
        s.erase(std::remove(s.begin(),s.end(),'\"'),s.end());
        s.erase(std::remove(s.begin(),s.end(),'\''),s.end());
    }
    TConfig parseStimCfg(const std::string &stimCfg) {
        std::ifstream input(stimCfg);
        bool set_section = false;
        bool parse_set = false;
        TConfig c;
        std::string line;
        std::string current_set;
        int nsets=0;
        while(std::getline(input, line)) { 
            if(!set_section) {
                std::string mdir = parseParam(line, "models_dir");
                if(mdir != "") {
                    clearQuotes(mdir);
                    c.models_dir = mdir;
                    continue;
                }
                std::string nin = parseParam(line, "nneurons_in");
                if(nin != "") {
                    c.nneurons_in = atoi(nin.c_str());
                    continue;
                }
                std::string ninw = parseParam(line, "neurons_in_start_weight");
                if(ninw != "") {
                    c.neurons_in_start_weight = atof(ninw.c_str());
                    continue;
                }
                std::string noutw = parseParam(line, "neurons_out_start_weight");
                if(noutw != "") {
                    c.neurons_out_start_weight = atof(noutw.c_str());
                    continue;
                }
                std::string nout = parseParam(line, "nneurons_out");
                if(nout != "") {
                    c.nneurons_out = atoi(nout.c_str());
                    continue;
                }
                std::string stimdir = parseParam(line, "stimuli_dir");
                if(stimdir!= "") {
                    clearQuotes(stimdir);
                    c.stimuli_dir = stimdir;
                    continue;
                }
                std::string respdir = parseParam(line, "responces_dir");
                if(respdir != "") {
                    clearQuotes(respdir);
                    c.responces_dir = respdir;
                    continue;
                }
                std::string s = parseString(line, '{');           
                if(s == "sets") {
                    set_section = true;
                    continue;
                }
            }
            if(set_section) {
                std::string param = parseParam(line, "duration");
                if(param != "") {
                    c.sets.duration = atof(param.c_str());
                    continue;
                }
                std::string s = parseString(line, '{');
                if(s != "") {
                    if (!parse_set) {
                        s.erase(std::remove(s.begin(),s.end(),' '),s.end());
                        parse_set = true;
                        current_set = s;
                        if(c.sets.ss.find(s) != c.sets.ss.end()) {
                            Log::Fatal << "Double declatation of set " << s << "\n";
                            throw SrmException("");
                        }
                        c.sets.ss.insert(std::pair<std::string, TStimSet>(s, TStimSet(s)));
                        continue;
                    }
                }
                s = line;
                s.erase(std::remove(s.begin(),s.end(),' '),s.end());
                if(s == "}") {
                    if(parse_set) {
                        parse_set = false;
                        continue;
                    }
                }
                if(parse_set) {
                    s = parseString(line, ':');
                    if(s != "") { 
                        std::string fname = parseString(line, ':', 2);
                        if(fname != "") {
                            s.erase(std::remove(s.begin(),s.end(),' '),s.end());
                            fname.erase(std::remove(fname.begin(),fname.end(),' '),fname.end());
                            TStimSet &ss = c.sets.ss.at(current_set);
                            ss.addClassSet(s, fname);
                            nsets++;
                        } else {
                            throw SrmException("Need fname for class\n");
                        }
                    } else {
                        throw SrmException("Wrong format of config\n");
                    }
                }
            }                
        }               
        Log::Info << stimCfg << " was parsed, and " << nsets << " files was found for " << c.sets.size() << " sets\n";
        return c;
    }
};

#endif

#ifndef ENTROPY_H
#define ENTROPY_H

#define DIM_MAX 3 
#define NCOMP DIM_MAX+1

namespace srm {

    class EntropyCalc {
    struct CubaSett {
        CubaSett() : method("Vegas"), MinEval(0), MaxEval(50000), EpsRel(1e-3), EpsAbs(1e-12), FullInt(false), VerboseInt(false),// all default
                     NStart(1000), NIncrease(500), NBatch(1000), GridNo(0),  // Vegas default
                     NNew(1000), Flatness(25.),  // Suave default
                     Key1(47), Key2(1), Key3(1), MaxPass(5), MaxChiSq(10.), MinDeviation(.25), NGiven(0), LDXGiven("NDIM"), NExtra(0), // Divonne default
                     Key(0), // Cuhre default
                     NumEval(100) // trapezium default
                     {} 

        std::string method;
        int    MinEval;
        int    MaxEval;
        double EpsRel;
        double EpsAbs;
        bool FullInt;
        bool VerboseInt;

        int            NStart;          //Vegas
        int            NIncrease;       //Vegas
        int            NBatch;          //Vegas
        int            GridNo;          //Vegas

        int            NNew;            //Suave
        double         Flatness;        //Suave

        int            Key1;            //Divonne
        int            Key2;            //Divonne
        int            Key3;            //Divonne
        int            MaxPass;         //Divonne
        double         Border;          //Divonne
        double         MaxChiSq;        //Divonne
        double         MinDeviation;    //Divonne
        int            NGiven;          //Divonne
        std::string    LDXGiven;        //Divonne
        int            NExtra;          //Divonne
        
        int Key; // Cuhre       

        int NumEval; // trapezium
    };
    public:    
        EntropyCalc(SrmNeuron *neuron_v, double T0v, double Tmaxv) : neuron(neuron_v), T0(T0v), Tmax(Tmaxv), cuba_verbose(0) {
            const char *env_verb = getenv("CUBAVERBOSE");
            const char *method_ch = getenv("CUBA_METHOD");
            if(method_ch) cs.method = std::string(method_ch);
            const char *full_int = getenv("CUBA_FULLINT");
            if(full_int) cs.FullInt = true;
            const char *verb_int = getenv("VERBOSE_INT");
            if(verb_int) cs.VerboseInt = true;


            if( env_verb ) cuba_verbose = atoi(env_verb);
            const char *mineval = getenv("CUBA_MINEVAL");
            const char *maxeval = getenv("CUBA_MAXEVAL");
            const char *epsrel =  getenv("CUBA_EPSREl");
            const char *epsabs =  getenv("CUBA_EPSABS");
            if(mineval) cs.MinEval =  atoi(mineval);
            if(maxeval) cs.MaxEval =  atoi(maxeval);
            if(epsrel ) cs.EpsRel  =  atof(epsrel );
            if(epsabs ) cs.EpsAbs  =  atof(epsabs );

            if(cs.method == "Vegas") {
                const char *nstart = getenv("CUBA_NSTART");
                const char *nincrease = getenv("CUBA_NINCREASE");
                const char *nbatch = getenv("CUBA_NBATCH");
                const char *gridno = getenv("CUBA_GRIDNO");
                if(nstart) cs.NStart = atoi(nstart);
                if(nincrease) cs.NIncrease = atoi(nincrease);
                if(nbatch) cs.NBatch = atoi(nbatch);
                if(gridno) cs.GridNo = atoi(gridno);
            }
            if(cs.method == "Suave") {
                const char *nnew = getenv("CUBA_NNEW");
                const char *flatness = getenv("CUBA_FLATNESS");
                if(nnew) cs.NNew =  atoi(nnew);
                if(flatness ) cs.Flatness=  atof(flatness);            
            }
            if(cs.method == "Divonne") {
                const char *key1 = getenv("CUBA_KEY1");
                const char *key2 = getenv("CUBA_KEY2");
                const char *key3 = getenv("CUBA_KEY3");
                const char *maxpass = getenv("CUBA_MAXPASS");
                const char *border = getenv("CUBA_BORDER");
                const char *maxchisq = getenv("CUBA_MAXCHISQ");
                const char *mindeviation= getenv("CUBA_MINDEVIATION");
                const char *ngiven = getenv("CUBA_NGIVEN");
                const char *ldxgiven = getenv("CUBA_LDXGIVEN");
                const char *nextra  = getenv("CUBA_NEXTRA");

                if(key1)         cs.Key1            = atoi(key1);
                if(key2)         cs.Key2            = atoi(key2);
                if(key3)         cs.Key3            = atoi(key3);
                if(maxpass)      cs.MaxPass         = atoi(maxpass);
                if(border)       cs.Border          = atoi(border);
                if(maxchisq)     cs.MaxChiSq        = atof(maxchisq);
                if(mindeviation) cs.MinDeviation    = atof(mindeviation);
                if(ngiven)       cs.NGiven          = atoi(ngiven);
                if(ldxgiven)     cs.LDXGiven        = std::string(ldxgiven);
                if(nextra )      cs.NExtra          = atoi(nextra );
            }
            if(cs.method == "Cuhre") {
                const char *key = getenv("CUBA_KEY");
                if(key) cs.Key = atoi(key);
            }
            if(cs.method == "trapezium") {
                const char *numeval = getenv("NUMEVAL");
                if(numeval) cs.NumEval = atoi(numeval);
            }
        }
        ~EntropyCalc() { 
        }
//        double run(size_t n_calc);
        double run();
        void IntPerfomance();
        static int IntegrandFull(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata);
        static int Integrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata);
        static double entropy_fn_int(const double &fn, EntropyCalc *ec);
    private:        
        SrmNeuron* neuron;
        int n;
        int n_cur;
        double T0;
        double Tmax;
        int cuba_verbose;
        CubaSett cs;
    };

};    
#endif

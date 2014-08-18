

#include "constants.h"

#include <util/templates_clean.h>
#define T pLConst
#include <util/util_vector_tmpl.c>

Constants* createConstants(const char *filename) {
    Constants *c = (Constants*)malloc(sizeof(Constants));
    c->adex = (AdExConstants*) malloc( sizeof(AdExConstants) );
    c->res_stdp = (ResourceSTDPConstants*) malloc( sizeof(ResourceSTDPConstants) );
    c->tr_stdp = (TripleSTDPConstants*) malloc( sizeof(TripleSTDPConstants) );
    c->preproc = (PreprocessConstants*) malloc( sizeof(PreprocessConstants) );
    c->pacemaker = (PacemakerConstants*) malloc( sizeof(PacemakerConstants) );
    c->wta = (WtaConstants*) malloc( sizeof(WtaConstants) );
    c->lc = TEMPLATE(createVector,pLConst)();

    if (ini_parse(filename, file_handler, c) < 0) {
        printf("Can't load %s\n", filename);
        return(NULL);
    }
    c->__target_rate = c->target_rate/c->sim_dim;
    c->__pr = c->pr/c->sim_dim;
    c->wta->__max_freq = c->wta->max_freq/c->sim_dim;
    c->res_stdp->__Aplus_max_Amin = 1.0/max(c->res_stdp->Aplus, c->res_stdp->Aminus);
    c->tr_stdp->__Aminus_cube_delim_p_target = c->tr_stdp->Aminus * 1.0/(c->tr_stdp->p_target * c->tr_stdp->p_target * c->tr_stdp->p_target);
    c->tr_stdp->__Aplus = (c->tr_stdp->Aminus * c->tr_stdp->tau_minus)/(c->tr_stdp->p_target * c->tr_stdp->tau_plus * c->tr_stdp->tau_minus);
    for(size_t i=0; i<c->lc->size; i++) {
        if((getLayerConstantsC(c,i)->determ)&&(getLayerConstantsC(c,i)->learn)) {
            printf("Can't learn anything in determenistic mode\n");
            exit(1);
        }
    }

    return(c);
}
void deleteConstants(Constants *c) {
    TEMPLATE(deleteVector,pLConst)(c->lc);
    free(c->adex);
    free(c->res_stdp);
    free(c->preproc);
    free(c->tr_stdp);
    free(c);
}

indVector* indVectorParse(const char *vals) {
    indVector *v = TEMPLATE(createVector,ind)();
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {
        TEMPLATE(insertVector,ind)(v, atoi(token));
    }
    free(string);
    return(v);
}

doubleVector* doubleVectorParse(const char *vals) {
    doubleVector *v = TEMPLATE(createVector,double)();
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {
        TEMPLATE(insertVector,double)(v, atof(token));
    }
    free(string);
    return(v);
}             

pccharVector* pccharVectorParse(const char *vals) {
    pccharVector *v = TEMPLATE(createVector,pcchar)();
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {
        TEMPLATE(insertVector,pcchar)(v, strdup(token));
    }
    free(string);
    return(v);
}

neuron_layer_t neuronTypeParse(char *str) {
    if(strcmp(str, "PoissonLayer") == 0) {
        return(EPoissonLayer);
    }
    if(strcmp(str, "WtaLayer") == 0) {
        return(EWtaLayer);
    }
    if(strcmp(str, "AdaptLayer") == 0) {
        return(EAdaptLayer);
    }
    if(strcmp(str, "WtaAdaptLayer") == 0) {
        return(EWtaAdaptLayer);
    }
    printf("Can't do parse of neuron type: %s\n", str);
    exit(1);
}

prob_fun_t probFunTypeParse(char *str) {
//    EExpHennequin, ELinToyoizumi, EExpBohte, EExp
    if(strcmp(str, "ExpHennequin") == 0) {
        return(EExpHennequin);
    }
    if(strcmp(str, "LinToyoizumi") == 0) {
        return(ELinToyoizumi);
    }
    if(strcmp(str, "ExpBohte") == 0) {
        return(EExpBohte);
    }
    if(strcmp(str, "Exp") == 0) {
        return(EExp);
    }
    printf("Can't do parse of function type: %s\n", str);
    exit(1);
}

learning_rule_t learningRuleParse(char *str) {
    if(strcmp(str, "OptimalSTDP") == 0) {
        return(EOptimalSTDP);
    }
    if(strcmp(str, "ResourceSTDP") == 0) {
        return(EResourceSTDP);
    }
    if(strcmp(str, "SimpleSTDP") == 0) {
        return(ESimpleSTDP);
    }
    if(strcmp(str, "TripleSTDP") == 0) {
        return(ETripleSTDP);
    }
    printf("Can't do parse of learning rule: %s\n", str);
    exit(1);
}

bool boolParse(char *str) {
    if(strcmp(str, "true") == 0) {
        return(true);
    }
    if(strcmp(str, "false") == 0) {
        return(false);
    }
    printf("Can't do parse of boolean field: %s\n", str);
    exit(1);
}

#define FILL_LAYER_CONST(name,type) {       \
        type##Vector *v = type##VectorParse(value); \
        size_t i;\
        for(i=0; i<v->size; i++) {   \
            if(!checkLC(c,i, #name)) break;  \
            getLayerConstantsC(c,i)->name = v->array[i]; \
        }                                   \
        for( ; i < c->lc->size; i++) { \
            getLayerConstantsC(c,i)->name = v->array[v->size-1]; \
        }\
        TEMPLATE(deleteVector,type)(v);        \
    } \

#define FILL_LAYER_CONST_FUN(name,type,fun) {       \
        type##Vector *v = type##VectorParse(value); \
        size_t i; \
        for(i=0; i<v->size; i++) {   \
            if(!checkLC(c,i, #name)) break;  \
            getLayerConstantsC(c,i)->name = fun(v->array[i]); \
        }                                   \
        for( ; i < c->lc->size; i++) { \
            getLayerConstantsC(c,i)->name = fun(v->array[v->size-1]); \
        }\
        TEMPLATE(deleteVector,type)(v);     \
    } \

int file_handler(void* user, const char* section, const char* name, const char* value) {
    Constants* c = (Constants*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("preprocess", "mult")) {
        c->preproc->mult = atof(value);
    } else 
    if (MATCH("preprocess", "gain")) {
        c->preproc->gain = atof(value);
    } else 
    if (MATCH("preprocess", "sigma")) {
        c->preproc->sigma = atof(value);
    } else 
    if (MATCH("preprocess", "dt")) {
        c->preproc->dt = atof(value);
    } else 
    if (MATCH("srm neuron", "e0")) {
        c->e0 = atof(value);
    } else 
    if (MATCH("srm neuron", "e_exc")) {
        c->e_exc = atof(value);
    } else 
     if (MATCH("srm neuron", "e_inh")) {
        c->e_inh = atof(value);
    } else 
    if (MATCH("srm neuron", "ts")) {
        c->ts = atof(value);
    } else 
    if (MATCH("srm neuron", "tm")) {
        c->tm = atof(value);
    } else 
    if (MATCH("srm neuron", "tsr")) {
        c->tsr = atof(value);
    } else 
    if (MATCH("srm neuron", "r0")) {
        c->r0 = atof(value);
    } else 
    if (MATCH("srm neuron", "alpha")) {
        c->alpha = atof(value);
    } else 
    if (MATCH("srm neuron", "beta")) {
        c->beta = atof(value);
    } else 
    if (MATCH("srm neuron", "u_tr")) {
        c->u_tr = atof(value);
    } else 
    if (MATCH("srm neuron", "gain_factor")) {
        c->gain_factor = atof(value);
    } else 
    if (MATCH("srm neuron", "pr")) {
        c->pr = atof(value);
    } else 
    if (MATCH("srm neuron", "u_rest")) {
        c->u_rest = atof(value);
    } else 
    if (MATCH("srm neuron", "ta")) {
        c->ta = atof(value);
    } else 
    if (MATCH("srm neuron", "tr")) {
        c->tr = atof(value);
    } else 
    if (MATCH("srm neuron", "tb")) {
        c->tb = atof(value);
    } else     
    if (MATCH("srm neuron", "qa")) {
        c->qa = atof(value);
    } else 
    if (MATCH("srm neuron", "qr")) {
        c->qr = atof(value);
    } else 
    if (MATCH("srm neuron", "qb")) {
        c->qb = atof(value);
    } else 
    if (MATCH("sim", "dt")) {
        c->dt = atof(value);
    } else 
    if (MATCH("sim", "sim_dim")) {
        c->sim_dim = atof(value);
    } else 
    if (MATCH("sim", "epochs")) {
        c->epochs = atof(value);
    } else 
    if (MATCH("sim", "duration")) {
        c->duration = atof(value);
    } else 
    if (MATCH("sim", "seed")) {
        c->seed = atoi(value);
    } else 
    if (MATCH("sim", "target_neurons")) {
        c->target_neurons = strcmp(value, "true") == 0;
    } else 
    if (MATCH("sim", "M")) {
        c->M = atoi(value);
    } else 
    if (MATCH("layer", "neuron_type")) {
        FILL_LAYER_CONST_FUN(neuron_type,pcchar,neuronTypeParse)
    } else 
    if (MATCH("layer", "prob_fun")) {
        FILL_LAYER_CONST_FUN(prob_fun,pcchar,probFunTypeParse)
    } else 
    if (MATCH("layer", "learning_rule")) {
        FILL_LAYER_CONST_FUN(learning_rule,pcchar,learningRuleParse)
    } else 
    if (MATCH("layer", "learn")) {
        FILL_LAYER_CONST_FUN(learn,pcchar,boolParse)
    } else 
    if (MATCH("layer", "determ")) {
        FILL_LAYER_CONST_FUN(determ,pcchar,boolParse)
    } else 
    if (MATCH("layer", "N")) {
        FILL_LAYER_CONST(N,ind)
    } else 
    if (MATCH("layer", "lrate")) {
        FILL_LAYER_CONST(lrate,double)
    } else 
    if (MATCH("layer", "net_edge_prob")) {
        FILL_LAYER_CONST(net_edge_prob,double)
    } else 
    if (MATCH("layer", "input_edge_prob")) {
        FILL_LAYER_CONST(input_edge_prob,double)
    } else 
    if (MATCH("layer", "output_edge_prob")) {
        FILL_LAYER_CONST(output_edge_prob,double)
    } else 
    if (MATCH("layer", "weight_decay_factor")) {
        FILL_LAYER_CONST(weight_decay_factor,double)
    } else 
    if (MATCH("layer", "weight_per_neuron")) {
        FILL_LAYER_CONST(weight_per_neuron,double)
    } else 
    if (MATCH("layer", "inhib_frac")) {
        FILL_LAYER_CONST(inhib_frac,double)
    } else
    if (MATCH("layer", "ws")) {
        FILL_LAYER_CONST(ws,double)
    } else 
    if (MATCH("layer", "wmax")) {
        FILL_LAYER_CONST(wmax,double)
    } else 
    if (MATCH("layer", "aw")) {
        FILL_LAYER_CONST(aw,double)
    } else 
    if (MATCH("layer", "weight_var")) {
        FILL_LAYER_CONST(weight_var,double)
    } else 
    if (MATCH("layer", "syn_delays_gain")) {
        FILL_LAYER_CONST(syn_delays_gain,double)
    } else 
    if (MATCH("layer", "syn_delays_rate")) {
        FILL_LAYER_CONST(syn_delays_rate,double)
    } else 
    if (MATCH("layer", "axonal_delays_gain")) {
        FILL_LAYER_CONST(axonal_delays_gain,double)
    } else 
    if (MATCH("layer", "axonal_delays_rate")) {
        FILL_LAYER_CONST(axonal_delays_rate,double)
    } else 
    if (MATCH("layer", "ltd_factor")) {
        FILL_LAYER_CONST(ltd_factor,double)
    } else 
    if (MATCH("optimal stdp", "tc")) {
        c->tc = atof(value);
    } else 
    if (MATCH("optimal stdp", "mean_p_dur")) {
        c->mean_p_dur = atof(value);
    } else 
    if (MATCH("optimal stdp", "target_rate")) {
        c->target_rate = atof(value);
    } else 
    if (MATCH("optimal stdp", "target_rate_factor")) {
        c->target_rate_factor = atof(value);
    } else 
    if (MATCH("triple stdp", "A+")) {
        c->tr_stdp->Aplus = atof(value);
    } else 
    if (MATCH("triple stdp", "A-")) {
        c->tr_stdp->Aminus = atof(value);
    } else 
    if (MATCH("triple stdp", "tau+")) {
        c->tr_stdp->tau_plus = atof(value);
    } else 
    if (MATCH("triple stdp", "tau-")) {
        c->tr_stdp->tau_minus = atof(value);
    } else 
    if (MATCH("triple stdp", "tau_y")) {
        c->tr_stdp->tau_y = atof(value);
    } else 
    if (MATCH("triple stdp", "p_target")) {
        c->tr_stdp->p_target = atof(value);
    } else 
    if (MATCH("triple stdp", "tau_average")) {
        c->tr_stdp->tau_average = atof(value);
    } else 
    if (MATCH("resource stdp", "A+")) {
        c->res_stdp->Aplus = atof(value);
    } else 
    if (MATCH("resource stdp", "A-")) {
        c->res_stdp->Aminus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau+")) {
        c->res_stdp->tau_plus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau-")) {
        c->res_stdp->tau_minus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau_res")) {
        c->res_stdp->tau_res = atof(value);
    } else 
    if (MATCH("learn", "reinforcement")) {
        c->reinforcement = strcmp(value, "true") == 0;
    } else 
    if (MATCH("learn", "reward_baseline")) {
        c->reward_baseline = atof(value);
    } else 
    if (MATCH("learn", "trew")) {
        c->trew = atof(value);
    } else 
    if (MATCH("learn", "reward_ltp")) {
        c->reward_ltp = atof(value);
    } else 
    if (MATCH("learn", "tel")) {
        c->tel = atof(value);
    } else 
    if (MATCH("learn", "reward_ltd")) {
        c->reward_ltd = atof(value);
    } else 
    if (MATCH("learn", "p_set")) {
        c->p_set = atof(value);
    } else 
    if (MATCH("adex neuron", "C")) {
        c->adex->C = atof(value);
    } else 
    if (MATCH("adex neuron", "gL")) {
        c->adex->gL = atof(value);
    } else 
    if (MATCH("adex neuron", "EL")) {
        c->adex->EL = atof(value);
    } else 
    if (MATCH("adex neuron", "Vtr")) {
        c->adex->Vtr = atof(value);
    } else 
    if (MATCH("adex neuron", "slope")) { 
        c->adex->slope = atof(value);
    } else 
    if (MATCH("adex neuron", "tau_w")) {
        c->adex->tau_w = atof(value);
    } else 
    if (MATCH("adex neuron", "a")) {
        c->adex->a = atof(value);
    } else 
    if (MATCH("adex neuron", "b")) {
        c->adex->b = atof(value);
    } else
    if (MATCH("pacemaker", "amplitude")) {
        c->pacemaker->amplitude = atof(value);
    } else 
    if (MATCH("pacemaker", "cumulative_period_delta")) {
        c->pacemaker->cumulative_period_delta = atof(value);
    } else 
    if (MATCH("pacemaker", "frequency")) {
        c->pacemaker->frequency = atof(value);
    } else 
    if (MATCH("pacemaker", "pacemaker_on")) {
        c->pacemaker->pacemaker_on = strcmp(value, "true") == 0;
    } else 
    if (MATCH("wta", "max_freq")) {
        c->wta->max_freq = atof(value);
    } else 
    {
        return(0);
    } 
    return(1);
}

LayerConstants* getLayerConstantsC(Constants *c, size_t i) {
    return(c->lc->array[i]);
}

bool checkLC(Constants *c, size_t i, const char *field) {
    if(c->lc->size > i) return(true); 
    if (strcmp(field, "N") == 0) {
        LayerConstants *new_lc = (LayerConstants*) malloc(sizeof(LayerConstants));
        TEMPLATE(insertVector,pLConst)(c->lc, new_lc);
        return(true);
    }
    return(false);
}

void doublePrint(double v) {
    printf("%f\n",v);
}
void uintPrint(unsigned int v) {
    printf("%d\n",v);
}
void intPrint(int v) {
    printf("%d\n",v);
}
void boolPrint(bool v) {
    if(v) printf("true\n");
    else printf("false\n");
}

void AdExConstantsPrint(AdExConstants *c) {
    printf("==================\n");
    printf("C->"); doublePrint(c->C);
    printf("gL->"); doublePrint(c->gL);
    printf("EL->"); doublePrint(c->EL);
    printf("Vtr->"); doublePrint(c->Vtr);
    printf("slope->"); doublePrint(c->slope);
    printf("tau_w->"); doublePrint(c->tau_w);
    printf("a->"); doublePrint(c->a);
    printf("b->"); doublePrint(c->b);
    printf("==================\n");
}

void TripleSTDPConstantsPrint(TripleSTDPConstants *c) {
    printf("==================\n");
    printf("Aplus->"); doublePrint(c->Aplus);
    printf("Aminus->"); doublePrint(c->Aminus);
    printf("tau_plus->"); doublePrint(c->tau_plus);
    printf("tau_minus->"); doublePrint(c->tau_minus);
    printf("tau_y->"); doublePrint(c->tau_y);
    printf("p_target->"); doublePrint(c->p_target);
    printf("tau_average->"); doublePrint(c->tau_average);
    printf("==================\n");
}


void ResourceSTDPConstantsPrint(ResourceSTDPConstants *c) {
    printf("==================\n");
    printf("Aplus->"); doublePrint(c->Aplus);
    printf("Aminus->"); doublePrint(c->Aminus);
    printf("tau_plus->"); doublePrint(c->tau_plus);
    printf("tau_minus->"); doublePrint(c->tau_minus);
    printf("tau_res->"); doublePrint(c->tau_res);
    printf("==================\n");
}

void PacemakerConstantsPrint(PacemakerConstants *c) {
    printf("==================\n");
    printf("pacemaker_on->"); boolPrint(c->pacemaker_on);
    printf("frequency->"); doublePrint(c->frequency);
    printf("cumulative_period_delta->"); doublePrint(c->cumulative_period_delta);
    printf("amplitude->"); doublePrint(c->amplitude);
    printf("==================\n");
}

void PreprocessConstantsPrint(PreprocessConstants *c) {
    printf("==================\n");
    printf("gain->"); doublePrint(c->gain);
    printf("sigma->"); doublePrint(c->sigma);
    printf("dt->"); doublePrint(c->dt);
    printf("mult->"); doublePrint(c->mult);
    printf("==================\n");
}

void pLConstVectorPrint(pLConstVector *v) {
    printf("==================\n");
    for(size_t i=0; i<v->size; i++) {
        printf("LayerPoisson %zu\n", i);
        LayerConstantsPrint(v->array[i]);
    }
    printf("==================\n");
}

void size_tPrint(size_t v) {
    printf("%zu\n", v);
}

void learning_rule_tPrint(learning_rule_t v) {
    if(v == EResourceSTDP) {
        printf("ResourceSTDP\n");
    }
    if(v == EOptimalSTDP) {
        printf("OptimalSTDP\n");
    }
    if(v == ESimpleSTDP) {
        printf("SimpleSTDP\n");
    }
    if(v == ETripleSTDP) {
        printf("TripleSTDP\n");
    }
}

void neuron_layer_tPrint(neuron_layer_t v) {
    if(v == EPoissonLayer) {
        printf("PoissonLayer\n");
    }
    if(v == EWtaLayer) {
        printf("WtaLayer\n");
    }
    if(v == EAdaptLayer) {
        printf("AdaptLayer\n");
    }
    if(v == EWtaAdaptLayer) {
        printf("WtaAdaptLayer\n");
    }
}

void prob_fun_tPrint(prob_fun_t v) {
    if(v == EExpHennequin) {
        printf("ExpHennequin\n");
    }
    if(v == EExpBohte) {
        printf("ExpBohte\n");
    }
    if(v == EExp) {
        printf("Exp\n");
    }
    if(v == ELinToyoizumi) {
        printf("LinToyoizumi\n");
    }
}

void LayerConstantsPrint(LayerConstants *c) {
    printf("N->"); size_tPrint(c->N);
    printf("learning_rule->"); learning_rule_tPrint(c->learning_rule);
    printf("neuron_type->"); neuron_layer_tPrint(c->neuron_type);
    printf("prob_fun->"); prob_fun_tPrint(c->prob_fun);
    printf("learn->"); boolPrint(c->learn);
    printf("determ->"); boolPrint(c->determ);
    printf("net_edge_prob->"); doublePrint(c->net_edge_prob);
    printf("input_edge_prob->"); doublePrint(c->input_edge_prob);
    printf("output_edge_prob->"); doublePrint(c->output_edge_prob);
    printf("inhib_frac->"); doublePrint(c->inhib_frac);
    printf("weight_per_neuron->"); doublePrint(c->weight_per_neuron);
    printf("wmax->"); doublePrint(c->wmax);
    printf("weight_decay_factor->"); doublePrint(c->weight_decay_factor);
    printf("weight_var->"); doublePrint(c->weight_var);
    printf("lrate->"); doublePrint(c->lrate);
    printf("axonal_delays_rate->"); doublePrint(c->axonal_delays_rate);
    printf("axonal_delays_gain->"); doublePrint(c->axonal_delays_gain);
    printf("syn_delays_rate->"); doublePrint(c->syn_delays_rate);
    printf("syn_delays_gain->"); doublePrint(c->syn_delays_gain);
    printf("ws->"); doublePrint(c->ws);
    printf("aw->"); doublePrint(c->aw);
    printf("ltd_factor->"); doublePrint(c->ltd_factor);
}


// sed -ne '78,132p' ./snn_sim/constants.h | sed -e 's/unsigned int/uint/g' | tr -d ';*' | grep -Ev '^[ ]*$'  | awk '{print "    printf(\""$2"->\"); "$1"Print(c->"$2");"}'
void printConstants(Constants *c) {
    printf("e0->"); doublePrint(c->e0);
    printf("e_exc->"); doublePrint(c->e_exc);
    printf("e_inh->"); doublePrint(c->e_inh);
    printf("ts->"); doublePrint(c->ts);
    printf("tm->"); doublePrint(c->tm);
    printf("tsr->"); doublePrint(c->tsr);
    printf("alpha->"); doublePrint(c->alpha);
    printf("beta->"); doublePrint(c->beta);
    printf("u_tr->"); doublePrint(c->u_tr);
    printf("r0->"); doublePrint(c->r0);
    printf("tr->"); doublePrint(c->tr);
    printf("ta->"); doublePrint(c->ta);
    printf("tb->"); doublePrint(c->tb);
    printf("qr->"); doublePrint(c->qr);
    printf("qa->"); doublePrint(c->qa);
    printf("qb->"); doublePrint(c->qb);
    printf("gain_factor->"); doublePrint(c->gain_factor);
    printf("pr->"); doublePrint(c->pr);
    printf("__pr->"); doublePrint(c->__pr);
    printf("u_rest->"); doublePrint(c->u_rest);
    printf("tc->"); doublePrint(c->tc);
    printf("mean_p_dur->"); doublePrint(c->mean_p_dur);
    printf("duration->"); doublePrint(c->duration);
    printf("dt->"); doublePrint(c->dt);
    printf("sim_dim->"); doublePrint(c->sim_dim);
    printf("seed->"); uintPrint(c->seed);
    printf("target_neurons->"); boolPrint(c->target_neurons);
    printf("M->"); intPrint(c->M);
    printf("lc->"); pLConstVectorPrint(c->lc);
    printf("target_rate->"); doublePrint(c->target_rate);
    printf("__target_rate->"); doublePrint(c->__target_rate);
    printf("target_rate_factor->"); doublePrint(c->target_rate_factor);
    printf("epochs->"); intPrint(c->epochs);
    printf("p_set->"); doublePrint(c->p_set);
    printf("adex->"); AdExConstantsPrint(c->adex);
    printf("res_stdp->"); ResourceSTDPConstantsPrint(c->res_stdp);
    printf("tr_stdp->"); TripleSTDPConstantsPrint(c->tr_stdp);
    printf("preproc->"); PreprocessConstantsPrint(c->preproc);
    printf("reinforcement->"); boolPrint(c->reinforcement);
    printf("reward_ltd->"); doublePrint(c->reward_ltd);
    printf("reward_ltp->"); doublePrint(c->reward_ltp);
    printf("reward_baseline->"); doublePrint(c->reward_baseline);
    printf("tel->"); doublePrint(c->tel);
    printf("trew->"); doublePrint(c->trew);
    printf("pacemaker->"); PacemakerConstantsPrint(c->pacemaker);
}

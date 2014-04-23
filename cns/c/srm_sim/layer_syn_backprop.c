


void propagateSpikeLayerSynBackProp(LayerSynBackProp  *l, const size_t *ni, const SynSpike *sp, const Constants *c) {
    SRMLayer *lb = &l->base;
    propagateSpikeSRMLayer(lb, ni, sp ,c);

    lb->syn[*ni][ sp->syn_id ] *= l->a[*ni];
}


void simulateSRMLayerNeuron(LayerSynBackProp *l, const size_t *id_to_sim, const Constants *c) {
    SRMLayer *lb = &l->base;
    double u = c->u_rest;

    indLNode *act_node;
    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *id_to_sim ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        u += lb->W[ *id_to_sim ][ *syn_id ] * lb->syn[ *id_to_sim ][ *syn_id ];
    }
    if(!c->determ) {
        double p = probf(&u, c) * c->dt;
        double coin = getUnif();
        if( p > coin ) {
            lb->fired[ *id_to_sim ] = 1;
            lb->pacc[ *id_to_sim ] += 1;
            l->a[ *id_to_sim ] = 0;
//            printf("spike %zu! p: %f, pacc: %f\n", *id_to_sim, p, lb->pacc[ *id_to_sim ]);
        }
        
        if(c->learn) {
            lb->B[ *id_to_sim ] = B_calc( &lb->fired[ *id_to_sim ], &p, &lb->pacc[ *id_to_sim ], c);
            while( (act_node = TEMPLATE(getNextLList,ind)(lb->learn_syn_ids[ *id_to_sim ]) ) != NULL ) {
//            for(size_t con_i=0; con_i < lb->nconn[ *id_to_sim ]; con_i++) {
//                const size_t *syn_id = &con_i;
                const size_t *syn_id = &act_node->value;
//                if( (lb->C[ *id_to_sim ][ *syn_id ] == 0) && (lb->syn[ *id_to_sim ][ *syn_id ] == 0) ) continue;
                double dC = C_calc( &lb->fired[ *id_to_sim ], &p, &u, &lb->syn[ *id_to_sim ][ *syn_id ], c);
                lb->C[ *id_to_sim ][ *syn_id ] += -lb->C[ *id_to_sim ][ *syn_id ]/c->tc + dC;
//                printf("dC: %f C: %f, params: %d %f %f %f\n", dC, lb->C[ *id_to_sim ][ *syn_id ], lb->fired[ *id_to_sim ], p, u, lb->syn[ *id_to_sim ][ *syn_id ]);
                double lrate = rate_calc(&lb->W[ *id_to_sim ][ *syn_id ], c);
                
#if RATE_NORM == PRESYNAPTIC
                double dw = c->added_lrate*lrate*( lb->C[ *id_to_sim ][ *syn_id ]*lb->B[ *id_to_sim ] -  \
                                            c->weight_decay_factor * lb->syn_fired[ *id_to_sim ][ *syn_id ] * lb->W[ *id_to_sim ][ *syn_id ] );
#elif RATE_NORM == POSTSYNAPTIC                
                double dw = c->added_lrate*lrate*( lb->C[ *id_to_sim ][ *syn_id ]*lb->B[ *id_to_sim ] -  \
                                            3*c->weight_decay_factor * lb->fired[ *id_to_sim ] * lb->W[ *id_to_sim ][ *syn_id ] );
#endif               
                lb->W[ *id_to_sim ][ *syn_id ] += dw;
                
                if(lb->saveStat) {
                    TEMPLATE(insertVector,double)(lb->stat_W[ *id_to_sim ][ *syn_id ], lb->W[ *id_to_sim ][ *syn_id ]);
                    TEMPLATE(insertVector,double)(lb->stat_C[ *id_to_sim ][ *syn_id ], lb->C[ *id_to_sim ][ *syn_id ]);
                }
                
                
                if( (lb->C[ *id_to_sim ][ *syn_id ] < LEARN_ACT_TOL ) && (lb->C[ *id_to_sim ][ *syn_id ] > -LEARN_ACT_TOL ) && 
                    (dC < LEARN_ACT_TOL ) && (dC > -LEARN_ACT_TOL ) ) {

                    TEMPLATE(dropNodeLList,ind)(lb->learn_syn_ids[ *id_to_sim ], act_node);
                }
                if( isnan(dw) ) { 
                    printf("\nFound bad value\n");
                    printf("nid: %zu, p: %f, u: %f, B: %f, pacc: %f, C: %f, lrate: %f, W: %f, dw: %f\n", *id_to_sim, p, u, lb->B[ *id_to_sim ], lb->pacc[ *id_to_sim ], lb->C[ *id_to_sim ][ *syn_id ], lrate, lb->W[ *id_to_sim ][ *syn_id ], dw);
                    printf("C params: Yspike: %d, synapse: %f, dC: %f, p': %f\n", lb->fired[ *id_to_sim],lb->syn[ *id_to_sim ][ *syn_id ], dC, pstroke(&u,c));
                    exit(1);
                }
 
           }
        }

        lb->pacc[ *id_to_sim ] -= lb->pacc[ *id_to_sim ]/c->mean_p_dur; 

        if(lb->saveStat) {
            TEMPLATE(insertVector,double)(lb->stat_p[ *id_to_sim ], p);
            TEMPLATE(insertVector,double)(lb->stat_u[ *id_to_sim ], u);
            TEMPLATE(insertVector,double)(lb->stat_B[ *id_to_sim ], lb->B[ *id_to_sim ]);
        }

    } else { 
        if( u >= c->tr ) {
            lb->fired[ *id_to_sim ] = 1;
            l->a[ *id_to_sim ] = 0;
        }
        if(lb->saveStat) {
            TEMPLATE(insertVector,double)(lb->stat_u[ *id_to_sim ], u);
        }
    }

    while( (act_node = TEMPLATE(getNextLList,ind)(l->active_syn_ids[ *id_to_sim ]) ) != NULL ) {
        const size_t *syn_id = &act_node->value;
        lb->syn[ *id_to_sim ][ *syn_id ] -= lb->syn[ *id_to_sim ][ *syn_id ]/c->tm;
        if(lb->syn_fired[ *id_to_sim ][ *syn_id ] == 1) {
            lb->syn[ *id_to_sim ][ *syn_id ] *= l->a[*id_to_sim];
            lb->syn_fired[ *id_to_sim ][ *syn_id ] = 0; 
        }
        if( lb->syn[ *id_to_sim ][ *syn_id ] < SYN_ACT_TOL ) {
            TEMPLATE(dropNodeLList,ind)(l->active_syn_ids[ *id_to_sim ], act_node);
        }
    }
    l->a[ *id_to_sim ] += (1 - l->a[ *id_to_sim ])/c->ta;

}

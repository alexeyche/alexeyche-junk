


ucharVector* serialize_doubleVector(doubleVector *x) {
    ucharVector *v = TEMPLATE(createVector,uchar)();
    for(size_t i=0; i < x->size; i++) {
        unsigned char b[100]; // 100 bytes enough for everyone
        unsigned int nb = pack(b, 'd', x->array[vi]);
        for(size_t bi=0; bi < nb; bi++) {
            TEMPLATE(insertVector,uchar)(b[bi]);
        }
    }
    return(v);
}

unsigned char* serialize_indVector(doubleVector *x) {
    unsigned char* buffer = (unsigned char*) malloc( sizeof(double)*x->size);    
    size_t vi=0;
    for(size_t bi=0; bi < sizeof(double)*x->size; bi+=sizeof(double)) {
        serialize_double(buffer+bi, x->array[vi]);
        vi+=1;
    }
}

MInputSpikes* deserialize_MInputSpikes(const char *buff, size_t size) {
    MInputSpikes *m = (MInputSpikes*) malloc(sizeof(MInputSpikes));
    ucharVector* 
    return(m);
}

const char* serialize_MInputSpikes(MInputSpikes *m) {


}

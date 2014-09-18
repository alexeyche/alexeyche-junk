
#define INIT_SIZE 10



TVEC(T)* TEMPLATE(createVector,T)() {
    TVEC(T) *a = (TVEC(T)*)malloc(sizeof(TVEC(T)));
    a->array = (T*)malloc(INIT_SIZE * sizeof(T));
    a->size = 0;
    a->alloc_size = INIT_SIZE;
    return(a);
}

void TEMPLATE(insertVector,T)(TVEC(T) *a, T element) {
  if (a->size == a->alloc_size) {
    a->alloc_size *= 2;
    a->array = (T *)realloc(a->array, a->alloc_size * sizeof(T));
  }
  a->array[a->size++] = element;
}

void TEMPLATE(deleteVector,T)(TVEC(T) *a) {
#ifdef DESTRUCT
  for(size_t ai=0; ai<a->size; ai++) {
    DESTRUCT(a->array[ai]);
  }
#else
#ifdef DESTRUCT_METHOD
  for(size_t ai=0; ai<a->size; ai++) {
    a->array[ai]->DESTRUCT_METHOD(a->array[ai]);
  }
#endif
#endif  
  free(a->array);
  a->array = NULL;
  a->alloc_size = a->size = 0;
}

void TEMPLATE(deleteVectorNoDestroy,T)(TVEC(T) *a) {
  free(a->array);
  a->array = NULL;
  a->alloc_size = a->size = 0;
}

TVEC(T)* TEMPLATE(copyFromArray,T)(T *a, size_t size) {
    TVEC(T) *v = TEMPLATE(createVector,T)();
    for(size_t eli=0; eli<size; eli++) {
        TEMPLATE(insertVector,T)(v, a[eli]);
    }
    return(v);
}    

TVEC(T)* TEMPLATE(copyVector,T)(TVEC(T) *oldv) {
    TVEC(T) *v = TEMPLATE(createVector,T)();
    for(size_t eli=0; eli<oldv->size; eli++) {
        TEMPLATE(insertVector,T)(v, oldv->array[eli]);
    }
    return(v);
}

TVEC(T)* TEMPLATE(tailVector,T)(TVEC(T) *v, size_t n) {
    assert(n <= v->size);
    TVEC(T) *cutted_v = TEMPLATE(createVector,T)();
    for(size_t i = (v->size-1); i >= (v->size-n); i--) {
        TEMPLATE(insertVector,T)(cutted_v, v->array[i]);
    }
    return(cutted_v);
}

TVEC(T)* TEMPLATE(headVector,T)(TVEC(T) *v, size_t n) {
    assert(n <= v->size);
    TVEC(T) *cutted_v = TEMPLATE(createVector,T)();
    for(size_t i = 0; i<n; i++) {
        TEMPLATE(insertVector,T)(cutted_v, v->array[i]);
    }
    return(cutted_v);
}

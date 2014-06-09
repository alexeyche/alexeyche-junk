
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



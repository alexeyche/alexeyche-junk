#ifdef T


typedef struct {
  T *array;
  size_t alloc_size;
  size_t size;
} TVEC(T);


TVEC(T)* TEMPLATE(createVector,T)(); //size_t initialSize);
void TEMPLATE(insertVector,T)(TVEC(T) *a, T element);
void TEMPLATE(deleteVector,T)(TVEC(T) *a);
void TEMPLATE(deleteVectorNoDestroy,T)(TVEC(T) *a);
TVEC(T)* TEMPLATE(copyFromArray,T)(T *a, size_t size);
TVEC(T)* TEMPLATE(copyVector,T)(TVEC(T) *v);
TVEC(T)* TEMPLATE(tailVector,T)(TVEC(T) *v, size_t n);
TVEC(T)* TEMPLATE(headVector,T)(TVEC(T) *v, size_t n);
#endif

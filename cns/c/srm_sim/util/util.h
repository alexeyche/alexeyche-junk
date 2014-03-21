#ifdef T
  
#define TVEC(T) TNAME(T,Vector)


typedef struct {
  T *array;
  size_t used;
  size_t size;
} TVEC(T);


TVEC(T)* TEMPLATE(createVector,T)(size_t initialSize);
void TEMPLATE(insertVector,T)(TVEC(T) *a, T element);
void TEMPLATE(deleteVector,T)(TVEC(T) *a);

#endif

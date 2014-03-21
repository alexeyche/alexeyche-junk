

TVEC(T)* TEMPLATE(createVector,T)(size_t initialSize) {
    TVEC(T) *a = (TVEC(T)*)malloc(sizeof(TVEC(T)));
    a->array = (T*)malloc(initialSize * sizeof(T));
    a->size = 0;
    a->alloc_size = initialSize;
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
  free(a->array);
  a->array = NULL;
  a->alloc_size = a->size = 0;
}

#ifdef T

//struct TNAME(T,LNode);

typedef struct TNAME(T,LNode) {
    T value;    
    struct TNAME(T,LNode) *next;
    struct TNAME(T,LNode) *prev;
} TNAME(T,LNode);


typedef struct {
    struct TNAME(T,LNode) *first;
    struct TNAME(T,LNode) *last;
    struct TNAME(T,LNode) *current;
    size_t size;
} TNAME(T,LList);

TNAME(T,LList)* TEMPLATE(createLList,T)();
TNAME(T,LNode)* TEMPLATE(getFirstLList,T)(TNAME(T,LList) *a);
TNAME(T,LNode)* TEMPLATE(getNextLList,T)(TNAME(T,LList) *a);
void TEMPLATE(addValueLList,T)(TNAME(T,LList) *a, T value);
void TEMPLATE(dropNodeLList,T)(TNAME(T,LList) *a, TNAME(T,LNode) *a_curr);
void TEMPLATE(deleteLList,T)(TNAME(T,LList) *a);



#endif

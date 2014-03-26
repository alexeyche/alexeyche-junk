#ifndef TEMPLATES_H_
#define TEMPLATES_H_

#define CAT(X,Y) X##_##Y
#define CAT2(X,Y) X####Y
#define TNAME(X,Y) CAT2(X,Y)
#define TEMPLATE(X,Y) CAT(X,Y)

typedef size_t ind;

#define TVEC(T) TNAME(T,Vector)



#endif

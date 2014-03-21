#ifndef UTIL_GEN_H
#define UTIL_GEN_H


#include "templates.h"

#ifdef T
#undef T
#endif
#define T double
#include "util.h"

#ifdef T
#undef T
#endif
#define T ind
#include "util.h"

#endif

#include "core.h"
#include "templates.h"

#ifdef T
#undef T
#endif
#define T double
#include "util.c"

#ifdef T
#undef T
#endif
#define T ind
#include "util.c"

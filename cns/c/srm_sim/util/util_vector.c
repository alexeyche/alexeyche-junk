#include "core.h"
#include "templates.h"

#ifdef T
#undef T
#endif
#define T double
#include "util_vector_tmpl.c"

#ifdef T
#undef T
#endif
#define T ind
#include "util_vector_tmpl.c"
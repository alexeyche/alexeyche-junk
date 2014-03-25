#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <core.h>
#include <templates.h>
#include <layer.h>


#ifdef T
#undef T
#endif
#define T double
#include "util_vector_tmpl.h"

#ifdef T
#undef T
#endif
#define T ind
#include "util_vector_tmpl.h"

#ifdef T
#undef T
#endif
#define T pSRMLayer
#include "util_vector_tmpl.h"

#endif

#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <core.h>
#include <templates.h>
#include <string.h>

#include <templates_clean.h>
#define T double
#include "util_vector_tmpl.h"

#include <templates_clean.h>
#define T ind
#include "util_vector_tmpl.h"

void printDoubleVector(doubleVector *dv);
void printIndVector(indVector *v);

#endif

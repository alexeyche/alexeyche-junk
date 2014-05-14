#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <core.h>
#include <util/templates.h>
#include <string.h>

#include <util/templates_clean.h>
#define T double
#include "util_vector_tmpl.h"

#include <util/templates_clean.h>
#define T ind
#include "util_vector_tmpl.h"

#include <util/templates_clean.h>
#define T int
#include "util_vector_tmpl.h"

#include <util/templates_clean.h>
#define T uchar
#include "util_vector_tmpl.h"


void printDoubleVector(doubleVector *dv);
void printIndVector(indVector *v);

#endif

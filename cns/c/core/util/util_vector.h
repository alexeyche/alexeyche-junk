#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <core/core.h>
#include <core/util/templates.h>
#include <string.h>

#include <core/util/templates_clean.h>
#define T double
#include "util_vector_tmpl.h"

#include <core/util/templates_clean.h>
#define T ind
#include "util_vector_tmpl.h"

#include <core/util/templates_clean.h>
#define T int
#include "util_vector_tmpl.h"

#include <core/util/templates_clean.h>
#define T uchar
#include "util_vector_tmpl.h"

#include <core/util/templates_clean.h>
#define T pcchar
#include "util_vector_tmpl.h"


void printDoubleVector(doubleVector *dv);
void printIndVector(indVector *v);
int int_compare (const void * a, const void * b);
int double_compare(const void * a, const void * b);
int ind_compare(const void * a, const void * b);
void sortDoubleVector(doubleVector *v);
void sortIntVector(intVector *v);
void sortIndVector(indVector *v);



#endif

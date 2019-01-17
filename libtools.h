#ifndef _LIBTOOLS_
#define _LIBTOOLS_

#ifndef _TYPES_
#include "types.h"
#endif

#ifdef _USE_COMPLEX_
#include <complex>
#endif

#include <cstdio>
#include <vector>
#include <algorithm>

void dprint(char[], const char*, vector_t);
void dprint(FILE* , const char*, vector_t);

template <typename type>
type total(const array<type>*);

template <typename type>
type total(const type*, const size_t nelements);

template <typename type>
int transpose(array<type>&, array<type>&);

#endif

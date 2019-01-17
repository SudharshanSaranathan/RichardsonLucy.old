#ifndef _LIBFITS_
#define _LIBFITS_
#endif

#ifndef _TYPES_
#include "types.h"
#endif

#ifndef _USE_CFITSIO_
#define _USE_CFITSIO_
#include "fitsio.h"
#endif

#include <ctime>
#include <string>
#include <unistd.h>
#include <algorithm>

template <typename type>
class fitsio{
public:
  template <class hdrtype>
  static hdrtype hdr(const char*, const char*);
  static int read(const char* , array<type>*);
  static int write(const char*, array<type>*, bool clobber);
};

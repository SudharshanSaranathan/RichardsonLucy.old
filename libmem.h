#ifndef _LIBMEM_
#define _LIBMEM_

#ifndef _USE_FFTW_
#define _USE_FFTW_
#endif
#ifndef _USE_COMPLEX_
#define _USE_COMPLEX_
#endif

#include <cstdlib>

template <class type>
class memory {
public:
    static type* allocate(const std::size_t);
    static type** allocate(const std::size_t, const std::size_t);
    static type*** allocate(const std::size_t, const std::size_t, const std::size_t);
    static type**** allocate(const std::size_t, const std::size_t, const std::size_t, const std::size_t);

    static void deallocate(type*);
    static void deallocate(type**);
    static void deallocate(type***);
    static void deallocate(type****);
};

#endif

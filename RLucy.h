#ifndef _RLUCY_
#define _RLUCY_
#endif

#ifndef _TYPES_
#include "types.h"
#endif

#ifndef _CONFIG_
#include "config.h"
#endif

#ifndef _LIBFITS_
#include "libfits.h"
#endif

#ifndef _LIBTOOLS_
#include "libtools.h"
#endif

#include "fftw3.h"
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <complex>
#include <pthread.h>

class RLucy_pthread_iterate_arguments{
public:
  FILE *fp;
  short lambda;
  array<double> *psf;
  array<double> *psf_t;
  array<double> *stokes;

public:
   RLucy_pthread_iterate_arguments(){
    lambda = -1;
    fp     = nullptr;
    psf    = nullptr;
    psf_t  = nullptr;
    stokes = nullptr;
  };
  ~RLucy_pthread_iterate_arguments(){
    lambda = -1;
    fp     = nullptr;
    psf    = nullptr;
    psf_t  = nullptr;
    stokes = nullptr;
  };
};
typedef RLucy_pthread_iterate_arguments RL_Args_t;

class RLucy_convolution_resources{
public:
  array<double> *fftw_image_cropped, *fftw_image_shifted, *fftw_psf_padded;
  array<std::complex<double>> *fftw_image_fourier, *fftw_psf_fourier;
public:
  RLucy_convolution_resources(){
    fftw_psf_padded = NULL;
    fftw_image_cropped = NULL;
    fftw_image_shifted = NULL;
    fftw_image_fourier = NULL;
    fftw_psf_fourier = NULL;
  };
 ~RLucy_convolution_resources(){
    fftw_psf_padded = NULL;
    fftw_image_cropped = NULL;
    fftw_image_shifted = NULL;
    fftw_image_fourier = NULL;
    fftw_psf_fourier = NULL;
  };
};
typedef RLucy_convolution_resources RL_Conv_t;

class fftw_plans{
public:
  static fftw_plan forward;
  static fftw_plan reverse;
};

void *RLucy_pthread_iterate(void*);
int   RLucy_pthread_init(array<double>*, array<double>*, array<double>*, FILE*);
void  convolve(array<double>*, array<double>*, RL_Conv_t &);

#ifndef _RLUCY_
#include "RLucy.h"
#endif

#include "operators.h"
fftw_plan fftw_plans::forward = NULL;
fftw_plan fftw_plans::reverse = NULL;

int RLucy_pthread_init(array<double> *chunk, array<double>* space_variant_psf,\
                       array<double> *space_variant_psf_transpose, FILE *logfile){

  char DBUFFER[BUFSZ];
  vector_t chunkshape = chunk->dims();

  pthread_t threads[chunkshape[0]];
  RL_Args_t RL_Args[chunkshape[0]];

  for(int iter=0; iter < chunkshape[0]; iter++){
    RL_Args[iter].fp     = logfile;
    RL_Args[iter].lambda = iter;
    RL_Args[iter].psf    = space_variant_psf;
    RL_Args[iter].psf_t  = space_variant_psf_transpose;
    RL_Args[iter].stokes  = chunk;
    pthread_create(&threads[iter], nullptr, RLucy_pthread_iterate, (void*) &(RL_Args[iter]));
  }

  for(int iter=0; iter < chunkshape[0]; iter++)
    pthread_join(threads[iter], nullptr);

  return(EXIT_SUCCESS);
}

void *RLucy_pthread_iterate(void *argument){

  RL_Args_t *RL_Arg = (RL_Args_t*)argument;

  vector_t chunkshape = RL_Arg->stokes->dims();
  vector_t imageshape = vector_t(chunkshape.begin()+1, chunkshape.end());
  vector_t fftw_dims  = imageshape;

  fftw_dims[0] = ((RL_Arg->psf)->dims())[1];

  array<double> fftw_image_shifted; fftw_image_shifted.init(fftw_dims);
  array<double> fftw_image_cropped(fftw_image_shifted);
  array<double> fftw_psf_padded(fftw_image_shifted);

  array<std::complex<double>> fftw_image_fourier, fftw_psf_fourier;
  fftw_image_fourier.init(fftw_dims);
  fftw_psf_fourier.init(fftw_dims);

  RL_Conv_t Resources;
  Resources.fftw_image_shifted = &fftw_image_shifted;
  Resources.fftw_image_cropped = &fftw_image_cropped;
  Resources.fftw_image_fourier = &fftw_image_fourier;
  Resources.fftw_psf_padded = &fftw_psf_padded;
  Resources.fftw_psf_fourier = &fftw_psf_fourier;

  array<double> observed_image(imageshape);
  array<double> convergent_image(imageshape);
  memcpy(observed_image.data, RL_Arg->stokes->data3D[RL_Arg->lambda][0], sizeof(double)*observed_image.nelements());

  array<double> iterative_image(observed_image);

  for(int iter = 0; iter < config::LR_ITER; iter++){
    memcpy(convergent_image.data, iterative_image.data, sizeof(double)*iterative_image.nelements());
    convolve(&iterative_image, RL_Arg->psf, Resources);
    divide(&iterative_image, &observed_image);
    convolve(&iterative_image, RL_Arg->psf_t, Resources);
    multiply(&iterative_image, &convergent_image);
  }
  memcpy(RL_Arg->stokes->data3D[RL_Arg->lambda][0], iterative_image.data, sizeof(double)*convergent_image.nelements());  
  return nullptr;
}

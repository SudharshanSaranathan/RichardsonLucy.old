#ifndef _LIBCONVOLVE_
#define _LIBCONVOLVE_
#endif

#ifndef _RLUCY_
#include "RLucy.h"
#endif

#ifndef _LIBFITS_
#include "libfits.h"
#endif
#include <cstring>
#include "operators.h"

void convolve(array<double> *image, array<double> *psf, RL_Conv_t &Resources){

  int py  = psf->dims()[1]; int pcy = py/2;
  int pz  = psf->dims()[2]; int pcz = pz/2;
  int dy  = image->dims()[0]; int dcy = dy/2;
  int dz  = image->dims()[1]; int dcz = dz/2;

  array<double> image_placeholder;
  image_placeholder.init(image->dims());

  for(int i = 0; i < dy; i++){
    for(int j = -pcy; j<= pcy; j++)
      for(int k = 0; k < dz; k++)
        Resources.fftw_image_cropped->data2D[j+pcy][k] = image->data2D[(i+j+dy)%dy][k];

    for(int j = 0; j < py; j++)
      for(int k = (dcz-pcz); k <= (dcz+pcz); k++)
        Resources.fftw_psf_padded->data2D[j][k] = psf->data3D[i][j][k-(dcz-pcz)];

    fftw_execute_dft_r2c(fftw_plans::forward, Resources.fftw_image_cropped->data, reinterpret_cast<fftw_complex*>(Resources.fftw_image_fourier->data));
    fftw_execute_dft_r2c(fftw_plans::forward, Resources.fftw_psf_padded->data, reinterpret_cast<fftw_complex*>(Resources.fftw_psf_fourier->data));
    multiply(Resources.fftw_image_fourier, Resources.fftw_psf_fourier);
    fftw_execute_dft_c2r(fftw_plans::reverse, reinterpret_cast<fftw_complex*>(Resources.fftw_image_fourier->data), Resources.fftw_image_shifted->data);
    
    for(int j = 0; j < py; j++)
      for(int k = 0; k < dz; k++)
        (Resources.fftw_image_cropped->data2D)[j][k] = Resources.fftw_image_shifted->data2D[(j+pcy)%py][(k+dcz)%dz]/(py*dz);
    
    memcpy(image_placeholder.data2D[i], Resources.fftw_image_cropped->data2D[pcy], sizeof(double)*dz);  
  }
  *image = image_placeholder;

}

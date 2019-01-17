#include "libfits.h"
#include <algorithm>

template <typename type>
int fitsio<type>::read(const char *filename, array<type> *obs){

  // This routine reads a FITS file (passed as input) and stores it in an object of class array.

  /* It takes as arguments a filename, and a pointer to an object of class 'array'
     For a definition of class array, see types.h. The input file to read must conform to the FITS standard
  */

  /* The following are housekeeping variables, they go out of scope at function exit.
     Information on the data itself is stored in the object. Here's a brief explanation
     of what the variables mean:

     fitsfile *fileptr                  File pointer to point to fits file.
     char     DBUFFER[100]              Variable to store log messages.
     int      n_axis                    Number of axes of FITS file (2D, 3D, 4D ...)
     int      status                    Required by cfitsio library functions to store error codes (if any)
                                        Default value of status should be 0. Not initializing status to 0 results
                                        in an error always.
     int      nelements                 Volume of the FITS cube.
     vector_t Dimensions       Dimensions of each axes
     vector_t fpix             Position to start reading data values from
  */

  fitsfile *fileptr = nullptr;
  char DBUFFER[1024];
  int  n_axis = 0;
  int  status = 0;
  size_t   count = 1;
  vector_t fpix;
  vector_t dimensions;

  fits_open_file(&fileptr, filename, READONLY, &status);                  // Open the FITS file for reading
  if(status != 0){
    fits_report_error(stdout, status);
    comms.print(INFO, "FITSIO-Error: Cannot open file!", "fitsio::read():");
    return(EXIT_FAILURE);
  }

  fits_get_img_dim(fileptr, &n_axis, &status); dimensions.resize(n_axis); // Get the number of axes of the FITS file
  fits_get_img_size(fileptr, n_axis, (long int*)dimensions.data(), &status);         // Get the dimension of each axes
  if(status != 0){
    fits_report_error(stdout, status);
    comms.print(INFO, "FITSIO-Error: Cannot open file!", "fitsio::read():");
    return(EXIT_FAILURE);;
  }

  std::reverse(dimensions.begin(), dimensions.end());   // Dimensions are conventionally stored reversed for a FITS file
                                                        // One needs to reverse dimensions during FITS r/w to preserve
                                                        // compatibility with the way the memory allocation is performed.
                                                        // NOT REVERSING BREAKS THE CODE! So don't delete this line.

  obs->init(dimensions);                                // Initialize object (allocate memory) with the FITS dimensions
  if(obs->isAllocated() == false)                       // Memory allocation check
    return(EXIT_FAILURE);

  count = nelements(dimensions);                        // nelements() returns the volume of a dimensions cube
                                                        // nelements() is defined in libtools.h

  fpix.resize(n_axis); std::fill(fpix.begin(), fpix.end(), 1);
  fits_read_pix(fileptr, TDOUBLE, (long int*)fpix.data(), count, nullptr, obs->data, nullptr, &status);
  if(status != 0){
    fits_report_error(stdout, status);
    comms.print(INFO, "FITSIO-Error: Cannot open file!", "fitsio::read():");
    return(EXIT_FAILURE);;
  }
  fits_close_file(fileptr, &status);
  return(EXIT_SUCCESS);
}

template <typename type>
int fitsio<type>::write(const char *name, array<type>*obs, bool clobber){

  int status = 0;
  char DBUFFER[1024];
  fitsfile *fileptr = nullptr;

  long ndims;
  vector_t datashape;
  std::string filename(name);

  datashape = obs->dims();
  std::reverse(std::begin(datashape), std::end(datashape));
  if(name[0]=='\0' || name == nullptr || (!access(name, F_OK) && !clobber)){
    comms.print(INFO, "Warning: filename empty or in use, using system time", "fitsio::write()");
    time_t system_time = time(nullptr);
    tm *system_time_struct = localtime(&system_time);
    std::string day   = std::to_string(system_time_struct->tm_mday);
    std::string mon   = std::to_string(++system_time_struct->tm_mon);
    std::string year  = std::to_string(system_time_struct->tm_year+1900);
    std::string hour  = std::to_string(system_time_struct->tm_hour);
    std::string minute  = std::to_string(system_time_struct->tm_min);
    std::string second  = std::to_string(system_time_struct->tm_sec);
    filename = "FITS." + day + '.' + mon + '.' + year + '.' + hour + ':' + minute + ':' + second + ".fits";
  }

  clobber==true ? filename = "!" + filename : filename;
  sprintf(DBUFFER, "Writing to file: %s", filename.c_str());
  comms.print(INFO, DBUFFER, "\nfitsio::write():");

  fits_create_file(&fileptr, filename.c_str(), &status);
  if(status != 0){
    fits_report_error(stdout, status);
    comms.print(INFO, "FITSIO-Error: Unable to create file", "fitsio::write()");
    return(EXIT_FAILURE);
  }
  fits_create_img(fileptr, -64, datashape.size(), (long int*)datashape.data(), &status);
  if(status != 0){
    fits_report_error(stdout, status);
    comms.print(INFO, "FITSIO-Error: Unable to create image", "fitsio::write()");
    return(EXIT_FAILURE);
  }
  switch(datashape.size()){
      case 1:
        fits_write_img(fileptr, TDOUBLE, 1, obs->nelements(), obs->data, &status);
        if(status != 0){
          fits_report_error(stdout, status);
          comms.print(INFO, "FITSIO-Error: Unable to write image", "fitsio::write()");
          return(EXIT_FAILURE);
        }
        break;
      case 2:
        fits_write_img(fileptr, TDOUBLE, 1, obs->nelements(), obs->data, &status);
        if(status != 0){
          fits_report_error(stdout, status);
          comms.print(INFO, "FITSIO-Error: Unable to write image", "fitsio::write()");
          return(EXIT_FAILURE);
        }
        break;
      case 3:
        fits_write_img(fileptr, TDOUBLE, 1, obs->nelements(), obs->data, &status);
        if(status != 0){
          fits_report_error(stdout, status);
          comms.print(INFO, "FITSIO-Error: Unable to write image", "fitsio::write()");
          return(EXIT_FAILURE);
        }
        break;
      case 4:
        fits_write_img(fileptr, TDOUBLE, 1, obs->nelements(), obs->data, &status);
        if(status != 0){
          fits_report_error(stdout, status);
          comms.print(INFO, "FITSIO-Error: Unable to write image", "fitsio::write()");
          return(EXIT_FAILURE);
        }
        break;
      default:
        comms.print(INFO, "Dimension-Error: Unsupported dimensions", "fitsio::write()");
        break;
    }

    sprintf(DBUFFER, "FITS file: %s", filename.c_str());
    comms.print(INFO, DBUFFER, "fitsio::write():");
    sprintf(DBUFFER, "Dimensions: %ld %ld %ld", datashape[2], datashape[1], datashape[0]);
    comms.print(INFO, DBUFFER, "fitsio::write():");

    fits_close_file(fileptr, &status);
    return EXIT_SUCCESS;
}

template class fitsio<double>;

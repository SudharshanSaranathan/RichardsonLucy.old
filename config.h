#ifndef _CONFIG_
#define _CONFIG_
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#define BUFSZ 1024

class config{
public:

  static std::string DATA_FILE;
  static std::string PSF_FILE;
  static std::string OUTPUT_FILE;
  static std::string FFTWISDOM;

  static unsigned int DATA_RANK;
  static unsigned int PSF_RANK;
  static unsigned int THREADS;
  static unsigned int LAMBDAL;
  static unsigned int LAMBDAU;
  static unsigned int LR_ITER;

  static bool SAVE;
  static bool CLOBBER;

  config(){}
 ~config(){}
  static int parse(const char*);
};

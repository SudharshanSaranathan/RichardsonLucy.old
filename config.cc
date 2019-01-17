#include "config.h"

std::string config::DATA_FILE       = "input.fits";
std::string config::PSF_FILE        = "psf.fits";
std::string config::OUTPUT_FILE     = "output.fits";
std::string config::FFTWISDOM       = "fftw.wisdom";

unsigned int config::DATA_RANK      = 3;
unsigned int config::PSF_RANK       = 3;
unsigned int config::THREADS        = 1;
unsigned int config::LAMBDAL	      = 0;
unsigned int config::LAMBDAU        = -1;
unsigned int config::LR_ITER        = 10;
bool config::SAVE                   = true;
bool config::CLOBBER                = true;

int config::parse(const char* filename){
  unsigned int counter = 0;
  std::ifstream infile(filename);
  std::string line;
  if(!infile)
    return(EXIT_FAILURE);

  while(std::getline(infile, line)){
    std::stringstream tokens(line);
    std::string key, value;
    std::getline(tokens, key, ':');
    tokens >> std::ws;
    std::getline(tokens, value, ':');
    if(key == "INPUT_FILE"){
      counter++;
      config::DATA_FILE = value;
    }else if(key == "PSF_FILE"){
      counter++;
      config::PSF_FILE = value;
    }else if(key == "OUTPUT_FILE"){
      counter++;
      if(value == "")
        config::SAVE = false;
      else
        config::SAVE = true;
      config::OUTPUT_FILE = value;
    }else if(key == "FFTWISDOM"){
        counter++;
        config::FFTWISDOM = value;
    }else if(key == "THREADS"){
      counter++;
      config::THREADS = std::stoi(value);
    }else if(key == "LAMBDAL"){
      counter++;
      config::LAMBDAL = std::stoi(value);
    }else if(key == "LAMBDAU"){
      counter++;
      config::LAMBDAU = std::stoi(value);
    }else if(key == "LR_ITER"){
      counter++;
      config::LR_ITER = std::stoi(value);
    }else if(key == "SAVE"){
      counter++;
      config::SAVE = (value == "Y");
    }else if(key == "CLOBBER"){
      counter++;
      config::CLOBBER = (value == "Y");
    }else if(key == "DATA_RANK"){
      counter++;
      config::DATA_RANK = std::stoi(value);
    }else if(key == "PSF_RANK"){
      counter++;
      config::PSF_RANK = std::stoi(value);
    }
  }
  return EXIT_SUCCESS;
}

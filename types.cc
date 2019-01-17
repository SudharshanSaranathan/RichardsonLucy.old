#include <cstring>

#include "types.h"
#include "libmem.h"

communications::communications(){
  info_file_pointer = nullptr;
  error_file_pointer = nullptr;
  ErrorFlag = false;
}
communications::communications(const char* InfoLog, const char *ErrorLog){
  info_file_pointer = fopen(InfoLog, "wb");
  error_file_pointer = fopen(ErrorLog, "wb");
  if(!info_file_pointer)
    info_file_pointer = stdout;
  if(!error_file_pointer)
    error_file_pointer = stdout;
  ErrorFlag = false;
}
communications::~communications(){
  fclose(info_file_pointer);
  fclose(error_file_pointer);
  ErrorFlag = false;
}

bool communications::IsError(){
  return ErrorFlag;
}
void communications::print(stream STREAM, const char* message, const char* function){

  strcpy(msg, message);
  strcpy(callback, function);

  FILE *file_pointer;
  if(STREAM == ERROR){
    file_pointer = error_file_pointer;
    ErrorFlag = true;
  }
  else if(STREAM == INFO)
    file_pointer = info_file_pointer;
  else if(STREAM == CONSOLE)
    file_pointer = stdout;
  else if(STREAM == BLANK)
    file_pointer = fopen("/dev/null", "wb");
  else
    file_pointer = nullptr;

  if(file_pointer){
    fseek(file_pointer, 0, SEEK_END);
    fprintf(file_pointer, "%s", function);
    fprintf(file_pointer, "\t\t\t%s\n", message);
    fflush(file_pointer);
  }
}
void communications::print(stream STREAM, const char* message){

  strcpy(msg, message);
  strcpy(callback, "\0");

  FILE *file_pointer;
  if(STREAM == ERROR){
    file_pointer = error_file_pointer;
    ErrorFlag = true;
  }
  else if(STREAM == INFO)
    file_pointer = info_file_pointer;
  else if(STREAM == CONSOLE)
    file_pointer = stdout;
  else if(STREAM == BLANK)
    file_pointer = fopen("/dev/null", "wb");
  else
    file_pointer = nullptr;

  if(file_pointer){
    fseek(file_pointer, 0, SEEK_END);
    fprintf(file_pointer, "%s", message);
    fflush(file_pointer);
  }
}

communications comms("INFO.comms", "ERROR.comms");

template <typename type>
type   nelements(const type* array, const size_t dims){
  type   rvalue = 1;
  size_t index = 0;
  while(index < dims){
    rvalue *= array[index];
    index++;
  }
  return rvalue;
}
size_t nelements(const vector_t vector){
  size_t rvalue = 1, index = 0;
  while(index < vector.size()){
    rvalue *= vector[index];
    index++;
  }
  return rvalue;
}

/*----------------------------------------------------------------------------*/

template <typename type>
array<type>::array(){
  status = false;
  number = 1;
  data   = nullptr;
  data1D = nullptr;
  data2D = nullptr;
  data3D = nullptr;
  data4D = nullptr;
}

template <typename type>
array<type>::~array(){

  memory<type>::deallocate(data1D);                                             // Better to use functions provided by libmem.h to deallocate
  memory<type>::deallocate(data2D);                                             // They won't throw an exception if the pointer being
  memory<type>::deallocate(data3D);                                             // freed is already nullptr. They will, however, throw an
  memory<type>::deallocate(data4D);                                             // exception if the pointer is dangling.

  if(data != nullptr)
    data = nullptr;

}

template <typename type>
array<type>::array(const vector_t &vector){

  status = false;
  data   = nullptr;
  data1D = nullptr;
  data2D = nullptr;
  data3D = nullptr;
  data4D = nullptr;

  number = ::nelements(vector);
  dimensions = vector;                                                          // Copy the input_dimensions vector to this->dimensions.
  switch(dimensions.size()){
    case 1:                                                                     // STREAM within the switch-case block is identical to the one
      data1D = memory<type>::allocate(dimensions[0]);                           // in the Copy constructor. Refer to it for comments.
      if(data1D != nullptr){
        data = data1D;
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 2:
      data2D = memory<type>::allocate(dimensions[0], dimensions[1]);
      if(data2D != nullptr){
        data = data2D[0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 3:
      data3D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2]);
      if(data3D != nullptr){
        data = data3D[0][0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 4:
      data4D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2], dimensions[3]);
      if(data4D != nullptr){
        data = data4D[0][0][0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    default:
      comms.print(INFO, "Init-Error: Expected <= 4 dimensions", "array::init(std::vector, type*)");
      dimensions.clear();
      break;
  }
}

template <typename type>
array<type>::array(const array<type> &copy_from){                               // Copy constructor

  status = false;
  data   = nullptr;
  data1D = nullptr;
  data2D = nullptr;
  data3D = nullptr;
  data4D = nullptr;

  dimensions = copy_from.dimensions;                                            // Copy the input_dimensions vector to this->dimensions.
  switch(dimensions.size()){
    case 1:                                                                   // STREAM within the switch-case block is identical to the one
      data1D = memory<type>::allocate(dimensions[0]);                         // in the Copy constructor. Refer to it for comments.
      if(data1D != nullptr){
        data = data1D;
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 2:
      data2D = memory<type>::allocate(dimensions[0], dimensions[1]);
      if(data2D != nullptr){
        data = data2D[0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 3:
      data3D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2]);
      if(data3D != nullptr){
        data = data3D[0][0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    case 4:
      data4D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2], dimensions[3]);
      if(data4D != nullptr){
        data = data4D[0][0][0];
        status = true;
      }else{
        comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
        dimensions.clear();
      }
      break;
    default:
      comms.print(INFO, "Init-Error: Expected <= 4 dimensions", "array::init(std::vector, type*)");
      dimensions.clear();
      break;
  }
  if(status){
    number = ::nelements(this->dimensions);
    memcpy(this->data, copy_from.data, sizeof(double)*copy_from.nelements());
  }
}

template <typename type>
void array<type>::init(const vector_t in_dims){                                 // Initializes memory and pointers for an array in the requested shape

  if(this->data != NULL){                                                           // Will only initialize when called from a freshly created instance.
    comms.print(INFO, "Init-Error: Cannot initialize allocated object", "array::init():");
  }else{
    number = ::nelements(in_dims);
    dimensions = in_dims;                                                       // Copy the input_dimensions vector to this->dimensions.
    switch(dimensions.size()){
      case 1:                                                                   // STREAM within the switch-case block is identical to the one
        data1D = memory<type>::allocate(dimensions[0]);                         // in the Copy constructor. Refer to it for comments.
        if(data1D != nullptr){
          data = data1D;
          status = true;
        }else{
          comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
          dimensions.clear();
        }
        break;
      case 2:
        data2D = memory<type>::allocate(dimensions[0], dimensions[1]);
        if(data2D != nullptr){
          data = data2D[0];
          status = true;
        }else{
          comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
          dimensions.clear();
        }
        break;
      case 3:
        data3D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2]);
        if(data3D != nullptr){
          data = data3D[0][0];
          status = true;
        }else{
          comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
          dimensions.clear();
        }
        break;
      case 4:
        data4D = memory<type>::allocate(dimensions[0], dimensions[1], dimensions[2], dimensions[3]);
        if(data4D != nullptr){
          data = data4D[0][0][0];
          status = true;
        }else{
          comms.print(INFO, "Memory-Error: Allocation failed!", "array::init():");
          dimensions.clear();
        }
        break;
      default:
        comms.print(INFO, "Init-Error: Expected <= 4 dimensions", "array::init(std::vector, type*)");
        dimensions.clear();
        break;
    }
  }
}

template <typename type>
size_t array<type>::nelements() const{                                          // Returns the number of elements in the data array
  size_t rvalue = 1, index = 0;                                                           // Nothing complicated here.
  for(size_t j = 0; j < this->dimensions.size(); j++){
    rvalue *= this->dimensions[j];
  }
  return rvalue;
}

template <typename type>
vector_t array<type>::dims() const{                                             // Returns dimensions vector of instance.
  return this->dimensions;
}

template <typename type>
bool array<type>::isAllocated() const{                                          // Allocation status - does the instance have valid allocated memory already?
  return this->status;
}

template <class type>
void swap(array<type>* objA, array<type>* objB){                                // Swap two instances of the class - more comments below

  bool temporary_status = objA->status;
  objA->status = objB->status;
  objB->status = temporary_status;

  vector_t temporary_dimensions = objA->dimensions;
  objA->dimensions = objB->dimensions;
  objB->dimensions = temporary_dimensions;

  type *tdata, *tdata1D, **tdata2D, ***tdata3D, ****tdata4D;
  tdata   = objA->data;
  tdata1D = objA->data1D;
  tdata2D = objA->data2D;
  tdata3D = objA->data3D;
  tdata4D = objA->data4D;
  objA->data   = objB->data;
  objA->data1D = objB->data1D;
  objA->data2D = objB->data2D;
  objA->data3D = objB->data3D;
  objA->data4D = objB->data4D;
  objB->data   = tdata;
  objB->data1D = tdata1D;
  objB->data2D = tdata2D;
  objB->data3D = tdata3D;
  objB->data4D = tdata4D;
}

template void swap(array<double>*, array<double>*);

template class array<double>;                                                   // Template instantiations - removing this will cause linker errors
template class array<complex_t>;
template void swap(array<complex_t>*, array<complex_t>*);

#include "types.h"
#include "libtools.h"

void dprint(char BUFFER[], const char* msg, vector_t dimensions){
  sprintf(BUFFER, msg);
  for(int i=0; i<dimensions.size(); i++){
    sprintf(BUFFER, " %d", dimensions[i]);
  }
  sprintf(BUFFER, "\n");
}

void dprint(FILE *file_pointer, const char* msg, vector_t dimensions){
  fprintf(file_pointer, msg);
  for(int i=0; i<dimensions.size(); i++){
    fprintf(file_pointer, " %d", dimensions[i]);
  }
  fprintf(file_pointer, "\n");
  fflush(file_pointer);
}

template <typename type>
type total(const array<type> *data){
  type sum = 0;
  for(size_t j = 0; j < data->nelements(); j++)
    sum += data->data[j];
  return sum;
}

template <typename type>
type total(const type *data, const size_t nelements){
  type sum = 0;
  for(size_t j = 0; j < nelements; j++)
    sum += data[j];
  return sum;
}

template <typename type>
int transpose(array<type> &obj, array<type> &obj_transpose){

  vector_t shape = obj.dims();
  vector_t shape_transpose(shape);   // The shape of a transposed array cannot be the original shape, of course.
                                              // But this is only an initialization, the values are reversed in the
                                              // switch statement below.

  if(obj_transpose.isAllocated() == true){
    comms.print(INFO, "Init-Error: Uninitialized argument expected, array not transposed", "transpose():");
    return(EXIT_FAILURE);
  }

  if(!comms.IsError()){
    switch(shape.size()){
      case 2:
        std::reverse(shape_transpose.begin(), shape_transpose.end());     // Here is where the shape vector is reversed
        obj_transpose.init(shape_transpose);
        if(obj_transpose.data2D != nullptr){
          for(size_t k = 0; k < shape[0]; k++){
            for(size_t l = 0; k < shape[1]; k++){
              obj_transpose.data2D[l][k] = obj.data2D[k][l];
            }
          }
        }
        break;
      case 3:
        std::reverse(shape_transpose.begin()+1, shape_transpose.end());   // Here is where the shape vector is reversed
        obj_transpose.init(shape_transpose);
        if(obj_transpose.data3D != nullptr){
          for(size_t j = 0; j < shape[0]; j++){
            for(size_t k = 0; k < shape[1]; k++){
              for(size_t l = 0; l < shape[2]; l++){
                obj_transpose.data3D[j][l][k] = obj.data3D[j][k][l];
              }
            }
          }
        }
        break;
      case 4:
        std::reverse(shape_transpose.begin()+1, shape_transpose.end());   // Here is where the shape vector is reversed
        obj_transpose.init(shape_transpose);
        if(obj_transpose.data4D != nullptr){
          for(size_t i = 0; i < shape[0]; i++){
            for(size_t j = 0; j < shape[1]; j++){
              for(size_t k = 0; k < shape[2]; k++){
                for(size_t l = 0; l < shape[3]; l++){
                  obj_transpose.data4D[i][j][l][k] = obj.data4D[i][j][k][l];
                }
              }
            }
          }
        }
        break;
      default:
        comms.print(INFO, "Dimension-Error: Dimension <= 4 expected, array not transposed", "transpose():");
        return(EXIT_FAILURE);
        break;
    }
  }
}


//-----------------------------------------------------------------------------------
// TEMPLATE INSTANTIATIONS - Do not remove the following lines to avoid linker errors
//-----------------------------------------------------------------------------------

template double total(const array<double>*);
template double total(const double*, const size_t);
template complex_t total(const array<complex_t>*);

template int transpose(array<double>&, array<double>&);

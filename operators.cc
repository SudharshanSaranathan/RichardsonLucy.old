#ifndef _TYPES_
#include "types.h"
#endif

#include <complex>
#include <iostream>

template <typename type>
array<type>& array<type>::operator=(array<type> instance){
  swap(this, &instance);
  return *this;
}

template <typename type>
array<type> array<type>::operator+(const array<type> &obj_B) const{
  array<type> temp(obj_B);
  if(this->dimensions != obj_B.dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "array<type>::operator+():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }
  for(size_t j = 0; j < obj_B.nelements(); j++)
    temp.data[j] = this->data[j] + obj_B.data[j];

  return temp;;
}

template <typename type>
array<type> array<type>::operator-(const array<type> &obj_B) const{
  array<type> temp(obj_B);
  if(this->dimensions != obj_B.dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "array<type>::operator+():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }
  for(size_t j = 0; j < obj_B.nelements(); j++)
    temp.data[j] = this->data[j] - obj_B.data[j];

  return temp;;
}

template <typename type>
array<type> array<type>::operator*(const array<type> &obj_B) const{
  array<type> temp(obj_B);
  if(this->dimensions != obj_B.dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "array<type>::operator+():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }
  for(size_t j = 0; j < obj_B.nelements(); j++)
    temp.data[j] = this->data[j] * obj_B.data[j];

  return temp;;
}

template <typename type>
array<type> array<type>::operator/(const array<type> &obj_B) const{
  array<type> temp(obj_B);
  if(this->dimensions != obj_B.dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "array<type>::operator/():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }

  for(size_t j = 0; j < obj_B.nelements(); j++){
    if(obj_B.data[j] == 0.)
      throw std::overflow_error("Divide by zero encountered, operator/()");

    temp.data[j] = this->data[j] / obj_B.data[j];
  }
  return temp;;

}

template <typename type>
void multiply(array<type> *obj_A, array<type> *obj_B){

  if(obj_A->dims() != obj_B->dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "multiply():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }
  for(size_t j = 0; j < obj_B->nelements(); j++)
      obj_A->data[j] = obj_A->data[j]*obj_B->data[j];
  
}

void divide(array<double> *obj_A, array<double> *obj_B){

  if(obj_A->dims() != obj_B->dims()){
    comms.print(INFO, "Dimension-Error: Unequal array dimensions", "divide():");
    throw std::logic_error("Dimension-Error: Unequal array dimensions");
  }
  for(size_t j = 0; j < obj_B->nelements(); j++)
      obj_A->data[j] = obj_B->data[j] / obj_A->data[j];

}

template class array<double>;
template class array<complex_t>;
template void multiply(array<double>*, array<double>*);
template void multiply(array<complex_t>*, array<complex_t>*);

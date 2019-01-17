#include "libmem.h"

template <class type>
type* memory<type>::allocate(const std::size_t xs){
  type *data = new type[xs]();
  return data;
}

template <class type>
type** memory<type>::allocate(const std::size_t xs, const std::size_t ys){
  type** data = new type*[xs]();
  if(data == nullptr)
    return nullptr;

  data[0] = new type[xs*ys]();
  if(data[0] == nullptr){
    delete[] data;
    return nullptr;
  }

  for(int i=1; i<xs; i++){
    data[i] = data[i-1] + ys;
  }
  return data;
}

template <class type>
type*** memory<type>::allocate(const std::size_t xs, const std::size_t ys, const std::size_t zs){
  type ***data = new type**[xs]();
  if(data == nullptr)
    return nullptr;

  data[0] = new type*[xs*ys]();
  if(data[0] == nullptr){
    delete[] data;
    return nullptr;
  }

  data[0][0] = new type[xs*ys*zs]();
  if(data[0][0] == nullptr){
    delete[] data[0];
    delete[] data;
    return nullptr;
  }

  for(int i=0; i<xs; i++){
    *(data+i) = *data + i*ys;
    for(int j=0; j<ys; j++)
      *(*(data+i)+j) = **data + i*ys*zs + j*zs;
  }
  return data;
}

template <class type>
type**** memory<type>::allocate(const std::size_t xs, const std::size_t ys, const std::size_t zs, const std::size_t ws){
  type ****data = new type***[xs]();
  if(data==nullptr)
    return(nullptr);

  data[0] = new type**[xs*ys]();
  if(data[0]==nullptr){
    delete[] data;
    return nullptr;
  }

  data[0][0] = new type*[xs*ys*zs]();
  if(data[0][0]==nullptr){
    delete[] data[0];
    delete[] data;
    return(nullptr);
  }

  data[0][0][0] = new type[xs*ys*zs*ws]();
  if(data[0][0][0] == nullptr){
    delete[] data[0][0];
    delete[] data[0];
    delete[] data;
    return nullptr;
  }

  for(int i=0; i<xs; i++){
    *(data+i) = *data + i*ys;
    for(int j=0; j<ys; j++){
      *(*(data+i)+j) = **data + i*ys*zs + j*zs;
      for(int k=0; k<zs; k++){
        *(*(*(data+i)+j)+k) = ***data + i*ys*zs*ws + j*zs*ws + k*ws;
      }
    }
  }
  return data;
}

template <class type>
void memory<type>::deallocate(type ****data){
  if(data){
    if(data[0]){
      if(data[0][0]){
        if(data[0][0][0]){
          delete[] data[0][0][0];
        }
        delete[] data[0][0];
      }
      delete[] data[0];
    }
    delete[] data;
  }
  data = nullptr;
}

template <class type>
void memory<type>::deallocate(type ***data){
  if(data){
    if(data[0]){
      if(data[0][0]){
        delete[] data[0][0];
      }
      delete[] data[0];
    }
    delete[] data;
  }
  data = nullptr;
}

template <class type>
void memory<type>::deallocate(type **data){
  if(data){
    if(data[0]){
      delete[] data[0];
    }
    delete[] data;
  }
  data = nullptr;
}

template <class type>
void memory<type>::deallocate(type *data){
  if(data != nullptr){
    delete[] data;
  }
  data = nullptr;
}

template class memory<double>;
template class memory<long>;
template class memory<int>;

#include <complex>
template class memory<std::complex<double>>;

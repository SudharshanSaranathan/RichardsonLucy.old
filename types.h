/*------------------------------------------------------------------------------
AUTHOR:       Sudharshan Saranathan
DATESTAMP:    27/09/2018, 19:00
------------------------------------------------------------------------------*/

#ifndef _TYPES_
#define _TYPES_
#endif

#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <complex>

/*------------------------------------------------------------------------------
  Convenience definitions of frequently used datatypes
------------------------------------------------------------------------------*/

typedef unsigned int  uint;
typedef unsigned long ulong;
typedef unsigned char uchar;
typedef std::size_t   size_t;
typedef std::string   string_t;

typedef std::vector<size_t>  vector_t;
typedef std::complex<double> complex_t;

enum stream {INFO, ERROR, CONSOLE, BLANK};
//------------------------------------------------------------------------------

class communications {
private:
  FILE *info_file_pointer;
  FILE *error_file_pointer;
  bool  ErrorFlag;
  char  msg[200];
  char  callback[200];
public:
  communications();
  communications(const char*, const char*);
 ~communications();
public:
  bool IsError();
  void print(stream, const char*, const char*);
  void print(stream, const char*);
};

extern communications comms;

template <typename type>
type   nelements(const type*, const size_t);
size_t nelements(const vector_t);

template <typename type>
class array {

/*------------------------------------------------------------------------------


------------
INTRODUCTION
------------

This class has been designed for handling multi-dimensional, dynamically allocated
arrays of multiple types.

-------
MEMBERS
-------

Members are:
----------------------------
vector_t dimensions
bool      status;
type*     data
type*     data1D
type**    data2D
type***   data3D
type****  data4D
----------------------------

What's the std::vector for?
The vector stores array dimensions. You can read it from array::dims(),
see below. The vector is READ-ONLY.

What is *data for?
This classes uses functions declared in libmem.h, which ALWAYS allocates
contiguous memory, irrespective of the dimension. *data is a pointer which
points to this chunk of contiguous memory. *data is all data.

Note:
If you want to do global operations, like add a value to all elements in the
array, or add two arrays of the same dimensions, you only need one for loop:

  for( i < number of elements ){
    class_instance.data[i] += value;
  }

What are the other *data1D, **data2D ... pointers for?
Pointers to pointers, pointers to pointers to pointers ... , are additionally
allocated to make accessing specific elements convenient. The details of how
this is done are in libmem.cc. Simply put, if you asked for allocation of a 3D
array with an instance, you can access the data like this:

  instance.data3D[i][j][k];


In function comments, I'll always refer to type *data as the data-pointer and
the other pointers as convenience-pointers.

Caution:
I would have liked to make the data private too, but providing additional
get/set methods makes data access too cumbersome, especially in the context of
parallelization with MPI or multi-threading. I hereby hold a placard with
huge inscribed letters - DON'T DO SOMETHING STUPID WITH THE DATA POINTERS.

 - Don't allocate them yourself.
 - Don't deallocate them yourself.
 - Don't fiddle with their values/layout.
 - Use them only to access and set values.

Memory management is handled by constructors/destructors and other auxilliary
functions. They have been provided precisely to save you headache. USE THEM.

-------
METHODS - Only the public methods
-------

size():             Returns the number of elements in the array

alloc_status():     Returns true if a given instance has valid allocated memory

dims():             Returns the dimensions vector.

init(std::vector):  Initializes memory to handle an array whose dimensions
                    are specified in the std::vector argument. Set the second
                    argument to NULL if you haven't already allocated memory for
                    and stored the data elsewhere. If you have, you can provide
                    it here, and all pointers will point to that location instead.

                    The purpose of this second functionality is to allow easy
                    communication during parallelization. You don't have to
                    worry about serialization. Just send the vector and *data
                    which are already contiguous. On the receiving end, you can
                    initialize a new instance with the address of the buffer.

                    Caution:
                    init() does not "copy" data from the address specified in
                    the second argument. It only makes the pointers point to
                    that address.

                    init() can only be called on a fresh instance. If the
                    instance already has data, init() will not erase and reset
                    the state of the instance.

operator=(array):   Overloaded assignment operator. Uses the copy-swap idiom.

------------------------------------------------------------------------------*/

private:
  bool     status;                                        //  Allocation status of the arrays - KEEP IT PRIVATE
  size_t   number;                                        //  Total number of elements in the array - KEEP IT PRIVATE
  vector_t dimensions;                                    //  Dimensions of the allocated arrays - KEEP IT PRIVATE

public:                                                   //  Pointers to the data array - public to allow easy access
  type *data;
  type *data1D;
  type **data2D;
  type ***data3D;
  type ****data4D;

public:                                                   //  Constructors section
  array();                                                //  Constructor       - Initializes all pointers to NULL
 ~array();                                                //  Destructor        - Frees memory pointed to by data pointers (if allocated - requires libmem.h/libmem.cc)
  array(const vector_t &);                                //  Constructor       - Takes the input dimensions vector and allocates an array accordingly
  array(const array<type>&);                              //  Copy Constructor  - Copies the state of another instantiated object (It's a deep copy)

public:                                                   //  Convenience functions
  bool isAllocated() const;                               //  Returns allocation status of the instance (true if data is pointing to some valid memory location)
  void init(const vector_t);                     //  Initialize/allocate memory for the data-pointer and convenience pointers in the requested shape
  size_t nelements() const;                               //  Returns total number of elements in the data array
  vector_t dims() const;                                  //  Returns the shape of the data array

  template <class stype>
  friend void swap(array<stype>*, array<stype>*);

  array<type>& operator=(array<type>);                     //  Overloaded assignment operator

  array<type> operator+(const array<type>&) const;        //  Overloaded addition operator
  array<type> operator-(const array<type>&) const;        //  Overloaded subtraction operator
  array<type> operator*(const array<type>&) const;        //  Overloaded multiplication operator
  array<type> operator/(const array<type>&) const;        //  Overloaded division operator

};

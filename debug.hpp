// -*- C++ -*-
#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

///
/// @file debug.hpp
/// @brief Debugging utility
///
/// $Id$
///

template <class T_array, class T_shape>
void debug_write(const char* filename, int dsize, T_array &array,
                 T_shape lbound, T_shape ubound)
{
  std::ofstream fp(filename, common::binary_append);

  switch(array.rank()) {
  case 1: // 1D
    for(int i0=lbound[0]; i0 <= ubound[0] ;i0++) {
      fp.write(reinterpret_cast<const char*>(&array(i0)), dsize);
    }
    break;
  case 2: // 2D
    for(int i0=lbound[0]; i0 <= ubound[0] ;i0++) {
      for(int i1=lbound[1]; i1 <= ubound[1] ;i1++) {
        fp.write(reinterpret_cast<const char*>(&array(i0,i1)), dsize);
      }
    }
    break;
  case 3: // 3D
    for(int i0=lbound[0]; i0 <= ubound[0] ;i0++) {
      for(int i1=lbound[1]; i1 <= ubound[1] ;i1++) {
        for(int i2=lbound[2]; i2 <= ubound[2] ;i2++) {
          fp.write(reinterpret_cast<const char*>(&array(i0,i1,i2)), dsize);
        }
      }
    }
    break;
  default:
    fprintf(stderr, "unable to handle a given dimension\n");
  }

  fp.flush();
  fp.close();
}

template <class T>
void debug_write_array_data(std::ofstream &ofs, T* ptr, size_t size)
{
  ofs.write(reinterpret_cast<const char*>(ptr), sizeof(T)*size);
}


template <class T_array>
void debug_write_array(T_array &array, const char* filename)
{
  size_t isize = sizeof(array.ndim);
  std::ofstream ofs;

  ofs.open(filename, common::binary_append);

  ofs.write(reinterpret_cast<const char*>(&array.ndim), isize);
  ofs.write(reinterpret_cast<const char*>(&array.dims[0]), isize*array.ndim);

  debug_write_array_data(ofs, array.ptr, array.size);

  ofs.close();
  ofs.flush();
}

#ifdef _DEBUG_
#define DEBUG_WRITE_ARRAY((array), (filename)) \
                          debug_write_array(array, filename)
#define DEBUG_EXIT((status)) exit(status)
#endif

// Local Variables:
// c-file-style   : "gnu"
// c-file-offsets : ((innamespace . 0) (inline-open . 0))
// End:
#endif

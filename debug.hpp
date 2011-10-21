// -*- C++ -*-
#ifndef _DEBUG_HPP_
#define _DEBUG_HPP_

///
/// @file debug.hpp
/// @brief Debugging utility
///
/// $Id$
///

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

// -*- C++ -*-
#ifndef _COMMON_H_
#define _COMMON_H_

///
/// Common Module
///
/// Author: Takanobu AMANO <amanot@stelab.nagoya-u.ac.jp>
/// $Id$
///
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include "boost/format.hpp"
#include "config.hpp"

namespace common
{
// common variables
//@{
// mathematical constants
const float64 pi  = M_PI;   ///< pi
const float64 pi2 = 2*M_PI; ///< 2 pi
const float64 pi4 = 4*M_PI; ///< 4 pi

// utility constants
const float64 TOLERANCE = 1.0e-08; ///< default tolerance
const float64 EPSILON   = 1.0e-15; ///< machine epsilon
const float64 NORMMIN   = 1.0e-12; ///< minimum norm (for matrix solvers)

// binary mode
const std::ios::openmode binary_write =
  std::ios::binary | std::ios::out | std::ios::trunc;
const std::ios::openmode binary_append =
  std::ios::binary | std::ios::out | std::ios::app;
const std::ios::openmode binary_read =
  std::ios::binary | std::ios::in;

// text mode
const std::ios::openmode text_write  = std::ios::out | std::ios::trunc;
const std::ios::openmode text_append = std::ios::out | std::ios::app;
const std::ios::openmode text_read   = std::ios::in;
//@}

// common functions
//@{
/// return elapsed time in second
inline double etime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1.0e-6;
}

/// return maximum
template <class T>
inline T maximum(T a, T b)
{
  return (a > b) ? a : b;
}

/// return minimum
template <class T>
inline T minimum(T a, T b)
{
  return (a < b) ? a : b;
}
//@}

}

// Local Variables:
// c-file-style   : "gnu"
// c-file-offsets : ((innamespace . 0) (inline-open . 0))
// End:
#endif

// -*- C++ -*-
#ifndef _MPIUTILS_HPP_
#define _MPIUTILS_HPP_

///
/// MPI utlility module for three dimensional domain decomposition
///
/// Author: Takanobu AMANO <amanot@stelab.nagoya-u.ac.jp>
/// $Id$
///
#define MPICH_IGNORE_CXX_SEEK
#include "common.hpp"
#include <mpi.h>
using namespace common;

//
// configuration of MPI library
//
#if   defined (OPEN_MPI)
//
// Open MPI
//
// Version 1.3.2 or below does not define MPI::INTEGER8 constant,
// which is MPI 2 standard.
//
#if OMPI_HAVE_FORTRAN_INTEGER8
namespace MPI
{
const Datatype INTEGER8 = MPI_INTEGER8;
}
#endif

#elif defined (MPICH2)
//
// MPICH 2
//
// MPICH_IGNORE_CXX_SEEK should be defined before including mpi.h
// for use of MPI 2 standard.
//

#endif

///
/// @class mpiutils mpiutils.hpp
/// @brief utility class for domain decomposition using MPI
///
/// This class is an implementation of singleton that means it must have only
/// one instance during execution. You should first initialize() and
/// finalize() at last. These corresponds to MPI::Init() and MPI::Finalize()
/// respectively, with some additional procedures.
/// Methods implemented here are all static, meaning that you can use them
/// like global functions.
///
/// Stream buffers std::cout and std::cerr are redirected to local files for
/// each PEs and by default, these are concatenated to original buffer when
/// finalize() is called.
///
class mpiutils
{
private:
  static mpiutils *instance; ///< unique instance
  static int tag[3][2];      ///< tag for MPI communications

  // cartesian topology communicator
  MPI::Cartcomm m_cart;

  // process, rank and neighbors
  int m_nprocess;         ///< number of process
  int m_thisrank;         ///< rank of current process
  int m_proc_dim[3];      ///< number of process in each dir.
  int m_rank_dim[3];      ///< rank of current process in eash dir.
  int m_nb_dim[3][2];     ///< neighbor process
  int m_coord[3];         ///< carteisan topology coordinate

  // for stdout/stderr
  bool            m_concat; ///< flag for concatenate cerr/cout
  std::string     m_outf;   ///< dummy standard output file
  std::string     m_errf;   ///< dummy standard error file
  std::ofstream  *m_out;    ///< dummy standard output
  std::ofstream  *m_err;    ///< dummy standard error
  std::streambuf *m_errbuf; ///< buffer of original cerr
  std::streambuf *m_outbuf; ///< buffer of original cout

  // remain undefined
  mpiutils();
  mpiutils(const mpiutils &);
  mpiutils& operator=(const mpiutils &);

  /// constructor
  mpiutils(int argc, char** argv,
           int domain[3], bool period[3], bool concat)
  {
    // initialize
    MPI::Init(argc, argv);

    m_nprocess = MPI::COMM_WORLD.Get_size();
    m_thisrank = MPI::COMM_WORLD.Get_rank();
    m_concat   = concat;

    // check consistency between argument and nprocess
    if( domain[0]*domain[1]*domain[2] != m_nprocess ) {
      if( m_thisrank == 0 ) {
        std::cerr <<
          boost::format("Error in mpiutils: invalid number of PEs (=%4d)\n"
                        "current domain decomposition = "
                        "[%3d, %3d, %3d] ===> expected PEs = %4d\n")
          % m_nprocess % domain[0] % domain[1] % domain[2]
          % (domain[0]*domain[1]*domain[2]);
      }
      // exit with error
      MPI::Finalize();
      exit(-1);
    }

    // three-dimensional domain decomposition
    {
      m_proc_dim[0] = domain[0];
      m_proc_dim[1] = domain[1];
      m_proc_dim[2] = domain[2];
      // cartesian topology
      m_cart = MPI::COMM_WORLD.Create_cart(3, domain, period, 0);
      m_cart.Get_topo(3, domain, period, m_rank_dim);
      // set neighbors
      m_cart.Shift(0, +1, m_nb_dim[0][0], m_nb_dim[0][1]);
      m_cart.Shift(1, +1, m_nb_dim[1][0], m_nb_dim[1][1]);
      m_cart.Shift(2, +1, m_nb_dim[2][0], m_nb_dim[2][1]);
      // coordinate
      m_cart.Get_coords(m_thisrank, 3, m_coord);
    }

    // open dummy standard error stream
    m_errf   = (boost::format("%s_PE%04d.stderr") % argv[0] % m_thisrank).str();
    m_err    = new std::ofstream(m_errf.c_str());
    m_errbuf = std::cerr.rdbuf(m_err->rdbuf());

    // open dummy standard output stream
    m_outf   = (boost::format("%s_PE%04d.stdout") % argv[0] % m_thisrank).str();
    m_out    = new std::ofstream(m_outf.c_str());
    m_outbuf = std::cout.rdbuf(m_out->rdbuf());
  }

  /// destructor
  ~mpiutils()
  {
    // close dummy stndard output
    m_out->flush();
    m_out->close();
    std::cout.rdbuf(m_outbuf);
    delete m_out;

    // close dummy standard error
    m_err->flush();
    m_err->close();
    std::cerr.rdbuf(m_errbuf);
    delete m_err;

    if( m_concat ) {
      // concatenate output stream
      concatenate_stream(m_outf, std::cout, 0, "stdout");
      concatenate_stream(m_errf, std::cerr, 1, "stderr");
    }

    // finalize
    MPI::Finalize();

    // set null
    instance = 0;
  }

  /// concatenate sorted stream
  void concatenate_stream(std::string &filename, std::ostream &dst,
                          const int tag, const char *label)
  {
    MPI::Request r;
    int dummy = 0;
    int nprocess = instance->m_nprocess;
    int thisrank = instance->m_thisrank;

    // recieve from the previous
    if( thisrank > 0 ) {
      r = MPI::COMM_WORLD.Irecv(&dummy, 1, MPI::INTEGER4, thisrank-1, tag);
      r.Wait();
    }

    // output
    std::ofstream f(filename.c_str(), std::ios::in);
    dst << boost::format("--- begin %s from PE =%4d ---\n") % label % thisrank;
    if( f.rdbuf()->in_avail() != 0 ) dst << f.rdbuf();
    dst << boost::format("--- end   %s from PE =%4d ---\n") % label % thisrank;
    dst.flush();

    // remove file
    std::remove(filename.c_str());

    // send to the next
    if( thisrank == nprocess-1 ) return;
    MPI::COMM_WORLD.Isend(&dummy, 1, MPI::INTEGER4, thisrank+1, tag);
  }

public:
  /// initialize MPI call
  static mpiutils* initialize(int argc, char** argv,
                              int domain[3], bool period[3],
                              bool concat=true)
  {
    if( instance == 0 ) {
      // create instance
      instance = new mpiutils(argc, argv, domain, period, concat);
    }
    return instance;
  }

  /// finalize MPI call
  static void finalize()
  {
    if( instance != 0 ) delete instance;
  }

  /// get unique instance
  static mpiutils* getInstance()
  {
    if( instance ) return instance;
    // report error
    std::cerr << "Error in getInstance() !"
              << std::endl
              << "===> initialization method should be called in advance."
              << std::endl;
    return 0;
  }

  /// get number of process
  static int getNProcess()
  {
    return instance->m_nprocess;
  }

  /// get rank
  static int getThisRank()
  {
    return instance->m_thisrank;
  }

  /// get coordinate
  static void getCoord(int coord[3])
  {
    coord[0] = instance->m_coord[0];
    coord[1] = instance->m_coord[1];
    coord[2] = instance->m_coord[2];
  }

  /// get neighbors
  static void getNeighbors(int neighbors[3][2])
  {
    for(int dir=0; dir < 3 ;dir++)
      for(int i=0; i < 2 ;i++)
        neighbors[dir][i] = instance->m_nb_dim[dir][i];
  }

  /// get wall clock time
  static double getTime()
  {
    return MPI::Wtime();
  }

  /// get filename with PE identifier
  static std::string getFilename(std::string prefix, std::string ext)
  {
    std::string filename =
      ( boost::format("%s-%02d-%02d-%02d.%s")
        % prefix
        % instance->m_rank_dim[0]
        % instance->m_rank_dim[1]
        % instance->m_rank_dim[2]
        % ext ).str();
    return filename;
  }

  /// show debugging information
  static void info(std::ostream &out)
  {
    out << boost::format("\n"
                         " <<< INFO: mpiutils >>>"
                         "\n"
                         "Number of Process   : %4d\n"
                         "This Rank           : %4d\n"
                         "Temporary std::cout : %s\n"
                         "Temporary std::cerr : %s\n")
      % instance->m_nprocess
      % instance->m_thisrank
      % instance->m_outf.c_str()
      % instance->m_errf.c_str();

    out << boost::format("Info for domain decomposition:\n").str();
    for(int dir=0; dir < 3 ; dir++) {
      int cl[3], cc[3], cu[3];
      // self
      instance->m_cart.Get_coords(instance->m_thisrank, 3, cc);
      // lower
      if ( instance->m_nb_dim[dir][0] != MPI::PROC_NULL ) {
        instance->m_cart.Get_coords(instance->m_nb_dim[dir][0], 3, cl);
      } else {
        cl[0] = cl[1] = cl[2] = -1;
      }
      // upper
      if ( instance->m_nb_dim[dir][1] != MPI::PROC_NULL ) {
        instance->m_cart.Get_coords(instance->m_nb_dim[dir][1], 3, cu);
      } else {
        cu[0] = cu[1] = cu[2] = -1;
      }
      out << boost::format(" neighbor in dir %1d: "
                           "[%2d,%2d,%2d] <= [%2d,%2d,%2d] => [%2d,%2d,%2d]\n")
        % dir
        % cl[0] % cl[1] % cl[2]
        % cc[0] % cc[1] % cc[2]
        % cu[0] % cu[1] % cu[2];
    }
    out << std::endl;
  }

  /// begin boundary exchange with non-blocking send/recv
  static void bc_exchange_begin(void *buf0, void *buf1, void *buf2,
                                int dsize, int count[3],
                                MPI::Request req[12]);
  /// begin directional boundary exchange with non-blocking send/recv
  static void bc_exchange_dir_begin(int dir, void *buf,
                                    int dsize, int count,
                                    MPI::Request req[4]);
  /// wait requests
  static void wait(MPI::Request req[], int n);
};

// Local Variables:
// c-file-style   : "gnu"
// c-file-offsets : ((innamespace . 0) (inline-open . 0))
// End:
#endif

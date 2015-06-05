// -*- C++ -*-

///
/// MPI utlility module for three dimensional domain decomposition
///
/// Author: Takanobu AMANO <amanot@stelab.nagoya-u.ac.jp>
/// $Id$
///
#include "mpiutils.hpp"

// definition of static variables
mpiutils *mpiutils::instance;
int mpiutils::tag[3][2] = { {0, 1}, {2, 3}, {4, 5} };

//
// begin boundary exchange
//
void mpiutils::bc_exchange_begin(void *buf0, void *buf1, void *buf2,
                                 int dsize, int count[3],
                                 MPI::Request req[12])
{
  {
    //
    // begin non-blocking send/recv in dir. 0
    //
    int size  = dsize*count[0];
    int lower = instance->m_nb_dim[0][0];
    int upper = instance->m_nb_dim[0][1];
    char *p[4];
    p[0] = static_cast<char*>(buf0);
    p[1] = p[0] + size;
    p[2] = p[1] + size;
    p[3] = p[2] + size;

    req[ 0] = MPI::COMM_WORLD.Isend(p[0], size, MPI::CHAR, lower, tag[0][0]);
    req[ 1] = MPI::COMM_WORLD.Isend(p[1], size, MPI::CHAR, upper, tag[0][1]);
    req[ 2] = MPI::COMM_WORLD.Irecv(p[2], size, MPI::CHAR, lower, tag[0][1]);
    req[ 3] = MPI::COMM_WORLD.Irecv(p[3], size, MPI::CHAR, upper, tag[0][0]);
  }

  {
    //
    // begin non-blocking send/recv in dir. 1
    //
    int size  = dsize*count[1];
    int lower = instance->m_nb_dim[1][0];
    int upper = instance->m_nb_dim[1][1];
    char *p[4];
    p[0] = static_cast<char*>(buf1);
    p[1] = p[0] + size;
    p[2] = p[1] + size;
    p[3] = p[2] + size;

    req[ 4] = MPI::COMM_WORLD.Isend(p[0], size, MPI::CHAR, lower, tag[1][0]);
    req[ 5] = MPI::COMM_WORLD.Isend(p[1], size, MPI::CHAR, upper, tag[1][1]);
    req[ 6] = MPI::COMM_WORLD.Irecv(p[2], size, MPI::CHAR, lower, tag[1][1]);
    req[ 7] = MPI::COMM_WORLD.Irecv(p[3], size, MPI::CHAR, upper, tag[1][0]);
  }

  {
    //
    // begin non-blocking send/recv in dir. 2
    //
    int size  = dsize*count[2];
    int lower = instance->m_nb_dim[2][0];
    int upper = instance->m_nb_dim[2][1];
    char *p[4];
    p[0] = static_cast<char*>(buf2);
    p[1] = p[0] + size;
    p[2] = p[1] + size;
    p[3] = p[2] + size;

    req[ 8] = MPI::COMM_WORLD.Isend(p[0], size, MPI::CHAR, lower, tag[2][0]);
    req[ 9] = MPI::COMM_WORLD.Isend(p[1], size, MPI::CHAR, upper, tag[2][1]);
    req[10] = MPI::COMM_WORLD.Irecv(p[2], size, MPI::CHAR, lower, tag[2][1]);
    req[11] = MPI::COMM_WORLD.Irecv(p[3], size, MPI::CHAR, upper, tag[2][0]);
  }
}

//
// begin non-blocking directional boundary exchange
//
void mpiutils::bc_exchange_dir_begin(int dir, void *buf,
                                     int dsize, int count,
                                     MPI::Request req[4])
{
  int size  = dsize*count;
  int lower = instance->m_nb_dim[dir][0];
  int upper = instance->m_nb_dim[dir][1];
  char *p[4];
  p[0] = static_cast<char*>(buf);
  p[1] = p[0] + size;
  p[2] = p[1] + size;
  p[3] = p[2] + size;

  req[0] = MPI::COMM_WORLD.Isend(p[0], size, MPI::CHAR, lower, tag[dir][0]);
  req[1] = MPI::COMM_WORLD.Isend(p[1], size, MPI::CHAR, upper, tag[dir][1]);
  req[2] = MPI::COMM_WORLD.Irecv(p[2], size, MPI::CHAR, lower, tag[dir][1]);
  req[3] = MPI::COMM_WORLD.Irecv(p[3], size, MPI::CHAR, upper, tag[dir][0]);
}

//
// wait MPI requests with status checking
//
void mpiutils::wait(MPI::Request req[], int n)
{
  MPI::Status status[n];

  MPI::Request::Waitall(n, req, status);

  // check status
  for(int i=0; i < n ;i++) {
    if( req[i].Test(status[i]) != true ) {
      std::cerr << tfm::format("MPI request[%2d] from rank %3d failed !\n",
                               i, instance->m_thisrank);
    }
  }
}

#ifdef __DEBUG_MPIUTILS__

//
// test code for mpiutils module
//
int main(int argc, char **argv)
{
  int32 domain[3] = {4, 4, 1};
  bool  period[3] = {1, 0, 0};

  // initialize with domain decomposition
  mpiutils::initialize(argc, argv, domain, period, false);

  // show info
  mpiutils::info(std::cerr);

  // how to use utility functions
  std::cerr << "time in sec = "
            << mpiutils::getTime() << std::endl
            << "filename    = "
            << mpiutils::getFilename("test", "dat") << std::endl;;

  // test directional send/recv
  {
    const int dir = 0;
    const int N = 3;
    int buf[4*N];
    MPI::Request req[4];

    // clear buffer
    memset(buf, -1, 4*N*sizeof(int));

    // fill send buffer by coordinate
    mpiutils::getCoord(&buf[0*N]);
    mpiutils::getCoord(&buf[1*N]);

    mpiutils::bc_exchange_dir_begin(dir, buf, sizeof(int), N, req);
    mpiutils::wait(req, 4);

    // show results
    std::cerr << "--- results of directional send/recv ---" << std::endl;
    std::cerr << tfm::format("send lower => [%2d,%2d,%2d]\n"
                             "send upper => [%2d,%2d,%2d]\n",
                             buf[0*N+0], buf[0*N+1], buf[0*N+2],
                             buf[1*N+0], buf[1*N+1], buf[1*N+2]);
    std::cerr << tfm::format("recv lower => [%2d,%2d,%2d]\n"
                             "recv upper => [%2d,%2d,%2d]\n",
                             buf[2*N+0], buf[2*N+1], buf[2*N+2],
                             buf[3*N+0], buf[3*N+1], buf[3*N+2]);
  }

  // test package send/recv
  {
    const int N0 = 3;
    const int N1 = 3;
    const int N2 = 3;
    int count[3] = {N0, N1, N2};
    int buf0[4*N0];
    int buf1[4*N1];
    int buf2[4*N2];
    MPI::Request req[12];

    // clear buffer
    memset(buf0, -1, 4*N0*sizeof(int));
    memset(buf1, -1, 4*N1*sizeof(int));
    memset(buf2, -1, 4*N2*sizeof(int));

    // fill send buffer by coordinate
    mpiutils::getCoord(&buf0[0*N0]);
    mpiutils::getCoord(&buf0[1*N0]);
    mpiutils::getCoord(&buf1[0*N1]);
    mpiutils::getCoord(&buf1[1*N1]);
    mpiutils::getCoord(&buf2[0*N2]);
    mpiutils::getCoord(&buf2[1*N2]);

    mpiutils::bc_exchange_begin(buf0, buf1, buf2, sizeof(int), count, req);
    mpiutils::wait(req, 12);

    // show results
    std::cerr << "--- results of send/recv in dir. 0 ---" << std::endl;
    std::cerr << tfm::format("send lower => [%2d,%2d,%2d]\n"
                             "send upper => [%2d,%2d,%2d]\n",
                             buf0[0*N0+0], buf0[0*N0+1], buf0[0*N0+2],
                             buf0[1*N0+0], buf0[1*N0+1], buf0[1*N0+2]);
    std::cerr << tfm::format("recv lower => [%2d,%2d,%2d]\n"
                             "recv upper => [%2d,%2d,%2d]\n",
                             buf0[2*N0+0], buf0[2*N0+1], buf0[2*N0+2],
                             buf0[3*N0+0], buf0[3*N0+1], buf0[3*N0+2]);
    std::cerr << "--- results of send/recv in dir. 1 ---" << std::endl;
    std::cerr << tfm::format("send lower => [%2d,%2d,%2d]\n"
                             "send upper => [%2d,%2d,%2d]\n",
                             buf1[0*N1+0], buf1[0*N1+1], buf1[0*N1+2],
                             buf1[1*N1+0], buf1[1*N1+1], buf1[1*N1+2]);
    std::cerr << tfm::format("recv lower => [%2d,%2d,%2d]\n"
                             "recv upper => [%2d,%2d,%2d]\n",
                             buf1[2*N1+0], buf1[2*N1+1], buf1[2*N1+2],
                             buf1[3*N1+0], buf1[3*N1+1], buf1[3*N1+2]);
    std::cerr << "--- results of send/recv in dir. 2 ---" << std::endl;
    std::cerr << tfm::format("send lower => [%2d,%2d,%2d]\n"
                             "send upper => [%2d,%2d,%2d]\n",
                             buf2[0*N2+0], buf2[0*N2+1], buf2[0*N2+2],
                             buf2[1*N2+0], buf2[1*N2+1], buf2[1*N2+2]);
    std::cerr << tfm::format("recv lower => [%2d,%2d,%2d]\n"
                             "recv upper => [%2d,%2d,%2d]\n",
                             buf2[2*N2+0], buf2[2*N2+1], buf2[2*N2+2],
                             buf2[3*N2+0], buf2[3*N2+1], buf2[3*N2+2]);
  }

  // finalize
  mpiutils::finalize();

  return 0;
}

#endif

// Local Variables:
// c-file-style   : "gnu"
// c-file-offsets : ((innamespace . 0) (inline-open . 0))
// End:

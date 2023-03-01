// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>

#ifdef _OPENMP
  #include "omp.h"
#endif

#include "vx_log.h"
#include "handle_openmp.h"

///////////////////////////////////////////////////////////////////////////////

void init_openmp() {

#ifdef _OPENMP

  // If OMP_NUM_THREADS was not set, use the OpenMP API to set the thread count
  // to 1 thread only.
  const char* env_omp_num_threads = std::getenv("OMP_NUM_THREADS");
  if (!env_omp_num_threads) {
    mlog << Debug(2) << "OMP_NUM_THREADS is not set." 
         << " Defaulting to 1 thread."
         << " Recommend setting OMP_NUM_THREADS for faster runtimes.\n";
    omp_set_num_threads(1);
  }

#pragma omp parallel
#pragma omp single
  {
     mlog << Debug(2) << "OpenMP running on " 
       << omp_get_num_threads() << " thread(s).\n";
  }

#else  /* _OPENMP */

  mlog << Debug(2) << "OpenMP disabled.\n";

#endif  /* _OPENMP */

}

///////////////////////////////////////////////////////////////////////////////


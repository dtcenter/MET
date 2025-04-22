// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
#include "is_number.h"

///////////////////////////////////////////////////////////////////////////////

void init_openmp() {

#ifdef _OPENMP

   // If OMP_NUM_THREADS is not set, use a single thread
   const char* env_omp_num_threads = std::getenv("OMP_NUM_THREADS");
   if(!env_omp_num_threads) {
      mlog << Debug(2) << "Defaulting unset OMP_NUM_THREADS to use 1 of "
           << omp_get_max_threads() << " available threads. Recommend "
           << "setting OMP_NUM_THREADS for faster runtimes.\n";
      omp_set_num_threads(1);
   }
   // If OMP_NUM_THREADS is non-integer, use a single thread
   else if(!is_integer(env_omp_num_threads)) {
      mlog << Debug(2) << "Resetting non-integer OMP_NUM_THREADS ("
           << env_omp_num_threads << ") to use 1 of "
           << omp_get_max_threads() << " available threads. Recommend "
           << "setting OMP_NUM_THREADS for faster runtimes.\n";
      omp_set_num_threads(1);
   }
   // If OMP_NUM_THREADS <= 0, use all available threads
   else {
      if(atoi(env_omp_num_threads) <= 0) {
         mlog << Debug(2) << "Resetting OMP_NUM_THREADS ("
              << env_omp_num_threads << ") to " << omp_get_max_threads()
              << ", the maximum number of threads available.\n";
         omp_set_num_threads(omp_get_max_threads());
      } 
   }

#pragma omp parallel
#pragma omp single
   {
       mlog << Debug(2) << "OpenMP running on " << omp_get_num_threads()
            << " thread(s).\n";
   }

#else  /* _OPENMP */

   mlog << Debug(2) << "OpenMP disabled at compilation time.\n";

#endif  /* _OPENMP */

}

///////////////////////////////////////////////////////////////////////////////

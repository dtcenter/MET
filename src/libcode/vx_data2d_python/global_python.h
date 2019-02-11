// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __GLOBAL_PYTHON__
#define  __GLOBAL_PYTHON__


////////////////////////////////////////////////////////////////////////


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


class GlobalPython {

   public:

      GlobalPython();
     ~GlobalPython();

      void initialize();
      void finalize();

      bool is_initialized;

};

inline GlobalPython::GlobalPython()   { is_initialized = false; };
inline GlobalPython::~GlobalPython()  { is_initialized = false; };


/////////////////////////////


inline void GlobalPython::initialize()

{ 

if ( ! is_initialized )  {

   Py_Initialize();

   is_initialized = true;

}

return;

}


/////////////////////////////


inline void GlobalPython::finalize()

{ 

if ( is_initialized )  {

   Py_Finalize();

   is_initialized = false;

}

return;

}


/////////////////////////////


extern GlobalPython GP;


////////////////////////////////////////////////////////////////////////


#endif   /*  __GLOBAL_PYTHON__  */


////////////////////////////////////////////////////////////////////////



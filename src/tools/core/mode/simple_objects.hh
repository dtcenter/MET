// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SIMPLE_OBJECTS_H__
#define  __SIMPLE_OBJECTS_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <memory>
#include <vector>
#include "multivar_data.h"
#include "mode_superobject.h"
#include "bool_calc.h"

class SimpleObjects {

 public:

   SimpleObjects();
   ~SimpleObjects() = default;

      //
      //  _mvd owns its MultiVarData, so these objects move rather than copy.
      //  The destructor is defaulted for the same reason: a user-declared one
      //  suppresses the implicit move constructor.
      //
      //  noexcept is declared rather than deduced.  The deduced specification
      //  is potentially throwing, because _super has no move of its own -
      //  ShapeData and BoolPlane declare destructors and so suppress theirs -
      //  and copying it allocates.  The only exception that can produce is
      //  std::bad_alloc, which no MET tool can ever observe: met_main() calls
      //  set_handlers(), whose set_new_handler(oom) exits the process before
      //  operator new can throw.
      //

   SimpleObjects(SimpleObjects &&) noexcept = default;
   SimpleObjects & operator=(SimpleObjects &&) noexcept = default;

   void init(ModeDataType dataType, int rIndex, int tIndex);
   void setSuper(bool isFcst, int n_fcst_files, bool do_clusters, BoolCalc &f_calc);
   void clear(void);
             
   ModeDataType _dataType;  /**< observations or forecasts */
   int _rIndex;             /**< Convolution radius index */
   int _tIndex;            /**< Convolution threshold index */
   std::vector<std::unique_ptr<MultiVarData>> _mvd;  /**< The data from each input */
   ModeSuperObject _super;   /**< The superobject created from the data */
};

#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////

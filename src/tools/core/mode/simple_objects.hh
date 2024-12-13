// ** Copyright UCAR (c) 1992 - 2024
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
#include <vector>
#include "multivar_data.h"
#include "mode_superobject.h"
#include "bool_calc.h"

class SimpleObjects {

 public:

   SimpleObjects();
   ~SimpleObjects();

   void init(ModeDataType dataType, int rIndex, int tIndex);
   void setSuper(bool isFcst, int n_fcst_files, bool do_clusters, BoolCalc &f_calc);
   void clear(void);
             
   ModeDataType _dataType;  /**< observations or forecasts */
   int _rIndex;             /**< Convolution radius index */
   int _tIndex;            /**< Convolution threshold index */
   std::vector<MultiVarData *> _mvd;  /**< The data from each input */
   ModeSuperObject _super;   /**< The superobject created from the data */
};

#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////

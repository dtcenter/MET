// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_SUPEROBJECT_H__
#define  __MODE_SUPEROBJECT_H__


////////////////////////////////////////////////////////////////////////


#include "combine_boolplanes.h"
#include "multivar_data.h"
#include "shapedata.h"
#include <iostream>
#include <vector>

class ModeSuperObject {

 private:

 public:

   ModeSuperObject(bool isFcst, int n_files, bool do_clusters,
                   const vector<MultiVarData *> &mvd,
                   BoolCalc &calc);
   inline ~ModeSuperObject() {}

   bool _isFcst;
   BoolPlane _simple_result;
   ShapeData _simple_sd;
   ShapeData _merge_sd_split;
};


#endif   /*  __MODE_SUPEROBJECT_H__ */

/////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_COMBINE_BOOLPLANES_H__
#define  __MODE_COMBINE_BOOLPLANES_H__


////////////////////////////////////////////////////////////////////////


#include "two_d_array.h"
#include "bool_calc.h"
#include "vx_pxm.h"
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////


//
//  grabs the objects from a MODE output netcdf file
//


////////////////////////////////////////////////////////////////////////


extern void combine_boolplanes(const std::string &name,
                               const BoolPlane * array, const int n_planes, 
                               BoolCalc & calc, 
                               BoolPlane & bp_out);


////////////////////////////////////////////////////////////////////////


//
//  useful mainly for debugging
//


extern void boolplane_to_pgm(const BoolPlane & in, Pgm & out);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_COMBINE_BOOLPLANES_H__  */


////////////////////////////////////////////////////////////////////////



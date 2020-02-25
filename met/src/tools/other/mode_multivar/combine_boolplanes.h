
////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


////////////////////////////////////////////////////////////////////////


   //
   //  grabs the objects from a MODE output netcdf file
   //


////////////////////////////////////////////////////////////////////////


extern void combine_boolplanes(const BoolPlane * array, const int n_planes, 
                               BoolCalc & calc, 
                               BoolPlane & bp_out);

                                


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_COMBINE_BOOLPLANES_H__  */


////////////////////////////////////////////////////////////////////////



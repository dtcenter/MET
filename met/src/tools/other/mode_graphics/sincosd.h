// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SINCOSD_H__
#define  __VX_SINCOSD_H__


////////////////////////////////////////////////////////////////////////


#include "trig.h"


////////////////////////////////////////////////////////////////////////


inline void sincosd (double _angle_deg_, double & _s_, double & _c_)

{

sincos(_angle_deg_*rad_per_deg, &_s_, &_c_);

return;

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SINCOSD_H__  */


////////////////////////////////////////////////////////////////////////



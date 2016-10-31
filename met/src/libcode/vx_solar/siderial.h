// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_ASTRO_SIDERIAL_H__
#define  __VX_ASTRO_SIDERIAL_H__


////////////////////////////////////////////////////////////////////////


#include "cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Greenwich mean siderial time, in degrees
   //

extern double gmt_to_gmst(Unixtime gmt);

   //
   //  local mean siderial time, in degrees
   //

extern double lmt_to_lmst(Unixtime lmt, int zone, double lon);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_ASTRO_SIDERIAL_H__  */


////////////////////////////////////////////////////////////////////////



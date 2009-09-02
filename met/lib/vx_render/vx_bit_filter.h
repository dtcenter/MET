// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_BIT_FILTER_H__
#define  __VX_BIT_FILTER_H__


////////////////////////////////////////////////////////////////////////


#include "vx_render/vx_ps_filter.h"


////////////////////////////////////////////////////////////////////////


class BitFilter : public PSFilter {

   public:

      BitFilter();
     ~BitFilter();

      BitFilter(int);

      unsigned char u;

      int shift;

      int bits_per_component;

      virtual void eat(unsigned char);

      virtual void eod();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_BIT_FILTER_H__


////////////////////////////////////////////////////////////////////////



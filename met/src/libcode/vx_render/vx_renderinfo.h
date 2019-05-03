// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __VX_RENDERINFO_H__
#define __VX_RENDERINFO_H__


////////////////////////////////////////////////////////////////////////


#include "vx_ps_filter.h"


////////////////////////////////////////////////////////////////////////


class RenderInfo {

   public:

      RenderInfo();
     ~RenderInfo();
      RenderInfo(const RenderInfo &);
      RenderInfo &operator=(const RenderInfo &);

      double x_ll;
      double y_ll;

      double magnification;   //  units: points per pixel

      int bw;            //  0 = color, 1 = grayscale

      int filter[max_filters];

      int n_filters;

      void add_filter(int);

};


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_RENDERINFO_H__


////////////////////////////////////////////////////////////////////////



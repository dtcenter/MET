// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ASCII85_FILTER_H__
#define  __ASCII85_FILTER_H__


////////////////////////////////////////////////////////////////////////


#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


class ASCII85EncodeFilter : public PSFilter {

   public:

      ASCII85EncodeFilter();
     ~ASCII85EncodeFilter();

      unsigned int u;

      int count;

      virtual void eat(unsigned char);

      virtual void eod();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __ASCII85_FILTER_H__


////////////////////////////////////////////////////////////////////////



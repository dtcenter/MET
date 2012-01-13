// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PS_OUTPUT_FILTER_H__
#define  __PS_OUTPUT_FILTER_H__


////////////////////////////////////////////////////////////////////////


#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


class PSOutputFilter : public PSFilter {

   public:

      PSOutputFilter();
     ~PSOutputFilter();

      PSOutputFilter(ofstream &);

      ofstream *file;

      int column;

      virtual void eat(unsigned char);

      virtual void eod();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __PS_OUTPUT_FILTER_H__


////////////////////////////////////////////////////////////////////////



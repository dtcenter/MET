// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __FLATE_FILTER_1_H__
#define  __FLATE_FILTER_1_H__


////////////////////////////////////////////////////////////////////////


#include <vector>
#include <memory>
#include <zlib.h>

#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


class FlateEncodeFilter : public PSFilter {

   public:

      FlateEncodeFilter();
     ~FlateEncodeFilter();

      int flush_mode;

      std::unique_ptr<z_stream> s;

      unsigned int inbytes;   //  # of bytes stored in the input buffer

      std::vector<unsigned char>  inbuf;

      std::vector<unsigned char> outbuf;

      virtual void eat(unsigned char);

      virtual void eod();

      void do_output();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __FLATE_FILTER_1_H__  */


////////////////////////////////////////////////////////////////////////



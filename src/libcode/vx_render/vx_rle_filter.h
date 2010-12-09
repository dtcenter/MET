// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_RLE_FILTER_H__
#define  __VX_RLE_FILTER_H__


////////////////////////////////////////////////////////////////////////


#include "vx_ps_filter.h"
#include "vx_uc_queue.h"


////////////////////////////////////////////////////////////////////////


static const int rle_enough  = 4;


////////////////////////////////////////////////////////////////////////


class RunLengthEncodeFilter : public PSFilter {

   private:

      enum RLE_mode { run, literal, start };

      RLE_mode mode;

      UCQueue q;

      void dump_literal(int length);

      void dump_run(int length);

      void run_eat    (unsigned char);
      void literal_eat(unsigned char);

   public:

      RunLengthEncodeFilter();
     ~RunLengthEncodeFilter();

      virtual void eat(unsigned char);

      virtual void eod();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_RLE_FILTER_H__


////////////////////////////////////////////////////////////////////////



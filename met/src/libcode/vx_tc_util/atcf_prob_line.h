// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_ATCF_PROB_LINE_H__
#define  __VX_ATCF_PROB_LINE_H__

////////////////////////////////////////////////////////////////////////
//
// Based on Ensemble Probability file format information at:
//    http://www.nrlmry.navy.mil/atcf_web/docs/database/new/edeck.txt
//
////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_cal.h"
#include "vx_util.h"
#include "atcf_line_base.h"

////////////////////////////////////////////////////////////////////////
//
// ATCFProbLine class stores the data in a single ATCF probability line.
//
////////////////////////////////////////////////////////////////////////

class ATCFProbLine : public ATCFLineBase {

   private:

      void init_from_scratch();

      void assign(const ATCFProbLine &);

   public:

      ATCFProbLine();
     ~ATCFProbLine();
      ATCFProbLine(const ATCFProbLine &);
      ATCFProbLine & operator= (const ATCFProbLine &);

      void dump(ostream &, int depth = 0) const;

      void clear();

      int read_line(LineDataFile *);   //  virtual from base class

         //
         // probability column values
         //

      int prob      () const;
      int prob_item () const;

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_ATCF_PROB_LINE_H__  */

////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __BY_CASE_INFO_H__
#define  __BY_CASE_INFO_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "mode_line.h"

#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class ByCaseInfo {

   private:

      void init_from_scratch();

      void assign(const ByCaseInfo &);

   public:

      ByCaseInfo();
     ~ByCaseInfo();
      ByCaseInfo(const ByCaseInfo &);
      ByCaseInfo & operator=(const ByCaseInfo &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void add(const ModeLine &);

         //
         //  things we keep track of
         //

      unixtime valid;

      int line_count;

      double area_matched;
      double area_unmatched;

      int n_fcst_matched;
      int n_fcst_unmatched;

      int n_obs_matched;
      int n_obs_unmatched;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __BY_CASE_INFO_H__  */


////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_GEN_INFO_H__
#define  __VX_PROB_GEN_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "prob_info_base.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbGenInfo class stores probability of rapid intensification.
//
////////////////////////////////////////////////////////////////////////

class ProbGenInfo : public ProbInfoBase {

   private:

      void init_from_scratch();
      void assign(const ProbGenInfo &);

      // Probability of Genesis specific values
      ConcatString Initials;
      ConcatString GenOrDis;
      unixtime     GenTime;

   public:

      ProbGenInfo();
     ~ProbGenInfo();
      ProbGenInfo(const ProbGenInfo &);
      ProbGenInfo & operator=(const ProbGenInfo &);

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const ConcatString & initials()   const;
      const ConcatString & gen_or_dis() const;
      unixtime             gen_time()   const;

         //
         //  do stuff
         //

      void initialize(const ATCFProbLine &, double);
      bool is_match  (const ATCFProbLine &) const;
      bool add       (const ATCFProbLine &, double, bool check_dup = false);
      
};

////////////////////////////////////////////////////////////////////////

inline const ConcatString & ProbGenInfo::initials()   const { return(Initials); }
inline const ConcatString & ProbGenInfo::gen_or_dis() const { return(GenOrDis); }
inline unixtime             ProbGenInfo::gen_time()   const { return(GenTime);  }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_GEN_INFO_H__  */

////////////////////////////////////////////////////////////////////////

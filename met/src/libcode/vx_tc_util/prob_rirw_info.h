// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_RI_INFO_H__
#define  __VX_PROB_RI_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "prob_info_base.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbRIRWInfo class stores probability of rapid intensification.
//
////////////////////////////////////////////////////////////////////////

class ProbRIRWInfo : public ProbInfoBase {

   private:

      void init_from_scratch();
      void assign(const ProbRIRWInfo &);

      // Prob RI specific values
      double       Value;
      ConcatString Initials;
      int          RIRWBeg; // hours
      int          RIRWEnd; // hours

   public:

      ProbRIRWInfo();
     ~ProbRIRWInfo();
      ProbRIRWInfo(const ProbRIRWInfo &);
      ProbRIRWInfo & operator=(const ProbRIRWInfo &);

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

      double               value()       const;
      const ConcatString & initials()    const;
      int                  rirw_beg()    const;
      int                  rirw_end()    const;
      int                  rirw_window() const;

         //
         //  do stuff
         //

      void initialize(const ATCFProbLine &);
      bool is_match  (const ATCFProbLine &) const;
      bool add       (const ATCFProbLine &, bool check_dup = false);
      void set       (const TCStatLine &);

};

////////////////////////////////////////////////////////////////////////

inline double               ProbRIRWInfo::value()    const { return(Value);    }
inline const ConcatString & ProbRIRWInfo::initials() const { return(Initials); }
inline int                  ProbRIRWInfo::rirw_beg() const { return(RIRWBeg);  }
inline int                  ProbRIRWInfo::rirw_end() const { return(RIRWEnd);  }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_RI_INFO_H__  */

////////////////////////////////////////////////////////////////////////

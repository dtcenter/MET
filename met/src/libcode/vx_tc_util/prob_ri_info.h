// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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
// ProbRIInfo class stores probability of rapid intensification.
//
////////////////////////////////////////////////////////////////////////

class ProbRIInfo : public ProbInfoBase {

   private:

      void init_from_scratch();
      void assign(const ProbRIInfo &);

      // Prob RI specific values
      double       Value;
      ConcatString Initials;
      int          RIBeg; // hours
      int          RIEnd; // hours

   public:

      ProbRIInfo();
     ~ProbRIInfo();
      ProbRIInfo(const ProbRIInfo &);
      ProbRIInfo & operator=(const ProbRIInfo &);

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

      double               value()     const;
      const ConcatString & initials()  const;
      int                  ri_beg()    const;
      int                  ri_end()    const;
      int                  ri_window() const;

         //
         //  do stuff
         //

      void initialize(const ATCFProbLine &);
      bool is_match  (const ATCFProbLine &) const;
      bool add       (const ATCFProbLine &, bool check_dup = false);
      void set       (const TCStatLine &);

};

////////////////////////////////////////////////////////////////////////

inline double               ProbRIInfo::value()    const { return(Value);    }
inline const ConcatString & ProbRIInfo::initials() const { return(Initials); }
inline int                  ProbRIInfo::ri_beg()   const { return(RIBeg);    }
inline int                  ProbRIInfo::ri_end()   const { return(RIEnd);    }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_RI_INFO_H__  */

////////////////////////////////////////////////////////////////////////

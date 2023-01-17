// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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
#include "genesis_info.h"

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
      unixtime     GenesisTime;
      int          GenesisLead;

   public:

      ProbGenInfo();
     ~ProbGenInfo();
      ProbGenInfo(const ProbGenInfo &);
      ProbGenInfo & operator=(const ProbGenInfo &);

      void clear();

      void         dump(std::ostream &, int = 0)  const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_best_gen(const GenesisInfo *);

         //
         //  get stuff
         //

      const ConcatString & initials()     const;
      const ConcatString & gen_or_dis()   const;
      unixtime             genesis_time() const;
      int                  genesis_lead() const;
      int                  genesis_fhr()  const;
      const GenesisInfo *  best_gen()     const;

         //
         //  do stuff
         //

      void initialize(const ATCFProbLine &, double);
      bool is_match  (const ATCFProbLine &) const;
      bool add       (const ATCFProbLine &, double, bool check_dup = false);

      bool is_match  (const TrackPoint &,
                      const double, const int, const int) const;
      bool is_match  (const GenesisInfo &,
                      const double, const int, const int) const;

};

////////////////////////////////////////////////////////////////////////

inline const ConcatString & ProbGenInfo::initials()     const { return(Initials);    }
inline const ConcatString & ProbGenInfo::gen_or_dis()   const { return(GenOrDis);    }
inline unixtime             ProbGenInfo::genesis_time() const { return(GenesisTime); }
inline int                  ProbGenInfo::genesis_lead() const { return(GenesisLead); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_GEN_INFO_H__  */

////////////////////////////////////////////////////////////////////////

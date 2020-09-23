// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_PAIR_INFO_H__
#define  __VX_PROB_PAIR_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "atcf_prob_line.h"
#include "prob_info_array.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbPairInfo class stores a ProbInfoBase class pointer and a
// verifying
//
////////////////////////////////////////////////////////////////////////

class ProbPairInfo {

   protected:

      void init_from_scratch();
      void assign(const ProbPairInfo &);

      vector<ProbPairInfoBase *> Pair;

   public:

      ProbPairInfo();
     ~ProbPairInfo();
      ProbPairInfo(const ProbPairInfo &);
      ProbPairInfo & operator=(const ProbPairInfo &);

      void clear();

      void         dump(ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  get stuff
         //

      const ProbPairInfoBase * operator[](int) const;
      int n_pairs() const;

         //
         //  do stuff
         //

      bool add(const ATCFProbLine &, bool check_dup = false);
      bool add(const ProbPairInfoBase *);

};

////////////////////////////////////////////////////////////////////////

inline int ProbPairInfo::n_pairs() const { return(Pair.size()); }

////////////////////////////////////////////////////////////////////////

extern ProbPairInfoBase * new_prob_pair(const ATCFLineType);

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_PAIR_INFO_H__  */

////////////////////////////////////////////////////////////////////////

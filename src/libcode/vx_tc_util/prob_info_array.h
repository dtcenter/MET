// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_INFO_ARRAY_H__
#define  __VX_PROB_INFO_ARRAY_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "atcf_prob_line.h"
#include "prob_info_base.h"
#include "prob_rirw_info.h"
#include "prob_gen_info.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbInfoArray class stores a vector of ProbInfoBase class pointers.
//
////////////////////////////////////////////////////////////////////////

class ProbInfoArray {

   protected:

      void init_from_scratch();
      void assign(const ProbInfoArray &);

      std::vector<ProbRIRWInfo> ProbRIRW;
      std::vector<ProbGenInfo>  ProbGen;

   public:

      ProbInfoArray();
     ~ProbInfoArray();
      ProbInfoArray(const ProbInfoArray &);
      ProbInfoArray & operator=(const ProbInfoArray &);

      void clear();

      void         dump(std::ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  get stuff
         //

      int n_probs() const;
      const ProbInfoBase * operator[](int) const;

      int n_prob_rirw() const;
      ProbRIRWInfo & prob_rirw(int);

      int n_prob_gen() const;
      ProbGenInfo & prob_gen(int);

      int n_technique() const;

         //
         //  do stuff
         //

      bool add(const ATCFProbLine &, double dland, bool check_dup = false);
      void add(const ProbRIRWInfo &);
      void add(const ProbGenInfo &);
};

////////////////////////////////////////////////////////////////////////

inline int ProbInfoArray::n_probs()     const { return(ProbRIRW.size() + ProbGen.size()); }
inline int ProbInfoArray::n_prob_rirw() const { return(ProbRIRW.size());                  }
inline int ProbInfoArray::n_prob_gen()  const { return(ProbGen.size());                   }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_INFO_ARRAY_H__  */

////////////////////////////////////////////////////////////////////////

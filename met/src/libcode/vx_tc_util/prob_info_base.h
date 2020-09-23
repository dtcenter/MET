// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_INFO_BASE_H__
#define  __VX_PROB_INFO_BASE_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "atcf_prob_line.h"
#include "track_info.h"
#include "tc_stat_line.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbInfoBase class is the base class for storing ATCF probabilities.
//
////////////////////////////////////////////////////////////////////////

class ProbInfoBase {

   protected:

      void init_from_scratch();
      void assign(const ProbInfoBase &);

      // Probability type
      ATCFLineType Type;

      // Storm and model identification
      ConcatString StormId;
      ConcatString Basin;
      ConcatString Cyclone;
      ConcatString Technique;

      // Timing information
      unixtime     InitTime;
      unixtime     ValidTime;

      // Location information
      double       Lat;
      double       Lon;

      // Probability information
      int          NProb;
      NumArray     Prob;
      NumArray     ProbItem;

      // Input ATCF Prob Lines
      StringArray  ProbLines;

   public:

      ProbInfoBase();
      virtual ~ProbInfoBase();
      ProbInfoBase(const ProbInfoBase &);
      ProbInfoBase & operator=(const ProbInfoBase &);

      void clear();

      virtual void         dump(ostream &, int = 0)  const;
      virtual ConcatString serialize()               const;
      virtual ConcatString serialize_r(int, int = 0) const;

         //
         //  set stuff
         //

      void set_dland(double d);

         //
         //  get stuff
         //

            ATCFLineType   type()           const;
      const ConcatString & storm_id()       const;
      const ConcatString & basin()          const;
      const ConcatString & cyclone()        const;
      const ConcatString & technique()      const;
      unixtime             init()           const;
      int                  init_hour()      const;
      unixtime             valid()          const;
      int                  valid_hour()     const;
      double               lat()            const;
      double               lon()            const;
      int                  n_prob()         const;
      double               prob(int i)      const;
      double               prob_item(int i) const;

         //
         //  do stuff
         //

      virtual void initialize(const ATCFProbLine &);
      virtual bool is_match  (const ATCFProbLine &) const;
              bool is_match  (const TrackInfo    &) const;
              bool has       (const ATCFProbLine &) const;
      virtual bool add       (const ATCFProbLine &, bool check_dup = false);
      virtual void set       (const TCStatLine &);

};

////////////////////////////////////////////////////////////////////////

inline       ATCFLineType   ProbInfoBase::type()           const { return(Type);                          }
inline const ConcatString & ProbInfoBase::storm_id()       const { return(StormId);                       }
inline const ConcatString & ProbInfoBase::basin()          const { return(Basin);                         }
inline const ConcatString & ProbInfoBase::cyclone()        const { return(Cyclone);                       }
inline const ConcatString & ProbInfoBase::technique()      const { return(Technique);                     }
inline unixtime             ProbInfoBase::init()           const { return(InitTime);                      }
inline int                  ProbInfoBase::init_hour()      const { return(unix_to_sec_of_day(InitTime));  }
inline unixtime             ProbInfoBase::valid()          const { return(ValidTime);                     }
inline int                  ProbInfoBase::valid_hour()     const { return(unix_to_sec_of_day(ValidTime)); }
inline double               ProbInfoBase::lat()            const { return(Lat);                           }
inline double               ProbInfoBase::lon()            const { return(Lon);                           }
inline int                  ProbInfoBase::n_prob()         const { return(NProb);                         }
inline double               ProbInfoBase::prob(int i)      const { return(Prob[i]);                       }
inline double               ProbInfoBase::prob_item(int i) const { return(ProbItem[i]);                   }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_INFO_BASE_H__  */

////////////////////////////////////////////////////////////////////////

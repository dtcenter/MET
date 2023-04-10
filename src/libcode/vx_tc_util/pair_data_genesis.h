// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __PAIR_DATA_GENESIS_H__
#define  __PAIR_DATA_GENESIS_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "vx_cal.h"
#include "vx_math.h"
#include "vx_util.h"

#include "genesis_info.h"

////////////////////////////////////////////////////////////////////////
//
// Genesis Pair Categories
//
////////////////////////////////////////////////////////////////////////

enum GenesisPairCategory {
   FYOYGenesis,    // Hit
   FYONGenesis,    // False Alarm
   FNOYGenesis,    // Miss
   DiscardGenesis, // Discard
   NoGenesisPairCategory
};

extern ConcatString genesispaircategory_to_string(const GenesisPairCategory);

////////////////////////////////////////////////////////////////////////
//
// Structure for Genesis Pair Differences
//
////////////////////////////////////////////////////////////////////////

struct GenesisPairDiff {
   double              DevDist;     // Distance between genesis events
   int                 DevDSec;     // Fcst - Best genesis time
   int                 OpsDSec;     // Best genesis - fcst initalization
   GenesisPairCategory DevCategory; // Contingency table category
   GenesisPairCategory OpsCategory; // Concingency table category

   GenesisPairDiff();
   void clear();
};

////////////////////////////////////////////////////////////////////////
//
// Class to store matched genesis pairs.
//
////////////////////////////////////////////////////////////////////////

class PairDataGenesis {

   private:

      void init_from_scratch();
      void assign(const PairDataGenesis &);

      // Describe this verification task
      ConcatString Desc;
      ConcatString Mask;
      ConcatString Model;

      // Number of pairs
      int NPair;

      // Arrays of info for each pair
      StringArray BestStormId;
      TimeArray   InitTime;
      IntArray    LeadTime;

      std::vector<const GenesisInfo *> FcstGen;
      std::vector<const GenesisInfo *> BestGen;
      std::vector<GenesisPairDiff>     GenDiff;

      //////////////////////////////////////////////////////////////////
   
      bool has_gen (const std::vector<const GenesisInfo *>&,
                    const GenesisInfo *, int &) const;
      bool has_case(const ConcatString &, const unixtime,
                    int &) const;

   public:

      PairDataGenesis();
      ~PairDataGenesis();
      PairDataGenesis(const PairDataGenesis &);
      PairDataGenesis & operator=(const PairDataGenesis &);

      //////////////////////////////////////////////////////////////////

      void clear();
   
      // Set stuff
      void set_desc (const ConcatString &);
      void set_mask (const ConcatString &);
      void set_model(const ConcatString &);


      // Get stuff
      ConcatString           desc()             const;
      ConcatString           mask()             const;
      ConcatString           model()            const;
      int                    n_pair()           const;
      const std::string      best_storm_id(int) const;
      unixtime               init         (int) const;
      int                    lead_time    (int) const;
      const GenesisInfo *    fcst_gen     (int) const;
      const GenesisInfo *    best_gen     (int) const;
      const GenesisPairDiff &gen_diff     (int) const;

      // Do stuff
      bool has_fcst_gen(const GenesisInfo *, int &) const;
      bool has_best_gen(const GenesisInfo *, int &) const;

      void add_fcst_gen(const GenesisInfo *);
      void add_best_gen(const GenesisInfo *,
                        const int, const int, const int,
                        const unixtime, const unixtime,
                        const TimeArray &, const TimeArray &,
                        const NumArray &, const NumArray &);
      
      void add_gen_pair(const GenesisInfo *, const GenesisInfo *);
      void set_gen_diff(int, const GenesisPairDiff &);
};

////////////////////////////////////////////////////////////////////////

inline void PairDataGenesis::set_desc (const ConcatString &s) { Desc  = s; }
inline void PairDataGenesis::set_mask (const ConcatString &s) { Mask  = s; }
inline void PairDataGenesis::set_model(const ConcatString &s) { Model = s; }

inline ConcatString      PairDataGenesis::desc()               const { return(Desc);           }
inline ConcatString      PairDataGenesis::mask()               const { return(Mask);           }
inline ConcatString      PairDataGenesis::model()              const { return(Model);          }
inline int               PairDataGenesis::n_pair()             const { return(NPair);          }
inline const std::string PairDataGenesis::best_storm_id(int i) const { return(BestStormId[i]); }
inline unixtime          PairDataGenesis::init(int i)          const { return(InitTime[i]);    }
inline int               PairDataGenesis::lead_time(int i)     const { return(LeadTime[i]);    }

////////////////////////////////////////////////////////////////////////

#endif   /*  __PAIR_DATA_GENESIS_H__  */

////////////////////////////////////////////////////////////////////////

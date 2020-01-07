// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_PROB_RI_PAIR_INFO_H__
#define  __VX_PROB_RI_PAIR_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "atcf_prob_line.h"
#include "prob_rirw_info.h"
#include "track_info.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbRIRWPairInfo class stores arrays of ProbRIRWInfo and verifying
// TrackInfo objects.
//
////////////////////////////////////////////////////////////////////////

class ProbRIRWPairInfo {

   protected:

      void init_from_scratch();
      void assign(const ProbRIRWPairInfo &);

      ProbRIRWInfo  ProbRIRW;
      const TrackInfo * BDeck; // not allocated

      // BDeck Info
      ConcatString StormName;
      ConcatString BModel;
      double BLat, BLon;

      // Distances to Land
      double ADLand, BDLand;

      // Track Error and X/Y Errors
      double TrackErr, XErr, YErr;

      // Wind Speeds
      double BBegV, BEndV;
      double BMinV, BMaxV;

      // Cyclone Levels
      CycloneLevel BBegLev, BEndLev;

      // TCStatLine read for this pair
      TCStatLine Line;

   public:

      ProbRIRWPairInfo();
     ~ProbRIRWPairInfo();
      ProbRIRWPairInfo(const ProbRIRWPairInfo &);
      ProbRIRWPairInfo & operator=(const ProbRIRWPairInfo &);

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString case_info()               const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  get stuff
         //

      const ProbRIRWInfo & prob_rirw()  const;
      const TrackInfo *    bdeck()      const;
      const ConcatString & storm_name() const;
      const ConcatString & bmodel()     const;
            double         blat()       const;
            double         blon()       const;
            double         adland()     const;
            double         bdland()     const;
            double         track_err()  const;
            double         x_err()      const;
            double         y_err()      const;
            double         bbegv()      const;
            double         bendv()      const;
            double         bminv()      const;
            double         bmaxv()      const;
            CycloneLevel   bbeglev()    const;
            CycloneLevel   bendlev()    const;
      const TCStatLine &   line()       const;

         //
         //  do stuff
         //

      bool set(const ProbRIRWInfo &, const TrackInfo &);
      void set(const TCStatLine &);

      void set_adland(double);
      void set_bdland(double);

};

////////////////////////////////////////////////////////////////////////

inline const ProbRIRWInfo & ProbRIRWPairInfo::prob_rirw()  const { return(ProbRIRW);  }
inline const TrackInfo *    ProbRIRWPairInfo::bdeck()      const { return(BDeck);     }
inline const ConcatString & ProbRIRWPairInfo::storm_name() const { return(StormName); }
inline const ConcatString & ProbRIRWPairInfo::bmodel()     const { return(BModel);    }
inline       double         ProbRIRWPairInfo::blat()       const { return(BLat);      }
inline       double         ProbRIRWPairInfo::blon()       const { return(BLon);      }
inline       double         ProbRIRWPairInfo::adland()     const { return(ADLand);    }
inline       double         ProbRIRWPairInfo::bdland()     const { return(BDLand);    }
inline       double         ProbRIRWPairInfo::track_err()  const { return(TrackErr);  }
inline       double         ProbRIRWPairInfo::x_err()      const { return(XErr);      }
inline       double         ProbRIRWPairInfo::y_err()      const { return(YErr);      }
inline       double         ProbRIRWPairInfo::bbegv()      const { return(BBegV);     }
inline       double         ProbRIRWPairInfo::bendv()      const { return(BEndV);     }
inline       double         ProbRIRWPairInfo::bminv()      const { return(BMinV);     }
inline       double         ProbRIRWPairInfo::bmaxv()      const { return(BMaxV);     }
inline       CycloneLevel   ProbRIRWPairInfo::bbeglev()    const { return(BBegLev);   }
inline       CycloneLevel   ProbRIRWPairInfo::bendlev()    const { return(BEndLev);   }
inline const TCStatLine &   ProbRIRWPairInfo::line()       const { return(Line);      }

inline void ProbRIRWPairInfo::set_adland(double d) { ADLand = d; return; }
inline void ProbRIRWPairInfo::set_bdland(double d) { BDLand = d; return; }

////////////////////////////////////////////////////////////////////////
//
// ProbRIRWPairInfoArray class stores an array of ProbRIRWPairInfo objects.
//
////////////////////////////////////////////////////////////////////////

class ProbRIRWPairInfoArray {

   private:

      void init_from_scratch();
      void assign(const ProbRIRWPairInfoArray &);

      vector<ProbRIRWPairInfo> Pairs;

   public:

      ProbRIRWPairInfoArray();
     ~ProbRIRWPairInfoArray();
      ProbRIRWPairInfoArray(const ProbRIRWPairInfoArray &);
      ProbRIRWPairInfoArray & operator=(const ProbRIRWPairInfoArray &);

      void clear();

      void         dump(ostream &, int = 0) const;
      ConcatString serialize()              const;
      ConcatString serialize_r(int = 0)     const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      const ProbRIRWPairInfo & operator[](int) const;
      int n_pairs() const;

         //
         //  do stuff
         //

      void add(const ProbRIRWPairInfo &);
      bool add(const ProbRIRWInfo &, const TrackInfo &);
};

////////////////////////////////////////////////////////////////////////

inline int ProbRIRWPairInfoArray::n_pairs() const { return(Pairs.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_RI_PAIR_INFO_H__  */

////////////////////////////////////////////////////////////////////////

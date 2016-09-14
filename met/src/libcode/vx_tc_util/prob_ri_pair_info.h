// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include "prob_ri_info.h"
#include "track_info.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// ProbRIPairInfo class stores arrays of ProbRIInfo and verifying
// TrackInfo objects.
//
////////////////////////////////////////////////////////////////////////

class ProbRIPairInfo {

   protected:

      void init_from_scratch();
      void assign(const ProbRIPairInfo &);

      ProbRIInfo  ProbRI;
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

      ProbRIPairInfo();
     ~ProbRIPairInfo();
      ProbRIPairInfo(const ProbRIPairInfo &);
      ProbRIPairInfo & operator=(const ProbRIPairInfo &);

      void clear();

      void         dump(ostream &, int = 0)  const;
      ConcatString case_info()               const;
      ConcatString serialize()               const;
      ConcatString serialize_r(int, int = 0) const;

         //
         //  get stuff
         //

      const ProbRIInfo &   prob_ri()    const;
      const TrackInfo *    bdeck()      const;
      const ConcatString & storm_name() const;
      const ConcatString & bmodel()     const;
      const double         blat()       const;
      const double         blon()       const;
      const double         adland()     const;
      const double         bdland()     const;
      const double         track_err()  const;
      const double         x_err()      const;
      const double         y_err()      const;
      const double         bbegv()      const;
      const double         bendv()      const;
      const double         bminv()      const;
      const double         bmaxv()      const;
      const CycloneLevel   bbeglev()    const;
      const CycloneLevel   bendlev()    const;
      const TCStatLine &   line()       const;

         //
         //  do stuff
         //

      bool set(const ProbRIInfo &, const TrackInfo &);
      void set(const TCStatLine &);

      void set_adland(double);
      void set_bdland(double);

};

////////////////////////////////////////////////////////////////////////

inline const ProbRIInfo &   ProbRIPairInfo::prob_ri()    const { return(ProbRI);    }
inline const TrackInfo *    ProbRIPairInfo::bdeck()      const { return(BDeck);     }
inline const ConcatString & ProbRIPairInfo::storm_name() const { return(StormName); }
inline const ConcatString & ProbRIPairInfo::bmodel()     const { return(BModel);    }
inline const double         ProbRIPairInfo::blat()       const { return(BLat);      }
inline const double         ProbRIPairInfo::blon()       const { return(BLon);      }
inline const double         ProbRIPairInfo::adland()     const { return(ADLand);    }
inline const double         ProbRIPairInfo::bdland()     const { return(BDLand);    }
inline const double         ProbRIPairInfo::track_err()  const { return(TrackErr);  }
inline const double         ProbRIPairInfo::x_err()      const { return(XErr);      }
inline const double         ProbRIPairInfo::y_err()      const { return(YErr);      }
inline const double         ProbRIPairInfo::bbegv()      const { return(BBegV);     }
inline const double         ProbRIPairInfo::bendv()      const { return(BEndV);     }
inline const double         ProbRIPairInfo::bminv()      const { return(BMinV);     }
inline const double         ProbRIPairInfo::bmaxv()      const { return(BMaxV);     }
inline const CycloneLevel   ProbRIPairInfo::bbeglev()    const { return(BBegLev);   }
inline const CycloneLevel   ProbRIPairInfo::bendlev()    const { return(BEndLev);   }
inline const TCStatLine &   ProbRIPairInfo::line()       const { return(Line);      }

inline void ProbRIPairInfo::set_adland(double d) { ADLand = d; return; }
inline void ProbRIPairInfo::set_bdland(double d) { BDLand = d; return; }

////////////////////////////////////////////////////////////////////////
//
// ProbRIPairInfoArray class stores an array of ProbRIPairInfo objects.
//
////////////////////////////////////////////////////////////////////////

class ProbRIPairInfoArray {

   private:

      void init_from_scratch();
      void assign(const ProbRIPairInfoArray &);

      vector<ProbRIPairInfo> Pairs;

   public:

      ProbRIPairInfoArray();
     ~ProbRIPairInfoArray();
      ProbRIPairInfoArray(const ProbRIPairInfoArray &);
      ProbRIPairInfoArray & operator=(const ProbRIPairInfoArray &);

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

      const ProbRIPairInfo & operator[](int) const;
      int n_pairs() const;

         //
         //  do stuff
         //

      void add(const ProbRIPairInfo &);
      bool add(const ProbRIInfo &, const TrackInfo &);
};

////////////////////////////////////////////////////////////////////////

inline int ProbRIPairInfoArray::n_pairs() const { return(Pairs.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __VX_PROB_RI_PAIR_INFO_H__  */

////////////////////////////////////////////////////////////////////////

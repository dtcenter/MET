// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef __MASK_SID_H__
#define __MASK_SID_H__

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////
//
// Class to store masking station id information
//
////////////////////////////////////////////////////////////////////////

class MaskSID {

   void init_from_scratch();

   void assign(const MaskSID &);

   // Mask name
   ConcatString Name;

   // Boolean for non-default weights 
   bool HasWeights;

   // Mapping of SID name to weight value
   std::map<std::string,double> SIDMap;

   public:

      MaskSID();
     ~MaskSID();
      MaskSID(const MaskSID &);
      MaskSID & operator=(const MaskSID &) noexcept;

      void clear();
      bool operator==(const MaskSID &) const;

      int n() const;
      std::string name() const;
      const std::map<std::string,double> & sid_map() const;

      void set_name(const std::string &);

      // Formatted as: station_name(numeric_weight)
      void add(const std::string &);
      void add_css(const std::string &);
      bool has(const std::string &) const ;
      bool has(const std::string &, double &) const;
};

////////////////////////////////////////////////////////////////////////

inline int         MaskSID::n()    const { return (int) SIDMap.size(); }
inline std::string MaskSID::name() const { return Name;                }

////////////////////////////////////////////////////////////////////////

#endif   //  __MASK_SID_H__

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef __POINT_WEIGHT_H__
#define __POINT_WEIGHT_H__

#include "vx_util.h"

#include "config_constants.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration for point_weight_flag configuration parameter
//

enum class PointWeightType {
   None, // Apply no point weighting
   SID,  // Apply station ID weighting
   KDE   // Apply kernel density estimation weighting
};

//
// Struct to store point location information
//
struct SIDWeight {
   std::string SID;
   double Lat;
   double Lon;
   double Wgt;
};

////////////////////////////////////////////////////////////////////////
//
// Class to store point weighting information
//
////////////////////////////////////////////////////////////////////////

class PointWeightInfo {

   void assign(const PointWeightInfo &);

   // Point weighting type 
   PointWeightType Type;

   // KDE Reference Angle
   double KDERefAngle;

   // Mapping of SID name to location and weight values
   std::vector<SIDWeight> SIDWeights;

   // Keep track of whether the weights have already been computed
   bool WeightsComputed;

   // Write weights to output file
   bool WriteWeights;

   public:

      PointWeightInfo();
     ~PointWeightInfo(); 
      PointWeightInfo(const PointWeightInfo &);
      PointWeightInfo & operator=(const PointWeightInfo &a) noexcept;

      void clear();

      void set_type(PointWeightType);
      void set_kde_ref_angle(double);
      void set_write_weights(bool);

      PointWeightType type() const;
      double kde_ref_angle() const;
      bool write_weights() const; 

      bool need_sid() const;
      void add_sid(const std::string &, double, double);
      bool has_sid(const std::string &, double &) const;
      void compute_kde_weights();
      int n() const;
};

////////////////////////////////////////////////////////////////////////

inline void PointWeightInfo::set_type(PointWeightType t) { Type = t; }
inline void PointWeightInfo::set_kde_ref_angle(double a) { KDERefAngle = a; }
inline void PointWeightInfo::set_write_weights(bool b) { WriteWeights = b; }
inline PointWeightType PointWeightInfo::type() const { return Type; }
inline double PointWeightInfo::kde_ref_angle() const { return KDERefAngle; }
inline bool PointWeightInfo::write_weights() const { return WriteWeights; }
inline bool PointWeightInfo::need_sid() const { return Type == PointWeightType::KDE; }
inline int PointWeightInfo::n() const { return (int) SIDWeights.size(); }

////////////////////////////////////////////////////////////////////////

extern PointWeightInfo parse_conf_point_weight(Dictionary *dict);

////////////////////////////////////////////////////////////////////////

#endif   //  __POINT_WEIGHT_H__

////////////////////////////////////////////////////////////////////////


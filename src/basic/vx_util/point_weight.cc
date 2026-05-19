// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_util.h"

#include "config_file.h"
#include "config_constants.h"

#include "point_weight.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Code for PointWeightInfo struct
//
///////////////////////////////////////////////////////////////////////////////

PointWeightInfo::PointWeightInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo::~PointWeightInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo::PointWeightInfo(const PointWeightInfo &m) {
   assign(m);
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo & PointWeightInfo::operator=(const PointWeightInfo &m) noexcept {

   if(this == &m) return *this;

   assign(m);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void PointWeightInfo::clear() {
   Type = PointWeightType::None;
   KDERefAngle = bad_data_double;
   SIDWeights.clear();
   WeightsComputed = false;
   WriteWeights = false;
   WeightFilePrefix.clear();
}

////////////////////////////////////////////////////////////////////////

void PointWeightInfo::assign(const PointWeightInfo & m) {
   Type = m.Type;
   KDERefAngle = m.KDERefAngle;
   SIDWeights = m.SIDWeights;
   WeightsComputed = m.WeightsComputed;
   WriteWeights = m.WriteWeights;
   WeightFilePrefix = m.WeightFilePrefix;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::add_sid(const string &sid, double lat, double lon) {

   // Add unique station id locations with which to compute weights
   if(!has_sid(sid)) {
      SIDWeight e = {sid, lat, lon, bad_data_double};
      SIDWeights.emplace_back(e);
      WeightsComputed = false;
   }
}

///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::add_wgt(const string &sid, double wgt) {

   // Add pre-computed weights for unique station ids
   if(!has_sid(sid)) {
      SIDWeight e = {sid, bad_data_double, bad_data_double, wgt};
      SIDWeights.emplace_back(e);
      WeightsComputed = true;
   }
}

///////////////////////////////////////////////////////////////////////////////

bool PointWeightInfo::has_sid(const string &sid) const {
   const char *method_name = "PointWeightInfo()::has_sid() -> ";
   bool found = false;

   // Search for a matching entry
   for(const auto &e : SIDWeights) {
      if(e.SID == sid) {
         found = true;
         break;
      }
   }

   return found;
}

///////////////////////////////////////////////////////////////////////////////

bool PointWeightInfo::has_sid(const string &sid, double &wgt) const {
   const char *method_name = "PointWeightInfo()::has_sid() -> ";
   bool found = false;

   // Check that the weights have been computed
   if(!WeightsComputed) {
      mlog << Warning << "\n" << method_name
           << "Accessing point weights before computing them!\n\n";
   }

   // Search for a matching entry
   for(const auto &e : SIDWeights) {
      if(e.SID == sid) {
         found = true;
         wgt = e.Wgt;
         break;
      }
   }

   // Return bad data for no match
   if(!found) wgt = bad_data_double;
 
   return found;
}

///////////////////////////////////////////////////////////////////////////////
//
// Reference:
//    Haiden, T., M.J. Rodwell, D.S. Richardson, A. Okagaki, T. Robinson, T. Hewson, 2012:
//    Intercomparison of Global Model Precipitation Forecast Skill in 2010/11
//    Using the SEEPS Score. Monthly Weather Review, 140, 2720-2733.
//    doi.org/10.1175/MWR-D-11-00301.1
//
///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::compute_kde_weights() {

   // Check for no work to do
   if(Type != PointWeightType::KDE || WeightsComputed) return;

   mlog << Debug(3) << "Computing KDE point weights using " << n_stn()
        << " observation locations.\n";

   // Store sums for the weights 
   vector<double> p_sum(n_stn(), 0.0);

   // Define e constant
   const double e = exp(1.0);

#pragma omp declare reduction(vec_double_plus : vector<double> :          \
                              transform(omp_out.begin(), omp_out.end(),   \
                                         omp_in.begin(), omp_out.begin(), \
                                        plus<double>()))                  \
                    initializer(omp_priv = decltype(omp_orig)(omp_orig.size()))

#pragma omp parallel default (none) \
   shared(SIDWeights, KDERefAngle, p_sum, e)
   {

      // Compute the sums of the pairwise distances
#pragma omp for reduction(vec_double_plus: p_sum) \
                collapse(2)
      for(int i=0; i<n_stn(); i++) {
         for(int j=i+1; j<n_stn(); j++) {
            double dlat = SIDWeights[i].Lat - SIDWeights[j].Lat;
            double dlon = angle_between(SIDWeights[i].Lon, SIDWeights[j].Lon);
            double ang  = sqrt(dlat*dlat + dlon*dlon);
            double exp  = ang/KDERefAngle;
            double p    = pow(e, -exp*exp);
            p_sum[i]   += p;
            p_sum[j]   += p;
         }
      }

      // Compute weights as the inverse of the p sums
#pragma omp for schedule(static)
      for(int i=0; i<n_stn(); i++) SIDWeights[i].Wgt = 1.0/p_sum[i];

   } // End omp parallel

   // Dump weights for high verbosity
   if(mlog.verbosity_level() >= 7) {
      mlog << Debug(7) << "Computed KDE weights for " << n_stn()
           << " observation locations:\n";
      for(int i=0; i<n_stn(); i++) {
         mlog << Debug(7) << " [" << i+1 << "] " << SIDWeights[i].SID
              << " (" << SIDWeights[i].Lat << ", " << -1.0*SIDWeights[i].Lon
              << ") " << SIDWeights[i].Wgt << "\n";
      }
   }

   // Note that the weights have been computed
   WeightsComputed = true;
}

///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::write_weights() const {
   const char *method_name = "PointWeightInfo()::write_weights() -> ";

   if(!WriteWeights || Type == PointWeightType::None) return;

   // Check for empty string
   if(WeightFilePrefix.empty()) {
      mlog << Error << "\n" << method_name
           << "the weight file prefix has not been set!\n\n";
      exit(1);
   }

   // Construct the output file name
   ConcatString file_name(WeightFilePrefix);
   ConcatString type_str(conf_val_none);
        if(Type == PointWeightType::SID) type_str = conf_val_sid;
   else if(Type == PointWeightType::KDE) type_str = conf_val_kde;
   file_name << "_" << type_str << "_point_weights.txt";

   // Open the output file
   ofstream out(file_name.c_str());
   if(!out) {
      mlog << Error << "\n" << method_name
           << "can't open the output file \"" << file_name
           << "\" for writing!\n\n";
      exit(1);
   }

   // List the output file
   mlog << Debug(1) << "Point weight file: " << file_name << "\n";

   // Write the weights
   out << type_str << "_POINT_WEIGHTS\n";
   for(int i=0; i<n_stn(); i++) {
      out << SIDWeights[i].SID << "(" << SIDWeights[i].Wgt << ")\n";
   }

   // Close the output file
   out.close();
}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
 
PointWeightInfo parse_conf_point_weight(Dictionary *dict) {
   const char *method_name = "parse_conf_point_weight() -> ";
   PointWeightInfo info;
   
   if(!dict) {
      mlog << Error << "\n" << method_name
           << "empty dictionary!\n\n";
      exit(1);
   } 

   // Conf: point_weight_flag
   
   // Get the integer flag value for the current entry
   int v = dict->lookup_int(conf_key_point_weight_flag);

   // Parse config constant values
   MetConfig conf_const(replace_path(config_const_filename).c_str());

   // Convert integer to enumerated GridWeightType
        if(v == conf_const.lookup_int(conf_val_none)) info.set_type(PointWeightType::None);
   else if(v == conf_const.lookup_int(conf_val_sid))  info.set_type(PointWeightType::SID);
   else if(v == conf_const.lookup_int(conf_val_kde))  info.set_type(PointWeightType::KDE);
   else {
      mlog << Error << "\n" << method_name
           << "Unexpected config file value of " << v << " for \""
           << conf_key_point_weight_flag << "\".\n\n";
      exit(1);
   }  
   
   // Conf: kde_ref_angle
   info.set_kde_ref_angle(dict->lookup_double(conf_key_kde_ref_angle));

   // Conf: write_weights
   info.set_write_weights(dict->lookup_bool(conf_key_write_weights));

   return info;
}

///////////////////////////////////////////////////////////////////////////////

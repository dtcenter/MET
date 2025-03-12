// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __IODA_H__
#define  __IODA_H__

////////////////////////////////////////////////////////////////////////

#include <netcdf>

#include "pair_base.h"
#include "ioda_data_conf_info.h"

////////////////////////////////////////////////////////////////////////

enum class e_ioda_format { v1, v2 };

////////////////////////////////////////////////////////////////////////

struct IODAHeaders {
   void clear();
};

////////////////////////////////////////////////////////////////////////

struct IODAMetadata {

   void clear();
};

////////////////////////////////////////////////////////////////////////

class IODAReader {
   public:

      int nlocs;
      int nrecs;
      int nvars;
      int ndatetime;
      int nstring;

      StringArray dim_names;
      StringArray metadata_vars;
      StringArray obs_value_vars;

      ConcatString nlocs_name;
      ConcatString datetime_name;
      ConcatString lon_name;
      ConcatString lat_name;
      ConcatString msg_type_name;
      ConcatString station_id_name;

      std::vector<double> lat_arr;
      std::vector<double> lon_arr;
      std::vector<double> elv_arr;
      std::vector<unixtime> vld_arr;   // valid times

      std::vector<std::string> msg_types;
      std::vector<std::string> station_ids;

      std::vector<double> obs_pres_arr;
      std::vector<double> obs_hght_arr;

   private:
      netCDF::NcFile *f_in;
      IODADataConfInfo conf_info;

      e_ioda_format ioda_format;
      std::vector<point_pair_t> point_pairs;

   public:
      bool validate_metadata() const;
      bool check_missing_thresh(double value) const;
      void clear();
      e_ioda_format get_format_ver() const;
      std::vector<point_pair_t> *get_point_pairs(const ConcatString &fcst_name,
                                                 const ConcatString &obs_name,
                                                 const int channel=bad_data_int);
      bool is_in_metadata_map(const std::string &metadata_key,
                              const StringArray &available_list) const;
      void read_ioda(netCDF::NcFile *f_in);
      void set_data_config(const char *, const char *);

   private:
      e_ioda_format find_ioda_format(netCDF::NcFile *_f_in);
      ConcatString find_meta_name(const std::string &meta_key,
                                  const StringArray &available_names) const;
      bool get_meta_data_chars(netCDF::NcVar &var, char *metadata_buf) const;
      bool get_meta_data_double(const char *metadata_key, double *metadata_buf);
      void get_obs_metadata_names_v1();
      void get_obs_metadata_names_v2();
      void read_header();
      void read_metadata_names();
      bool read_point_data(const ConcatString &data_name,
                           const int channel,
                           std::vector<double> &vals);
      bool read_string_data(const char *var_name,
                            std::vector<std::string> &hdr_data, int str_length);
      bool read_time();
      bool read_time_as_number(netCDF::NcVar *hdr_vld_var);

};

////////////////////////////////////////////////////////////////////////

inline e_ioda_format IODAReader::get_format_ver() const { return ioda_format; };

////////////////////////////////////////////////////////////////////////

#endif   /*  __IODA_H__  */

////////////////////////////////////////////////////////////////////////

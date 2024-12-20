

////////////////////////////////////////////////////////////////////////


#ifndef  __IODA_H__
#define  __IODA_H__


////////////////////////////////////////////////////////////////////////

#include <netcdf>

#include "pair_base.h"
#include "ioda_data_conf_info.h"

////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

enum class e_ioda_format { v1, v2 };

////////////////////////////////////////////////////////////////////////

struct iodaHeaders {
   void clear();
};

////////////////////////////////////////////////////////////////////////

struct iodaMetadata {

   void clear();
};

////////////////////////////////////////////////////////////////////////

class iodaFile {
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
      std::vector<unixtime> vld_arr;   // valid times`

      std::vector<std::string> msg_types;
      std::vector<std::string> station_ids;

      std::vector<double> obs_pres_arr;
      std::vector<double> obs_hght_arr;

   private:
      netCDF::NcFile *f_in;
      IODADataConfInfo *conf_info;

      e_ioda_format ioda_format;
      std::vector<point_pair_t> point_pairs;

   public:
      iodaFile();

      bool check_missing_thresh(float value) const;
      void clear();
      e_ioda_format get_format_ver() const;
      std::vector<point_pair_t> &get_point_pairs(const char *var_name_f, const char *var_name_o,
                                                 const char *group_name_f=nullptr,
                                                 const char *group_name_o=nullptr,
                                                 const int channel=bad_data_int);
      bool is_in_metadata_map(const std::string &metadata_key, const StringArray &available_list) const;
      void read_ioda(netCDF::NcFile *f_in);
      void set_data_config(IODADataConfInfo *_conf_info);

   private:
      e_ioda_format find_ioda_format(netCDF::NcFile *_f_in);
      ConcatString find_meta_name(const std::string &meta_key, const StringArray &available_names);
      bool get_meta_data_chars(netCDF::NcVar &var, char *metadata_buf) const;
      bool get_meta_data_double(const char *metadata_key, double *metadata_buf);
      void get_obs_metadata_names_v1();
      void get_obs_metadata_names_v2();
      void initialize();
      bool read_data_to_vector(const char *var_name,
                               std::vector<std::string> &hdr_data, int nstring);
      void read_header();
      void read_metadata_names();
      bool read_obs_data(double *data_buf, const char *var_name,
                         const char *group_name, const int channel);
      bool read_time();

};


////////////////////////////////////////////////////////////////////////

inline e_ioda_format iodaFile::get_format_ver() const { return ioda_format; };
inline void iodaFile::set_data_config(IODADataConfInfo *_conf_info) { conf_info = _conf_info; };

////////////////////////////////////////////////////////////////////////

#endif   /*  __IODA_H__  */


////////////////////////////////////////////////////////////////////////


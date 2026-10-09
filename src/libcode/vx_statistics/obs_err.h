// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __OBS_ERR_H__
#define  __OBS_ERR_H__

////////////////////////////////////////////////////////////////////////

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class ObsErrEntry {

   private:

      void init_from_scratch();

      void assign(const ObsErrEntry &);

   public:

      ObsErrEntry();
     ~ObsErrEntry();
      ObsErrEntry(const ObsErrEntry &);
      ObsErrEntry & operator=(const ObsErrEntry &);

      void clear();

      void dump(std::ostream &, int = 0) const;

      // Line number of the table
      int         line_number;

      // Observation matching criteria
      StringArray var_name;
      StringArray msg_type;
      StringArray sid;
      NumArray    pb_rpt_type;
      NumArray    in_rpt_type;
      NumArray    inst_type;
      NumArray    hgt_range;
      NumArray    prs_range;
      NumArray    val_range;

      // Observation error settings
      double      bias_scale;
      double      bias_offset;
      DistType    dist_type;
      NumArray    dist_parm;

      // Valid range of perturbed values
      double      v_min;
      double      v_max;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      double variance() const;

      // Check whether this entry actually requires bias correction
      // and/or perturbation
      bool need_bias_correction() const;
      bool need_perturbation() const;

         //
         //  do stuff
         //

      bool parse_line(const DataLine &);

      bool is_header(const DataLine &);

      bool is_match(const char *, const char *, const char *,
                    int, int, int, double, double, double,
                    bool skip_var_name = false);

      void validate();
};

////////////////////////////////////////////////////////////////////////

class ObsErrTable {

   private:

      void assign(const ObsErrTable &);

      void extend(int);

      std::vector<ObsErrEntry> e;   //  elements

      bool IsSet = false;

      // Cache of table row indices, subsetted by variable name, to
      // avoid rescanning (and re-running regex matches over) the full
      // table on every lookup() call for a given variable name
      std::map<std::string, std::vector<int>, std::less<>> VarSubsetCache;

      // Index of the most recently matched table row which is checked
      // first since consecutive lookups often produce the same match
      int LastMatchIndex = -1;

      const std::vector<int> & var_subset(const char *cur_var_name);

   public:

      ObsErrTable() = default;
     ~ObsErrTable();
      ObsErrTable(const ObsErrTable &);
      ObsErrTable(ObsErrTable &&) noexcept;
      ObsErrTable & operator=(const ObsErrTable &);
      ObsErrTable & operator=(ObsErrTable &&) noexcept;

      void clear();

      void dump(std::ostream &, int = 0) const;

         //
         // set stuff
         //

         //
         // get stuff
         //

      int n() const;

      bool is_set() const;

         //
         // do stuff
         //

      void initialize();

      bool read(const char * filename);

      // for point observations
      const ObsErrEntry * lookup(const char *, const char *, const char *,
                                 int, int, int, double, double, double);

      // for gridded analyses
      const ObsErrEntry * lookup(const char *, const char *,
                                 double cur_val = bad_data_double);

      bool has(const char *, const char *);
};

////////////////////////////////////////////////////////////////////////

inline int  ObsErrTable::n()      const { return (int) e.size(); }
inline bool ObsErrTable::is_set() const { return IsSet;      }

////////////////////////////////////////////////////////////////////////

//
//  Global instance of ObsErrTable
//

extern ObsErrTable obs_err_table;

////////////////////////////////////////////////////////////////////////

//
// Struct to store observation error information from config files
//

struct ObsErrInfo {
   bool          flag;  // TRUE or FALSE
   ObsErrEntry   entry; // Defines perturbation

   gsl_rng * rng_ptr;   // not allocated

   void clear();
   void validate();

   ObsErrInfo &operator=(const ObsErrInfo &a) noexcept;
};

////////////////////////////////////////////////////////////////////////

//
// External utility functions
//

extern ObsErrInfo   parse_conf_obs_err(Dictionary *dict, gsl_rng *);

extern double       add_obs_err_inc(const gsl_rng *, FieldType,
                                    const ObsErrEntry *, const double,
                                    double, bool log_detail = true);
extern DataPlane    add_obs_err_inc(const gsl_rng *, FieldType,
                                    const ObsErrEntry *,
                                    const DataPlane &in_dp,
                                    const DataPlane &obs_dp,
                                    const char *, const char *);

extern double       add_obs_err_bc(FieldType,
                                   const ObsErrEntry *, double,
                                   bool log_detail = true);
extern DataPlane    add_obs_err_bc(FieldType,
                                   const ObsErrEntry *,
                                   const DataPlane &in_dp,
                                   const DataPlane &obs_dp,
                                   const char *, const char *);

// Build a per-gridpoint cache of resolved ObsErrEntry pointers by
// doing one table lookup per point to avoid repeating the table
// lookup for each ensemble member.
extern std::vector<const ObsErrEntry *> build_obs_err_entry_grid(
                                    const DataPlane &val_dp,
                                    const char *var_name,
                                    const char *obtype);

// Variants that consume a precomputed per-gridpoint entry cache
// instead of a single entry or a var_name/obtype table lookup
extern DataPlane    add_obs_err_inc(const gsl_rng *, FieldType,
                                    const std::vector<const ObsErrEntry *> &entry_grid,
                                    const DataPlane &in_dp,
                                    const DataPlane &obs_dp);
extern DataPlane    add_obs_err_bc(FieldType,
                                   const std::vector<const ObsErrEntry *> &entry_grid,
                                   const DataPlane &in_dp);

////////////////////////////////////////////////////////////////////////

#endif   // __OBS_ERR_H__

////////////////////////////////////////////////////////////////////////

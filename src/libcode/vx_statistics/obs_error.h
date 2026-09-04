// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __OBS_ERROR_H__
#define  __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <vector>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class ObsErrorEntry {

   private:

      void init_from_scratch();

      void assign(const ObsErrorEntry &);

   public:

      ObsErrorEntry();
     ~ObsErrorEntry();
      ObsErrorEntry(const ObsErrorEntry &);
      ObsErrorEntry & operator=(const ObsErrorEntry &);

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

class ObsErrorTable {

   private:

      void init_from_scratch();

      void assign(const ObsErrorTable &);

      void extend(int);

      ObsErrorEntry * e;   //  elements ... allocated

      int N_elements;

      int N_alloc;

      bool IsSet;

      // Cache of table row indices, subsetted by variable name, to
      // avoid rescanning (and re-running regex matches over) the full
      // table on every lookup() call for a given variable name
      std::map<std::string, std::vector<int>> VarSubsetCache;

      // Index of the most recently matched table row which is checked
      // first since consecutive lookups often produce the same match
      int LastMatchIndex;

      const std::vector<int> & var_subset(const char *cur_var_name);

   public:

      ObsErrorTable();
     ~ObsErrorTable();
      ObsErrorTable(const ObsErrorTable &);
      ObsErrorTable & operator=(const ObsErrorTable &);

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
      ObsErrorEntry * lookup(const char *, const char *, const char *,
                             int, int, int, double, double, double);

      // for gridded analyses
      ObsErrorEntry * lookup(const char *, const char *,
                             double cur_val = bad_data_double);

      bool has(const char *, const char *);
};

////////////////////////////////////////////////////////////////////////

inline int  ObsErrorTable::n()      const { return N_elements; }
inline bool ObsErrorTable::is_set() const { return IsSet;      }

////////////////////////////////////////////////////////////////////////

//
//  Global instance of ObsErrorTable
//

extern ObsErrorTable obs_error_table;

////////////////////////////////////////////////////////////////////////

//
// Struct to store observation error information from config files
//

struct ObsErrorInfo {
   bool          flag;  // TRUE or FALSE
   ObsErrorEntry entry; // Defines perturbation

   gsl_rng * rng_ptr;   // not allocated

   void clear();
   void validate();

   ObsErrorInfo &operator=(const ObsErrorInfo &a) noexcept;
};

////////////////////////////////////////////////////////////////////////

//
// External utility functions
//

extern ObsErrorInfo parse_conf_obs_error(Dictionary *dict, gsl_rng *);

extern double       add_obs_error_inc(const gsl_rng *, FieldType,
                                      const ObsErrorEntry *, const double,
                                      double, bool log_detail = true);
extern DataPlane    add_obs_error_inc(const gsl_rng *, FieldType,
                                      const ObsErrorEntry *,
                                      const DataPlane &in_dp,
                                      const DataPlane &obs_dp,
                                      const char *, const char *);

extern double       add_obs_error_bc(FieldType,
                                     const ObsErrorEntry *, double,
                                     bool log_detail = true);
extern DataPlane    add_obs_error_bc(FieldType,
                                     const ObsErrorEntry *,
                                     const DataPlane &in_dp,
                                     const DataPlane &obs_dp,
                                     const char *, const char *);

// Build a per-gridpoint cache of resolved ObsErrorEntry pointers by
// doing one table lookup per point to avoid repeating the table
// lookup for each ensemble member.
extern std::vector<const ObsErrorEntry *> build_obs_error_entry_grid(
                                      const DataPlane &val_dp,
                                      const char *var_name,
                                      const char *obtype);

// Variants that consume a precomputed per-gridpoint entry cache
// instead of a single entry or a var_name/obtype table lookup
extern DataPlane    add_obs_error_inc(const gsl_rng *, FieldType,
                                      const std::vector<const ObsErrorEntry *> &entry_grid,
                                      const DataPlane &in_dp,
                                      const DataPlane &obs_dp);
extern DataPlane    add_obs_error_bc(FieldType,
                                     const std::vector<const ObsErrorEntry *> &entry_grid,
                                     const DataPlane &in_dp);

////////////////////////////////////////////////////////////////////////

#endif   // __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////

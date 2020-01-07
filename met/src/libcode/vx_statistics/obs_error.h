// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __OBS_ERROR_H__
#define  __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////

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

      void dump(ostream &, int = 0) const;

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

         //
         //  do stuff
         //

      bool parse_line(const DataLine &);

      bool is_header(const DataLine &);

      bool is_match(const char *, const char *, const char *,
                    int, int, int, double, double, double);

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

   public:

      ObsErrorTable();
     ~ObsErrorTable();
      ObsErrorTable(const ObsErrorTable &);
      ObsErrorTable & operator=(const ObsErrorTable &);

      void clear();

      void dump(ostream &, int = 0) const;

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

inline int  ObsErrorTable::n()      const { return(N_elements); }
inline bool ObsErrorTable::is_set() const { return(IsSet);      }

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
};

////////////////////////////////////////////////////////////////////////

//
// External utility functions
//

extern ObsErrorInfo parse_conf_obs_error(Dictionary *dict, gsl_rng *);

extern double       add_obs_error_inc(const gsl_rng *, FieldType,
                                      const ObsErrorEntry *, const double,
                                      double);
extern DataPlane    add_obs_error_inc(const gsl_rng *, FieldType,
                                      const ObsErrorEntry *,
                                      const DataPlane &in_dp,
                                      const DataPlane &obs_dp,
                                      const char *, const char *);

extern double       add_obs_error_bc(const gsl_rng *, FieldType,
                                     const ObsErrorEntry *, double);
extern DataPlane    add_obs_error_bc(const gsl_rng *, FieldType,
                                     const ObsErrorEntry *,
                                     const DataPlane &in_dp,
                                     const DataPlane &obs_dp,
                                     const char *, const char *);

////////////////////////////////////////////////////////////////////////

#endif   // __OBS_ERROR_H__

////////////////////////////////////////////////////////////////////////

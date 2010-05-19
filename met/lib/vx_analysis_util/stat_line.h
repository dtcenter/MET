// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_ANALYSIS_STAT_H__
#define  __MET_ANALYSIS_STAT_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "vx_analysis_util/data_line.h"

#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Enumerate all the possible line types
   //


enum STATLineType {

      //
      //  STAT Line Types
      //

   stat_sl1l2  = 0,
   stat_sal1l2 = 1,
   stat_vl1l2  = 2,
   stat_val1l2 = 3,
   stat_fho    = 4,
   stat_ctc    = 5,
   stat_cts    = 6,
   stat_cnt    = 7,
   stat_pct    = 8,
   stat_pstd   = 9,
   stat_pjc    = 10,
   stat_prc    = 11,
   stat_mpr    = 12,
   stat_nbrctc = 13,
   stat_nbrcts = 14,
   stat_nbrcnt = 15,
   stat_isc    = 16,
   stat_wdir   = 17,
   stat_rhist  = 18,
   stat_orank  = 19,

      //
      //  flag value
      //

   no_stat_line_type = 20

};
static const int n_statlinetypes = 21;

static const char * const statlinetype_str[n_statlinetypes] = {
   "SL1L2", "SAL1L2", "VL1L2",  "VAL1L2",
   "FHO",   "CTC",    "CTS",    "CNT",
   "PCT",   "PSTD",   "PJC",    "PRC",
   "MPR",   "NBRCTC", "NBRCTS", "NBRCNT",
   "ISC",   "WDIR",   "RHIST",  "ORANK",
   "NA"
};

extern const char * statlinetype_to_string(const STATLineType);
extern void         statlinetype_to_string(const STATLineType, char *);
extern STATLineType string_to_statlinetype(const char *);

////////////////////////////////////////////////////////////////////////


class STATLine : public DataLine {

   private:

      void determine_line_type();

      STATLineType Type;

   public:

      STATLine();
     ~STATLine();
      STATLine(const STATLine &);
      STATLine & operator=(const STATLine &);

      void dump(ostream &, int depth = 0) const;

      int read_line(LineDataFile *);   //  virtual from base class

      int is_ok() const;               //  virtual from base class

         //
         //  retrieve values of the header columns
         //

      const char * get_item   (int) const;

      const char * version       () const;
      const char * model         () const;
      int          fcst_lead     () const;
      unixtime     fcst_valid_beg() const;
      unixtime     fcst_valid_end() const;
      int          obs_lead      () const;
      unixtime     obs_valid_beg () const;
      unixtime     obs_valid_end () const;
      const char * fcst_var      () const;
      const char * fcst_lev      () const;
      const char * obs_var       () const;
      const char * obs_lev       () const;
      const char * obtype        () const;
      const char * vx_mask       () const;
      const char * interp_mthd   () const;
      int          interp_pnts   () const;
      SingleThresh fcst_thresh   () const;
      SingleThresh obs_thresh    () const;
      SingleThresh cov_thresh    () const;
      double       alpha         () const;
      const char * line_type     () const;

         //
         //  retrieve stuff
         //

      unixtime fcst_init_beg   () const; // fcst_valid_beg - fcst_lead
      unixtime fcst_init_end   () const; // fcst_valid_beg - fcst_lead
      unixtime obs_init_beg    () const; // obs_valid_beg  - obs_lead
      unixtime obs_init_end    () const; // obs_valid_beg  - obs_lead

      int      fcst_init_hour  () const; // fcst_init_beg%sec_per_day
      int      obs_init_hour   () const; // obs_init_beg%sec_per_day

      STATLineType type        () const;

};


////////////////////////////////////////////////////////////////////////


inline  STATLineType  STATLine::type () const { return ( Type ); }


////////////////////////////////////////////////////////////////////////


extern StringArray get_stat_filenames(const StringArray &);

extern StringArray get_stat_filenames_from_dir(
                      const char * directory_path);

extern int is_stat_filename(const char * path);

extern int determine_column_offset(STATLineType, const char *);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_ANALYSIS_STAT_H__  */


////////////////////////////////////////////////////////////////////////



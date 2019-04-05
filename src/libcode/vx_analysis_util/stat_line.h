// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __STAT_LINE_H__
#define  __STAT_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_config.h"


////////////////////////////////////////////////////////////////////////


class STATLine : public DataLine {

   private:

      STATLineType Type;

      const AsciiHeaderLine *HdrLine;   //  not allocated

      void init_from_scratch();

      void assign(const STATLine &);

   public:

      STATLine();
     ~STATLine();
      STATLine(const STATLine &);
      STATLine & operator=(const STATLine &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int read_line(LineDataFile *);   //  virtual from base class

      int is_ok() const;               //  virtual from base class

      int is_header() const;           //  virtual from base class

         //
         //  retrieve values of the header columns
         //

      bool         has      (const char *)                       const;
      ConcatString get      (const char *, bool check_na = true) const;
      const char * get_item (const char *, bool check_na = true) const;
      const char * get_item (int,          bool check_na = true) const;

      const char * version        () const;
      const char * model          () const;
      const char * desc           () const;

      int          fcst_lead      () const;
      unixtime     fcst_valid_beg () const;
      unixtime     fcst_valid_end () const;
      int          fcst_valid_hour() const;

      int          obs_lead       () const;
      unixtime     obs_valid_beg  () const;
      unixtime     obs_valid_end  () const;
      int          obs_valid_hour () const;

      const char * fcst_var       () const;
      const char * fcst_units     () const;
      const char * fcst_lev       () const;
      const char * obs_var        () const;
      const char * obs_units      () const;
      const char * obs_lev        () const;
      const char * obtype         () const;
      const char * vx_mask        () const;
      const char * interp_mthd    () const;
      int          interp_pnts    () const;
      ThreshArray  fcst_thresh    () const;
      ThreshArray  obs_thresh     () const;
      SetLogic     thresh_logic   () const;
      ThreshArray  cov_thresh     () const;
      double       alpha          () const;
      const char * line_type      () const;

         //
         //  retrieve stuff
         //

      unixtime fcst_init_beg   () const; // fcst_valid_beg - fcst_lead
      unixtime fcst_init_end   () const; // fcst_valid_beg - fcst_lead
      int      fcst_init_hour  () const; // fcst_init_beg%sec_per_day

      unixtime obs_init_beg    () const; // obs_valid_beg - obs_lead
      unixtime obs_init_end    () const; // obs_valid_beg - obs_lead
      int      obs_init_hour   () const; // obs_init_beg%sec_per_day

      STATLineType type        () const;

};


////////////////////////////////////////////////////////////////////////


inline  STATLineType  STATLine::type () const { return ( Type ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __STAT_LINE_H__  */


////////////////////////////////////////////////////////////////////////

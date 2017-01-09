// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_LINE_H__
#define  __MODE_LINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "vx_util.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class ModeLine : public DataLine {

   private:

      bool HdrFlag;

      const AsciiHeaderLine *HdrLine;   //  not allocated

      void init_from_scratch();

      void assign(const ModeLine &);

   public:

      ModeLine();
     ~ModeLine();
      ModeLine(const ModeLine &);
      ModeLine & operator=(const ModeLine &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int read_line(LineDataFile *);   //  virtual from base class

      int is_ok() const;               //  virtual from base class

      int is_header() const;           //  virtual from base class

         //
         //  retrieve stuff
         //

      const char * get_item (const char *, bool check_na = true) const;
      const char * get_item (int,          bool check_na = true) const;

      const char * version                    () const;   //  column  1
      const char * model                      () const;   //  column  2
      const char * desc                       () const;   //  column  3

      int          fcst_lead                  () const;   //  column  4
      unixtime     fcst_valid                 () const;   //  column  5
      int          fcst_valid_hour            () const;   //  valid%sec_per_day
      unixtime     fcst_init                  () const;   //  Compute as valid - lead
      int          fcst_init_hour             () const;   //  HHMMSS portion of init time
      int          fcst_accum                 () const;   //  column  6

      int          obs_lead                   () const;   //  column  7
      unixtime     obs_valid                  () const;   //  column  8
      int          obs_valid_hour             () const;   //  valid%sec_per_day
      unixtime     obs_init                   () const;   //  Compute as valid - lead
      int          obs_init_hour              () const;   //  HHMMSS portion of init time
      int          obs_accum                  () const;   //  column  9

      int          fcst_rad                   () const;   //  column 10
      const char * fcst_thr                   () const;   //  column 11
      int          obs_rad                    () const;   //  column 12
      const char * obs_thr                    () const;   //  column 13

      const char * fcst_var                   () const;   //  column 14
      const char * fcst_lev                   () const;   //  column 15
      const char * obs_var                    () const;   //  column 16
      const char * obs_lev                    () const;   //  column 17

      const char * object_id                  () const;   //  column 18
      const char * object_cat                 () const;   //  column 19

      double       centroid_x                 () const;   //  column 20
      double       centroid_y                 () const;   //  column 21
      double       centroid_lat               () const;   //  column 22
      double       centroid_lon               () const;   //  column 23

      double       axis_ang                   () const;   //  column 24

      double       length                     () const;   //  column 25
      double       width                      () const;   //  column 26
      double       aspect_ratio               () const;   //  Compute as width/length

      int          area                       () const;   //  column 27
      int          area_filter                () const;   //  column 28
      int          area_thresh                () const;   //  column 29

      double       curvature                  () const;   //  column 30
      double       curvature_x                () const;   //  column 31
      double       curvature_y                () const;   //  column 32

      double       complexity                 () const;   //  column 33

      double       intensity_10               () const;   //  column 34
      double       intensity_25               () const;   //  column 35
      double       intensity_50               () const;   //  column 36
      double       intensity_75               () const;   //  column 37
      double       intensity_90               () const;   //  column 38
      double       intensity_user             () const;   //  column 39

      double       intensity_sum              () const;   //  column 40
      double       centroid_dist              () const;   //  column 41
      double       boundary_dist              () const;   //  column 42
      double       convex_hull_dist           () const;   //  column 43
      double       angle_diff                 () const;   //  column 44

      double       area_ratio                 () const;   //  column 45
      int          intersection_area          () const;   //  column 46
      int          union_area                 () const;   //  column 47
      int          symmetric_diff             () const;   //  column 48
      double       intersection_over_area     () const;   //  column 49

      double       complexity_ratio           () const;   //  column 50
      double       percentile_intensity_ratio () const;   //  column 51

      double       interest                   () const;   //  column 52

         //
         //
         //

      int          is_fcst                    () const;
      int          is_obs                     () const;

      int          is_single                  () const;
      int          is_pair                    () const;

      int          is_simple                  () const;
      int          is_cluster                 () const;

      int          is_matched                 () const;
      int          is_unmatched               () const;

         //
         //  object numbers start at zero, with -1 as a flag value
         //

      int simple_obj_number   () const;   //  has to be a single simple object,
                                          //  else -1

      int cluster_obj_number() const;     //  if it's a pair, then -1
                                          //
                                          //  if it's simple, then the number of
                                          //    the cluster it's part of, if any
                                          //
                                          //  if it's a cluster, then the number

      bool pair_obj_numbers(int & nf, int & no) const;   //  returns -1 if it's not a pair
                                                         //
                                                         //   can be either simple or cluster

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_LINE_H__  */


////////////////////////////////////////////////////////////////////////



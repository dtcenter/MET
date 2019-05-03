// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

      const char * version                    () const;
      const char * model                      () const;
      const char * desc                       () const;

      int          fcst_lead                  () const;
      unixtime     fcst_valid                 () const;
      int          fcst_valid_hour            () const;   //  valid%sec_per_day
      unixtime     fcst_init                  () const;   //  Compute as valid - lead
      int          fcst_init_hour             () const;   //  HHMMSS portion of init time
      int          fcst_accum                 () const;

      int          obs_lead                   () const;
      unixtime     obs_valid                  () const;
      int          obs_valid_hour             () const;   //  valid%sec_per_day
      unixtime     obs_init                   () const;   //  Compute as valid - lead
      int          obs_init_hour              () const;   //  HHMMSS portion of init time
      int          obs_accum                  () const;

      int          fcst_rad                   () const;
      const char * fcst_thr                   () const;
      int          obs_rad                    () const;
      const char * obs_thr                    () const;

      const char * fcst_var                   () const;
      const char * fcst_units                 () const;
      const char * fcst_lev                   () const;
      const char * obs_var                    () const;
      const char * obs_units                  () const;
      const char * obs_lev                    () const;

      const char * object_id                  () const;
      const char * object_cat                 () const;

      double       centroid_x                 () const;
      double       centroid_y                 () const;
      double       centroid_lat               () const;
      double       centroid_lon               () const;

      double       axis_ang                   () const;

      double       length                     () const;
      double       width                      () const;
      double       aspect_ratio               () const;   //  Compute as width/length

      int          area                       () const;
      int          area_thresh                () const;

      double       curvature                  () const;
      double       curvature_x                () const;
      double       curvature_y                () const;

      double       complexity                 () const;

      double       intensity_10               () const;
      double       intensity_25               () const;
      double       intensity_50               () const;
      double       intensity_75               () const;
      double       intensity_90               () const;
      double       intensity_user             () const;

      double       intensity_sum              () const;
      double       centroid_dist              () const;
      double       boundary_dist              () const;
      double       convex_hull_dist           () const;
      double       angle_diff                 () const;


      double       aspect_diff                () const;
      double       area_ratio                 () const;
      int          intersection_area          () const;
      int          union_area                 () const;
      int          symmetric_diff             () const;
      double       intersection_over_area     () const;

      double       curvature_ratio            () const;
      double       complexity_ratio           () const;
      double       percentile_intensity_ratio () const;

      double       interest                   () const;

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



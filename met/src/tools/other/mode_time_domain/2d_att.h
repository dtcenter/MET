// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_2D_ATTRIBUTES_H__
#define  __MTD_2D_ATTRIBUTES_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"

#include "mtd_file.h"


////////////////////////////////////////////////////////////////////////


typedef MtdIntFile Object;


////////////////////////////////////////////////////////////////////////


class SingleAtt2D {

      friend SingleAtt2D calc_single_atts(const Object & mask_2d, const int obj_number);   //  1-based

   public:

      void init_from_scratch();

      void assign(const SingleAtt2D &);

      int ObjectNumber;

      int ClusterNumber;

      int Area;

      double Xbar, Ybar;

      double CentroidLat, CentroidLon;

      double AxisAngle;

      int TimeIndex;

      bool IsFcst;

      unixtime ValidTime;

   public:

      SingleAtt2D();
     ~SingleAtt2D();
      SingleAtt2D(const SingleAtt2D &);
      SingleAtt2D & operator=(const SingleAtt2D &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_object_number  (int);
      void set_cluster_number (int);

      void set_area (int);

      void set_centroid(double _xbar, double _ybar);

      void set_axis(double);

      void set_time_index(int);

      void set_fcst (bool = true);
      void set_obs  (bool = true);

      void set_valid_time(const unixtime);

         //
         //  get stuff
         //

      int object_number  () const;
      int cluster_number () const;

      int area () const;

      void centroid (double & xbar, double & ybar) const;

      double xbar () const;
      double ybar () const;

      double centroid_lat () const;
      double centroid_lon () const;

      double axis() const;

      int time_index () const;

      bool is_fcst () const;
      bool is_obs  () const;

      unixtime valid_time() const;

         //
         //  do stuff
         //

      void write_txt(AsciiTable &, const int row) const;

};


////////////////////////////////////////////////////////////////////////


inline int SingleAtt2D::object_number() const { return ( ObjectNumber ); }

inline int SingleAtt2D::cluster_number() const { return ( ClusterNumber ); }

inline int SingleAtt2D::time_index() const { return ( TimeIndex ); }

inline void SingleAtt2D::set_object_number  (int _n) { ObjectNumber  = _n;  return; }
inline void SingleAtt2D::set_cluster_number (int _n) { ClusterNumber = _n;  return; }

inline int SingleAtt2D::area() const { return ( Area ); }

inline void SingleAtt2D::set_area(int _A) { Area = _A;  return; }

inline void SingleAtt2D::set_time_index(int _t) { TimeIndex = _t;  return; }


inline double SingleAtt2D::xbar() const { return ( Xbar ); }
inline double SingleAtt2D::ybar() const { return ( Ybar ); }

inline double SingleAtt2D::centroid_lat() const { return ( CentroidLat ); }
inline double SingleAtt2D::centroid_lon() const { return ( CentroidLon ); }

inline double SingleAtt2D::axis() const { return ( AxisAngle ); }

inline bool   SingleAtt2D::is_fcst() const { return (   IsFcst ); }
inline bool   SingleAtt2D::is_obs () const { return ( ! IsFcst ); }

inline unixtime SingleAtt2D::valid_time () const { return ( ValidTime ); }


////////////////////////////////////////////////////////////////////////


extern SingleAtt2D calc_2d_single_atts(const Object & mask_2d, const int obj_number);   //  1-based 


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_2D_ATTRIBUTES_H__  */


////////////////////////////////////////////////////////////////////////




// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

   public:

      void init_from_scratch();

      void assign(const SingleAtt2D &);

      int ObjectNumber;

      int ClusterNumber;

      int Area;

      double Xbar, Ybar;

      double CentroidLat, CentroidLon;

      double AxisAngle;

      double Ptile_10;
      double Ptile_25;
      double Ptile_50;
      double Ptile_75;
      double Ptile_90;

      int    Ptile_Value;
      double Ptile_User;

      int TimeIndex;

      bool IsFcst;

      bool Is_Cluster;   //  as opposed to simple

      unixtime ValidTime;

      int Lead_Time;


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

      void set_is_cluster (bool = true);
      void set_is_simple  (bool = true);

      void set_valid_time(const unixtime);

      void set_lead_time(const int);

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

      double ptile_10() const;
      double ptile_25() const;
      double ptile_50() const;
      double ptile_75() const;
      double ptile_90() const;

      int    ptile_value() const;
      double ptile_user()  const;

      int time_index () const;

      bool is_fcst () const;
      bool is_obs  () const;

      bool is_cluster () const;
      bool is_simple  () const;

      unixtime valid_time() const;

      int lead_time() const;

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

inline double SingleAtt2D::ptile_10() const { return ( Ptile_10 ); }
inline double SingleAtt2D::ptile_25() const { return ( Ptile_25 ); }
inline double SingleAtt2D::ptile_50() const { return ( Ptile_50 ); }
inline double SingleAtt2D::ptile_75() const { return ( Ptile_75 ); }
inline double SingleAtt2D::ptile_90() const { return ( Ptile_90 ); }

inline int    SingleAtt2D::ptile_value() const { return ( Ptile_Value ); }
inline double SingleAtt2D::ptile_user()  const { return ( Ptile_User  ); }

inline bool   SingleAtt2D::is_fcst() const { return (   IsFcst ); }
inline bool   SingleAtt2D::is_obs () const { return ( ! IsFcst ); }

inline bool   SingleAtt2D::is_cluster () const { return (   Is_Cluster ); }
inline bool   SingleAtt2D::is_simple  () const { return ( ! Is_Cluster ); }

inline unixtime SingleAtt2D::valid_time () const { return ( ValidTime ); }

inline int      SingleAtt2D::lead_time  () const { return ( Lead_Time ); }


////////////////////////////////////////////////////////////////////////


extern SingleAtt2D calc_2d_single_atts(const Object & mask_2d, const DataPlane & raw_2d, const int obj_number, const int ptile_value);   //  1-based


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_2D_ATTRIBUTES_H__  */


////////////////////////////////////////////////////////////////////////




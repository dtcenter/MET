

////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_ATTRIBUTES_H__
#define  __MTD_ATTRIBUTES_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "mtd_file.h"


////////////////////////////////////////////////////////////////////////


typedef ModeTimeDomainFile Object;


////////////////////////////////////////////////////////////////////////


class SingleAttributes {

      friend SingleAttributes calc_single_atts(const Object & mask, const Object & raw, const char * model, int obj_number);

   private:

      void init_from_scratch();

      void assign(const SingleAttributes &);

      int ObjectNumber;

      int Volume;

      double Xbar, Ybar, Tbar;

      int Xmin, Xmax;
      int Ymin, Ymax;
      int Tmin, Tmax;

      double Complexity;

      double Xvelocity;
      double Yvelocity;

      double SpatialAxisAngle;

      double Ptile_10;
      double Ptile_25;
      double Ptile_50;
      double Ptile_75;
      double Ptile_90;

   public:

      SingleAttributes();
     ~SingleAttributes();
      SingleAttributes(const SingleAttributes &);
      SingleAttributes & operator=(const SingleAttributes &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_object_number (int);

      void set_volume (int);

      void set_centroid(double _xbar, double _ybar, double _tbar);

      void set_bounding_box (int _xmin, int _xmax, 
                             int _ymin, int _ymax, 
                             int _tmin, int _tmax);

      void set_complexity(double);

      void set_velocity(double, double);

      void set_spatial_axis(double);

         //
         //  get stuff
         //

      int object_number () const;

      int volume () const;

      void centroid ( double & xbar, double & ybar, double & tbar) const;

      double xbar () const;
      double ybar () const;
      double tbar () const;

      void bounding_box (int & xmin, int & xmax, 
                         int & ymin, int & ymax, 
                         int & tmin, int & tmax) const;

      int xmin() const;
      int xmax() const;

      int ymin() const;
      int ymax() const;

      int tmin() const;
      int tmax() const;

      int n_times() const;

      double complexity() const;

      void velocity(double &, double &) const;

      double speed() const;

      double xdot() const;
      double ydot() const;

      double spatial_axis() const;

      double ptile_10() const;
      double ptile_25() const;
      double ptile_50() const;
      double ptile_75() const;
      double ptile_90() const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline int SingleAttributes::object_number() const { return ( ObjectNumber ); }

inline void SingleAttributes::set_object_number(int _n) { ObjectNumber = _n;  return; }

inline int SingleAttributes::volume() const { return ( Volume ); }

inline void SingleAttributes::set_volume(int _v) { Volume = _v;  return; }

inline double SingleAttributes::complexity() const { return ( Complexity ); }

inline void SingleAttributes::set_complexity(double _v) { Complexity = _v;  return; }


inline double SingleAttributes::xbar() const { return ( Xbar ); }
inline double SingleAttributes::ybar() const { return ( Ybar ); }
inline double SingleAttributes::tbar() const { return ( Tbar ); }

inline int SingleAttributes::xmin() const { return ( Xmin ); }
inline int SingleAttributes::xmax() const { return ( Xmax ); }

inline int SingleAttributes::ymin() const { return ( Ymin ); }
inline int SingleAttributes::ymax() const { return ( Ymax ); }

inline int SingleAttributes::tmin() const { return ( Tmin ); }
inline int SingleAttributes::tmax() const { return ( Tmax ); }

inline double SingleAttributes::xdot() const { return ( Xvelocity ); }
inline double SingleAttributes::ydot() const { return ( Yvelocity ); }

inline double SingleAttributes::spatial_axis() const { return ( SpatialAxisAngle ); }

inline double SingleAttributes::ptile_10() const { return ( Ptile_10 ); }
inline double SingleAttributes::ptile_25() const { return ( Ptile_25 ); }
inline double SingleAttributes::ptile_50() const { return ( Ptile_50 ); }
inline double SingleAttributes::ptile_75() const { return ( Ptile_75 ); }
inline double SingleAttributes::ptile_90() const { return ( Ptile_90 ); }


////////////////////////////////////////////////////////////////////////


class PairAttributes {

      friend PairAttributes calc_pair_atts(const Object & _fcst_obj, 
                                           const Object & _obs_obj, 
                                           const SingleAttributes & _fa, 
                                           const SingleAttributes & _oa);

   private:

      void init_from_scratch();

      void assign(const PairAttributes &);

      int FcstObjectNumber;
      int ObsObjectNumber;

      int IntersectionVol;
      int UnionVol;

      double TimeCentroidDist;
      double SpaceCentroidDist;

      double DirectionDifference;

      double SpeedDifference;

   public:

      PairAttributes();
     ~PairAttributes();
      PairAttributes(const PairAttributes &);
      PairAttributes & operator=(const PairAttributes &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      // SingleAttributes Fcst;
      // SingleAttributes Obs;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

       int fcst_obj_number() const;
       int  obs_obj_number() const;

       int intersection_vol () const;
       int union_vol        () const;

       double  time_centroid_dist () const;
       double space_centroid_dist () const;

       double direction_difference () const; 

       double speed_difference () const;

         //
         //  do stuff
         //

};


////////////////////////////////////////////////////////////////////////


inline int PairAttributes::fcst_obj_number() const { return ( FcstObjectNumber ); }
inline int  PairAttributes::obs_obj_number() const { return ( ObsObjectNumber ); }

inline int PairAttributes::intersection_vol () const { return ( IntersectionVol ); }
inline int PairAttributes::union_vol        () const { return ( UnionVol ); }

inline double  PairAttributes::time_centroid_dist () const { return ( TimeCentroidDist ); }
inline double PairAttributes::space_centroid_dist () const { return ( SpaceCentroidDist ); }

inline double PairAttributes::direction_difference () const { return ( DirectionDifference ); }
inline double PairAttributes::speed_difference () const { return ( SpeedDifference ); }


////////////////////////////////////////////////////////////////////////


extern SingleAttributes calc_single_atts(const Object & mask, const Object & raw, const char * model, int obj_number);

extern PairAttributes   calc_pair_atts(const Object & _fcst_obj, 
                                       const Object & _obs_obj, 
                                       const SingleAttributes & _fa, 
                                       const SingleAttributes & _oa);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_ATTRIBUTES_H__  */


////////////////////////////////////////////////////////////////////////




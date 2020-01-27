// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __DEGREE_TRIG_H__
#define  __DEGREE_TRIG_H__


////////////////////////////////////////////////////////////////////////


#include <cmath>


////////////////////////////////////////////////////////////////////////


   //
   //   degree <-> radian conversion factors
   //


static const double deg_per_rad = 180.0/M_PI;

static const double rad_per_deg = M_PI/180.0;


////////////////////////////////////////////////////////////////////////


   //
   //  sin, cos, tan, sec, csc, cot functions that take degree arguments
   //


inline double sind (double _angle_deg_) { return ( sin(_angle_deg_*rad_per_deg) ); }
inline double cosd (double _angle_deg_) { return ( cos(_angle_deg_*rad_per_deg) ); }
inline double tand (double _angle_deg_) { return ( tan(_angle_deg_*rad_per_deg) ); }

inline double secd (double _angle_deg_) { return ( 1.0/cos(_angle_deg_*rad_per_deg) ); }
inline double cscd (double _angle_deg_) { return ( 1.0/sin(_angle_deg_*rad_per_deg) ); }

inline double cotd (double _angle_deg_) { return ( tan((90.0 - _angle_deg_)*rad_per_deg) ); }


////////////////////////////////////////////////////////////////////////


   //
   //  inverse sin, cos, tan functions that return degree values
   //


inline double asind (double _t_) { return ( deg_per_rad*asin(_t_) ); }
inline double acosd (double _t_) { return ( deg_per_rad*acos(_t_) ); }
inline double atand (double _t_) { return ( deg_per_rad*atan(_t_) ); }


////////////////////////////////////////////////////////////////////////


   //
   //  two-argument arctangent that returns degree values
   //


inline double atan2d (double _y_, double _x_) { return ( deg_per_rad*atan2(_y_, _x_) ); }


////////////////////////////////////////////////////////////////////////


   //
   //  a variation on the two-argument arctangent that takes "x"
   //     as its first argument, rather than "y".
   //
   //     I did this because my HP calculator has an arg function, 
   //     which I use a lot.
   //
   //     return value is in radians
   //


inline double arg  (double _x_, double _y_) { return ( atan2(_y_, _x_) ); }


   //
   //  same as above, except return value is in degrees
   //


inline double argd (double _x_, double _y_) { return ( deg_per_rad*atan2(_y_, _x_) ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __DEGREE_TRIG_H__


////////////////////////////////////////////////////////////////////////



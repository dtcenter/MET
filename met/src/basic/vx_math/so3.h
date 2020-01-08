

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_MATH_SO3_H__
#define  __VX_MATH_SO3_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class SO3 {

      friend SO3 operator*(const SO3 &, const SO3 &);

   protected:

      void init_from_scratch();

      void assign(const SO3 &);

         //
         //  matrix elements
         //

      double M11;
      double M12;
      double M13;

      double M21;
      double M22;
      double M23;

      double M31;
      double M32;
      double M33;

   public:

      SO3();
     ~SO3();
      SO3(const SO3 &);
      SO3 & operator=(const SO3 &);


      void clear();

      void dump(ostream &, int depth = 0) const;


         //
         //  set stuff
         //

      void set_rotx(double angle);
      void set_roty(double angle);
      void set_rotz(double angle);

      void set_axis_angle(double ax, double ay, double az, double angle);

      void set_identity();

         //
         //  get stuff
         //

      void get_axis_angle(double & ax, double & ay, double & az, double & angle) const;

      double operator()(int row, int col) const;   //  indices start at 0

      bool is_identity () const;

      bool is_x_axis () const;   //  is this a rotation about the x axis?
      bool is_y_axis () const;   //  is this a rotation about the y axis?
      bool is_z_axis () const;   //  is this a rotation about the z axis?

         //
         //  do stuff
         //

      void invert();

         //
         //  As you look down the axis vector from the tip to the origin,
         //
         //  "forward" rotated points CCW about the axis
         //
         //  "reverse" rotated points CW about the axis
         //

      bool forward(double u, double v, double w, double & x, double & y, double & z) const;
      bool reverse(double x, double y, double z, double & u, double & v, double & w) const;

      bool ccw (double u, double v, double w, double & x, double & y, double & z) const;
      bool  cw (double x, double y, double z, double & u, double & v, double & w) const;

            //
            //  pre-multiply by the given rotation
            //

      void pre_axis_angle(double ax, double ay, double az, double angle);

      void pre_rotx(double angle);
      void pre_roty(double angle);
      void pre_rotz(double angle);

            //
            //  post-multiply by the given rotation
            //

      void post_axis_angle(double ax, double ay, double az, double angle);

      void post_rotx(double angle);
      void post_roty(double angle);
      void post_rotz(double angle);

};


////////////////////////////////////////////////////////////////////////


inline void SO3::set_rotx  (double _angle) { set_axis_angle  (1.0, 0.0, 0.0, _angle);  return; }
inline void SO3::set_roty  (double _angle) { set_axis_angle  (0.0, 1.0, 0.0, _angle);  return; }
inline void SO3::set_rotz  (double _angle) { set_axis_angle  (0.0, 0.0, 1.0, _angle);  return; }

inline void SO3::pre_rotx  (double _angle) { pre_axis_angle  (1.0, 0.0, 0.0, _angle);  return; }
inline void SO3::pre_roty  (double _angle) { pre_axis_angle  (0.0, 1.0, 0.0, _angle);  return; }
inline void SO3::pre_rotz  (double _angle) { pre_axis_angle  (0.0, 0.0, 1.0, _angle);  return; }

inline void SO3::post_rotx (double _angle) { post_axis_angle (1.0, 0.0, 0.0, _angle);  return; }
inline void SO3::post_roty (double _angle) { post_axis_angle (0.0, 1.0, 0.0, _angle);  return; }
inline void SO3::post_rotz (double _angle) { post_axis_angle (0.0, 0.0, 1.0, _angle);  return; }


////////////////////////////////////////////////////////////////////////


extern SO3 operator*(const SO3 &, const SO3 &);

extern void operator*=(SO3 &, const SO3 &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_MATH_SO3_H__  */


////////////////////////////////////////////////////////////////////////



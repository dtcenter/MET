// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_MATH_AFFINE_H__
#define  __VX_MATH_AFFINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class Box;   //  forward reference


////////////////////////////////////////////////////////////////////////


enum ViewGravity {

   view_center_gravity,

   view_north_gravity,
   view_south_gravity,
   view_east_gravity,
   view_west_gravity,

   view_northwest_gravity,
   view_northeast_gravity,

   view_southwest_gravity,
   view_southeast_gravity,

   fill_viewport, 


   no_view_gravity,    //  flag value

};


////////////////////////////////////////////////////////////////////////


   //
   //  abstract base class for all affine classes
   //

class AffineInterface {

   public:

      AffineInterface();
      virtual ~AffineInterface();

      virtual void clear() = 0;

      virtual void dump(std::ostream &, int depth = 0) const = 0;

         //
         //  set stuff
         //

      virtual void set_pin (double x_from, double y_from, double x_to, double y_to) = 0;

      virtual void set_fixed_point (double u, double v) = 0;

      virtual void set_translation (double, double) = 0;

         //
         //  get stuff
         //

      virtual double m11() const = 0;
      virtual double m12() const = 0;
      virtual double m21() const = 0;
      virtual double m22() const = 0;

      virtual double tx () const = 0;
      virtual double ty () const = 0;

      virtual double det() const = 0;

      virtual bool is_conformal() const = 0;

      virtual bool is_diagonal() const = 0;

         //
         //  do stuff
         //

      virtual void move(double, double) = 0;

      virtual void invert() = 0;


      virtual void     forward (double  u, double  v, double &  x, double &  y) const = 0;
      virtual void     reverse (double  x, double  y, double &  u, double &  v) const = 0;

      virtual void der_forward (double du, double dv, double & dx, double & dy) const = 0;
      virtual void der_reverse (double dx, double dy, double & du, double & dv) const = 0;

      virtual void     operator()(double  u, double  v, double &  x, double &  y) const = 0;

      virtual AffineInterface * copy() const = 0;

};


////////////////////////////////////////////////////////////////////////


class Affine : public AffineInterface {

      friend Affine operator*(const Affine &, const Affine &);

   protected:

      void init_from_scratch();

      void assign(const Affine &);

      void calc_det();

      double Det;

      double M11;
      double M12;
      double M21;
      double M22;

      double TX;
      double TY;

   public:

      Affine();
      Affine(double _m11, double _m12, double _m21, double _m22, double _b1, double _b2);
      Affine(double _m11, double _m12, double _m21, double _m22);   //  sets TX = TY = 0.0
      virtual ~Affine();
      Affine(const Affine &);
      Affine & operator=(const Affine &);

      void clear();  //  set to identity

      virtual void dump(std::ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void  set_pin  (double x_from, double y_from, double x_to, double y_to);

      void  set_fixed_point  (double u, double v);

      void  set_translation  (double, double);

      void  set_mb  (double _m11, double _m12, double _m21, double _m22, double _b1, double _b2);

      void  set_m   (double _m11, double _m12, double _m21, double _m22);   //  sets TX = TY = 0.0

      void  set_b   (double _b1, double _b2);

         //
         //  version 1
         //

      void  set_three_points_v1  (double x1_from, double y1_from, 
                                  double x2_from, double y2_from, 
                                  double x3_from, double y3_from, 

                                  double x1_to,   double y1_to,   
                                  double x2_to,   double y2_to,   
                                  double x3_to,   double y3_to);

         //
         //  version 2
         //

      void  set_three_points_v2  (double x1_from, double y1_from, 
                                  double x1_to,   double y1_to,   

                                  double x2_from, double y2_from, 
                                  double x2_to,   double y2_to,   

                                  double x3_from, double y3_from, 
                                  double x3_to,   double y3_to);

         //
         //  get stuff
         //

      double m11 () const;
      double m12 () const;
      double m21 () const;
      double m22 () const;

      double tx  () const;
      double ty  () const;

      double det () const;

      bool is_conformal () const;

      bool is_diagonal () const;

      Affine inverse() const;

         //
         //  do stuff
         //

      void move(double, double);

      void invert();


      void     forward (double  u, double  v, double &  x, double &  y) const;
      void     reverse (double  x, double  y, double &  u, double &  v) const;

      void der_forward (double du, double dv, double & dx, double & dy) const;
      void der_reverse (double dx, double dy, double & du, double & dv) const;

      void     operator()(double  u, double  v, double &  x, double &  y) const;

      AffineInterface * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double Affine::m11() const { return ( M11 ); }
inline double Affine::m12() const { return ( M12 ); }
inline double Affine::m21() const { return ( M21 ); }
inline double Affine::m22() const { return ( M22 ); }

inline double Affine::tx () const { return ( TX ); }
inline double Affine::ty () const { return ( TY ); }

inline double Affine::det() const { return ( Det ); }

inline void Affine::calc_det() { Det = M11*M22 - M21*M12;  return; }

inline void Affine::set_translation(double _tx, double _ty) { TX = _tx;  TY = _ty;  return; }

inline void Affine::set_fixed_point(double _u, double _v) { set_pin(_u, _v, _u, _v);  return; }


inline void Affine::operator()(double _u, double _v, double & _x, double & _y) const

   { forward(_u, _v, _x, _y);  return; }


////////////////////////////////////////////////////////////////////////


extern Affine operator*(const Affine &, const Affine &);


////////////////////////////////////////////////////////////////////////


class DiagonalAffine : public AffineInterface {

      friend DiagonalAffine operator*(const DiagonalAffine &, const DiagonalAffine &);

   protected:

      void init_from_scratch();

      void assign(const DiagonalAffine &);

      void calc_det();

      double Det;

      double MX;
      double MY;

      double TX;
      double TY;

   public:

      DiagonalAffine();
      virtual ~DiagonalAffine();
      DiagonalAffine(const DiagonalAffine &);
      DiagonalAffine & operator=(const DiagonalAffine &);

      void clear();  //  set to identity

      virtual void dump(std::ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void  set_pin  (double x_from, double y_from, double x_to, double y_to);

      void  set_fixed_point  (double u, double v);

      void  set_translation  (double, double);

      void  set_mb  (double _mx, double _my, double _bx, double _by);

      void  set  (const Box & From, const Box & To);

      void  set_two_points  (double x1_from, double y1_from, 
                             double x2_from, double y2_from, 

                             double x1_to,   double y1_to,   
                             double x2_to,   double y2_to);

      void  set_two_points_v2  (double x1_from, double y1_from, double x1_to, double y1_to,   
                                double x2_from, double y2_from, double x2_to, double y2_to);

         //
         //  get stuff
         //

      double m11 () const;
      double m12 () const;
      double m21 () const;
      double m22 () const;

      double tx  () const;
      double ty  () const;

      double mx  () const;
      double my  () const;

      double det () const;

      bool is_conformal () const;

      bool is_diagonal () const;

         //
         //  do stuff
         //

      void move(double, double);

      void invert();


      void     forward (double  u, double  v, double &  x, double &  y) const;
      void     reverse (double  x, double  y, double &  u, double &  v) const;

      void der_forward (double du, double dv, double & dx, double & dy) const;
      void der_reverse (double dx, double dy, double & du, double & dv) const;

      double x_forward(double) const;
      double x_reverse(double) const;

      double y_forward(double) const;
      double y_reverse(double) const;

      double x_der_forward(double) const;
      double x_der_reverse(double) const;

      double y_der_forward(double) const;
      double y_der_reverse(double) const;

      void     operator()(double  u, double  v, double &  x, double &  y) const;

      Box      operator()(const Box &) const;

      AffineInterface * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double DiagonalAffine::m11() const { return ( MX  ); }
inline double DiagonalAffine::m12() const { return ( 0.0 ); }
inline double DiagonalAffine::m21() const { return ( 0.0 ); }
inline double DiagonalAffine::m22() const { return ( MY  ); }

inline double DiagonalAffine::tx () const { return ( TX ); }
inline double DiagonalAffine::ty () const { return ( TY ); }

inline double DiagonalAffine::mx () const { return ( MX ); }
inline double DiagonalAffine::my () const { return ( MY ); }

inline double DiagonalAffine::det() const { return ( Det ); }

inline void DiagonalAffine::calc_det() { Det = MX*MY;  return; }

inline void DiagonalAffine::set_translation(double _tx, double _ty) { TX = _tx;  TY = _ty;  return; }

inline void DiagonalAffine::set_fixed_point(double _u, double _v) { set_pin(_u, _v, _u, _v);  return; }

inline bool DiagonalAffine::is_diagonal() const { return ( true ); }


inline void DiagonalAffine::operator()(double  _u, double  _v, double &  _x, double &  _y) const

   { forward(_u, _v, _x, _y);  return; }


////////////////////////////////////////////////////////////////////////


extern DiagonalAffine operator*(const DiagonalAffine &, const DiagonalAffine &);


////////////////////////////////////////////////////////////////////////


   //
   //  conformal, orientation preserving, affine, two-dimensional
   //    coordinate transformation:
   //
   //    x = scale*(  u*cos(theta) + v*sin(theta) )  + tx;
   //
   //    y = scale*( -u*sin(theta) + v*cos(theta) )  + ty;
   //


////////////////////////////////////////////////////////////////////////


class ConformalAffine : public AffineInterface {

      friend ConformalAffine operator*(const ConformalAffine &, const ConformalAffine &);

   private:

      void init_from_scratch();

      void assign(const ConformalAffine &);


      double CosAngle;
      double SinAngle;

      double Angle;

      double Scale;

      double TX;
      double TY;

   public:

      ConformalAffine();
      virtual ~ConformalAffine();
      ConformalAffine(const ConformalAffine &);
      ConformalAffine & operator=(const ConformalAffine &);

      void clear();

      void dump (std::ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_scale(double);
      void set_angle(double);
      void set_translation(double, double);

      void set_pin(double x_from, double y_from, double x_to, double y_to);

      void set_fixed_point(double u, double v);

      void set(const Box & image, const Box & view);   //  image -> view

      void set(const Box & image, const Box & view, const ViewGravity);   //  image -> view

         //
         //  get stuff
         //

      double m11() const;
      double m12() const;
      double m21() const;
      double m22() const;

      double tx () const;
      double ty () const;

      double det() const;

      bool is_conformal() const;

      bool is_diagonal() const;

      double angle() const;

      double scale() const;

      Affine affine_equivalent() const;

         //
         //  do stuff
         //

      ConformalAffine inverse() const;

      void move(double, double);

      void invert();


      void     forward (double  u, double  v, double &  x, double &  y) const;
      void     reverse (double  x, double  y, double &  u, double &  v) const;

      void der_forward (double du, double dv, double & dx, double & dy) const;
      void der_reverse (double dx, double dy, double & du, double & dv) const;

      void     operator()(double  u, double  v, double &  x, double &  y) const;

      AffineInterface * copy() const;

};


////////////////////////////////////////////////////////////////////////


inline double ConformalAffine::angle() const { return ( Angle ); }

inline double ConformalAffine::scale() const { return ( Scale ); }

inline bool   ConformalAffine::is_conformal() const { return ( true ); }


inline double ConformalAffine::m11() const { return (  Scale*CosAngle ); }
inline double ConformalAffine::m12() const { return (  Scale*SinAngle ); }

inline double ConformalAffine::m21() const { return ( -Scale*SinAngle ); }
inline double ConformalAffine::m22() const { return (  Scale*CosAngle ); }

inline double ConformalAffine::tx () const { return ( TX ); }
inline double ConformalAffine::ty () const { return ( TY ); }


inline double ConformalAffine::det() const { return ( Scale*Scale ); }


inline void   ConformalAffine::set_translation(double _tx, double _ty) { TX = _tx;  TY = _ty;  return; }

inline void   ConformalAffine::set_fixed_point(double _u, double _v) { set_pin(_u, _v, _u, _v);  return; }

inline void ConformalAffine::operator()(double  _u, double  _v, double &  _x, double &  _y) const

   { forward(_u, _v, _x, _y);  return; }


////////////////////////////////////////////////////////////////////////


extern ConformalAffine operator*(const ConformalAffine &, const ConformalAffine &);


////////////////////////////////////////////////////////////////////////


   //
   //  The "Box" class represents a rectangle whose sides
   //
   //   are parallel to the coordinate axes
   //


class Box {

      friend bool do_they_intersect(const Box &, const Box &);

      friend Box intersect(const Box &, const Box &);

      friend Box surround(const Box &, const Box &);

   // private:
   protected:

      virtual void init_from_scratch();

      virtual void assign(const Box &);

      void calc_is_empty();

      double Left;
      double Right;
      double Top;
      double Bottom;

      bool IsEmpty;

   public:

      Box();
      Box(double L, double R, double B, double T);
      virtual ~Box();
      Box(const Box &);
      Box & operator=(const Box &);


      virtual void clear();

      virtual void dump(std::ostream &, int = 0) const;

         //
         //  set stuff
         //

      virtual void set_lrbt(double L, double R, double B, double T);
      virtual void set_llwh(double x_ll, double y_ll, double width, double height);

         //
         //  get stuff
         //

      virtual double left   () const;
      virtual double right  () const;
      virtual double top    () const;
      virtual double bottom () const;

      virtual double width  () const;
      virtual double height () const;

      virtual double aspect () const;   //  width divided by height

      virtual double area() const;

      virtual double diagonal() const;   //  length of diagonal

      virtual void center(double &, double &) const;

      virtual double x_center() const;
      virtual double y_center() const;

      virtual bool is_empty    () const;
      virtual bool is_nonempty () const;

         //
         //  do stuff
         //

      void set_center(const Box &);   //  makes the center of this box the same as that of the given box

      void   translate(double dx, double dy);
      void x_translate(double dx);
      void y_translate(double dy);

      void pin(double x, double y, double u, double v);

      void scale_from_origin(double);            //  rescale with (0, 0) fixed
      void scale_from_origin(double, double );   //  rescale with (0, 0) fixed

      void scale_from_center(double);   //  rescale with center fixed

      void scale_from_ll(double);       //  rescale with lower-left corner fixed

      void enclose(const Box &);        //  expand current box to enclose given box

      void place(const Box & viewport, const ViewGravity);   //  position current box inside given box, 
                                                             //  expanding it as much as possible
                                                             //  while preserving aspect ratio

      void pad (double);
      void pad (double px, double py);

      void shrink (double);
      void shrink (double px, double py);

      void round ();

      DiagonalAffine xy_to_uv() const;
      DiagonalAffine uv_to_xy() const;

      void xy_to_uv (double x, double y, double & u, double & v) const;
      void uv_to_xy (double u, double v, double & x, double & y) const;

      double x_to_u (double x) const;
      double y_to_v (double y) const;

      double u_to_x (double u) const;
      double v_to_y (double v) const;

      bool is_inside(double x_test, double y_test) const;

};


//////////////////////////////////////////////////////////////


inline double Box::left   () const { return ( Left   ); }
inline double Box::right  () const { return ( Right  ); }
inline double Box::bottom () const { return ( Bottom ); }
inline double Box::top    () const { return ( Top    ); }

inline double Box::width  () const { return ( Right - Left ); }
inline double Box::height () const { return ( Top - Bottom ); }

inline double Box::x_center () const { return ( 0.5*(Left + Right) ); }
inline double Box::y_center () const { return ( 0.5*(Top + Bottom) ); }

inline bool Box::is_empty    () const { return (   IsEmpty ); }
inline bool Box::is_nonempty () const { return ( ! IsEmpty ); }

inline double Box::x_to_u(double __x) const { return ( (__x - Left)/(Right - Left) ); }
inline double Box::y_to_v(double __y) const { return ( (__y - Bottom)/(Top - Bottom) ); }

inline double Box::u_to_x(double __u) const { return ( Left +   __u*(Right - Left) ); }
inline double Box::v_to_y(double __v) const { return ( Bottom + __v*(Top - Bottom) );  }

inline void Box::shrink(double _delta) { pad(-_delta);  return;  }
inline void Box::shrink(double _x_delta, double _y_delta) { pad(-_x_delta, -_y_delta);  return;  }


//////////////////////////////////////////////////////////////


extern bool do_they_intersect(const Box &, const Box &);

extern Box intersect(const Box &, const Box &);

extern Box surround(const Box &, const Box &);

extern Box surround(const Box *, int n_boxes);

extern Box surround(const double * _x, const double * _y, const int _n);


////////////////////////////////////////////////////////////////////////


extern void viewgravity_to_uv(const ViewGravity, double & u, double & v);

extern double calc_aspect(double width, double height);

extern double calc_mag(double image_width, double image_height,
                       double  view_width, double  view_height);

extern double calc_mag(const Box & image, const Box & view);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_MATH_AFFINE_H__  */


////////////////////////////////////////////////////////////////////////





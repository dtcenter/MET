

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_VX_VECTOR_H__
#define  __MET_VX_VECTOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class Vector {

   protected:

      void init_from_scratch();

      void assign(const Vector &);

      double X;
      double Y;
      double Z;

   public:


      Vector();
     ~Vector();
      Vector(double _x, double _y);                   //  x, y
      Vector(double _x, double _y, double _z);        //  x, y, z
      Vector(double _x, double _y, double _z, char);  //  unit vector
      Vector(const Vector &);
      Vector & operator=(const Vector &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_xy     (double, double);   //  sets Z to zero

      void set_xyz    (double, double, double);

      void set_aad    (double alt, double azi, double dist);

      void set_altaz  (double  alt, double  azi);

      void set_latlon (double  lat, double  lon);

         //
         //  get stuff
         //

      double x () const;
      double y () const;
      double z () const;

      double abs         () const;
      double abs_squared () const;

      void get_altaz(double & alt, double & azi) const;

      void get_latlon(double & lat, double & lon) const;

         //
         //  do stuff
         //

      void normalize();

      void rotate(const Vector & axis, double angle);

         //
         //  vector algebra
         //

      void operator+=(const Vector &);
      void operator-=(const Vector &);

      void operator*=(double);
      void operator/=(double);

      void operator*=(int);
      void operator/=(int);

};


////////////////////////////////////////////////////////////////////////


inline double Vector::x() const { return ( X ); }
inline double Vector::y() const { return ( Y ); }
inline double Vector::z() const { return ( Z ); }


////////////////////////////////////////////////////////////////////////


extern double abs         (const Vector &);
extern double abs_squared (const Vector &);

extern double included_angle (const Vector &, const Vector &);

extern double dot   (const Vector &, const Vector &);
extern Vector cross (const Vector &, const Vector &);

extern double tsp (const Vector & A, const Vector & B, const Vector & C);  //  (A x B).C

extern Vector proj_onto      (const Vector & a, const Vector & b);  //  a projected onto b
extern Vector proj_onto_perp (const Vector & a, const Vector & b);  //  a projected onto b-perp

extern Vector operator-(const Vector &);   //  unary minus 

extern Vector operator*(double, const Vector &);
extern Vector operator*(const Vector &, double);

extern Vector operator*(int, const Vector &);
extern Vector operator*(const Vector &, int);

extern Vector operator+(const Vector &, const Vector &);
extern Vector operator-(const Vector &, const Vector &);

extern Vector operator/(const Vector &, double);
extern Vector operator/(const Vector &, int);

extern Vector altaz_to_east  (double alt, double azi);
extern Vector altaz_to_north (double alt, double azi);

extern Vector latlon_to_east  (double lat, double lon);
extern Vector latlon_to_north (double lat, double lon);

extern Vector normalize(const Vector &);


////////////////////////////////////////////////////////////////////////


extern Vector latlon_to_vector(double lat, double lon);

extern void   latlon_to_vector(double lat, double lon, double & x, double & y, double & z);


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const Vector &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_VX_VECTOR_H__  */


////////////////////////////////////////////////////////////////////////



// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __SPHERIOD_H__
#define  __SPHERIOD_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


static const double default_flattening  = 1.0/298.257;

static const double default_semimajor   = 6378.140;   //  kilometers

static const double default_semiminor   = default_semimajor*(1.0 - default_flattening);


////////////////////////////////////////////////////////////////////////


class Spheroid {

   private:

      void init_from_scratch();

      void assign(const Spheroid &);

      double F;        //  flattening ... f = 1 - b/a

      double E;        //  eccentricity   e = sqrt( 1.0 - (B/A)^2 )

      double A_km;     // semimajor axis in kilometers

      double B_km;     // semiminor axis in kilometers

      ConcatString Name;

   public:

      Spheroid();
      Spheroid(const char * _name_, double _a_, double _b_);
     ~Spheroid();
      Spheroid(const Spheroid &);
      Spheroid & operator=(const Spheroid &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_ab(double _semimajor_km, double _semiminor_km);
      void set_af(double _semimajor_km, double _flattening);

      void set_name(const char *);

         //
         //  get stuff
         //

      double a_km() const;
      double b_km() const;

      double f() const;
      double e() const;

      const char * name() const;

      bool is_sphere () const;

         //
         //  do stuff
         //

      void geographic_to_geocentric(double lat, double h_km, double & rho_km, double & phi_prime) const;

      void geocentric_to_geographic(double rho_km, double phi_prime, double & lat_deg, double & h_km) const;

};


////////////////////////////////////////////////////////////////////////


inline double Spheroid::a_km() const { return ( A_km ); }
inline double Spheroid::b_km() const { return ( B_km ); }

inline double Spheroid::f() const { return ( F ); }
inline double Spheroid::e() const { return ( E ); }

inline const char * Spheroid::name() const { return ( Name.c_str() ); }

inline bool Spheroid::is_sphere() const { return ( A_km == B_km ); }


////////////////////////////////////////////////////////////////////////


extern const Spheroid Meeus;
extern const Spheroid Gsphere;

extern const Spheroid IUGG_1980;

extern const Spheroid Everest;
extern const Spheroid Bessel;
extern const Spheroid Airy;
extern const Spheroid Clarke_1858;
extern const Spheroid Clarke_1866;
extern const Spheroid Clarke_1880;
extern const Spheroid Hayford;
extern const Spheroid Krasovski;
extern const Spheroid Hough;
extern const Spheroid Fischer_60;
extern const Spheroid Kaula;
extern const Spheroid IUGG_67;
extern const Spheroid Fischer_68;
extern const Spheroid WGS_72;
extern const Spheroid IUGG_75;
extern const Spheroid WGS_84;


////////////////////////////////////////////////////////////////////////


#endif   /*  __SPHERIOD_H__  */


////////////////////////////////////////////////////////////////////////




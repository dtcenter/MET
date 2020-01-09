// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research(UCAR)
// ** National Center for Atmospheric Research(NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef __GNOMON_H__
#define __GNOMON_H__


////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////


class GnomonicProjection {

   private:

      void init_from_scratch();

      void assign(const GnomonicProjection &);

      double Ex;
      double Ey;
      double Ez;

      double Nx;
      double Ny;
      double Nz;

      double Ux;
      double Uy;
      double Uz;

   public:

      GnomonicProjection();
     ~GnomonicProjection();
      GnomonicProjection(const GnomonicProjection &);
      GnomonicProjection & operator=(const GnomonicProjection &);

      void clear();

      void set_center(double lat, double lon);

      int latlon_to_uv(double lat, double lon, double & u, double & v) const;

};


////////////////////////////////////////////////////////////////////////


extern void gnomon_latlon_to_xy(double, double, double &, double &);

extern void gnomon_xy_to_latlon(double, double, double &, double &);


////////////////////////////////////////////////////////////////////////


#endif   //  __GNOMON_H__


////////////////////////////////////////////////////////////////////////




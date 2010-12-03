// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MASK_POLY_H__
#define  __MASK_POLY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////

   //
   //  Polyline masking done using a PlateCarree (Lat/Lon) Projection
   //

////////////////////////////////////////////////////////////////////////


class MaskPoly {

   private:

      void init_from_scratch();

      void assign(const MaskPoly &);

      char * Name;

      double * Lat;
      double * Lon;

      double * U;
      double * V;

      int Npoints;

   public:

      MaskPoly();
     ~MaskPoly();
      MaskPoly(const MaskPoly &);
      MaskPoly & operator=(const MaskPoly &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void load(const char * filename);

      int latlon_is_inside (double lat, double lon) const;

      const char * name()     const;
      int          n_points() const;

};


////////////////////////////////////////////////////////////////////////


inline const char * MaskPoly::name()     const { return ( Name    ); }
inline       int    MaskPoly::n_points() const { return ( Npoints ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MASK_POLY_H__  */


////////////////////////////////////////////////////////////////////////

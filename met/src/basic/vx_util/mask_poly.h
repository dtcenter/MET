// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////

   //
   //  Polyline masking done using a PlateCarree (Lat/Lon) Projection
   //

////////////////////////////////////////////////////////////////////////


class MaskPoly {

   private:

      void init_from_scratch();

      void assign(const MaskPoly &);

      ConcatString Name;
      ConcatString FileName;

      NumArray Lat;
      NumArray Lon;

      NumArray U;
      NumArray V;

      double LonShift;

      int Npoints;

   public:

      MaskPoly();
     ~MaskPoly();
      MaskPoly(const MaskPoly &);
      MaskPoly & operator=(const MaskPoly &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      ConcatString name()      const;

      ConcatString file_name() const;

      int          n_points()  const;

      double       lat(int i)  const;
      double       lon(int i)  const;

         //
         //  do stuff
         //

      void load(const char * filename);

      bool latlon_is_inside      (double lat, double lon) const;
      bool latlon_is_inside_dege (double lat, double lon) const;

};


////////////////////////////////////////////////////////////////////////


inline ConcatString MaskPoly::name()      const { return ( Name     ); }
inline ConcatString MaskPoly::file_name() const { return ( FileName ); }
inline       int    MaskPoly::n_points()  const { return ( Npoints  ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MASK_POLY_H__  */


////////////////////////////////////////////////////////////////////////

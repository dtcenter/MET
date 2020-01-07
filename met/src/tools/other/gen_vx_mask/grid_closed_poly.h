// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   polyline.h
//
//   Description:
//
//
//   Mod#   Date       Name           Description
//   ----   ----       ----           -----------
//   000    11-03-06   Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __DATA2D_UTIL_GCP_H__
#define  __DATA2D_UTIL_GCP_H__

///////////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "polyline.h"
#include "ncrr_array.h"
#include "shp_file.h"
#include "shp_poly_record.h"
#include "vx_grid.h"


///////////////////////////////////////////////////////////////////////////////


class GridClosedPoly : public Polyline

{

   protected:

      void gcp_init_from_scratch();

      void gcp_assign(const GridClosedPoly &);

         //  bounding box info

      double u_min, u_max;
      double v_min, v_max;

   public:

      GridClosedPoly();
     ~GridClosedPoly();
      GridClosedPoly(const GridClosedPoly &);
      GridClosedPoly & operator=(const GridClosedPoly &);

      void clear();

      int is_inside(double u_test, double v_test) const;   //  test bounding box first

      void add_point(double, double);   //  updates bounding box

};


///////////////////////////////////////////////////////////////////////////////


class GridClosedPolyArray : public NCRR_Array<GridClosedPoly>

{

   public:

      bool is_inside(double u_test, double v_test) const;

      void set (const ShpPolyRecord &, const Grid &);

};


///////////////////////////////////////////////////////////////////////////////


#endif   //  __DATA2D_UTIL_GCP_H__


///////////////////////////////////////////////////////////////////////////////




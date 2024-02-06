// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_closed_poly.h
//
//   Description:
//
//
//   Mod#   Date       Name           Description
//   ----   ----       ----           -----------
//   000    11-03-06   Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_CLOSED_POLY_H__
#define  __GRID_CLOSED_POLY_H__

///////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "polyline.h"
#include "ncrr_array.h"
#include "shp_file.h"
#include "shp_poly_record.h"
#include "vx_grid.h"

///////////////////////////////////////////////////////////////////////////////

class GridClosedPoly : public Polyline {

   protected:

      void gcp_init_from_scratch();

      void gcp_assign(const GridClosedPoly &);

      // bounding box limits
      double u_min, u_max;
      double v_min, v_max;

   public:

      GridClosedPoly();
     ~GridClosedPoly();
      GridClosedPoly(const GridClosedPoly &);
      GridClosedPoly & operator=(const GridClosedPoly &);

      void clear();

      // tests bounding box first for efficiency
      int is_inside(double u_test, double v_test) const;

      // updates bounding box for each new point
      void add_point(double, double);

};

///////////////////////////////////////////////////////////////////////////////

class GridClosedPolyArray : public NCRR_Array<GridClosedPoly> {

   public:

      bool is_inside(double u_test, double v_test) const;

      void set(const ShpPolyRecord &, const Grid &);

};

///////////////////////////////////////////////////////////////////////////////

#endif   //  __GRID_CLOSED_POLY_H__

///////////////////////////////////////////////////////////////////////////////

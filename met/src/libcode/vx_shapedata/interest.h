// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __VX_WRFMODE_INTEREST_H__
#define  __VX_WRFMODE_INTEREST_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "shapedata.h"

////////////////////////////////////////////////////////////////////////

struct DistributionPercentiles  {
   double p10;
   double p25;
   double p50;
   double p75;
   double p90;
   double pth;
   double sum;
};

////////////////////////////////////////////////////////////////////////

class SingleFeature {

   private:

      void init_from_scratch();

      void assign(const SingleFeature &);

   public:

      SingleFeature();
     ~SingleFeature();
      SingleFeature(const SingleFeature &);
      SingleFeature & operator=(const SingleFeature &);

      void clear();

      void set(const ShapeData &raw,  const ShapeData &thresh,
               const ShapeData &mask, const int perc,
               const bool precip_flag);

      const ShapeData * Raw;    //  NOT allocated, so DON'T delete!
      const ShapeData * Thresh; //  NOT allocated, so DON'T delete!
      const ShapeData * Mask;   //  NOT allocated, so DON'T delete!

      int object_number;

      //
      // attributes
      //
      double centroid_x;
      double centroid_y;
      double axis_ang;
      double length;
      double width;
      double aspect_ratio;
      double area;
      double area_thresh;
      double curvature;
      double curvature_x;
      double curvature_y;
      double complexity;

      DistributionPercentiles intensity_ptile;
      double user_ptile;

      Polyline  convex_hull;
      Polyline *boundary;   //  allocated
      int       n_bdy;
};

////////////////////////////////////////////////////////////////////////

extern ostream & operator<<(ostream &, const SingleFeature &);

////////////////////////////////////////////////////////////////////////

class PairFeature {

   private:

      void init_from_scratch();

      void assign(const PairFeature &);

   public:
      PairFeature();
     ~PairFeature();
      PairFeature(const PairFeature &);
      PairFeature & operator=(const PairFeature &);

      void clear();

      void set(const SingleFeature &fcst, const SingleFeature &obs,
               double max_centroid_dist);

      const SingleFeature *Obs;    //  NOT allocated, so DON'T delete
      const SingleFeature *Fcst;   //

      int pair_number;

      //
      // attributes
      //
      double centroid_dist;
      double boundary_dist;
      double convex_hull_dist;
      double angle_diff;
      double aspect_diff;
      double area_ratio;
      double intersection_area;
      double union_area;
      double symmetric_diff;
      double intersection_over_area;
      double curvature_ratio;
      double complexity_ratio;
      double percentile_intensity_ratio;
};

////////////////////////////////////////////////////////////////////////

extern ostream & operator<<(ostream &, const PairFeature &);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_WRFMODE_INTEREST_H__

////////////////////////////////////////////////////////////////////////

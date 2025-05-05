// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//  Filename:   interest.cc
//
//  Description:
//
//
//  Mod#   Date      Name           Description
//  ----   ----      ----           -----------
//  000    04/15/05  Halley Gotway
//  001    01/10/12  Bullock        Ported to new repository
//  002    11/02/23  Halley Gotway  MET #2724 improve efficiency
//  003    04/29/25  Halley Gotway  MET #3132 add OpenMP
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <numeric>

#include "interest.h"
#include "vx_math.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static void get_percentiles(DistributionPercentiles &,
                            const ShapeData &, const ShapeData &,
                            const int, bool);

////////////////////////////////////////////////////////////////////////
//
// Code for class SingleFeature
//
////////////////////////////////////////////////////////////////////////

SingleFeature::SingleFeature()

{

init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

SingleFeature::~SingleFeature()

{

   clear();

}

////////////////////////////////////////////////////////////////////////

SingleFeature::SingleFeature(const SingleFeature &s)

{

init_from_scratch();

assign(s);

}

////////////////////////////////////////////////////////////////////////

SingleFeature & SingleFeature::operator=(const SingleFeature &s) {

   if(this == &s) return *this;

   assign(s);

   return *this;
}

////////////////////////////////////////////////////////////////////////


void SingleFeature::init_from_scratch()

{

boundary = (Polyline *) nullptr;

clear();

}


////////////////////////////////////////////////////////////////////////

void SingleFeature::clear()

{

   centroid_x          = 0.0;
   centroid_y          = 0.0;
   axis_ang            = 0.0;
   length              = 0.0;
   width               = 0.0;
   aspect_ratio        = 1.0;
   area                = 0.0;
   area_thresh         = 0.0;
   curvature           = 0.0;
   curvature_x         = 0.0;
   curvature_y         = 0.0;
   complexity          = 0.0;
   intensity_ptile.p10 = 0.0;
   intensity_ptile.p25 = 0.0;
   intensity_ptile.p50 = 0.0;
   intensity_ptile.p75 = 0.0;
   intensity_ptile.p90 = 0.0;
   intensity_ptile.pth = 0.0;
   intensity_ptile.sum = 0.0;
   user_ptile          = 0.0;

   Raw    = (const ShapeData *) nullptr;
   Thresh = (const ShapeData *) nullptr;
   Split  = (const ShapeData *) nullptr;

   convex_hull.clear();

   //
   // Deallocate memory
   //
   if(boundary) { delete [] boundary;  boundary = (Polyline *) nullptr; }
   n_bdy = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleFeature::assign(const SingleFeature & s)

{

   int i;

   clear();

   centroid_x          = s.centroid_x;
   centroid_y          = s.centroid_y;
   axis_ang            = s.axis_ang;
   length              = s.length;
   width               = s.width;
   aspect_ratio        = s.aspect_ratio;
   area                = s.area;
   area_thresh         = s.area_thresh;
   curvature           = s.curvature;
   curvature_x         = s.curvature_x;
   curvature_y         = s.curvature_y;
   complexity          = s.complexity;
   intensity_ptile.p10 = s.intensity_ptile.p10;
   intensity_ptile.p25 = s.intensity_ptile.p25;
   intensity_ptile.p50 = s.intensity_ptile.p50;
   intensity_ptile.p75 = s.intensity_ptile.p75;
   intensity_ptile.p90 = s.intensity_ptile.p90;
   intensity_ptile.pth = s.intensity_ptile.pth;
   intensity_ptile.sum = s.intensity_ptile.sum;
   user_ptile          = s.user_ptile;

   Raw    = s.Raw;
   Thresh = s.Thresh;
   Split  = s.Split;

   convex_hull = s.convex_hull;

   //
   // Allocate memory
   //
   n_bdy    = s.n_bdy;
   boundary = new Polyline [n_bdy];
   for(i=0; i<n_bdy; i++) boundary[i] = s.boundary[i];

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleFeature::set(const ShapeData &raw_sd, const ShapeData &thresh_sd,
                        const ShapeData &split_sd, const ShapeData &mask_sd,
                        const int perc, bool precip_flag)

{

   clear();

   Raw    = &raw_sd;
   Thresh = &thresh_sd;
   Split  = &split_sd;

   //
   // Centroid
   //
   mask_sd.centroid(centroid_x, centroid_y);

   //
   // Axis angle
   //
   axis_ang = mask_sd.angle_degrees();

   //
   // Length and Width
   //
   mask_sd.calc_length_width(length, width);

   //
   // Aspect ratio
   //
   aspect_ratio = width/length;

   //
   // Object area
   //
   area = mask_sd.area();

   //
   // Object threshold area: the area of the raw field inside the mask
   // area that meets the threshold criteria
   //
   area_thresh = (double) ShapeData_intersection(*Thresh, mask_sd);

   //
   // Curvature, Curvature_x, Curvature_y
   //
   curvature = mask_sd.curvature(curvature_x, curvature_y);

   //
   // Complexity
   //
   complexity = mask_sd.complexity();

   //
   // Compute the Intensity Percentiles
   //
   get_percentiles(intensity_ptile, raw_sd, mask_sd, perc, precip_flag);

   //
   // User Percentile
   //
   user_ptile = (double) perc;

   //
   // Convex hull
   //
   convex_hull = mask_sd.convex_hull();

   //
   // Boundary:
   // Split the mask field and store the boundary for each object.
   //
   ShapeData cur_split_sd = split(mask_sd, n_bdy);
   boundary = new Polyline [n_bdy];

#pragma omp parallel default(none) \
   shared(n_bdy, cur_split_sd, boundary)
   {

#pragma omp for schedule(static)
      for(int i=0; i<n_bdy; i++) {
         ShapeData cur_obj_sd  = select(cur_split_sd, i+1);
         boundary[i] = cur_obj_sd.single_boundary();
      }
   } // End omp parallel

   //
   // Done
   //

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PairFeature
//
////////////////////////////////////////////////////////////////////////

PairFeature::PairFeature()

{

init_from_scratch();

}

////////////////////////////////////////////////////////////////////////

PairFeature::~PairFeature() {

   clear();

}

////////////////////////////////////////////////////////////////////////

PairFeature::PairFeature(const PairFeature &p) {

   init_from_scratch();

   assign(p);
}

////////////////////////////////////////////////////////////////////////

PairFeature & PairFeature::operator=(const PairFeature &p)

{

   if(this == &p) return *this;

   assign(p);

   return *this;
}

////////////////////////////////////////////////////////////////////////


void PairFeature::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////

void PairFeature::clear()

{

   centroid_dist              = 0.0;
   boundary_dist              = 0.0;
   convex_hull_dist           = 0.0;
   angle_diff                 = 0.0;
   aspect_diff                = 0.0;
   fo_area_ratio              = 0.0;
   area_ratio                 = 0.0;
   intersection_area          = 0.0;
   union_area                 = 0.0;
   symmetric_diff             = 0.0;
   intersection_over_area     = 0.0;
   curvature_ratio            = 0.0;
   complexity_ratio           = 0.0;
   percentile_intensity_ratio = 0.0;

   Obs  = (const SingleFeature *) nullptr; // DON'T delete
   Fcst = (const SingleFeature *) nullptr; // DON'T delete

   return;
}

////////////////////////////////////////////////////////////////////////

void PairFeature::assign(const PairFeature &p) {

   clear();

   centroid_dist              = p.centroid_dist;
   boundary_dist              = p.boundary_dist;
   convex_hull_dist           = p.convex_hull_dist;
   angle_diff                 = p.angle_diff;
   aspect_diff                = p.aspect_diff;
   fo_area_ratio              = p.fo_area_ratio;
   area_ratio                 = p.area_ratio;
   intersection_area          = p.intersection_area;
   union_area                 = p.union_area;
   symmetric_diff             = p.symmetric_diff;
   intersection_over_area     = p.intersection_over_area;
   curvature_ratio            = p.curvature_ratio;
   complexity_ratio           = p.complexity_ratio;
   percentile_intensity_ratio = p.percentile_intensity_ratio;

   Obs  = p.Obs;
   Fcst = p.Fcst;

   return;
}

////////////////////////////////////////////////////////////////////////

void PairFeature::set(const SingleFeature &fcst,
                      const SingleFeature &obs,
                      double max_centroid_dist)

{

   clear();

   Fcst = &fcst;
   Obs  = &obs;

   double dx, dy;
   double a1, a2;

   //
   // Centroid distance
   //
   dx = (Obs->centroid_x) - (Fcst->centroid_x);
   dy = (Obs->centroid_y) - (Fcst->centroid_y);

   centroid_dist = sqrt(dx*dx + dy*dy);
   if(centroid_dist > max_centroid_dist) return;

   //
   // Boundary distance:
   // Compute it as the minimum distance between any two pairs of
   // polylines.
   //
   boundary_dist = 1.0e30;
   for(int i=0; i<Obs->n_bdy; i++) {
      for(int j=0; j<Fcst->n_bdy; j++) {
         double d = polyline_dist(Obs->boundary[i], Fcst->boundary[j]);
         if(d < boundary_dist) boundary_dist = d;
         if(is_eq(boundary_dist, 0.0)) break;
      }
      if(is_eq(boundary_dist, 0.0)) break;
   }

   //
   // Convex hull distance
   //
   convex_hull_dist = polyline_dist(Obs->convex_hull, Fcst->convex_hull);

   //
   // Angle diff
   //
   a1 = Obs->axis_ang;
   a2 = Fcst->axis_ang;
   angle_diff = angle_between(a1, a2);

   //
   // Aspect ratio diff
   //
   aspect_diff = fabs(Fcst->aspect_ratio - Obs->aspect_ratio);

   //
   // Area ratio
   //
   fo_area_ratio = (Fcst->area)/(Obs->area);

   area_ratio    = min( (Obs->area)/(Fcst->area),
                        (Fcst->area)/(Obs->area) );

   //
   // Intersection, union, and symmetric diff areas
   //
   intersection_area = union_area = 0.0;
   symmetric_diff = 0.0;
   int nxy = Fcst->Split->data.nxy();

#pragma omp parallel default(none) \
   shared(nxy, Fcst, Obs, intersection_area, union_area, symmetric_diff)
   {

#pragma omp for schedule(static) \
                reduction(+: intersection_area, union_area, symmetric_diff)
      for(int i=0; i<nxy; ++i) {

         bool fcst_on = (nint(Fcst->Split->data.data()[i]) == Fcst->object_number ? true : false);
         bool obs_on  = (nint( Obs->Split->data.data()[i]) ==  Obs->object_number ? true : false);

         if(   fcst_on &&  obs_on)   intersection_area++;
         if(   fcst_on ||  obs_on)   union_area++;
         if( ( fcst_on && !obs_on) ||
             (!fcst_on &&  obs_on) ) symmetric_diff++;
      }
   } // End omp parallel

   //
   // Intersection over area
   //
   intersection_over_area =
      intersection_area/(min(Obs->area, Fcst->area));

   //
   // Curvature ratio
   //
   curvature_ratio = min( (Obs->curvature)/(Fcst->curvature),
                          (Fcst->curvature)/(Obs->curvature) );

   //
   // Complexity Ratio
   //

   // Both complexities are non-zero
   if(Obs->complexity > 0.0 && Fcst->complexity > 0.0) {
      complexity_ratio = min( (Obs->complexity)/(Fcst->complexity),
                              (Fcst->complexity)/(Obs->complexity) );
   }
   // At least one complexity is zero
   else {
      complexity_ratio = 1.0 - max(Obs->complexity, Fcst->complexity);
   }

   //
   // Nth Percentile intensity ratio
   //

   // Both percentile intensities are non-zero
   if(Obs->intensity_ptile.pth > 0.0 && Fcst->intensity_ptile.pth > 0.0) {
      percentile_intensity_ratio = min( (Obs->intensity_ptile.pth)/(Fcst->intensity_ptile.pth),
                                        (Fcst->intensity_ptile.pth)/(Obs->intensity_ptile.pth) );
   }
   // At least one percentile intensity is zero
   else {
      percentile_intensity_ratio = 0.0;
   }

   //
   // Done
   //

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

ostream & operator<<(ostream & out, const SingleFeature & s)

{

   // ShapeDataHeader h = s.Raw->get_header();

   out << "Object Number          = "  << (s.object_number)   << "\n";
   out << "Centroid               = (" << (s.centroid_x)      << ", "
                                       << (s.centroid_y)      << ")\n";
   out << "Axis Angle             = "  << (s.axis_ang)        << "\n";
   out << "Length                 = "  << (s.length)          << "\n";
   out << "Width                  = "  << (s.width)           << "\n";
   out << "Aspect Ratio           = "  << (s.aspect_ratio)    << "\n";
   out << "Area                   = "  << nint(s.area)        << "\n";
   out << "Area Thresh            = "  << nint(s.area_thresh) << "\n";
   out << "Curvature              = "  << (s.aspect_ratio)    << "\n";
   out << "Center of Curvature    = (" << (s.curvature_x)     << ", "
                                       << (s.curvature_y)     << ")\n";
   out << "Complexity             = "  << (s.complexity)      << "\n";
   out << "Intensity P10          = "  << (s.intensity_ptile.p10) << "\n";
   out << "Intensity P25          = "  << (s.intensity_ptile.p25) << "\n";
   out << "Intensity P50          = "  << (s.intensity_ptile.p50) << "\n";
   out << "Intensity P75          = "  << (s.intensity_ptile.p75) << "\n";
   out << "Intensity P90          = "  << (s.intensity_ptile.p90) << "\n";
   out << "Intensity P"
       << nint(s.user_ptile)
                     << "         = "  << (s.intensity_ptile.pth) << "\n";
   out << "Intensity Sum          = "  << (s.intensity_ptile.sum) << "\n";
   out.flush();

   return out;
}

////////////////////////////////////////////////////////////////////////

ostream & operator<<(ostream & out, const PairFeature & p)

{

   out << "Fcst Object Number                = " << (p.Fcst->object_number)        << "\n";
   out << "Obs  Object Number                = " << (p.Obs->object_number)         << "\n";
   out << "Pair Number                       = " << p.pair_number                  << "\n";
   out << "Centroid Distance                 = " << (p.centroid_dist)              << "\n";
   out << "Boundary Distance                 = " << (p.boundary_dist)              << "\n";
   out << "Convex Hull Distance              = " << (p.convex_hull_dist)           << "\n";
   out << "Angle Difference                  = " << (p.angle_diff)                 << "\n";
   out << "Aspect Difference                 = " << (p.aspect_diff)                << "\n";
   out << "Fcst/Obs Area Ratio               = " << (p.fo_area_ratio)              << "\n";
   out << "Area Ratio                        = " << (p.area_ratio)                 << "\n";
   out << "Intersection Area                 = " << nint(p.intersection_area)      << "\n";
   out << "Union Area                        = " << nint(p.union_area)             << "\n";
   out << "Symmetric Difference              = " << nint(p.symmetric_diff)         << "\n";
   out << "Intersection Over Area            = " << (p.intersection_over_area)     << "\n";
   out << "Curvature Ratio                   = " << (p.curvature_ratio)            << "\n";
   out << "Complexity Ratio                  = " << (p.complexity_ratio)           << "\n";
   out << "Percentile Intensity Ratio        = " << (p.percentile_intensity_ratio) << "\n";
   out.flush();

   return out;
}

////////////////////////////////////////////////////////////////////////

void get_percentiles(DistributionPercentiles &ptile,
                     const ShapeData &raw, const ShapeData &mask,
                     const int perc, const bool precip_flag)

{
   const char *method_name = "get_percentiles() -> ";

   int nx = raw.data.nx();
   int ny = raw.data.ny();

   //
   // Count values.
   // Only collect precipitation values greater than zero.
   //
   vector<double> v;
   v.reserve(nx*ny);
   for(int x=0; x<nx; ++x) {
      for(int y=0; y<ny; ++y) {
         if((mask.s_is_on(x, y)) &&
            (raw.is_valid_xy(x, y)) &&
            (!precip_flag || (precip_flag && raw.data(x, y) > 0))) {
            v.emplace_back(raw.data(x, y));
         }
      }
   }

   //
   // Sort
   //
   sort(v.begin(), v.end());

   //
   // Get percentiles
   //
   auto n_values = (int) v.size();
   ptile.p10 = percentile(v.data(), n_values, 0.10);
   ptile.p25 = percentile(v.data(), n_values, 0.25);
   ptile.p50 = percentile(v.data(), n_values, 0.50);
   ptile.p75 = percentile(v.data(), n_values, 0.75);
   ptile.p90 = percentile(v.data(), n_values, 0.90);

   //
   // Compute the sum
   //
   ptile.sum = accumulate(v.begin(), v.end(), 0.0);

   //
   // User-specified percentile
   //
   if(perc == 101) { // mean
      ptile.pth = (n_values == 0 ? bad_data_double :
                                   ptile.sum/n_values);
   }
   else if(perc == 102) { // sum
      ptile.pth = ptile.sum;
   }
   else {
      ptile.pth = percentile(v.data(), n_values, (double) perc/100.0);
   }

   //
   // Done
   //

   return;
}

////////////////////////////////////////////////////////////////////////



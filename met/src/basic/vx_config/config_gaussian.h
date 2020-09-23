// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_GAUSSIAN_H__
#define __CONFIG_GAUSSIAN_H__

////////////////////////////////////////////////////////////////////////

//
// Struct to store Gaussian interpolation and regridding options
// Defaults chosen to mimic Hazardous Weather Testbed settings.
//
static const double default_trunc_factor = 3.5;
static const double default_gaussian_dx = 81.271;
static const double default_gaussian_radius = 120.0;

struct GaussianInfo {

   // Inputs
   double   dx;             // delta distance of the target gerid for Gaussian
   double   radius;         // radius of influence for Gaussian
   double   trunc_factor;   // truncation factor

   // Derived from above inputs
   int      max_r;          // max distance for the gaussian weights
   int      weight_cnt;     // the count of the valid weights (>0)
   double   weight_sum;     // the sum of the weights
   double   *weights;       // 2D for gaussian weight (2*max_r+1) by (2*max_r+1)

   GaussianInfo();
   void     clear();
   void     compute();
   int      compute_max_r();
   void     validate();     // Ensure that required inputs are accordant
};

////////////////////////////////////////////////////////////////////////

#endif   //  __CONFIG_GAUSSIAN_H__

////////////////////////////////////////////////////////////////////////

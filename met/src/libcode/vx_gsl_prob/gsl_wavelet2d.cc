// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

#include "vx_log.h"
#include "gsl_wavelet2d.h"

////////////////////////////////////////////////////////////////////////
//
// Allocate a wavelet of the type specified using the wavelet member
// specified by the parameter k.
//
////////////////////////////////////////////////////////////////////////

gsl_wavelet * wavelet_set(gsl_wavelet_type *t, int k) {
   gsl_wavelet *w = (gsl_wavelet *) 0;

   //
   // Allocate space for the wavelet specified
   //
   if(!(w = gsl_wavelet_alloc(t, k))) {
      mlog << Error << "\nwavelet_set() -> "
           << "error allocating the specified wavelet!\n\n";
      exit(1);
   }

   return(w);
}

////////////////////////////////////////////////////////////////////////
//
// Free a wavelet.
//
////////////////////////////////////////////////////////////////////////

void wavelet_free(gsl_wavelet *w) {

   //
   // Free space for the wavelet specified
   //
   gsl_wavelet_free(w);

   w = (gsl_wavelet *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Get the name of the wavelet.
//
////////////////////////////////////////////////////////////////////////

const char *wavelet_name(const gsl_wavelet *w) {

   return(gsl_wavelet_name(w));
}

////////////////////////////////////////////////////////////////////////
//
// Allocate a wavelet workspace of the size specified.
//
////////////////////////////////////////////////////////////////////////

gsl_wavelet_workspace * wavelet_workspace_set(int n) {
   gsl_wavelet_workspace *work = (gsl_wavelet_workspace *) 0;

   //
   // Allocate space for the wavelet_workspace specified
   //
   if(!(work = gsl_wavelet_workspace_alloc(n))) {
      mlog << Error << "\nwavelet_workspace_set() -> "
           << "error allocating the wavelet workspace of size "
           << n << "!\n\n";
      exit(1);
   }

   return(work);
}

////////////////////////////////////////////////////////////////////////
//
// Free a wavelet workspace.
//
////////////////////////////////////////////////////////////////////////

void wavelet_workspace_free(gsl_wavelet_workspace *work) {

   //
   // Free space for the wavelet workspace specified
   //
   gsl_wavelet_workspace_free(work);

   work = (gsl_wavelet_workspace *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Standard wavelet transform operations:
// Discrete wavelet transform on rows of the matrix followed by columns.
//
////////////////////////////////////////////////////////////////////////

void wavelet2d_transform(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol, int dir,
                         gsl_wavelet_workspace *work) {
   gsl_wavelet_direction gsl_dir = (gsl_wavelet_direction) dir;

   if(gsl_wavelet2d_transform(w, data, n, nrow, ncol, gsl_dir, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_transform() -> "
           << "error performing the standard wavelet transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void wavelet2d_transform_forward(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol,
                         gsl_wavelet_workspace *work) {

   if(gsl_wavelet2d_transform_forward(w, data, n, nrow, ncol, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_transform_forward() -> "
           << "error performing the standard forward wavelet "
           << "transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void wavelet2d_transform_inverse(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol,
                         gsl_wavelet_workspace *work) {

   if(gsl_wavelet2d_transform_inverse(w, data, n, nrow, ncol, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_transform_inverse() -> "
           << "error performing the standard inverse wavelet "
           << "transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Non-standard wavelet transform operations:
// Discrete wavelet transform using interleaved passes on the rows
// and the columns of the matrix.  Typically used in image analysis.
//
////////////////////////////////////////////////////////////////////////

void wavelet2d_nstransform(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol, int dir,
                         gsl_wavelet_workspace *work) {
   gsl_wavelet_direction gsl_dir = (gsl_wavelet_direction) dir;

   if(gsl_wavelet2d_nstransform(w, data, n, nrow, ncol, gsl_dir, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_nstransform() -> "
           << "error performing the non-standard wavelet "
           << "transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void wavelet2d_nstransform_forward(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol,
                         gsl_wavelet_workspace *work) {

   if(gsl_wavelet2d_nstransform_forward(w, data, n, nrow, ncol, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_nstransform_forward() -> "
           << "error performing the non-standard forward wavelet "
           << "transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void wavelet2d_nstransform_inverse(const gsl_wavelet *w, double *data,
                         int n, int nrow, int ncol,
                         gsl_wavelet_workspace *work) {

   if(gsl_wavelet2d_nstransform_inverse(w, data, n, nrow, ncol, work)
      != GSL_SUCCESS) {
      mlog << Error << "\nwavelet2d_nstransform_inverse() -> "
           << "error performing the non-standard inverse wavelet "
           << "transform!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

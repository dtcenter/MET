// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#ifndef  __MET_GSL_WAVELET2D_H__
#define  __MET_GSL_WAVELET2D_H__

////////////////////////////////////////////////////////////////////////

#include "vx_util.h"
#include "gsl/gsl_wavelet2d.h"

////////////////////////////////////////////////////////////////////////
//
// Setup and free a wavelet
//
////////////////////////////////////////////////////////////////////////

extern gsl_wavelet * wavelet_set(gsl_wavelet_type *t, int k);
extern void wavelet_free(gsl_wavelet *w);

extern const char *wavelet_name(const gsl_wavelet *w);

extern gsl_wavelet_workspace * wavelet_workspace_set(int n);
extern void wavelet_workspace_free(gsl_wavelet_workspace *work);

////////////////////////////////////////////////////////////////////////
//
// Standard wavelet transform operations:
// Discrete wavelet transform on rows of the matrix followed by columns.
//
////////////////////////////////////////////////////////////////////////

extern void wavelet2d_transform(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol, int dir,
               gsl_wavelet_workspace *work);

extern void wavelet2d_transform_forward(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol,
               gsl_wavelet_workspace *work);

extern void wavelet2d_transform_inverse(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol,
               gsl_wavelet_workspace *work);

////////////////////////////////////////////////////////////////////////
//
// Non-standard wavelet transform operations:
// Discrete wavelet transform using interleaved passes on the rows
// and the columns of the matrix.  Typically used in image analysis.
//
////////////////////////////////////////////////////////////////////////

extern void wavelet2d_nstransform(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol, int dir,
               gsl_wavelet_workspace *work);

extern void wavelet2d_nstransform_forward(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol,
               gsl_wavelet_workspace *work);

extern void wavelet2d_nstransform_inverse(
               const gsl_wavelet *w, double *data,
               int n, int nrow, int ncol,
               gsl_wavelet_workspace *work);

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_GSL_WAVELET2D_H__  */

////////////////////////////////////////////////////////////////////////


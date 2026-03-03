// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __MET_GSL_FFT2D_H__
#define  __MET_GSL_FFT2D_H__

////////////////////////////////////////////////////////////////////////

#include <vector>

#include "vx_util.h"
#include "gsl/gsl_fft_real.h"

////////////////////////////////////////////////////////////////////////

extern void dct_typeII_2d(double *data, int nrow, int ncol);

////////////////////////////////////////////////////////////////////////

#endif   /*  __MET_GSL_FFT2D_H__  */

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

#include "gsl/gsl_matrix.h"

#include "vx_log.h"
#include "gsl_fft2d.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
// Apply discrete cosine transform to 2D data by transforming the
// columns (nx) and rows (nx) separately
//
////////////////////////////////////////////////////////////////////////

extern void dct_typeII_2d(double *data, int ncol, int nrow) {

   // Store input data in a gsl matrix
   gsl_matrix *m = gsl_matrix_alloc(nrow, ncol);
   for(int i=0; i<nrow; i++) {
      for(int j=0; j<ncol; j++) {
         int n = DefaultTO.two_to_one(ncol, nrow, j, i);
         gsl_matrix_set(m, i, j, data[n]);
      }
   }

   // Transform Rows
   gsl_fft_real_wavetable *col_wt = gsl_fft_real_wavetable_alloc(ncol);
   gsl_fft_real_workspace *col_ws = gsl_fft_real_workspace_alloc(ncol);

   for(int i=0; i<nrow; i++) {
      gsl_vector_view row = gsl_matrix_row(m, i);
      gsl_fft_real_transform(row.vector.data, row.vector.stride,
                             ncol, col_wt, col_ws);
   }

   // Transform Columns
   gsl_fft_real_wavetable *row_wt = gsl_fft_real_wavetable_alloc(nrow);
   gsl_fft_real_workspace *row_ws = gsl_fft_real_workspace_alloc(nrow);

   for(int j=0; j<ncol; j++) {
      gsl_vector_view col = gsl_matrix_column(m, j);
      gsl_fft_real_transform(col.vector.data, col.vector.stride,
                             nrow, row_wt, row_ws);
   }

   // Store the result
   for(int i=0; i<nrow; i++) {
      for(int j=0; j<ncol; j++) {
         int n = DefaultTO.two_to_one(ncol, nrow, j, i);
         data[n] = gsl_matrix_get(m, i, j);
      }
   }

   // Free resources
   gsl_matrix_free(m);
   gsl_fft_real_wavetable_free(row_wt);
   gsl_fft_real_wavetable_free(col_wt);
   gsl_fft_real_workspace_free(row_ws);
   gsl_fft_real_workspace_free(col_ws);
}

////////////////////////////////////////////////////////////////////////

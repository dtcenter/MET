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
// Apply a 1D orthonormal DCT Type-II transform in place, computed via
// a real FFT using the even/odd reordering and twiddle-factor rotation
// described by Makhoul (1980). This produces coefficients consistent
// with scipy.fftpack.dct(type=2, norm='ortho'):
//
//   X[k] = 2 * sum_n x[n] * cos(pi*k*(2n+1)/(2N)), 0 <= k < N
//   scaled by sqrt(1/(4N)) for k=0, sqrt(1/(2N)) for k>0.
//
// GSL has no native DCT, so this reconstructs true cosine
// coefficients from a real FFT rather than (incorrectly) applying
// DCT-II scale factors directly to half-complex FFT output.
//
// Equations to recover the unnormalized DCT-II coefficients:
//   X[k] = 2 * Re( V[k] * exp(-i*pi*k/(2n)) )
//        = 2 * (Re(V[k])*cos(theta) + Im(V[k])*sin(theta))
//
////////////////////////////////////////////////////////////////////////

static void dct_typeII_1d(double *x, size_t stride, int n,
                          const gsl_fft_real_wavetable *wt,
                          gsl_fft_real_workspace *ws) {

   // Reorder the input into a contiguous buffer:
   //   v[k]       = x[2k]   for the first half
   //   v[n-1-k]   = x[2k+1] for the second half
   // This is the standard permutation used to compute a DCT-II via a
   // same-length FFT, valid for both even and odd n.
   vector<double> v(n);
   int half = n / 2;
   for(int k=0; k<half; k++) {
      v[k]         = x[(2*k)     * stride];
      v[n - 1 - k] = x[(2*k + 1) * stride];
   }
   if(n % 2 == 1) v[half] = x[(n - 1) * stride];

   // Real FFT of the reordered sequence (contiguous, stride 1),
   // returned in GSL's half-complex packed format.
   gsl_fft_real_transform(v.data(), 1, n, wt, ws);

   // Unpack the half-complex format into (re, im) for any k in
   // [0, n), using conjugate symmetry V[k] = conj(V[n-k]) for the
   // upper half, since GSL only stores k = 0..n/2 explicitly.
   auto get_re_im = [&](int k, double &re, double &im) {
      if(k == 0) {
         re = v[0];
         im = 0.0;
      }
      else if(n % 2 == 0 && k == n/2) {
         re = v[n - 1];
         im = 0.0;
      }
      else if(k <= (n - 1)/2) {
         re = v[2*k - 1];
         im = v[2*k];
      }
      else {
         int kk = n - k;
         re =  v[2*kk - 1];
         im = -v[2*kk];
      }
   };

   // Recover the unnormalized DCT-II coefficients
   vector<double> X(n);
   for(int k=0; k<n; k++) {
      double re;
      double im;
      get_re_im(k, re, im);
      double theta = M_PI * k / (2.0 * n);
      X[k] = 2.0 * (re * cos(theta) + im * sin(theta));
   }

   // Apply orthonormalization scaling factors for DCT Type II:
   //   https://docs.scipy.org/doc/scipy/reference/generated/scipy.fftpack.dct.html
   double scale_first = 1.0 / sqrt(4.0 * n);
   double scale_rest  = 1.0 / sqrt(2.0 * n);
   X[0] *= scale_first;
   for(int k=1; k<n; k++) X[k] *= scale_rest;

   // Store the result back into the (possibly strided) input buffer
   for(int k=0; k<n; k++) x[k * stride] = X[k];

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Apply the discrete cosine transform to 2D data by transforming the
// rows (ncol) and columns (nrow) separately, as a separable 2D DCT-II.
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

   // Transform rows (length ncol)
   gsl_fft_real_wavetable *col_wt = gsl_fft_real_wavetable_alloc(ncol);
   gsl_fft_real_workspace *col_ws = gsl_fft_real_workspace_alloc(ncol);

   for(int i=0; i<nrow; i++) {
      gsl_vector_view row = gsl_matrix_row(m, i);
      dct_typeII_1d(row.vector.data, row.vector.stride, ncol,
                    col_wt, col_ws);
   }

   // Transform columns (length nrow)
   gsl_fft_real_wavetable *row_wt = gsl_fft_real_wavetable_alloc(nrow);
   gsl_fft_real_workspace *row_ws = gsl_fft_real_workspace_alloc(nrow);

   for(int j=0; j<ncol; j++) {
      gsl_vector_view col = gsl_matrix_column(m, j);
      dct_typeII_1d(col.vector.data, col.vector.stride, nrow,
                    row_wt, row_ws);
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

   return;
}

////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <vector>

#include "combine_boolplanes.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

//
//  assumes all the input BoolPlanes (and the output BoolPlane) are the same size
//

void combine_boolplanes(const string &name,
                        int rIndex, int tIndex,
                        const BoolPlane * bpa, const int n_planes, 
                        BoolCalc & calc, 
                        BoolPlane & bp_out) {
   const int nx = bp_out.nx();
   const int ny = bp_out.ny();
   vector<bool> v(n_planes);
   double nTotal = (double)(nx*ny);
   double nTrue = 0.0;

#pragma omp parallel default(none) \
   shared(nx, ny, n_planes, v, bpa, calc, nTrue, bp_out)
   {

#pragma omp for schedule(static) \
                reduction(+: nTrue) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {
            for(int j=0; j<n_planes; j++) {
               v[j] = bpa[j].get(x, y);
            } // for j

            bool tf = calc.run(v);
            if (tf) nTrue++;
            bp_out.put(tf, x, y);

         } // for y
      } // for x
   } // End omp parallel

   mlog << Debug(1) << name << " has " << nTrue << " superobject points. "
        << " rIndex[" << rIndex << "] tIndex[" << tIndex << "]\n";

   return;
}

////////////////////////////////////////////////////////////////////////

void boolplane_to_pgm(const BoolPlane & in, Pgm & out) {
   const Color white(255, 255, 255);
   const Color black(  0,   0,   0);

   out.set_size_xy(in.nx(), in.ny());
   out.all_white();

#pragma omp parallel default(none) \
   shared(out, in, black, white)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<(out.nx()); x++) {
         for(int y=0; y<(out.ny()); y++) {
            bool tf = in.get(x, y);
            out.putxy((tf ? black : white), x, y);
         } // for y
      } // for x 
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

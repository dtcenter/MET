// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <vector>

#include "combine_boolplanes.h"


////////////////////////////////////////////////////////////////////////

//
//  assumes all the input BoolPlanes (and the output BoolPlane) are the same size
//

void combine_boolplanes(const string &name,
                        const BoolPlane * bpa, const int n_planes, 
                        BoolCalc & calc, 
                        BoolPlane & bp_out)


{

   int j, x, y;
   const int nx = bp_out.nx();
   const int ny = bp_out.ny();
   vector<bool> v(n_planes);
   bool tf = false;
   double nTotal = (double)(nx*ny);
   double nTrue = 0.0;


   for (x=0; x<nx; ++x)  {

      for (y=0; y<ny; ++y)  {

         for (j=0; j<n_planes; ++j)  {

            v[j] = bpa[j].get(x, y);

         }   //  for j

         tf = calc.run(v);
         if (tf) ++ nTrue;
         bp_out.put(tf, x, y);

      }   //  for y

   }   //  for x

   mlog << Debug(1) << name << " has " << nTrue << " superobject points.\n";

   //
   //  done
   //

   return;

}


////////////////////////////////////////////////////////////////////////


void boolplane_to_pgm(const BoolPlane & in, Pgm & out)

{

   int x, y;
   bool tf = false;
   const Color white (255, 255, 255);
   const Color black (  0,   0,   0);


   out.set_size_xy(in.nx(), in.ny());

   out.all_white();

   for (x=0; x<(out.nx()); ++x)  {

      for (y=0; y<(out.ny()); ++y)  {

         tf = in.get(x, y);

         out.putxy ( (tf ? black : white), x, y);

      }   //  for y

   }   //  for s




   return;

}


////////////////////////////////////////////////////////////////////////





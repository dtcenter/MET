// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <netcdf>

#include "series_pdf.h"

using namespace std;
using namespace netCDF;

////////////////////////////////////////////////////////////////////////

void init_pdf(int n, vector<long long>& pdf) {
   pdf.insert(pdf.end(), n, 0);
}

////////////////////////////////////////////////////////////////////////

void init_pdf(double min, double max, double delta,
              vector<long long>& pdf) {
   int n = (max - min) / delta;
   pdf.insert(pdf.end(), n, 0);
}

////////////////////////////////////////////////////////////////////////

void init_joint_pdf(int n_A, int n_B, vector<long long>& pdf) {
   pdf.insert(pdf.end(), n_A * n_B, 0);
}

////////////////////////////////////////////////////////////////////////

void update_pdf(double min, double delta, vector<long long>& pdf,
                const DataPlane& dp, const MaskPlane& mp) {

#pragma omp parallel default(none) \
   shared(min, delta, pdf, dp, mp)
   {

      // Update pdf counts
#pragma omp for schedule(static)
      for(int x=0; x<dp.nx(); x++) {
         for(int y=0; y<dp.ny(); y++) {
            double value = dp.get(x, y);
            if(mp.s_is_on(x, y) && !is_bad_data(value)) {

               int bin = floor((value - min) / delta);
                    if(bin < 0)           bin = 0;
               else if(bin >= pdf.size()) bin = pdf.size() - 1;

               pdf[bin]++;
            }
         } // end for y
      } // end for x
   } // End omp parallel
}

////////////////////////////////////////////////////////////////////////

void update_joint_pdf(int n_A, int n_B, double min_A, double min_B,
                      double delta_A, double delta_B,
                      vector<long long>& pdf,
                      const DataPlane& dp_A, const DataPlane& dp_B,
                      const MaskPlane& mp) {

#pragma omp parallel default(none)                  \
   shared(n_A, n_B, min_A, min_B, delta_A, delta_B) \
   shared(pdf, dp_A, dp_B, mp)
   {

      // Update joint pdf counts
#pragma omp for schedule(static)
      for(int x=0; x<dp_A.nx(); x++) {
         for(int y=0; y<dp_A.ny(); y++) {
            double value_A = dp_A.get(x, y);
            double value_B = dp_B.get(x, y);
            if(mp.s_is_on(x, y)      && 
               !is_bad_data(value_A) && 
               !is_bad_data(value_B)) {

               int k_A = floor((value_A - min_A) / delta_A);
                    if(k_A < 0)    k_A = 0;
               else if(k_A >= n_A) k_A = n_A - 1;

               int k_B = floor((value_B - min_B) / delta_B);
                    if(k_B < 0) k_B = 0;
               else if(k_B >= n_B) k_B = n_B - 1;

               int bin = k_A * n_B + k_B;

	       pdf[bin]++;
            }
         } // end for y
      } // end for x
   } // End omp parallel
}

////////////////////////////////////////////////////////////////////////

void print_pdf(double min, double delta,
               const vector<long long>& pdf) {

   for(int k=0; k<pdf.size(); k++) {
      double bin_min = min + k * delta;
      double bin_max = min + (k + 1) * delta;
      cout << "[" << bin_min << ", " << bin_max << "] "
           << pdf[k] << "\n";
   }
}

////////////////////////////////////////////////////////////////////////

void write_nc_pdf(NcFile* nc_out, const VarInfo& info,
                  double min, double delta,
                  const vector<long long>& pdf) {

   vector<double> bin_min;
   vector<double> bin_max;
   vector<double> bin_mid;

   for(int k=0; k<pdf.size(); k++) {
      bin_min.emplace_back(min + delta * k);
      bin_max.emplace_back(min + delta * (k + 1));
      bin_mid.emplace_back(min + delta * (k + 0.5));
   }

   ConcatString var_name = info.name();
   ConcatString var_min_name = var_name;
   ConcatString var_max_name = var_name;
   ConcatString var_mid_name = var_name;

   var_min_name.add("_min");
   var_max_name.add("_max");
   var_mid_name.add("_mid");

   ConcatString var_pdf_name("hist_");
   var_pdf_name.add(var_name);

   NcDim var_dim = nc_out->addDim(info.name(), pdf.size());
   NcVar var_min = nc_out->addVar(var_min_name, ncFloat, var_dim);
   NcVar var_max = nc_out->addVar(var_max_name, ncFloat, var_dim);
   NcVar var_mid = nc_out->addVar(var_mid_name, ncFloat, var_dim);
   NcVar var_pdf = nc_out->addVar(var_pdf_name, ncUint, var_dim);
   var_min.putAtt("units", info.units());
   var_max.putAtt("units", info.units());
   var_mid.putAtt("units", info.units());
   var_min.putVar(bin_min.data());
   var_max.putVar(bin_max.data());
   var_mid.putVar(bin_mid.data());
   var_pdf.putVar(pdf.data());
}

////////////////////////////////////////////////////////////////////////

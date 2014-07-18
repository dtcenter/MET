// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_math.h"
#include "nav.h"

#include "tc_poly_array.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCPolyArray
//
////////////////////////////////////////////////////////////////////////

TCPolyArray::TCPolyArray() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCPolyArray::~TCPolyArray() {
   clear();
}

////////////////////////////////////////////////////////////////////////

TCPolyArray::TCPolyArray(const TCPolyArray & a) {
   
   init_from_scratch();

   assign(a);
}

////////////////////////////////////////////////////////////////////////

TCPolyArray & TCPolyArray::operator=(const TCPolyArray & a) {

   if(this == &a) return(*this);

   assign(a);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::init_from_scratch() {

   Poly = (Polyline *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::clear() {

   if(Poly) { delete [] Poly;  Poly = (Polyline *) 0; }

   NPolys = NAlloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::assign(const TCPolyArray & a) {
   int i;

   clear();

   if(a.NPolys == 0) return;

   extend(a.NPolys);

   for(i=0; i<(a.NPolys); i++) Poly[i] = a.Poly[i];

   NPolys = a.NPolys;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::extend(int n) {
   int i;

   if(NAlloc >= n) return;

   int k = n/tc_poly_array_alloc_inc;

   if(n%num_array_alloc_inc) k++;

   n = k*tc_poly_array_alloc_inc;

   Polyline * p = (Polyline *) 0;

   p = new Polyline [n];

   if(!p) {
      mlog << Error
           << "\nvoid TCPolyArray::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   if(Poly) {
      for(i=0; i<NPolys; i++) p[i] = Poly[i];
      delete [] Poly; Poly = (Polyline *) 0;
   }

   Poly = p; p = (Polyline *) 0;

   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

Polyline TCPolyArray::operator[](int n) const {

   if(n < 0 || n >= NPolys) {
      mlog << Error << "\nTCPolyArray::operator[](int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   return(Poly[n]);
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::add(const Polyline &p) {

   extend(NPolys + 1);
   
   Poly[NPolys++] = p;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCPolyArray::add_file(const char *filename) {
   ifstream in;
   Polyline p;
   int n;
   
   // Open the input file
   in.open(filename);

   if(!in) {
      mlog << Error
           << "\nTCPolyArray::add_file(const char *) -> "
           << "can't open land region file \"" << filename
           << "\" for reading\n\n";
      exit(1);
   }
   
   // Parse the TC Polylines
   while(in >> p) { add(p); n++; }
   
   mlog << Debug(3)
        << "Read " << n << " TC land regions from file: "
        << filename << "\n";
   
   return(true);
}

////////////////////////////////////////////////////////////////////////

double TCPolyArray::min_dist(double clat, double clon) const {
   int i, j;
   double cur_d, poly_d, min_d;

   // Loop through the polylines
   for(i=0, min_d=bad_data_double; i<NPolys; i++) {

      // Loop through the current polyline points
      for(j=0, poly_d=bad_data_double; j<Poly[i].n_points-1; j++) {

         // Compute current line segment distance
         cur_d = min_dist_linesegment(Poly[i].u[j],   Poly[i].v[j],
                                      Poly[i].u[j+1], Poly[i].v[j+1],
                                      clon, clat);

         // Keep track of the polyline minimum
         if(is_bad_data(poly_d)) poly_d = cur_d;
         if(cur_d < poly_d)      poly_d = cur_d;
      }

      // Inside the polyline is negative distance      
      if(Poly[i].is_inside(clon, clat) != 0) poly_d *= -1.0;

      // Keep track of the array minimum
      if(is_bad_data(min_d)) min_d = poly_d;
      if(poly_d < min_d)     min_d = poly_d;
   }

   // Convert degrees to km
   min_d *= 111.12;

   return(min_d);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

bool operator>>(istream & in, Polyline & p) {
   int i, n;

   p.clear();

   ConcatString line, name_cs;
   StringArray a;
   char name[8];

   // Read the map region meta data
   if(!line.read_line(in)) return (false);

   // Split up the meta data line
   a = line.split(" ");
   
   // Parse header line: NNN AAAAAAAA 2
   // NNN is the 3-digit number of points
   // AAAAAAAA is the 8-character name of the region
   n = atoi(a[0]);
   strncpy(name, line+4, 8);
   name[8] = '\0';
   name_cs = name;
   name_cs.ws_strip();
   p.set_name(name_cs);
   
   mlog << Debug(4)
        << "Reading " << n << " points for the \""
        << p.name << "\" region.\n";

   // Parse the lat/lon data lines
   for(i=0; i<n; i++) {
      if(!line.read_line(in)) return (false);
      a = line.split(" ");
      // Convert from degrees east to west
      p.add_point(rescale_lon(-1.0*atof(a[0])), atof(a[1]));
   }
   
   if(p.n_points < 2) {
      mlog << Error
           << "bool operator>>(istream & in, Polyline & p) -> "
           << "region \"" << p.name << "\" must have at least 2 points.\n\n";
      exit(1);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

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
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_math.h"
#include "nav.h"

#include "tc_poly.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCPoly
//
////////////////////////////////////////////////////////////////////////

TCPoly::TCPoly() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCPoly::~TCPoly() {
   clear();
}

////////////////////////////////////////////////////////////////////////

TCPoly::TCPoly(const TCPoly & p) {
   
   init_from_scratch();

   assign(p);
}

////////////////////////////////////////////////////////////////////////

TCPoly & TCPoly::operator=(const TCPoly & p) {

   if(this == &p) return(*this);

   assign(p);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCPoly::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPoly::clear() {

   Name.clear();
   LatLon.clear();
   LatCen = bad_data_double;
   LonCen = bad_data_double;
   GnomonProj.clear();
   GnomonXY.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPoly::assign(const TCPoly & p) {

   clear();

   Name = p.Name;
   LatLon = p.LatLon;
   LatCen = p.LatCen;
   LonCen = p.LonCen;
   GnomonProj = p.GnomonProj;
   GnomonXY = p.GnomonXY;

   return;
}

////////////////////////////////////////////////////////////////////////

double TCPoly::min_dist(double lat, double lon) const {
   int i, j;
   double dcur, dmin, x, y;

   // Loop through the current polyline points
   for(i=0, dmin=bad_data_double; i<LatLon.n_points; i++) {

      // Wrap around from the last point back to the first
      j = (i+1) % LatLon.n_points;

      dcur = gc_dist_to_line(LatLon.v[i], LatLon.u[i],
                             LatLon.v[j], LatLon.u[j],
                             lat, lon);

      // Keep track of the polyline minimum
      if(is_bad_data(dmin)) dmin = dcur;
      if(dcur < dmin)       dmin = dcur;
   }

   // Attempt to convert from lat/lon to gnmon xy
   if(GnomonProj.latlon_to_uv(lat, lon, x, y)) {

      // Inside the polyline is negative distance
      if(GnomonXY.is_inside(x, y)) dmin *= -1.0;
   }

   return(dmin);
}

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

   Poly = (TCPoly *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::clear() {

   if(Poly) { delete [] Poly;  Poly = (TCPoly *) 0; }
   
   NPolys = NAlloc = 0;

   CheckDist = bad_data_double;

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
   CheckDist = a.CheckDist;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::extend(int n) {
   int i;

   if(NAlloc >= n) return;

   int k = n/tc_poly_array_alloc_inc;

   if(n%num_array_alloc_inc) k++;

   n = k*tc_poly_array_alloc_inc;

   TCPoly * p = (TCPoly *) 0;

   p = new TCPoly [n];

   if(!p) {
      mlog << Error
           << "\nvoid TCPolyArray::extend(int) -> "
           << "memory allocation error\n\n";
      exit(1);
   }

   if(Poly) {
      for(i=0; i<NPolys; i++) p[i] = Poly[i];
      delete [] Poly; Poly = (TCPoly *) 0;
   }

   Poly = p; p = (TCPoly *) 0;

   NAlloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

TCPoly TCPolyArray::operator[](int n) const {

   if(n < 0 || n >= NPolys) {
      mlog << Error << "\nTCPolyArray::operator[](int) const -> "
           << "range check error\n\n";
      exit(1);
   }

   return(Poly[n]);
}

////////////////////////////////////////////////////////////////////////

void TCPolyArray::add(const TCPoly & p) {

   extend(NPolys + 1);
   
   Poly[NPolys++] = p;

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCPolyArray::add_file(const char *filename) {
   ifstream in;
   TCPoly p;
   int n = 0;
   
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

void TCPolyArray::set_check_dist() {
   int i;
   NumArray dsp, dnp;
 
   // Compute the distance from the polyline centroid to the north
   // and south poles.
   for(i=0; i<NPolys; i++) {
      dnp.add(gc_dist(Poly[i].LatCen, Poly[i].LonCen,  90.0, 0.0));
      dsp.add(gc_dist(Poly[i].LatCen, Poly[i].LonCen, -90.0, 0.0));
   }
   
   // Find the maximum of the minimum distances to each pole.
   CheckDist = max(dnp.min(), dsp.min()) + earth_radius_km;

   mlog << Debug(3)
        << "Skipping distance calculations for points more than "
        << CheckDist << " km from TC land region centerpoints.\n";

   return;
}

////////////////////////////////////////////////////////////////////////

double TCPolyArray::min_dist(double lat, double lon, int &imin) const {
   int i;
   double dcur, dmin, dcen;
   
   // Loop through the polylines
   for(i=0, dmin=bad_data_double; i<NPolys; i++) {

      // Skip polylines whose centroids are too far away
      if(!is_bad_data(CheckDist)) {
         dcen = gc_dist(Poly[i].LatCen, Poly[i].LonCen, lat, lon);
         if(dcen > CheckDist) continue;
      }

      // Get the minimum distance to the current polyline
      dcur = Poly[i].min_dist(lat, lon);

      // Keep track of the array minimum
      if(is_bad_data(dmin)) { dmin = dcur; imin = i; }
      if(dcur < dmin)       { dmin = dcur; imin = i; }
   }

   return(dmin);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
////////////////////////////////////////////////////////////////////////

bool operator>>(istream & in, TCPoly & p) {
   int i, n;
   double x, y;

   p.clear();

   ConcatString line, name_cs;
   StringArray a;
   char name[9];

   // Read the map region meta data
   if(!line.read_line(in)) return (false);

   // Split up the meta data line
   a = line.split(" ");
   
   // Parse header line: NNN AAAAAAAA 2
   // NNN is the 3-digit number of points
   // AAAAAAAA is the 8-character name of the region
   n = atoi(a[0].c_str());
   strncpy(name, line.c_str()+4, 8);
   name[8] = '\0';
   name_cs = name;
   name_cs.ws_strip();
   p.Name = name_cs;
   
   mlog << Debug(4)
        << "Reading " << n << " points for TC land region \""
        << p.Name << "\".\n";

   // Parse the lat/lon data lines
   for(i=0; i<n; i++) {
      if(!line.read_line(in)) return (false);
      a = line.split(" ");
      // Convert from degrees east to west
      p.LatLon.add_point(rescale_lon(-1.0*atof(a[0].c_str())), atof(a[1].c_str()));
   }
   
   if(p.LatLon.n_points < 2) {
      mlog << Error
           << "bool operator>>(istream & in, Polyline & p) -> "
           << "region \"" << p.Name
           << "\" must have at least 2 points.\n\n";
      exit(1);
   }

   // Set up the gnomonic projection
   p.LatLon.centroid(p.LonCen, p.LatCen);
   mlog << Debug(4) << "TC land region \"" << p.Name
        << "\" centerpoint ("  << p.LatCen << ", " << p.LonCen << ").\n";
   p.GnomonProj.set_center(p.LatCen, p.LonCen);

   // Convert from lat/lon to gnomon xy
   for(i=0; i<p.LatLon.n_points; i++) {
      p.GnomonProj.latlon_to_uv(p.LatLon.v[i], p.LatLon.u[i], x, y);
      p.GnomonXY.add_point(x, y);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef __VX_TC_POLY_H__
#define __VX_TC_POLY_H__

////////////////////////////////////////////////////////////////////////

#include "vx_log.h"
#include "vx_util.h"
#include "gnomon.h"

////////////////////////////////////////////////////////////////////////

static const int n_tc_poly_header_elements = 3;
static const int tc_poly_array_alloc_inc   = 100;

////////////////////////////////////////////////////////////////////////

class TCPolyArray; // forward reference

////////////////////////////////////////////////////////////////////////

class TCPoly {
   
   friend class TCPolyArray;
   
   friend bool operator>>(istream &, TCPoly &);

   private:

      void init_from_scratch();

      void assign(const TCPoly &);

      ConcatString       Name;

      Polyline           LatLon;
      double             LatCen;
      double             LonCen;

      GnomonicProjection GnomonProj;
      Polyline           GnomonXY;
      
   public:

      TCPoly();
     ~TCPoly();
      TCPoly(const TCPoly &);
      TCPoly & operator=(const TCPoly &);

      void clear();

      ConcatString name() const;

      double min_dist(double lat, double lon) const;
};

////////////////////////////////////////////////////////////////////////

inline ConcatString TCPoly::name() const { return(Name); }

////////////////////////////////////////////////////////////////////////

extern bool operator>>(istream &, TCPoly &);

////////////////////////////////////////////////////////////////////////

class TCPolyArray {

   private:

      void init_from_scratch();

      void assign(const TCPolyArray &);

      void extend(int, bool exact = true);

      TCPoly * Poly;
      int      NPolys;
      int      NAlloc;
      double   CheckDist;

   public:

      TCPolyArray();
     ~TCPolyArray();
      TCPolyArray(const TCPolyArray &);
      TCPolyArray & operator=(const TCPolyArray &);

      void clear();

      TCPoly operator[](int) const;
      int n_polys() const;

      void add(const TCPoly &);
      bool add_file(const char *filename);

      void set_check_dist();

      double min_dist(double lat, double lon, int &imin) const;
};

////////////////////////////////////////////////////////////////////////

inline int TCPolyArray::n_polys() const { return(NPolys); }

////////////////////////////////////////////////////////////////////////

#endif  //  __VX_TC_POLY_H__

////////////////////////////////////////////////////////////////////////

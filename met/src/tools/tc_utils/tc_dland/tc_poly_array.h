// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2014
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef _VX_TC_POLY_ARRAY_H_
#define _VX_TC_POLY_ARRAY_H_

////////////////////////////////////////////////////////////////////////

#include "vx_log.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

static const int n_tc_poly_header_elements = 3;
static const int tc_poly_array_alloc_inc   = 100;

////////////////////////////////////////////////////////////////////////

extern bool operator>>(istream &, Polyline &);

////////////////////////////////////////////////////////////////////////

class TCPolyArray {

   private:

      void init_from_scratch();

      void assign(const TCPolyArray &);

      void extend(int);

      Polyline * Poly;
      int        NPolys;
      int        NAlloc;
      
   public:

      TCPolyArray();
     ~TCPolyArray();
      TCPolyArray(const TCPolyArray &);
      TCPolyArray & operator=(const TCPolyArray &);

      void clear();

      Polyline operator[](int) const;
      int n_polys() const;

      void add(const Polyline &);
      bool add_file(const char *filename);

      double min_dist(double lat, double lon) const;
};

////////////////////////////////////////////////////////////////////////

inline int TCPolyArray::n_polys() const { return(NPolys); }

////////////////////////////////////////////////////////////////////////

#endif  //  _VX_TC_POLY_ARRAY_H_

////////////////////////////////////////////////////////////////////////

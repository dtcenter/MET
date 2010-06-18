
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __VX_GRID_FIND_GRID_BY_NAME_H__
#define  __VX_GRID_FIND_GRID_BY_NAME_H__


////////////////////////////////////////////////////////////////////////


#include "vx_data_grids/grid.h"


////////////////////////////////////////////////////////////////////////


class GridInfo {

   private:

      void init_from_scratch();

      void assign(const GridInfo &);

   public:

      GridInfo();
     ~GridInfo();
      GridInfo(const GridInfo &);
      GridInfo & operator=(const GridInfo &);

      void clear();

      bool ok() const;

      void set_grid(Grid &) const;

      bool allocated;   //  do the pointers below point into the heap?

         //
         //  at most ONE of these should be nonzero
         //

      const LambertData       * lc;
      const StereographicData * st;
      const LatLonData        * ll;
      const MercatorData      * m;

};


////////////////////////////////////////////////////////////////////////


extern bool find_grid_by_name(const char *, Grid &);

extern bool find_grid_by_name(const char *, GridInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_GRID_FIND_GRID_BY_NAME_H__  */


////////////////////////////////////////////////////////////////////////





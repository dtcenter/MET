// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __PROJ_GRID_H__
#define  __PROJ_GRID_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <memory>

#include "vx_math.h"

extern "C" {

#include "proj.h"

}


////////////////////////////////////////////////////////////////////////


struct ProjInfo {

   PJ_CONTEXT * C;   //   = 0

   PJ * pj;

      //////////

   ProjInfo() { C = 0;  pj = 0; }

  ~ProjInfo() { 

      if ( pj )  { proj_destroy (pj);   pj = 0; }

   }

};


////////////////////////////////////////////////////////////////////////


class ProjGrid {

   protected:

      void init_from_scratch();

      void assign(const ProjGrid &);

      std::shared_ptr<ProjInfo> info;

      ConcatString Proj_Set;   //  string used to initialize PROJ

      Affine Aff;

      int Nx;
      int Ny;

      ConcatString Name;

   public:

      ProjGrid();
     ~ProjGrid();
      ProjGrid(const ProjGrid &);
      ProjGrid & operator=(const ProjGrid &);

      void clear();

      void dump(std::ostream &, int = 0) const;


         //
         //  set stuff
         //

      void set_proj(const char *);   //  string used to initialize the PROJ stuff

      void set_affine(const Affine &);

      void set_size(int _nx, int _ny);

      void set_name(const char *);

         //
         //  get stuff
         //

      int nx() const;
      int ny() const;

      const char * name() const;

      ConcatString proj_set_string() const;

         //
         //  do stuff
         //

      void xy_to_latlon (double   x, double   y, double & lat, double & lon) const;

      void latlon_to_xy (double lat, double lon, double &   x, double &   y) const;


};


////////////////////////////////////////////////////////////////////////


inline void ProjGrid::set_size(int _nx, int _ny)  { Nx = _nx;  Ny = _ny;  return; }

inline void ProjGrid::set_name(const char * _name)  { Name = _name;  return; }

inline void ProjGrid::set_affine(const Affine & _aff)  { Aff = _aff;  return; }

inline int  ProjGrid::nx() const  { return ( Nx ); }
inline int  ProjGrid::ny() const  { return ( Ny ); }

inline const char * ProjGrid::name() const  { return ( Name.text() ); }

inline ConcatString ProjGrid::proj_set_string() const  { return ( Proj_Set ); }


////////////////////////////////////////////////////////////////////////


#endif  /*  __PROJ_GRID_H__  */


////////////////////////////////////////////////////////////////////////

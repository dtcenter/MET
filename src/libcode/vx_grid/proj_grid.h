

////////////////////////////////////////////////////////////////////////


#ifndef  __PROJ_GRID_H__
#define  __PROJ_GRID_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_math.h"

extern "C" {

#include "proj.h"

}


////////////////////////////////////////////////////////////////////////


class ProjGrid {

   protected:

      void init_from_scratch();

      void assign(const ProjGrid &);


      PJ_CONTEXT * C;   //   = 0
      PJ * pj;

      ConcatString Proj_Set;   //  string used to initialize pj

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

      void set_proj(const char *);   //  string used to iniitialize the PROJ stuff

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



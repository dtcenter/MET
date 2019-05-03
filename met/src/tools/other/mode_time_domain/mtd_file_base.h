// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_TIME_DOMAIN_BASE_FILE_H__
#define  __MODE_TIME_DOMAIN_BASE_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include <netcdf>
using namespace netCDF;

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_grid.h"

#include "int_array.h"


////////////////////////////////////////////////////////////////////////


static const char mtd_file_suffix [] = ".mtd";


////////////////////////////////////////////////////////////////////////


inline int mtd_three_to_one(int _Nx, int _Ny, int _Nt, int x, int y, int t)

{

// return ( (y*_Nx + x)*_Nt + t );

return ( (t*_Ny + y)*_Nx + x );

}


////////////////////////////////////////////////////////////////////////


enum MtdFileType {

   mtd_file_raw, 
   mtd_file_conv, 
   mtd_file_mask, 
   mtd_file_object, 

   no_mtd_file_type

};


////////////////////////////////////////////////////////////////////////


class MtdFileBase {

   protected:

      void base_init_from_scratch();

      void base_assign(const MtdFileBase &);

      virtual void read  (NcFile &);
      virtual void write (NcFile &) const;


      Grid * G;        //  allocated

      int Nx, Ny, Nt;

      unixtime StartValidTime;

      int DeltaT;   //  seconds

      IntArray Lead_Times;

      ConcatString Filename;

      MtdFileType FileType;

   public:

      MtdFileBase();
      virtual ~MtdFileBase();

      virtual void clear();

      virtual void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      // virtual void set_size(int _nx, int _ny, int _nt) = 0;

      void set_grid(const Grid &);

      void set_start_valid_time (unixtime);
      void set_delta_t          (int);   //  seconds

      void set_lead_time(int index, int value);

      void set_filetype(MtdFileType);

         //
         //  get stuff
         //

      ConcatString filename() const;

      MtdFileType filetype() const;

      int nx() const;
      int ny() const;
      int nt() const;

      Grid grid() const;

      const Grid * grid_p() const;

      unixtime start_valid_time () const;
      int      delta_t          () const;   //  seconds

      unixtime valid_time (int) const;

      int lead_time (int index) const;

         //
         //  do stuff
         //

      void latlon_to_xy (double lat, double lon, double &   x, double &   y) const;
      void xy_to_latlon (double   x, double   y, double & lat, double & lon) const;

};


////////////////////////////////////////////////////////////////////////


inline int MtdFileBase::nx() const { return ( Nx ); }
inline int MtdFileBase::ny() const { return ( Ny ); }
inline int MtdFileBase::nt() const { return ( Nt ); }

inline unixtime MtdFileBase::start_valid_time() const { return ( StartValidTime ); }

inline int MtdFileBase::delta_t() const { return ( DeltaT ); }

inline MtdFileType MtdFileBase::filetype() const { return ( FileType ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_TIME_DOMAIN_BASE_FILE_H__  */


////////////////////////////////////////////////////////////////////////



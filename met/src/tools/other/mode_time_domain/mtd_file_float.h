// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_TIME_DOMAIN_FLOAT_FILE_H__
#define  __MODE_TIME_DOMAIN_FLOAT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_grid.h"
#include "vx_regrid.h"

#include "mtd_file_base.h"
#include "data_plane.h"


////////////////////////////////////////////////////////////////////////


class MtdIntFile;   //  forward reference


////////////////////////////////////////////////////////////////////////


class MtdFloatFile : public MtdFileBase {

   protected:

      void float_init_from_scratch();

      void float_assign(const MtdFloatFile &);

      virtual void read  (NcFile &);
      virtual void write (NcFile &) const;


      float * Data;   //  allocated

      float DataMin;
      float DataMax;

      int Spatial_Radius;   //  = -1 if not a convolved file

      int TimeBeg;
      int TimeEnd;

   public:

      MtdFloatFile();
      virtual ~MtdFloatFile();
      MtdFloatFile(const MtdFloatFile &);
      MtdFloatFile & operator=(const MtdFloatFile &);

      virtual void clear();

      virtual void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_size(int _nx, int _ny, int _nt);

      void set_data_minmax(double _data_min, double _data_max);

      void set_spatial_radius(int);

      void set_time_window(int, int);

         //
         //  get stuff
         //

      float data_min() const;
      float data_max() const;

      float operator()(int _x, int _y, int _t) const;

      int spatial_radius() const;

      int time_beg() const;

      int time_end() const;

      const float * data() const;

      void get_data_plane(const int t, DataPlane & out);

         //
         //  do stuff
         //

      void put   (const float, int _x, int _y, int _t);

      void put   (const DataPlane &, const int t);

      bool read  (const char * filename);

      void write (const char * filename) const;

      MtdIntFile threshold(double) const;
      void       threshold(double, MtdIntFile &) const;

      MtdIntFile threshold(const SingleThresh &) const;
      void       threshold(const SingleThresh &, MtdIntFile &) const;

      MtdFloatFile const_t_slice(int t) const;

      MtdFloatFile convolve(const int spatial_r, const int time_beg, const int time_end) const;

      void calc_data_minmax();

      void put_data_plane(const int t, const DataPlane &);

      void regrid(const Grid & to_grid, const RegridInfo &);

};


////////////////////////////////////////////////////////////////////////


inline int MtdFloatFile::spatial_radius() const { return ( Spatial_Radius ); }

inline int MtdFloatFile::time_beg() const { return ( TimeBeg ); }
inline int MtdFloatFile::time_end() const { return ( TimeEnd ); }

inline float MtdFloatFile::data_min() const { return ( DataMin ); }
inline float MtdFloatFile::data_max() const { return ( DataMax ); }

inline const float * MtdFloatFile::data() const { return ( Data ); }

inline float MtdFloatFile::operator()(int _x, int _y, int _t) const

{

return ( Data[mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t)] );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_TIME_DOMAIN_FLOAT_FILE_H__  */


////////////////////////////////////////////////////////////////////////



// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_TIME_DOMAIN_INT_FILE_H__
#define  __MODE_TIME_DOMAIN_INT_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_grid.h"

#include "mtd_file_float.h"
#include "3d_moments.h"
#include "2d_moments.h"


////////////////////////////////////////////////////////////////////////


class MtdIntFile : public MtdFileBase {

      friend class MtdFloatFile;

   protected:

      void int_init_from_scratch();

      void int_assign(const MtdIntFile &);

      virtual void read  (NcFile &);
      virtual void write (NcFile &) const;



      int * Data;   //  allocated

      int * ObjVolume;    //  volume, allocated, used after splitting

      int Nobjects;   //  object numbers run from 1 to Nobjects inclusive

      int DataMin;
      int DataMax;

      int Radius;

      double Threshold;

   public:

      MtdIntFile();
      virtual ~MtdIntFile();
      MtdIntFile(const MtdIntFile &);
      MtdIntFile & operator=(const MtdIntFile &);

      virtual void clear();

      virtual void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_size(int _nx, int _ny, int _nt);

      void set_data_minmax(double _data_min, double _data_max);

      void set_radius    (int);
      void set_threshold (double);

      void set_volumes(int n, const int *);

      void set_n_objects(int);

         //
         //  get stuff
         //

      int data_min() const;
      int data_max() const;

      int operator()(int _x, int _y, int _t) const;

      int radius() const;

      double threshold() const;

      int n_objects() const;

      int volume(int) const;   //  0-based

      int total_volume() const;

      const int * data() const;

         //
         //  do stuff
         //

      void put   (const int, int _x, int _y, int _t);

      bool read  (const char * filename);

      void write (const char * filename) const;

      void split ();

      MtdIntFile split_const_t(int & n_shapes) const;

      MtdIntFile select         (int)              const;   //  1-based, for selecting simple objects
      MtdIntFile select_cluster (const IntArray &) const;   //  1-based, for selecting cluster objects

      MtdIntFile  const_t_slice (const int t) const;
      MtdIntFile  const_t_mask  (const int t, const int obj_num) const;   //  obj_num is 1-based

      void fatten();   //  for 2D slices

      void set_to_zeroes();

      void zero_border(int);

      void toss_small_objects (int min_volume);

      void sift_objects(const int n_new, const int * new_to_old);

      Mtd_3D_Moments calc_3d_moments() const;
      Mtd_2D_Moments calc_2d_moments() const;

      void calc_3d_bbox(int & x_min, int & x_max, int & y_min, int & y_max, int & t_min, int & t_max) const;

      void calc_3d_centroid(double & xbar, double & ybar, double & tbar) const;

      void calc_2d_centroid_at_t(const int t, double & xbar, double & ybar) const;

      int x_left  (const int y) const;
      int x_right (const int y) const;

};


////////////////////////////////////////////////////////////////////////


inline int MtdIntFile::radius() const { return ( Radius ); }

inline double MtdIntFile::threshold() const { return ( Threshold ); }

inline int MtdIntFile::data_min() const { return ( DataMin ); }
inline int MtdIntFile::data_max() const { return ( DataMax ); }

inline int MtdIntFile::n_objects() const { return ( Nobjects ); }

inline const int * MtdIntFile::data() const { return ( Data ); }

inline int MtdIntFile::operator()(int _x, int _y, int _t) const

{

return ( Data[mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t)] );

}


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_TIME_DOMAIN_INT_FILE_H__  */


////////////////////////////////////////////////////////////////////////



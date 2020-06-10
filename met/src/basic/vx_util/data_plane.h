// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __DATA_PLANE_H__
#define  __DATA_PLANE_H__

////////////////////////////////////////////////////////////////////////

#include <vector>

#include "two_to_one.h"
#include "threshold.h"
#include "thresh_array.h"
#include "num_array.h"
#include "two_d_array.h"
#include "config_funcs.h"

#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

typedef TwoD_Array<bool> MaskPlane;

////////////////////////////////////////////////////////////////////////

class DataPlane {

   friend class ShapeData;

   private:

      void init_from_scratch();
      void assign(const DataPlane &);

      std::vector<double> Data;

      int Nx;
      int Ny;

      int Nxy;   //  Nx*Ny

      unixtime InitTime;      // Initialization time in unixtime
      unixtime ValidTime;     // Valid time in unixtime
      int      LeadTime;      // Lead time in seconds
      int      AccumTime;     // Accumulation time in seconds

   public:

      DataPlane();
     ~DataPlane();
      DataPlane(const DataPlane &);
      DataPlane & operator=(const DataPlane &);

      void clear();

      void erase();

      void dump(ostream &, int = 0) const;

         //
         // Set functions
         //

      void set_size(int Nx, int Ny);

      void set(double, int, int);   //  x, y
      void set_block(double *, int, int);   //  nx, ny

      void set_constant(double);

      void set_init(unixtime);
      void set_valid(unixtime);
      void set_lead(int);
      void set_accum(int);

         //
         // Get functions
         //

      int      nx() const;
      int      ny() const;
      bool     is_empty() const;
      unixtime init() const;
      unixtime valid() const;
      int      lead() const;
      int      accum() const;
      double   get(int x, int y) const;
      double   operator () (int x, int y) const;

      const double * data() const;
      std::vector<double> & buf();

         //
         // Do stuff
         //

      void threshold(const SingleThresh &);
      void convert  (const UserFunc_1Arg &);
      void censor   (const ThreshArray &, const NumArray &);

      void replace_bad_data(const double value);

      int  two_to_one(int x, int y) const;
      void one_to_two(int n, int &x, int &y) const;

      bool s_is_on(int x, int y) const;
      bool f_is_on(int x, int y) const;

      void data_range(double & data_min, double & data_max) const;

      MaskPlane mask_plane() const;

      void shift_right  (int n);

      void put(const double, const int __x__, const int __y__);

      bool fitwav_1d_old (const int start_wave, const int end_wave);
      bool fitwav_1d     (const int start_wave, const int end_wave);

};

////////////////////////////////////////////////////////////////////////

inline int DataPlane::nx() const { return (Nx); }
inline int DataPlane::ny() const { return (Ny); }

inline bool DataPlane::is_empty() const { return (Nxy == 0); }

inline unixtime DataPlane::init()  const { return (InitTime);  }
inline unixtime DataPlane::valid() const { return (ValidTime); }
inline int      DataPlane::lead()  const { return (LeadTime);  }
inline int      DataPlane::accum() const { return (AccumTime); }

inline double DataPlane::operator()(int x, int y) const { return(get(x, y)); }

inline const double * DataPlane::data() const { return ( Data.data() ); }
inline std::vector<double> & DataPlane::buf() { return ( Data ); }

////////////////////////////////////////////////////////////////////////

static const int dataplane_default_alloc_inc = 20;

////////////////////////////////////////////////////////////////////////

class DataPlaneArray {

   protected:

      void init_from_scratch();

      void assign(const DataPlaneArray &);

      void extend(int, bool exact = true);

      void check_xy_size(const DataPlane &) const;   //  check to make sure all planes added are same size


      double * Lower;       //  allocated

      double * Upper;       //  allocated

      DataPlane ** Plane;   //  allocated

      int Nplanes;
      int Nalloc;

      int AllocInc;

   public:

      DataPlaneArray();
     ~DataPlaneArray();
      DataPlaneArray(const DataPlaneArray &);
      DataPlaneArray & operator=(const DataPlaneArray &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

     void set_alloc_inc(int);

     void set_levels (int, double _low, double _up);

         //
         //  get stuff
         //

      int n_planes() const;

      int nx () const;
      int ny () const;

      double lower (int) const;
      double upper (int) const;

      void levels (int, double & _low, double & _up) const;
      void level_range (double & _low, double & _up) const;

      double data (int plane, int x, int y) const;
      void   set  (double, int plane, int x, int y);

      DataPlane & operator[](int) const;

         //
         //  do stuff
         //

      void add(const DataPlane &, double _low, double _up);   //  for two-level plane

      void replace_bad_data(const double value);

};

////////////////////////////////////////////////////////////////////////

inline int DataPlaneArray::n_planes () const { return ( Nplanes ); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __DATA_PLANE_H__  */

////////////////////////////////////////////////////////////////////////

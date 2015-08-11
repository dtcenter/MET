

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_TIME_DOMAIN_FILE_H__
#define  __MODE_TIME_DOMAIN_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "netcdf.hh"

#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"
#include "vx_grid.h"


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

      unixtime StartTime;

      int DeltaT;   //  seconds

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

      void set_start_time (unixtime);
      void set_delta_t    (int);   //  seconds

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

      unixtime start_time () const;
      int      delta_t    () const;   //  seconds

      unixtime valid_time (int) const;

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

inline unixtime MtdFileBase::start_time() const { return ( StartTime ); }

inline int MtdFileBase::delta_t() const { return ( DeltaT ); }

inline MtdFileType MtdFileBase::filetype() const { return ( FileType ); }


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

      int Radius;   //  = -1 if not a convolved file

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

      void set_radius(int);

         //
         //  get stuff
         //

      float data_min() const;
      float data_max() const;

      float operator()(int _x, int _y, int _t) const;

      int radius() const;

         //
         //  do stuff
         //

      void put   (const float, int _x, int _y, int _t);

      bool read  (const char * filename);

      void write (const char * filename) const;

      MtdIntFile threshold(double) const;
      void       threshold(double, MtdIntFile &) const;

      MtdFloatFile  const_t_slice(int t) const;

};


////////////////////////////////////////////////////////////////////////


inline int MtdFloatFile::radius() const { return ( Radius ); }

inline float MtdFloatFile::data_min() const { return ( DataMin ); }
inline float MtdFloatFile::data_max() const { return ( DataMax ); }

inline float MtdFloatFile::operator()(int _x, int _y, int _t) const

{

return ( Data[mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t)] );

}


////////////////////////////////////////////////////////////////////////


class MtdIntFile : public MtdFileBase {

      friend class MtdFloatFile;

   protected:

      void int_init_from_scratch();

      void int_assign(const MtdIntFile &);

      virtual void read  (NcFile &);
      virtual void write (NcFile &) const;


      int * Data;   //  allocated

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

         //
         //  get stuff
         //

      int data_min() const;
      int data_max() const;

      int operator()(int _x, int _y, int _t) const;

      int radius() const;

      double threshold() const;

         //
         //  do stuff
         //

      void put   (const int, int _x, int _y, int _t);

      bool read  (const char * filename);

      void write (const char * filename) const;

      void split ();

      MtdIntFile select(int) const;

      MtdIntFile const_t_slice(int t) const;

};


////////////////////////////////////////////////////////////////////////


inline int MtdIntFile::radius() const { return ( Radius ); }

inline double MtdIntFile::threshold() const { return ( Threshold ); }

inline int MtdIntFile::data_min() const { return ( DataMin ); }
inline int MtdIntFile::data_max() const { return ( DataMax ); }

inline int MtdIntFile::operator()(int _x, int _y, int _t) const

{

return ( Data[mtd_three_to_one(Nx, Ny, Nt, _x, _y, _t)] );

}


////////////////////////////////////////////////////////////////////////

/*
class ModeTimeDomainFile {

   private:

      void init_from_scratch();

      void assign(const ModeTimeDomainFile &);

      // int three_to_one(int x, int y, int t) const;

      // void parse_st_grid (NcFile *);
      // void write_st_grid (NcFile *);

      double int_to_double (int)    const;
      int    double_to_int (double) const;



      void sift_objects(const int n_new, const int * new_to_old);


      Grid        * G;        //  allocated
      MtdDataType * Data;     //  allocated

      int Nx, Ny, Nt;

      int Radius;          //  -1   if not convolved
      double Threshold;    //  -1.0 if not thresholded


      unixtime StartTime;

      int DeltaT;   //  seconds

      ConcatString Filename;


      int NObjects;

      int    * Volumes;            //  allocated
      double * MaxConvIntensity;   //  allocated

   public:

      ModeTimeDomainFile();
     ~ModeTimeDomainFile();
      ModeTimeDomainFile(const ModeTimeDomainFile &);
      ModeTimeDomainFile & operator=(const ModeTimeDomainFile &);


      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_size(int _nx, int _ny, int _nt);

      void set_grid(const Grid &);

      void set_radius     (int);
      void set_threshold  (double);

      void set_start_time (unixtime);
      void set_delta_t    (int);   //  seconds

         //
         //  get stuff
         //

      ConcatString filename() const;

      int nx() const;
      int ny() const;
      int nt() const;

      double radius    () const;
      double threshold () const;

      unixtime start_time () const;
      int      delta_t    () const;   //  seconds

      unixtime valid_time (int) const;


      int    ival(int x, int y, int t) const;
      double dval(int x, int y, int t) const;

      double operator()(int x, int y, int t) const;

      void ival_x_row(int y, int t, int *   ) const;
      void dval_x_row(int y, int t, double *) const;

      Grid grid() const;

      bool s_is_on(int x, int y, int t) const;

      bool f_is_on(int x, int y, int t) const;

      int volume () const;
      int volume (int object_number) const;

      double max_conv_intensity (int object_number) const;

      int n_objects() const;

      bool object_closest_to_xy(const int x_test, const int y_test, int & obj_num, double & dist) const;   //  distance in grid units
      bool object_closest_to_xy(const int min_volume, const int x_test, const int y_test, int & obj_num, double & dist) const;   //  distance in grid units

      double closest_approach(const int obj_num, const int x_test, const int y_test) const;   //  distance in grid units


         //
         //  do stuff
         //

      bool read  (const char * filename);
      bool write (const char * filename);

      void put (double value, int x, int y, int t);

      void do_threshold(double value);

      void calc_data_minmax(double & dmin, double & dmax) const;

      void latlon_to_xy (double lat, double lon, double & x, double & y) const;
      void xy_to_latlon (double x, double y, double & lat, double & lon) const;

         //
         //  spatial attributes at a given time
         //

      void centroid(double & xbar, double & ybar, double & tbar) const;

      bool xy_centroid(const int t, double & xbar, double & ybar) const;

         //
         //  velocity
         //

      bool centroid_velocity(const int t, double & xdot, double & ydot) const;   //  velocity between t and t + 1

      bool average_centroid_velocity(double & xdot, double & ydot) const;


         //
         //  area
         //

      int area_at_t(const int t) const;   //  could be zero

      double average_area() const;   //  could be zero;


      ModeTimeDomainFile const_time_slice(int t) const;

      Moments moments_at_t(const int t) const;


      void bbox(int & xmin, int & xmax, int & ymin, int & ymax, int & tmin, int & tmax) const;

      void set_to_zeroes();

      void zero_border(int size);

      void split();

      void toss_small_objects     (int min_volume);
      void toss_small_intensities (double min_intensity);

      void set_volumes(int, const int *);

      void set_max_conv_intensities(const double *);

};
*/

////////////////////////////////////////////////////////////////////////

/*
inline ConcatString ModeTimeDomainFile::filename () const { return ( Filename ); }

inline int ModeTimeDomainFile::nx () const { return ( Nx ); }
inline int ModeTimeDomainFile::ny () const { return ( Ny ); }
inline int ModeTimeDomainFile::nt () const { return ( Nt ); }

inline double ModeTimeDomainFile::radius    () const { return ( Radius ); }
inline double ModeTimeDomainFile::threshold () const { return ( Threshold ); }

inline unixtime ModeTimeDomainFile::start_time () const { return ( StartTime ); }
inline int      ModeTimeDomainFile::delta_t    () const { return ( DeltaT ); }

inline int      ModeTimeDomainFile::n_objects  () const { return ( NObjects ); }

inline double ModeTimeDomainFile::operator()(int x, int y, int t) const { return ( dval(x, y, t) ); }
*/

////////////////////////////////////////////////////////////////////////

/*
extern ModeTimeDomainFile split(const ModeTimeDomainFile &, int & n_shapes);

extern ModeTimeDomainFile split_const_t(const ModeTimeDomainFile &, int & n_shapes);

extern ModeTimeDomainFile select(const ModeTimeDomainFile &, int n);

extern int intersection_volume(const ModeTimeDomainFile &, const ModeTimeDomainFile &);
*/

////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_TIME_DOMAIN_FILE_H__  */


////////////////////////////////////////////////////////////////////////



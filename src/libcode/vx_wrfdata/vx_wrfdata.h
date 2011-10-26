// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   vx_wrfdata.h
//
//   Description:
//      Contains the declaration of the vx_wrfdata class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-03-07  Halley Gotway
//   001    02-22-11  Halley Gotway  Add bilinear interpolation option.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __WRFDATA_WRFDATA_H__
#define  __WRFDATA_WRFDATA_H__

///////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "shape.h"
#include "filterbox.h"
#include "polyline.h"
#include "node.h"
#include "interpmthd.h"
#include "vx_cal.h"

///////////////////////////////////////////////////////////////////////////////

//
// String to be put in wrfdata header
//
static const char wrfdata_magic_wrf001[] = "WRF001";
static const char wrfdata_magic[]        = "WRF001";

//
// Valid data is from 0 to wrfdata_int_data_max inclusive.
// Integers between wrfdata_int_data_max and wrfdata_int_max
// are reserved for flag values.
//
static const int wrfdata_int_max      = 65535;
static const int wrfdata_int_data_max = 65525;

///////////////////////////////////////////////////////////////////////////////

//
// Enumerations used in computing the boundary of a WrfData object
//
enum StepCase {
   ll_case       = 0,
   lr_case       = 1,
   ul_ll_case    = 2,
   lr_ur_case    = 3,
   lr_ul_case    = 4,
   ur_ll_case    = 5,
   ur_ul_ll_case = 6,
   lr_ur_ul_case = 7
};

enum StepDirection {
   plus_x  = 0,
   plus_y  = 1,
   minus_x = 2,
   minus_y = 3
};

///////////////////////////////////////////////////////////////////////////////

struct WrfDataHeader {     // magic = wrf001

   char magic[8];          // wrfdata_magic + nulls
   char units[8];          // forecast units (null-terminated)
   char gridname[16];      // grid name (null-terminated)

   int nx;                 // points in the x-direction
   int ny;                 // points in the y-direction

   unixtime creation_time; // file creation (unixtime)
   unixtime valid_time;    // forecast valid time (unixtime)
   int lead_time;          // forecast lead time (seconds)
   int accum_time;         // forecast accumulation period (seconds)

   double m;               // data scale value
   double b;               // data offset value
};

///////////////////////////////////////////////////////////////////////////////

class WrfData : public Shape {

   private:

      unsigned short *data;

      WrfDataHeader h;

      Moments mom;         // moments

      int nx;              // points in the x-direction
      int ny;              // points in the y-direction

      unixtime valid_time; // forecast valid time (unixtime)
      int lead_time;       // forecast lead time (seconds)
      int accum_time;      // forecast accumulation period (seconds)

      double m;            // data scale value
      double b;            // data offset value

      void assign(const WrfData &);
      int x_left(int) const;
      int x_right(int) const;

   public:

      //
      //  canonical members
      //

      WrfData();
      virtual ~WrfData();
      WrfData(const WrfData &);
      WrfData & operator=(const WrfData &);

      //
      //
      //

      int two_to_one(int, int) const;

      void one_to_two(int, int &, int &) const;

      void combine(const WrfData *, int, unixtime, int, int, int, double);

      void expand(int);

      //
      //
      //

      void put_xy_int(int, int x, int y);

      void put_xy_double(double, int x, int y);

      void set_size(int nx, int ny);

      void set_valid_time(unixtime);

      void set_lead_time(int);

      void set_accum_time(int);

      void set_m(double);
      void set_b(double);

      void set_header(const WrfDataHeader &);

      //
      //
      //

      double int_to_double(int) const;
      int double_to_int(double) const;

      int get_xy_int(int, int) const;
      double get_xy_double(int, int) const;

      double get_m() const;
      double get_b() const;

      void   get_range(double &, double &) const;

      const char * get_units() const;
      const char * get_gridname() const;

      int get_nx() const;
      int get_ny() const;

      unixtime get_init_time() const;
      unixtime get_valid_time() const;

      int get_lead_time() const;
      int get_accum_time() const;

      WrfDataHeader get_header() const;

      int s_is_on(int, int) const;
      int f_is_on(int, int) const;

      int is_bad_xy(int, int) const;
      int is_valid_xy(int, int) const;

      void bounding_box(BoundingBox &) const;

      Polyline convex_hull() const;

      Polyline single_boundary() const;

      Polyline single_boundary(bool, int) const;

      Polyline single_boundary_offset(double) const;

      Polyline single_boundary_offset(bool, int, double) const;

      void composite_boundary(Node &) const;

      void composite_boundary(bool, int, Node &) const;

      Polyline single_outline_dist(int) const;

      Polyline single_outline_step(int) const;

      Polyline single_outline_angle(int) const;

      int single_outline_point(double, double, double, Polyline &, bool,
                               double &, double &) const;

      double complexity() const;

      int s_area() const;

      //
      //
      //

      //
      // Filter the raw values based on the threshold and threshold indicator
      // and zero out the rest
      //
      void filter(SingleThresh);

      //
      // Convolution filter
      //
      void conv_filter(const FilterBox &);

      void conv_filter_circ(int diameter, double);  // diameter must be an odd number

      void coverage_filter(const FilterBox &, double);

      void zero_border(int);

      void zero_border(int, int);

      void translate(int, int);

      //
      // Threshold using a value and a threshold type to create a 0/1 mask field
      //
      void threshold(int t);
      void threshold(SingleThresh);

      void threshold_double(double t);
      void threshold_double(SingleThresh);

      //
      // Eliminate any shapes with an area that doesn't meet the threshold
      // criteria
      //
      void threshold_area(SingleThresh);

      //
      // Eliminate any shapes with intensity that doesn't meet the threshold
      // criteria for the intensity percentile specified
      //
      void threshold_intensity(const WrfData *, int perc, SingleThresh);

      //
      //
      //

      int read(const char *);

      void rescale_data(double, double);

      int write(const char *, const char *, const char *) const;

      //
      //  virtuals from base class Shape
      //

      Moments moments() const;
      void calc_moments();
      void centroid(double &, double &) const;
      double angle_degrees() const;
      double curvature(double &, double &) const;
      double area()  const;
      void calc_length_width(double &, double &)  const;
      double length() const;
      double width() const;
      void clear();
};

///////////////////////////////////////////////////////////////////////////////

static const int max_cell_elements = 2000;

static const int max_cells = 1000;

///////////////////////////////////////////////////////////////////////////////

class Cell {

   public:

      int e[max_cell_elements];

      int n;

      void assign(const Cell &);

      int has(int) const;

      void add(int);

      void clear();

      Cell();
      ~Cell();
      Cell(const Cell &);
      Cell & operator=(const Cell &);
};

///////////////////////////////////////////////////////////////////////////////

extern ostream & operator<<(ostream &, const Cell &);

///////////////////////////////////////////////////////////////////////////////

class Partition {

   public:

      Cell c[max_cells];

      int n;

      void clear();
      void assign(const Partition &);

      int has(int) const;

      int which_cell(int) const;

      void merge_cells(int, int);
      void merge_values(int, int);

      void add(int);

      Partition();
      ~Partition();
      Partition(const Partition &);
      Partition & operator=(const Partition &);
};

///////////////////////////////////////////////////////////////////////////////

inline double WrfData::get_m() const { return ( m ); }
inline double WrfData::get_b() const { return ( b ); }

inline unixtime WrfData::get_init_time() const { return ( (unixtime) (valid_time - lead_time) ); }
inline unixtime WrfData::get_valid_time() const { return ( valid_time ); }
inline int WrfData::get_lead_time() const { return ( lead_time ); }
inline int WrfData::get_accum_time() const { return ( accum_time ); }

inline int WrfData::get_nx() const { return ( nx ); }
inline int WrfData::get_ny() const { return ( ny ); }

inline WrfDataHeader WrfData::get_header() const { return ( h ); }

inline Moments WrfData::moments() const { return ( mom ); }

inline void WrfData::set_valid_time(unixtime t) { valid_time = t; }
inline void WrfData::set_lead_time(int t) { lead_time = t; }
inline void WrfData::set_accum_time(int t) { accum_time = t; }
inline void WrfData::set_m(double mm) { m = mm; }
inline void WrfData::set_b(double bb) { b = bb; }

inline const char * WrfData::get_units() const { return ( h.units ); }
inline const char * WrfData::get_gridname() const { return ( h.gridname ); }

///////////////////////////////////////////////////////////////////////////////

extern WrfData split(const WrfData &, int &);

extern WrfData select(const WrfData &, int);

extern WrfData combine(const WrfData *, int);

extern WrfData combine_split(const WrfData *, int);

extern int closest_dist(const WrfData &, int, const WrfData &, int);

extern void split_field_to_polyline(const WrfData &, int, Node &, bool, int);

extern WrfData reverse_video_polyline(const WrfData &, const Node *);

extern void zero_field_polyline(WrfData &, const Node *);

extern void zero_field(WrfData &);

extern void polyline_to_wrfdata(Polyline &, WrfData &);

extern bool bb_intersect(const BoundingBox &, const BoundingBox &);

extern int wrfdata_intersection(const WrfData &, const WrfData &);

extern void apply_mask(WrfData &, const char *);

extern void apply_mask(WrfData &, WrfData &);

extern void apply_mask(const WrfData &, const WrfData &, const WrfData &,
                       NumArray &, NumArray &);

extern void mask_bad_data(WrfData &, const WrfData &);

extern int mask_double_double(WrfData &, WrfData &, double);

extern int mask_double_double(NumArray &, NumArray &, double);

extern WrfData fractional_coverage(const WrfData &, int, SingleThresh, double);

extern WrfData smooth_field(const WrfData &, InterpMthd mthd, int wdth,
                            double t);

extern void rescale_probability(WrfData &);

extern double interp_min    (const WrfData &, int, int, int, double);
extern double interp_max    (const WrfData &, int, int, int, double);
extern double interp_median (const WrfData &, int, int, int, double);
extern double interp_uw_mean(const WrfData &, int, int, int, double);
extern double interp_dw_mean(const WrfData &, int, int, int, double, double,
                             int, double);
extern double interp_ls_fit (const WrfData &, int, int, int, double, double,
                             double);
extern double interp_bilin  (const WrfData &, double, double);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __WRFDATA_WRFDATA_H__

///////////////////////////////////////////////////////////////////////////////

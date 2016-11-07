// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   shape_data.h
//
//   Description:
//      Contains the declaration of the ShapeData class.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-05-31  Halley Gotway  Adapated from wrfdata.h.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __SHAPE_DATA_H__
#define  __SHAPE_DATA_H__

///////////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////////

#include "moments.h"
#include "vx_util.h"
#include "polyline.h"

///////////////////////////////////////////////////////////////////////////////

//
// Enumerations used in computing the boundary of a ShapeData object
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

class ShapeData {

   private:

      Moments mom;         // moments

      void assign(const ShapeData &);
      int x_left(int)  const;
      int x_right(int) const;

   public:

         //
         //  canonical members
         //

      ShapeData();
      virtual ~ShapeData();
      ShapeData(const ShapeData &);
      ShapeData & operator=(const ShapeData &);

      void clear();

      DataPlane data;        // stores the data

         //
         //  get functions
         //

      Moments moments()  const;

      bool is_zero    (int x, int y) const;
      bool is_nonzero (int x, int y) const;

         //
         //  check if a point or it's neighbors are non-zero
         //

      bool s_is_on(int, int) const;
      bool f_is_on(int, int) const;

      bool is_valid_xy (int x, int y) const;
      bool is_bad_data (int x, int y) const;

         //
         //  object attributes
         //

      void    calc_moments();

      void    centroid(double &xbar, double &ybar)    const;
      double  angle_degrees()                         const;
      double  curvature(double &xcurv, double &ycurv) const;
      double  area()                                  const;
      void    calc_length_width(double &l, double &w) const;
      double  length()                                const;
      double  width()                                 const;
      double  complexity()                            const;

         //
         //  object polylines
         //

      Polyline convex_hull()                             const;
      Polyline single_boundary()                         const;
      Polyline single_boundary(bool, int)                const;
      Polyline single_boundary_offset(double)            const;
      Polyline single_boundary_offset(bool, int, double) const;

         //
         //  object operations
         //

      void conv_filter_circ(int diameter, double bd_thresh);  // diameter must be an odd number

      int  n_objects() const;

      void threshold(double t);
      void threshold(SingleThresh);
      void threshold_area(SingleThresh);
      void threshold_intensity(const ShapeData *, int perc, SingleThresh);

      void filter(SingleThresh);

      void zero_border(int size);
      void zero_border(int size, double value);

      void zero_field();

};

///////////////////////////////////////////////////////////////////////////////


inline Moments ShapeData::moments() const { return(mom); }

inline bool ShapeData::is_valid_xy (int x, int y) const { return ( ! ::is_bad_data(data(x, y)) ); }
inline bool ShapeData::is_bad_data (int x, int y) const { return (   ::is_bad_data(data(x, y)) ); }

inline bool ShapeData::is_zero    (int x, int y) const { return (   is_eq(data(x, y), 0.0) ); }
inline bool ShapeData::is_nonzero (int x, int y) const { return ( ! is_eq(data(x, y), 0.0) ); }


///////////////////////////////////////////////////////////////////////////////


static const int      cell_alloc_inc = 500;

static const int partition_alloc_inc = 500;


///////////////////////////////////////////////////////////////////////////////


class Cell {

   public:

      void init_from_scratch();

      void assign(const Cell &);

      void extend(int);



      int * e;

      int n;

      int n_alloc;


   public:

      Cell();
     ~Cell();
      Cell(const Cell &);
      Cell & operator=(const Cell &);

      void clear();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool has(int) const;

         //
         //  do stuff
         //

      void add(int);


};


///////////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const Cell &);


///////////////////////////////////////////////////////////////////////////////


class Partition {

   public:

      void init_from_scratch();

      void assign(const Partition &);

      void extend(int);


      Cell ** c;

      int n;

      int n_alloc;

   public:

      Partition();
     ~Partition();
      Partition(const Partition &);
      Partition & operator=(const Partition &);

      void clear();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool has(int) const;

      int which_cell(int) const;

         //
         //  do stuff
         //

      void merge_cells(int, int);
      void merge_values(int, int);

      void add(int);

};

///////////////////////////////////////////////////////////////////////////////


extern int ShapeData_intersection(const ShapeData &, const ShapeData &);

extern ShapeData select(const ShapeData &, int);

extern ShapeData split(const ShapeData &, int &);

extern void apply_mask(ShapeData &, ShapeData &);


///////////////////////////////////////////////////////////////////////////////
/*

///////////////////////////////////////////////////////////////////////////////


      void     bounding_box(Box &) ;




      void     composite_boundary(Node &) ;
      void     composite_boundary(bool, int, Node &) ;
      Polyline single_outline_dist(int) ;
      Polyline single_outline_step(int) ;
      Polyline single_outline_angle(int) ;
      int      single_outline_point(double, double, double, Polyline &, bool,
                                          double &, double &) ;


      //
      //  object polylines
      //

extern      void translate(int dx, int dy);



extern      void translate(int dx, int dy);


      //
      // Threshold using a value and a threshold type to create a 0/1 mask field
      //

extern      void rescale_data(double, double);



extern ShapeData combine(const ShapeData *, int);

extern ShapeData combine_split(const ShapeData *, int);

extern int closest_dist(const ShapeData &, int, const ShapeData &, int);

extern void split_field_to_polyline(const ShapeData &, int, Node &, bool, int);

extern ShapeData reverse_video_polyline(const ShapeData &, const Node *);

extern void zero_field_polyline(ShapeData &, const Node *);


extern void polyline_to_ShapeData(Polyline &, ShapeData &);

extern bool bb_intersect(const Box &, const Box &);



extern void mask_bad_data(ShapeData &, const ShapeData &);

extern int mask_double_double(ShapeData &, ShapeData &, double);

extern int mask_double_double(NumArray &, NumArray &, double);


*/
///////////////////////////////////////////////////////////////////////////////

#endif   //  __SHAPE_DATA_H__

///////////////////////////////////////////////////////////////////////////////

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __PIECEWISE_LINEAR_H__
#define  __PIECEWISE_LINEAR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


static const int pwl_alloc_inc = 10;


////////////////////////////////////////////////////////////////////////


class PiecewiseLinear {

   private:

      void init_from_scratch();

      void assign(const PiecewiseLinear &);

   protected:

      void extend(int);

      int N;

      int n_alloc;

      ConcatString Name;

      double * X;
      double * Y;


   public:

      PiecewiseLinear();
     ~PiecewiseLinear();
      PiecewiseLinear(const PiecewiseLinear &);
      PiecewiseLinear & operator=(const PiecewiseLinear &);

      PiecewiseLinear(const char *, int Npoints, const double * XX, const double * YY);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set_name(const ConcatString);

         //
         //  get stuff
         //

      int n_points() const;

      double x(int) const;
      double y(int) const;

      const char * name() const;

         //
         //  do stuff
         //

      void add_point(double, double);

      double operator()(double) const;

};


////////////////////////////////////////////////////////////////////////


inline int PiecewiseLinear::n_points() const { return ( N ); }

inline const char * PiecewiseLinear::name() const { return ( Name.c_str() ); }


////////////////////////////////////////////////////////////////////////


extern int pwl_interpolate(const double * y, const double * x, int n, double x_in, double & y_out);


////////////////////////////////////////////////////////////////////////


#endif   /*  __PIECEWISE_LINEAR_H__  */


////////////////////////////////////////////////////////////////////////



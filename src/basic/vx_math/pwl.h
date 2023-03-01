// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
#include <vector>

////////////////////////////////////////////////////////////////////////


static const int pwl_alloc_inc = 10;


////////////////////////////////////////////////////////////////////////


class PiecewiseLinear {

   private:

      void init_from_scratch();

      void assign(const PiecewiseLinear &);

   protected:

      ConcatString Name;

      std::vector<double> X;
      std::vector<double> Y;

   public:

      PiecewiseLinear();
     ~PiecewiseLinear();
      PiecewiseLinear(const PiecewiseLinear &);
      PiecewiseLinear & operator=(const PiecewiseLinear &);

      PiecewiseLinear(const char *, int Npoints, const double * XX, const double * YY);

      void clear();

      void dump(std::ostream &, int depth = 0) const;

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


inline int PiecewiseLinear::n_points() const { return ( X.size() ); }

inline const char * PiecewiseLinear::name() const { return ( Name.c_str() ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __PIECEWISE_LINEAR_H__  */


////////////////////////////////////////////////////////////////////////



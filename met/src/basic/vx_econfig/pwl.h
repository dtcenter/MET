

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

      int N;

      int n_alloc;

      char * Name;

      double * X;
      double * Y;


   public:

      PiecewiseLinear();
     ~PiecewiseLinear();
      PiecewiseLinear(const PiecewiseLinear &);
      PiecewiseLinear & operator=(const PiecewiseLinear &);



      PiecewiseLinear(const char *, int Npoints, const double * XX, const double * YY);

      void clear();

      void add_point(double, double);

      void extend(int);

      void set_name(const char *);



      double operator()(double) const;

      int n_points() const;

      double x(int) const;
      double y(int) const;

      const char * name() const;


      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int PiecewiseLinear::n_points() const { return ( N ); }

inline const char * PiecewiseLinear::name() const { return ( (const char *) Name ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __PIECEWISE_LINEAR_H__  */


////////////////////////////////////////////////////////////////////////



// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __INTERPOLATOR_H__
#define  __INTERPOLATOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_math.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


class InterpolationValue {

   private:

      void init_from_scratch();

      void assign(const InterpolationValue &);

   public:

      InterpolationValue();
     ~InterpolationValue();
      InterpolationValue(const InterpolationValue &);
      InterpolationValue & operator=(const InterpolationValue &);

      void clear();

      void set_good (double);
      void set_bad  ();

      double value;

      bool ok;   //  value good or bad

};


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const InterpolationValue &);


////////////////////////////////////////////////////////////////////////


class Interpolator {

   protected:

      void init_from_scratch();

      void assign(const Interpolator &);

      int two_to_one(int x, int y) const;


      int Width;   //  must be odd

      int Wm1o2;   //  (Width - 1)/2

      int NgoodNeeded;   //  # of good data values needed 
                         //  to produce good interpolated value

      InterpolationValue * Data;

   public:

      Interpolator();
      virtual ~Interpolator();

      virtual void clear();

         //
         //
         //

      virtual void set_size(int);

      virtual void set_ngood_needed(int);

      virtual void put_good (int x, int y, double value);
      virtual void put_bad  (int x, int y);

      virtual int n_good () const;
      virtual int n_bad  () const;

      int width() const;
      int wm1o2() const;


      virtual InterpolationValue operator()(double x, double y) const = 0;

      virtual Interpolator * copy() const = 0;

      virtual void dump(ostream &, int = 0) const = 0;

};


////////////////////////////////////////////////////////////////////////


inline int Interpolator::width() const { return ( Width ); }

inline int Interpolator::wm1o2() const { return ( Wm1o2 ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __INTERPOLATOR_H__  */


////////////////////////////////////////////////////////////////////////



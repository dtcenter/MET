// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_INTEREST_CALCULATOR_H__
#define  __MTD_INTEREST_CALCULATOR_H__


////////////////////////////////////////////////////////////////////////


#include "3d_att.h"

#include "pwl.h"


////////////////////////////////////////////////////////////////////////


typedef PiecewiseLinear * PWL;

typedef double PairAtt3D::* Argument;


////////////////////////////////////////////////////////////////////////


class InterestCalculator {

   private:

      void init_from_scratch();

      void assign(const InterestCalculator &);

      void extend(int);


      double * W;     //  allocated

      PWL * F;        //  array is allocated, elements are not

      Argument * A;   //  array is allocated, elements are not

      double Scale;

      int Nalloc;

      int Nelements;

   public:

      InterestCalculator();
     ~InterestCalculator();
      InterestCalculator(const InterestCalculator &);
      InterestCalculator & operator=(const InterestCalculator &);

      void clear();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

         //
         //  do stuff
         //

      void add(double _weight, PWL _func, Argument);

      void check();

      double operator()(const PairAtt3D &);


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_INTEREST_CALCULATOR_H__  */


////////////////////////////////////////////////////////////////////////



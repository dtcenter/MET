

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __NUMBER_STACK_H__
#define  __NUMBER_STACK_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "is_number.h"


////////////////////////////////////////////////////////////////////////


class NumberStack {

   protected:

      void init_from_scratch();

      void assign(const NumberStack &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      Number * e;   //  allocated


   public:

      NumberStack();
     ~NumberStack();
      NumberStack(const NumberStack &);
      NumberStack & operator=(const NumberStack &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_alloc_inc(int = 0);   //  0 means default value (100)

         //
         //  get stuff
         //

      int depth() const;

         //
         //  do stuff
         //

      void push(const Number &);

      void push_int    (const int);
      void push_double (const double);

      Number pop();

      void pop2(Number & a, Number & b);

      Number peek() const;

};


////////////////////////////////////////////////////////////////////////


inline int NumberStack::depth() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __NUMBER_STACK_H__  */


////////////////////////////////////////////////////////////////////////



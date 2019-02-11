// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_COLOR_STACK_H__
#define  __VX_COLOR_STACK_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_color.h"


////////////////////////////////////////////////////////////////////////


class ColorStack {

   private:

      void init_from_scratch();

      void assign(const ColorStack &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      Color ** e;


   public:

      ColorStack();
     ~ColorStack();
      ColorStack(const ColorStack &);
      ColorStack & operator=(const ColorStack &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (10)

      int depth() const;

      void push(const Color &);

      Color pop();

      Color peek() const;

};


////////////////////////////////////////////////////////////////////////


inline int ColorStack::depth() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_COLOR_STACK_H__  */


////////////////////////////////////////////////////////////////////////



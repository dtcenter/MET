

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_CALCULATOR_H__
#define  __MET_CALCULATOR_H__


////////////////////////////////////////////////////////////////////////


#include "icode.h"
#include "number_stack.h"
#include "dictionary.h"
#include "builtin.h"


////////////////////////////////////////////////////////////////////////


class Calculator : public NumberStack {

   protected:

      DictionaryStack * dict_stack;   //  not allocated

      void do_builtin_1_arg(const BuiltinInfo & info);
      void do_builtin_2_arg(const BuiltinInfo & info);

   public:

      Calculator();
     ~Calculator();
      Calculator(const Calculator &);
      Calculator & operator=(const Calculator &);

      //
      //  set stuff
      //


      //
      //  get stuff
      //


      //
      //  do stuff
      //

   void run(const IcodeVector &);

   void store(DictionaryEntry &);

      //
      //  operations
      //

   void do_negate   ();

   void do_add      ();
   void do_subtract ();
   void do_multiply ();
   void do_divide   ();

   void do_power    ();
   void do_square   ();

   void do_nint     ();
   void do_sign     ();

   void do_builtin  (int which);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CALCULATOR_H__  */


////////////////////////////////////////////////////////////////////////



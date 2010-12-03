// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_MACHINE_H__
#define  __CONFIG_MACHINE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "icode.h"
#include "symtab.h"


////////////////////////////////////////////////////////////////////////


class Machine {

      friend class CodeGenerator;

   protected:

      void init_from_scratch();

      void assign(const Machine &);

      SymbolTableStack sts;

      CellStack cstack;

      void do_add();
      void do_subtract();
      void do_multiply();
      void do_divide();

      void do_power();
      void do_square();

      void do_negate();

      void do_pwl(const PiecewiseLinear &);

      void do_function_call(const SymbolTableEntry &);

      void do_array(const SymbolTableEntry &);

      void do_builtin(int);

      void do_builtin_1(const BuiltinInfo &);
      void do_builtin_2(const BuiltinInfo &);

      void do_nint();
      void do_sign();

   public:

      Machine();
      virtual ~Machine();
      Machine(const Machine &);
      Machine & operator=(const Machine &);


      void read(const char *);


      void run(const IcodeVector &);

      void run(const SymbolTableEntry &);


      SymbolTableEntry * find(const char *) const;


      IcodeCell pop();

      void push(const IcodeCell &);

      int depth() const;

      void clear();


         //
         //  virtuals
         //


      virtual void st_dump(ostream &, int depth = 0) const;

      virtual void store(const SymbolTableEntry &);



      virtual void dump_cell(ostream &, int, int) const;


      virtual void eval(const char *);


      virtual double func(const char * name, double);
      virtual double func(const char * name, double, double);

      virtual double func(const char * name, double *, int);


      virtual void algebraic_dump(ostream &) const;

      virtual void dump_constants(ostream &);

};


////////////////////////////////////////////////////////////////////////


inline int Machine::depth() const { return ( cstack.depth() ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_MACHINE_H__  */


////////////////////////////////////////////////////////////////////////




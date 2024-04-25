

////////////////////////////////////////////////////////////////////////


#ifndef  __BOOL_CALC_H__
#define  __BOOL_CALC_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <stack>

#include "token.h"


////////////////////////////////////////////////////////////////////////


class BoolCalc {

   private:

      void init_from_scratch();


      std::stack<bool> * s;     //  allocated

      Program * program;   //  allocated

      

   public:

      BoolCalc();
     ~BoolCalc();

      void clear();

      void dump_program(std::ostream &) const;

      // check that all the args from 1 to expectedNargs are found
      bool check_args(int expectedNargs) const;

      int Max_depth;  //   maximum stack depth needed to run the program

      int Max_local;  //   largest local variable number in program



      void set(const char *);   //  algebraic boolean expression

      bool run(const std::vector<bool>);

     // return true if one of the operations is a union (or)
     bool has_union() const; 

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __BOOL_CALC_H__  */


////////////////////////////////////////////////////////////////////////



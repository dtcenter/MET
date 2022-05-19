

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


      stack<bool> * s;     //  allocated

      Program * program;   //  allocated

      

   public:

      BoolCalc();
     ~BoolCalc();

      void clear();

      void dump_program(ostream &) const;



      int Max_depth;  //   maximum stack depth needed to run the program

      int Max_local;  //   largest local variable number in program



      void set(const char *);   //  algebraic boolean expression

      bool run(const vector<bool>);


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __BOOL_CALC_H__  */


////////////////////////////////////////////////////////////////////////



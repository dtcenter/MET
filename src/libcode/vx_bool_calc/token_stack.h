

////////////////////////////////////////////////////////////////////////


#ifndef  __OPERAND_STACK_H__
#define  __OPERAND_STACK_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "token.h"


////////////////////////////////////////////////////////////////////////


class TokenStack {

   private:

      void init_from_scratch();

      void assign(const TokenStack &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      Token * e;


   public:

      TokenStack();
     ~TokenStack();
      TokenStack(const TokenStack &);
      TokenStack & operator=(const TokenStack &);

      void clear();

      void dump(std::ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (50)

      int depth() const;

      bool    empty() const;
      bool nonempty() const;

      void push(const Token &);

      Token pop();

      Token peek() const;

      int top_prec() const;   //  the "in" prec

      // char top_value() const;

      bool top_is_mark() const;

};


////////////////////////////////////////////////////////////////////////


inline int TokenStack::depth() const { return Nelements; }

inline bool TokenStack::empty() const { return ( Nelements == 0 ); }

inline bool TokenStack::nonempty() const { return ( Nelements > 0 ); }


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const TokenStack &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __OPERAND_STACK_H__  */


////////////////////////////////////////////////////////////////////////



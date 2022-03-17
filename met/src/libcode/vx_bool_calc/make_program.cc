

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

#include "vx_util.h"

#include "tokenizer.h"
#include "token_stack.h"
#include "make_program.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static bool balanced(const char *);   //  check balanced parentheses

static void process(const Token &, TokenStack &, Program &);


////////////////////////////////////////////////////////////////////////


void make_program(const char * input, Program & program)

{

Tokenizer tiz;
Token tok;
TokenStack op;   //  operator


if ( ! balanced(input) )  {

   mlog << Error << "\nmake_program() -> "
        << "unbalanced parentheses!\n\n";

   exit ( 1 );

}

program.clear();


tiz.set(input);


   ////////////////

while ( 1 )  {

   tok = tiz.next_token();

   if ( tok.is_eof() )  break;

   process(tok, op, program);

   // op.dump(cout);

}   //  while


   //
   //  there should be no marks left on the stack
   //

while ( op.nonempty() )  {

   tok = op.pop();

   if ( tok.is_mark() )  {

      mlog << Error << "\nmake_program() -> "
           << "extra paranthesis?\n\n";

      exit ( 1 );

   }

   program.push_back(tok);

}


// cout << "\n\n";
// 
// cout << "Input  \"" << input.text()  << "\"\n\n";
// 
// cout << "Result \"" << result.text() << "\"\n\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool balanced(const char * text)

{

int j = 0;
char c;
int count = 0;

while ( (c = text[j++]) != 0 )  {

   if ( c == '(' )  ++count;
   if ( c == ')' )  --count;

}


return ( count == 0 );

}


////////////////////////////////////////////////////////////////////////


void process(const Token & t, TokenStack & op, Program & program)

{

Token tt;

   //////////////////////////////////////


if ( t.is_mark() )  { op.push(t);  return; }


   //////////////////////////////////////


if ( t.is_operand() )  { program.push_back(t);  return; }


   //////////////////////////////////////


if ( t.is_operator() )  {


        if ( op.empty() )                 { op.push(t);  }
   else if ( t.prec() >= op.top_prec() )  { op.push(t);  }
   else if ( t.prec() < op.top_prec() )   {

      while ( op.nonempty() && (t.prec() <= op.top_prec()) )  {
   // while ( op.nonempty() && (t.prec() <  op.top_prec()) )  {

         if ( op.top_is_mark() )  break;

         tt = op.pop();

         program.push_back(tt);

      }   //  while

      op.push(t);

   } else {

      cout << "\n\n  this shouldn't happen!\n\n" << flush;

      exit ( 1 );

   }

   // last_was_operator = true;

   return;

}   //  if t.is_operator


   //////////////////////////////////////


bool mark_found = false;


if ( t.is_unmark() )  {

   while ( op.nonempty() )  { 

      tt = op.pop();  

      if ( tt.is_mark() )  { mark_found = true;  break; }
      else                 program.push_back(tt);

   }

}

if ( ! mark_found )  {

   cout << "\n\n  process() -> mark not found! ... unbalanced parentheses?\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int max_local(const Program & program)   //   largest local variable number in program

{

int j, k, n;

n = 0;

for (j=0; j<(int) program.size(); ++j)  {

    if ( program[j].type != tok_local_var )  continue;

    k = program[j].number_1b;

   if ( k > n )  n = k;

}   //  for j


return ( n );

}


////////////////////////////////////////////////////////////////////////


int max_depth(const Program & program)      //   maximum stack depth needed to run the program

{

int j, n, d;
Token tok;


n = d = 0;


for (j=0; j<(int) program.size(); ++j)  {

   tok = program[j];

   switch ( tok.type )  {

      case tok_local_var:  d += 1;  break;      

      case tok_union:         d -= 1;   break;
      case tok_intersection:  d -= 1;   break;


      default:
         d = 0;
         break;

   }   //  switch

   if ( d > n )  n = d;

}   //  for j





return ( n );

}


////////////////////////////////////////////////////////////////////////





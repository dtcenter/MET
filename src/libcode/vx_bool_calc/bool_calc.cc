

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "bool_calc.h"
#include "make_program.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class BoolCalc
   //


////////////////////////////////////////////////////////////////////////


BoolCalc::BoolCalc()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


BoolCalc::~BoolCalc() {  

clear();

}


////////////////////////////////////////////////////////////////////////


void BoolCalc::init_from_scratch()

{

s = 0;

program = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void BoolCalc::clear()

{

if ( s )  { delete s;  s = 0; }

if ( program )  { delete program;  program = 0; }

Max_depth = Max_local = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void BoolCalc::set(const char * algebraic)

{

program = new Program;

make_program(algebraic, *program);

s = new stack<bool>;

Max_depth = max_depth(*program);

Max_local = max_local(*program);

return;

}


////////////////////////////////////////////////////////////////////////


void BoolCalc::dump_program(ostream & out) const

{

int j;
const Program P = *program;

for (j=0; j<(int) (program->size()); ++j)  {

   out << "\nElement # " << j << " ... \n";

   P[j].dump(out, 1);

}   //  for j


return;

}

////////////////////////////////////////////////////////////////////////


bool BoolCalc::check_args(int expectedNargs) const
{
  Program & P = *program;
  Token tok;
  vector<int> args;
  bool status = true;
  
  for (size_t j=0; j<P.size(); ++j) {
    tok = P[j];
    if (tok.type == tok_local_var) {
      int index = tok.number_1b - 1;
      if (find(args.begin(), args.end(), index) == args.end()) {
	args.push_back(index);
      }
    }
  }
  if ((int)args.size() != expectedNargs) {
    mlog << Error << "\nBoolCalc::check_args() -> "
	 << "expect " << expectedNargs << " args, got " << args.size() << "\n\n";
    status = false;
  }
  for (size_t j=0; j<args.size(); ++j) {
    if (args[j] < 0 || args[j] >= expectedNargs) {
      mlog << Error << "\nBoolCalc::check_args() -> "
	   << "arg out of range " << args[j] << " range is 1 to " << expectedNargs << "\n\n";
      status = false;
    }
  }
  if (!status) {
    dump_program(cerr);
  }
  return status;
}

////////////////////////////////////////////////////////////////////////


bool BoolCalc::run(const vector<bool> arg)

{

int j;
Token tok;
bool tf = false;
bool tf2 = false;
bool result = false;
Program & P = *program;


for (j=0; j<((int) P.size()); ++j)  {

   tok = P[j];

   switch ( tok.type )  {


      case tok_local_var:
         tf = arg[tok.number_1b - 1];   //  don't forget the -1
         s->push(tf);
         break;


      case tok_negation:
         tf = s->top();
         s->pop();
         s->push(!tf);
         break;


      case tok_union:
         tf2 = s->top();
         s->pop();
         tf  = s->top();
         s->pop();
         s->push(tf || tf2);
         break;


      case tok_intersection:
         tf2 = s->top();
         s->pop();
         tf  = s->top();
         s->pop();
         s->push(tf && tf2);
         break;


      default:
         mlog << Error << "\nBoolCalc::run(const vector<bool>) -> "
              << "bad token in program ... \n\n";
         tok.dump(cerr, 1);
         exit ( 1 );
         break;

   }   //  switch

}   //  for j


if ( s->size() != 1 )  {

   mlog << Error << "\nBoolCalc::run(const vector<bool>) -> "
        << "too many elements left on stack! ("
        << (s->size()) << ")\n\n";

   exit ( 1 );

}

result = s->top();

s->pop();

return ( result );

}


////////////////////////////////////////////////////////////////////////





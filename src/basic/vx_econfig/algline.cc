

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "algline.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AlgLine
   //


////////////////////////////////////////////////////////////////////////


AlgLine::AlgLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AlgLine::~AlgLine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AlgLine::AlgLine(const AlgLine & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


AlgLine & AlgLine::operator=(const AlgLine & a)

{

if ( this == &a )  return ( * this );

assign(a);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AlgLine::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::clear()

{

L = R = -1;

prec = 0;

memset(line, 0, sizeof(line));

return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::assign(const AlgLine & a)

{

clear();

L = a.L;
R = a.R;

prec = a.prec;

memcpy(line, a.line, sizeof(line));


return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::start(const char * text)

{

int j, n;

n = strlen(text);

if ( n >= (alg_line_size - 1) )  {

   mlog << Error << "\nAlgLine::start(const char *) -> text too long!\n\n";

   exit ( 1 );

}

L = alg_line_size/2 - n/2;

R = L + (n - 1);

for (j=0; j<n; ++j)  {

   line[L + j] = text[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::prepend(const char * text)

{

if ( (L < 0) && (R < 0) )  {

   start(text);

   return;

}

int j, n;

n = strlen(text);

if ( n >= L )  {

   mlog << Error << "\nAlgLine::prepend(const char *) -> text too long!\n\n";

   exit ( 1 );

}


for (j=0; j<n; ++j)  {

   line[L - n + j] = text[j];

}

L -= n;


return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::append(const char * text)

{

if ( (L < 0) && (R < 0) )  {

   start(text);

   return;

}

int j, n;

n = strlen(text);

if ( (R + 1 + n) >= (alg_line_size - 1) )  {

   mlog << Error << "\nAlgLine::append(const char *) -> text too long!\n\n";

   exit ( 1 );

}


for (j=0; j<n; ++j)  {

   line[R + 1 + j] = text[j];

}

R += n;


return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::prepend(const AlgLine & a)

{

prepend(a.line + a.L);

return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::append(const AlgLine & a)

{

append(a.line + a.L);

return;

}


////////////////////////////////////////////////////////////////////////


void AlgLine::parenthesize()

{

prepend("(");

append (")");


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AlgLineStack
   //


////////////////////////////////////////////////////////////////////////


AlgLineStack::AlgLineStack()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AlgLineStack::~AlgLineStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AlgLineStack::AlgLineStack(const AlgLineStack & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


AlgLineStack & AlgLineStack::operator=(const AlgLineStack & a)

{

if ( this == &a )  return ( * this );


assign(a);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AlgLineStack::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void AlgLineStack::clear()

{

Depth = 0;

int j;

for (j=0; j<max_algline_stack_depth; ++j)  {

   s[j].clear();

}

return;

}


////////////////////////////////////////////////////////////////////////


void AlgLineStack::assign(const AlgLineStack & a)

{

clear();

int j;

Depth = a.Depth;

for (j=0; j<Depth; ++j)  {

   s[j] = a.s[j];

}


return;

}


////////////////////////////////////////////////////////////////////////


void AlgLineStack::push(const AlgLine & a)

{

if ( Depth >= max_algline_stack_depth )  {

   mlog << Error << "\nAlgLineStack::push(const AlgLine &) -> stack full!\n\n";

   exit ( 1 );

}

s[Depth] = a;

++Depth;


return;

}


////////////////////////////////////////////////////////////////////////


const AlgLine & AlgLineStack::pop()

{

if ( Depth <= 0 )  {

   mlog << Error << "\nAlgLineStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

--Depth;

return ( s[Depth] );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const AlgLine & a)

{

s << (a.line + a.L);

return ( s );

}


////////////////////////////////////////////////////////////////////////


Logger & operator<<(Logger & l, const AlgLine & a)

{

l << (a.line + a.L);

return (l);

}


////////////////////////////////////////////////////////////////////////










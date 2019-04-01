

%{


////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>


#include "vx_util.h"
#include "nint.h"

#include "color_parser.h"
#include "color.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        colorwrap();




// extern char *         colortext;

char *         colortext;

FILE *         colorin;

extern const char *   input_filename;

extern ColorTable *   the_table;


   //
   //  definitions that have external linkage
   //


ColorList clist;

int color_file_line_number          = 1;

int color_file_column               = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  static objects
   //


   //
   //  static functions
   //


static Dcolor do_simple_color(const ColorNumber & r, const ColorNumber & g, const ColorNumber & b);

static ColorNumber int_to_num(int);
static ColorNumber int_to_double(double);

static double number_to_double(const ColorNumber &);

static Dcolor blend(const Dcolor & c1, const Dcolor & c2, const ColorNumber & num);

static Dcolor hsv(const ColorNumber &, const ColorNumber &, const ColorNumber &);

static Dcolor cmyk(const ColorNumber &, const ColorNumber &, const ColorNumber &, const ColorNumber &);

// static double min3(double, double, double);

static Dcolor do_gray(const ColorNumber &);

static Dcolor color_lookup(int);

static void range_check(Dcolor &);

static void range_check(double &);

static void assign_color_1(const std::string name, const Dcolor &);
static void assign_color_2(int, const Dcolor &);

static void add_to_table(const ColorNumber &, const Dcolor &);
static void add_2_to_table(const ColorNumber &, const ColorNumber &, const Dcolor &);

static Color dcolor_to_color(const Dcolor &);


////////////////////////////////////////////////////////////////////////


%}



%union {

   char text[129];

   int ival;

   double dval;

   Dcolor cval;

   ColorNumber nval;

}




%token ID COLOR_NAME INTEGER QUOTED_STRING
%token FLOAT
%token BLEND HSV GRAYVALUE CMYK



%type <text>  ID
%type <ival>  INTEGER
%type <ival>  COLOR_NAME
%type <dval>  FLOAT
%type <cval>  color
%type <nval>  number



%%


statement_list : statement
               | statement_list statement
               ;


statement : color_assignment
          | ctable_entry
          ;


ctable_entry : number color              { add_to_table($1, $2); }
             | number number color       { add_2_to_table($1, $2, $3); }
             ;


color_assignment : ID '=' color          { assign_color_1($1, $3); }
                 | COLOR_NAME '=' color  { assign_color_2($1, $3); }
                 ;


color : '{' number ',' number ',' number '}'                     { $$ = do_simple_color($2, $4, $6); }
      | BLEND '(' color  ',' color  ',' number ')'               { $$ = blend($3, $5, $7); }
      | HSV   '(' number ',' number ',' number ')'               { $$ = hsv($3, $5, $7); }
      | CMYK  '(' number ',' number ',' number ',' number ')'    { $$ = cmyk($3, $5, $7, $9); }
      | GRAYVALUE  '(' number ')'                                { $$ = do_gray($3); }
      | COLOR_NAME                                               { $$ = color_lookup($1); }
      ;


number : INTEGER   { $$ = int_to_num($1); }
       | FLOAT     { $$ = int_to_double($1); }
       ;



%%


////////////////////////////////////////////////////////////////////////


Dcolor do_simple_color(const ColorNumber & r, const ColorNumber & g, const ColorNumber & b)

{

Dcolor d;

d.r = number_to_double(r);
d.g = number_to_double(g);
d.b = number_to_double(b);


range_check(d);


return ( d );

}


////////////////////////////////////////////////////////////////////////


ColorNumber int_to_num(int i)

{

ColorNumber n;

n.is_int = 1;

n.i = i;

return ( n );

}


////////////////////////////////////////////////////////////////////////


ColorNumber int_to_double(double x)

{

ColorNumber n;

n.is_int = 0;

n.d = x;

return ( n );

}


////////////////////////////////////////////////////////////////////////


double number_to_double(const ColorNumber & n)

{

double x;

if ( n.is_int )  x = (double) (n.i);
else             x = n.d;


return ( x );

}


////////////////////////////////////////////////////////////////////////


Dcolor blend(const Dcolor & c1, const Dcolor & c2, const ColorNumber & num)

{

double p, q;
Dcolor result;

p = number_to_double(num);

if ( p < 0.0 )  p = 0.0;
if ( p > 1.0 )  p = 1.0;

q = 1.0 - p;

result.r = q*(c1.r) + p*(c2.r);
result.g = q*(c1.g) + p*(c2.g);
result.b = q*(c1.b) + p*(c2.b);

range_check(result);

return ( result );

}


////////////////////////////////////////////////////////////////////////


Dcolor hsv(const ColorNumber & h, const ColorNumber & s, const ColorNumber & v)

{

double H, S, V;
double R, G, B;
Dcolor result;

// cout << "\n\n  In hsv!\n\n" << flush;


H = number_to_double(h);
S = number_to_double(s);
V = number_to_double(v);


H -= floor(H);

// S -= floor(S);
// V -= floor(V);

if ( S < 0.0 )  S = 0.0;
if ( S > 1.0 )  S = 1.0;

if ( V < 0.0 )  V = 0.0;
if ( V > 1.0 )  V = 1.0;


dhsv_to_drgb(H, S, V, R, G, B);


result.r = 255.0*R;
result.g = 255.0*G;
result.b = 255.0*B;


range_check(result);

return ( result );

}


////////////////////////////////////////////////////////////////////////


Dcolor cmyk(const ColorNumber & Cyan, const ColorNumber & Magenta, const ColorNumber & Yellow, const ColorNumber & Black)

{

Dcolor d;
double C, M, Y, K;
double R, G, B;


d.r = d.g = d.b = 0.0;

C = number_to_double(Cyan);
M = number_to_double(Magenta);
Y = number_to_double(Yellow);
K = number_to_double(Black);

if ( C < 0.0 )  C = 0.0;
if ( M < 0.0 )  M = 0.0;
if ( Y < 0.0 )  Y = 0.0;
if ( K < 0.0 )  K = 0.0;

if ( C > 1.0 )  C = 1.0;
if ( M > 1.0 )  M = 1.0;
if ( Y > 1.0 )  Y = 1.0;
if ( K > 1.0 )  K = 1.0;

// R = (1.0 - C)*(1.0 - K);
// G = (1.0 - M)*(1.0 - K);
// B = (1.0 - Y)*(1.0 - K);

C += K;
M += K;
Y += K;

R = 1.0 - C;
G = 1.0 - M;
B = 1.0 - Y;

d.r = 255.0*R;
d.g = 255.0*G;
d.b = 255.0*B;

range_check(d);

return ( d );

}


////////////////////////////////////////////////////////////////////////

/*
double min3(double a, double b, double c)

{

double m, min_ab;


min_ab = ( (a < b) ? a : b );

m = ( (c < min_ab) ? c : min_ab );


return ( m );

}
*/

////////////////////////////////////////////////////////////////////////


Dcolor do_gray(const ColorNumber & n)

{

Dcolor d;
double x;

x = number_to_double(n);

if ( x < 0.0 )  x = 0.0;

if ( x > 255.0 )  x = 255.0;

d.r = d.g = d.b = x;


return ( d );

}


////////////////////////////////////////////////////////////////////////


Dcolor color_lookup(int index)

{

if ( (index < 0) || (index >= clist.n_elements()) )  {

   cerr << "\n\n  color_lookup(int) -> bad index ... " << index << "\n\n";

   exit ( 1 );

}

return ( clist[index].dc() );

}


////////////////////////////////////////////////////////////////////////


void range_check(Dcolor & c)

{

range_check(c.r);
range_check(c.g);
range_check(c.b);

return;

}


////////////////////////////////////////////////////////////////////////


void range_check(double & x)

{

if ( x < 0.0 )  x = 0.0;

if ( x > 255.0 )  x = 255.0;


return;

}


////////////////////////////////////////////////////////////////////////


void assign_color_1(const std::string name, const Dcolor & d)

{

ClistEntry e;


e.set_name(name);

e.set_color(d);

clist.add(e);

return;

}


////////////////////////////////////////////////////////////////////////


void assign_color_2(int index, const Dcolor & d)

{

if ( (index < 0) || (index >= clist.n_elements()) )  {

   cerr << "\n\n  void assign_color_2(int, const Dcolor &) -> bad index ... " << index << "\n\n";

   exit ( 1 );

}

const char * name = clist[index].name();

assign_color_1(name, d);

return;

}


////////////////////////////////////////////////////////////////////////


void add_to_table(const ColorNumber & number, const Dcolor & d)

{

CtableEntry ce;
double value;
Color color;


value = number_to_double(number);

color = dcolor_to_color(d);

ce.set_value(value);

ce.set_color(color);

the_table->add_entry(ce);



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void add_2_to_table(const ColorNumber & n1, const ColorNumber & n2, const Dcolor & d)

{

CtableEntry ce;
double value1, value2;
Color color;


value1 = number_to_double(n1);
value2 = number_to_double(n2);

color = dcolor_to_color(d);

ce.set_values(value1, value2);

ce.set_color(color);

the_table->add_entry(ce);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Color dcolor_to_color(const Dcolor & d)

{

int R, G, B;
Color color;

R = nint(d.r);
G = nint(d.g);
B = nint(d.b);

if ( R < 0 )  R = 0;
if ( G < 0 )  G = 0;
if ( B < 0 )  B = 0;

if ( R > 255 )  R = 255;
if ( G > 255 )  G = 255;
if ( B > 255 )  B = 255;

color.set_rgb((unsigned char) R, (unsigned char) G, (unsigned char) B);

return ( color );

}


////////////////////////////////////////////////////////////////////////



















////////////////////////////////////////////////////////////////////////


   //
   //  standard yacc stuff
   //


////////////////////////////////////////////////////////////////////////


void yyerror(const char * s)

{

int j, j1, j2;
int line_len, text_len;
int c;
char line[512];
ifstream in;


c = (int) (color_file_column - strlen(colortext));

cout << "\n\n"
     << "  syntax error in file \"" << input_filename << "\"\n\n"
     << "      line   = " << color_file_line_number << "\n\n"
     << "      color_file_column = " << c << "\n\n"
     << "      text   = \"" << colortext << "\"\n\n";

in.open(input_filename);

for (j=1; j<color_file_line_number; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




cout << "\n\n"
     << line
     << "\n";

line_len = strlen(line);

text_len = strlen(colortext);

j1 = c;
j2 = c + text_len - 1;


for (j=1; j<=line_len; ++j)  {   //  j starts a one here, not zero

   if ( (j >= j1) && (j <= j2) )  cout.put('^');
   else                           cout.put('_');

}





cout << "\n\n";

cout.flush();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int colorwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////











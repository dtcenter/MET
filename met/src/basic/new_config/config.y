

%{

////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "scanner_stuff.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations that have external linkage
   //


extern int            yylex();

extern void           yyerror(const char *);

extern "C" int        configwrap();


extern char *         configtext;

extern FILE *         configin;


   //
   //  definitions that have external linkage
   //


int               LineNumber            = 1;

int               Column                = 1;

const char *      bison_input_filename  = (const char *) 0;


////////////////////////////////////////////////////////////////////////


%}


%union {

   char text[max_id_length];

   Number nval;

   bool bval;

   Comparison cval;

}


%token IDENTIFIER QUOTED_STRING INTEGER FLOAT BOOLEAN
%token COMPARISON


%type <text> IDENTIFIER QUOTED_STRING
%type <nval> FLOAT
%type <nval> INTEGER BOOLEAN
%type <nval> number
%type <cval> COMPARISON

%left '+' '-'
%left '*' '/'
%left '^'
%nonassoc UNARY_MINUS


%%


assignment_list : assignment
                | assignment_list assignment
                ;


assignment : IDENTIFIER '=' number ';'
           | IDENTIFIER '=' BOOLEAN ';'
           | IDENTIFIER '=' IDENTIFIER ';'
           | IDENTIFIER '=' QUOTED_STRING ';'
           | IDENTIFIER '=' dictionary
           | array_assignment
           | function_assignment
           ;


dictionary : '{' assignment_list '}' { }
           ;


array_assignment : IDENTIFIER '=' '[' string_list    ']' ';'
                 | IDENTIFIER '=' '[' threshold_list ']' ';'
                 ;


string_list : QUOTED_STRING
            | string_list ',' QUOTED_STRING
            ;


threshold_list : threshold
               | threshold_list ',' threshold
               ;


threshold : COMPARISON number   //  COMPARISON is <, >, <=, >=
          ;


number : INTEGER { }
       | FLOAT   { }
       ;


function_assignment : IDENTIFIER '(' IDENTIFIER ')' '=' expression ';'
                    | IDENTIFIER '(' IDENTIFIER ')' '=' piecewise_linear
                    ;


expression : number                                     { }  // { do_number($1); }
           | IDENTIFIER                                 { }  // { do_id($1); }
           | expression '+' expression                  { }  // { do_op('+'); }
           | expression '-' expression                  { }  // { do_op('-'); }
           | expression '*' expression                  { }  // { do_op('*'); }
           | expression '/' expression                  { }  // { do_op('/'); }
           | expression '^' expression                  { }  // { do_op('^'); }
           | '-' expression  %prec UNARY_MINUS          { }  // { do_negate(); }
           | '(' expression ')'                         { }  // { do_paren_exp(); }
           ;


piecewise_linear : '{' point_list '}'   { }
                 ;


point_list : point              { }
           | point_list point   { }
           ;


point : '(' number ',' number ')'   { }   //  { add_point(); }







%%


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


c = (int) (Column - strlen(configtext));

cout << "\n\n"
     << "   yyerror() -> syntax error in file \"" << bison_input_filename << "\"\n\n"
     << "      line   = " << LineNumber << "\n\n"
     << "      column = " << c << "\n\n"
     << "      text   = \"" << configtext << "\"\n\n";

in.open(bison_input_filename);

for (j=1; j<LineNumber; ++j)  {   //  j starts at one here, not zero

   in.getline(line, sizeof(line));

}

in.getline(line, sizeof(line));

in.close();




cout << "\n\n"
     << line
     << "\n";

line_len = strlen(line);

text_len = strlen(configtext);

j1 = c;
j2 = c + text_len - 1;


for (j=1; j<=line_len; ++j)  {   //  j starts at one here, not zero

   if ( (j >= j1) && (j <= j2) )  cout.put('^');
   else                           cout.put('_');

}


cout << "\n\n";

cout.flush();

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int configwrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////



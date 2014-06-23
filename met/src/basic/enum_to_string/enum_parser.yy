

%{


////////////////////////////////////////////////////////////////////////


#define YYDEBUG 1


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "info.h"
#include "code.h"
#include "scope.h"


////////////////////////////////////////////////////////////////////////


extern int yylex();

extern void yyerror(const char *);

extern int bracket_level;

extern bool verbose;

extern bool do_concat_string;

   //
   //  definitions that should have external linkage
   //

EnumInfo einfo;

const char * header_filename = (const char *) 0;

ScopeStack ss;

ScopeStackElement sse;

int saw_class_prefix = 0;

int enum_mode = 0;


////////////////////////////////////////////////////////////////////////


static void check_balance(int, int);

static void set_name(const char *);

static void do_enum_dec(const char *,  int, int);

static void do_enum();
static void do_class_prefix(const char *);


////////////////////////////////////////////////////////////////////////


%}



%union {

   char name[128];

   int ival;

}




%token ENUM L_CURLY R_CURLY EQ INTEGER ID CLASS


%type <name> ID class_prefix
%type <ival> INTEGER L_CURLY R_CURLY



%%


something_list : something
               | something_list something
               ;

something : enum_dec
          | class_prefix       { }
          ;


enum_dec : enum ID  L_CURLY edec_list R_CURLY      { do_enum_dec($2, $3, $5); }
         | enum     L_CURLY edec_list R_CURLY ID   { do_enum_dec($5, $2, $4); }
         ;



enum : ENUM { do_enum(); }
     ;


class_prefix : CLASS ID  { strcpy($$, $2);  do_class_prefix($2); }
             ;



edec_list : edec 
          | edec_list edec
          ;


edec : ID                     { einfo.add_id($1); }
     | ID ','                 { einfo.add_id($1); }
     | ID EQ INTEGER          { einfo.add_id($1); }
     | ID EQ INTEGER ','      { einfo.add_id($1); }
     ;




%%


////////////////////////////////////////////////////////////////////////


void check_balance(int a, int b)

{

if ( a != b )  {

   cerr << "\n\n  check_balance() -> bracket level doesn't match! ... "
        << a << ", " << b << "\n\n";

   exit ( 1 );

}



return;

}


////////////////////////////////////////////////////////////////////////


void set_name(const char * text)

{

int j, n;
const char * c = (const char *) 0;
ostringstream s;


n = ss.n_elements();

for (j=0; j<n; ++j)  {

   c = ss.peek(j).name();

   if ( c )  {

      // cout << "set_name() -> scope = \"" << (ss.peek(j).name()) << "\"\n";

      if ( j != 0 )  s << "::";

      s << c;

   }

}

einfo.set_name(text);

if ( n > 0 )   einfo.set_scope(s.str().c_str());

// cout << "set_name() -> stack = \n" << ss << "\n";

// cout << "set_name() -> finished name  = \"" << (einfo.name())  << "\"\n";
// cout << "set_name() -> finished scope = \"" << (einfo.scope()) << "\"\n";

return;

}


////////////////////////////////////////////////////////////////////////


void do_enum_dec(const char * text, int a, int b)

{


check_balance(a, b);

set_name(text);

einfo.set_header(header_filename);

   //
   //  generate the code
   //

if ( verbose )  cout << "\n";

if ( do_concat_string )  {

   write_cs_header(einfo);

   write_cs_source(einfo);

} else {

   write_header(einfo);

   write_source(einfo);

}

   //
   //  done
   //

einfo.clear();

enum_mode = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void do_class_prefix(const char * text)

{

sse.set_name(text);

sse.set_level(bracket_level);

ss.level_push(sse);

saw_class_prefix = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void do_enum()

{

enum_mode = 1;

return;

}


////////////////////////////////////////////////////////////////////////




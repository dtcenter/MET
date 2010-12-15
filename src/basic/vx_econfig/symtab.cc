

////////////////////////////////////////////////////////////////////////


static const int verbose = 0;


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <cmath>

#include "indent.h"
#include "algline.h"
#include "symtab.h"

#include "file_exists.h"

#include "celltype_to_string.h"
#include "stetype_to_string.h"


////////////////////////////////////////////////////////////////////////


static const char tsort [] = "/usr/bin/tsort";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SymbolTableEntry
   //


////////////////////////////////////////////////////////////////////////


SymbolTableEntry::SymbolTableEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry::~SymbolTableEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry::SymbolTableEntry(const SymbolTableEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry & SymbolTableEntry::operator=(const SymbolTableEntry & e)

{

if ( this == &e )  return ( *this );

assign(e);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::clear()

{


if ( name )        { delete [] name;      name = (char *) 0; }

if ( st )          { delete st;           st = (SymbolTable *) 0; }

if ( v )           { delete v;            v = (IcodeVector *) 0; }

if ( pl )          { delete pl;           pl = (PiecewiseLinear *) 0; }

if ( local_vars )  { delete local_vars;   local_vars = (IdentifierArray *) 0; }

if ( ai )          { delete ai;           ai = (ArrayInfo *) 0; }

if ( text )        { delete [] text;      text = (char *) 0; }



i = 0;

d = 0.0;


type = no_ste_type;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::init_from_scratch()

{


v = (IcodeVector *) 0;

st = (SymbolTable *) 0;

name = (char *) 0;

pl = (PiecewiseLinear *) 0;

local_vars = (IdentifierArray *) 0;

ai = (ArrayInfo *) 0;

text = (char *) 0;



i = 0;

d = 0.0;

type = no_ste_type;


}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::assign(const SymbolTableEntry & e)

{

char junk[128];


clear();


switch ( e.type )  {


   case ste_integer:
      set_name(e.name);
      i = e.i;
      type = e.type;
      break;


   case ste_double:
      set_name(e.name);
      d = e.d;
      type = e.type;
      break;



   case ste_variable:
      assign_variable(e);
      break;


   case ste_function:
      assign_function(e);
      break;


   case ste_pwl:
      assign_pwl(e);
      break;

   case ste_array:
      assign_array(e);
      break;



   default:
      stetype_to_string(e.type, junk);
      cerr << "\n\n  SymbolTableEntry::assign(const SymbolTableEntry &) -> unrecognized type \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::assign_variable(const SymbolTableEntry & e)

{


set_name(e.name);

set_icodevector( *(e.v) );

type = e.type;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::assign_pwl(const SymbolTableEntry & e)

{

set_name(e.name);

set_pwl( *(e.pl) );

type = e.type;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::assign_function(const SymbolTableEntry & e)

{


set_name(e.name);


if ( !local_vars )  local_vars = new IdentifierArray;

*local_vars = *(e.local_vars);


set_icodevector( *(e.v) );

set_symboltable( *(e.st) );

type = e.type;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::assign_array(const SymbolTableEntry & e)

{

set_name(e.name);

type = e.type;

ai = new ArrayInfo;

*ai = *(e.ai);


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_name(const char * Name)

{

if ( name )  { delete [] name;   name = (char *) 0; }

name = new char [1 + strlen(Name)];

strcpy(name, Name);


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_text(const char * Text)

{

if ( text )  { delete [] text;   text = (char *) 0; }

text = new char [1 + strlen(Text)];

strcpy(text, Text);


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_icodevector(const IcodeVector & icv)

{

if ( v )  { delete v;  v = (IcodeVector *) 0; }

v = new IcodeVector;

*v = icv;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_pwl(const PiecewiseLinear & P)

{

if ( pl )  { delete pl;   pl = (PiecewiseLinear *) 0; }

pl = new PiecewiseLinear;

*pl = P;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_symboltable(const SymbolTable & ST)

{

if ( st )  { delete st;  st = (SymbolTable *) 0; }

st = new SymbolTable;

*st = ST;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_integer(const char * Name, int value)

{

clear();

set_name(Name);

type = ste_integer;

i = value;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_double(const char * Name, double value)

{

clear();


set_name(Name);

type = ste_double;

d = value;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_variable(const char * Name, const IcodeVector & I)

{

clear();


set_name(Name);

set_icodevector(I);


type = ste_variable;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_pwl(const char * Name, const PiecewiseLinear & P)

{

clear();

set_name(Name);

set_pwl(P);

type = ste_pwl;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::set_function(const char * FunctionName, const IdentifierArray & Locals, const IcodeVector & I)

{

int j;
SymbolTableEntry e;


clear();


st = new SymbolTable;


for (j=0; j<(Locals.n_elements()); ++j)  {

   e.set_integer(Locals[j].name, 0);

   st->store(e);

   e.clear();

}



set_name(FunctionName);

set_icodevector (I);


if ( !local_vars )  local_vars = new IdentifierArray;

*local_vars = Locals;


type = ste_function;


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableEntry::dump(ostream & out, int indent_depth) const

{

char junk[256];
Indent prefix;


prefix.depth = indent_depth;


stetype_to_string(type, junk);



switch ( type )  {

   case ste_integer:
      out << prefix << "Name  = " << name << "\n";
      out << prefix << "Type  = " << junk << "\n";
      out << prefix << "Value = " << i << "\n";
      break;


   case ste_double:
      out << prefix << "Name  = " << name << "\n";
      out << prefix << "Type  = " << junk << "\n";
      out << prefix << "Value = " << d << "\n";
      break;


   case ste_variable:
      out << prefix << "Name = " << name << "\n";
      out << prefix << "Type = " << junk << "\n";
      out << prefix << "Program for variable " << name << " ... \n";
      v->dump(out, indent_depth + 1);
      break;


   case ste_pwl:
      out << prefix << "Name   = " << name << "\n";
      out << prefix << "Type   = " << junk << "\n";
      out << prefix << "Points = " << (pl->n_points()) << "\n";
      pl->dump(out, indent_depth + 1);
      break;


   case ste_function:

      out << prefix << "Name = " << name << "\n";

      out << prefix << "Type = " << junk << "\n";

      out << prefix << "Local Variable Names = ";
      local_vars->dump(out, indent_depth + 1);
      out << "\n";

      out << prefix << "Program for function " << name << " ... \n";
      v->dump(out, indent_depth + 1);

      out << prefix << "SymbolTable for function " << name << " ... \n";
      st->dump(out, indent_depth + 1);

      break;


   case ste_array:
      out << prefix << "Name = " << name << "\n";
      out << prefix << "Type = " << junk << "\n";
      out << prefix << "ArrayInfo ... \n";
      ai->dump(out, indent_depth + 1);
      break;


   default:
      cerr << "\n\n  SymbolTableEntry::dump() const -> unrecognized type \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;


}   //  switch



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SymbolTable
   //


////////////////////////////////////////////////////////////////////////


SymbolTable::SymbolTable()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SymbolTable::~SymbolTable()

{

if ( Entry )  { 

   int j;

   for (j=0; j<Nalloc; ++j)  {

      delete Entry[j];  Entry[j] = (SymbolTableEntry *) 0;

   }

   delete [] Entry;  Entry = (SymbolTableEntry **) 0;

}

Nentries = Nalloc = 0;

}


////////////////////////////////////////////////////////////////////////


SymbolTable::SymbolTable(const SymbolTable & s)

{

init_from_scratch();

assign(s);

}


////////////////////////////////////////////////////////////////////////


SymbolTable & SymbolTable::operator=(const SymbolTable & s)

{

if ( this == &s )  return ( *this );

assign(s);

return ( *this );

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::init_from_scratch()

{

Entry = (SymbolTableEntry **) 0;

Nentries = Nalloc = 0;

extend(1);

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::assign(const SymbolTable & s)

{

clear();

extend(s.Nentries);

int j;

for (j=0; j<(s.Nentries); ++j)  {

   store( *(s.Entry[j]) );

}


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::clear()

{

if ( Nalloc == 0 )  return;

int j;

for (j=0; j<Nentries; ++j)  {

   delete Entry[j];   Entry[j] = (SymbolTableEntry *) 0;

}

delete [] Entry;   Entry = (SymbolTableEntry **) 0;

Nalloc = Nentries = 0;

extend(1);

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::extend(int n)

{

if ( n <= Nalloc )  return;

n = (n + symtab_alloc_jump - 1)/symtab_alloc_jump;

n *= symtab_alloc_jump;

SymbolTableEntry **u = (SymbolTableEntry **) 0;

u = new SymbolTableEntry * [n];

if ( !u )  {

   cerr << "\n\n  SymbolTable::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<Nentries; ++j)  u[j] = Entry[j];

for (j=Nentries; j<n; ++j)  u[j] = (SymbolTableEntry *) 0;

delete [] Entry;  Entry = (SymbolTableEntry **) 0;

Entry = u;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry * SymbolTable::find(const char * var) const

{

int j;

if ( verbose )  cout << "\n\n  SymbolTable::find(const char *) -> looking for \"" << var << "\"\n\n" << flush;

for (j=0; j<Nentries; ++j)  {

   if ( strcmp(Entry[j]->name, var) == 0 )  {

      if ( verbose )  cout << "Found\n\n" << flush;

      return ( Entry[j] );

   }

}

if ( verbose )  cout << "Not found\n\n" << flush;

return ( (SymbolTableEntry *) 0 );

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::store(const SymbolTableEntry & e)

{

   //
   //  is this just a duplicate of an existing entry?
   //     if it is, just return
   //

int j;

for (j=0; j<Nentries; ++j)  {

   if ( Entry[j] == &e )  return;

}


   //
   //  see if an entry by this name is already here.
   //     if it is, remove it
   //


SymbolTableEntry * u = (SymbolTableEntry *) 0;

u = find(e.name);

if ( u )  { remove(u);   u = (SymbolTableEntry *) 0; }


   //
   //  make a new entry
   //


extend(Nentries + 1);

Entry[Nentries] = new SymbolTableEntry;

if ( !(Entry[Nentries]) )  {

   cerr << "\n\n  SymbolTable::store(const RpnObject *, const char *) -> memory allocation error\n\n";

   exit ( 1 );

}

*(Entry[Nentries]) = e;

++Nentries;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::remove(const SymbolTableEntry * u)

{

int j, k;
int found;


found = 0;

for (j=0; j<Nentries; ++j)  {

   if ( Entry[j] == u )  { found = 1;  break; }

}

if ( !found )  {

      //
      //  should we report an error here?  nah, just return
      //

   return;

}

   //
   //  delete the entry
   //

delete Entry[j];   Entry[j] = (SymbolTableEntry *) 0;

   //
   //  slide the entry values past j down one notch
   //

for (k=j; k<(Nentries - 1); ++k)  {

   Entry[k] = Entry[k + 1];

}



   //
   //  done
   //

Entry[Nentries] = (SymbolTableEntry *) 0;

--Nentries;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix;


prefix.depth = indent_depth;


out << prefix << "SymbolTable output .... \n";

     if ( Nentries == 0 )  out << prefix << "There are no entries\n";
else if ( Nentries == 1 )  out << prefix << "There is 1 entry\n";
else                       out << prefix << "There are " << Nentries << " entries\n";



for (j=0; j<Nentries; ++j)  {

   out << prefix << "Entry number " << j << "\n";

   Entry[j]->dump(out, indent_depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::store_as_int(const char * Name, int i)

{

SymbolTableEntry e;

e.set_integer(Name, i);

store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::store_as_double(const char * Name, double d)

{

SymbolTableEntry e;

e.set_double(Name, d);

store(e);


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump(ostream & out) const

{

int j, k, m;
int status;
char junk[128];
const char * temp_filename_1 = "./temp1";
const char * temp_filename_2 = "./temp2";
const IcodeVector * v = (const IcodeVector *) 0;
ofstream poset;
ifstream in;
char command [512];;
char line[128];
SymbolTableEntry * e = (SymbolTableEntry *) 0;


   //
   //  dump poset file
   //

poset.open(temp_filename_1);

if ( !poset )  {

   cerr << "\n\n  void SymbolTable::algebraic_dump(ostream &) const -> can't open poset file\n\n";

   exit ( 1 );

}


for (j=0; j<Nentries; ++j)  {

   switch ( Entry[j]->type )  {

      case ste_integer:
         break;

      case ste_double:
         break;

      case ste_pwl:
         poset << (Entry[j]->name) << "   " << (Entry[j]->name) << "\n";
         break;





      case ste_variable:
         v = Entry[j]->v;
         for (k=0; k<(v->length()); ++k)  {
            if ( v->cell(k).type == identifier )  {
               poset << (v->cell(k).name) << "   " << (Entry[j]->name) << "\n";
            }
         }
         poset << (Entry[j]->name) << "   " << (Entry[j]->name) << "\n";
         v = (const IcodeVector *) 0;
         break;




      case ste_function:
         v = Entry[j]->v;

         for (k=0; k<(v->length()); ++k)  {
            if ( (v->cell(k).type == identifier) && (Entry[j]->local_vars->has(v->cell(k).name) == 0) )  {
               poset << (v->cell(k).name) << "   " << (Entry[j]->name) << "\n";
            }
         }

         poset << (Entry[j]->name) << "   " << (Entry[j]->name) << "\n";
         v = (const IcodeVector *) 0;
         break;


      case ste_array:
         for (m=0; m<(Entry[j]->ai->n_alloc()); ++m)  {
            v = Entry[j]->ai->get(m);
            for (k=0; k<(v->length()); ++k)  {
               if ( v->cell(k).type == identifier )  {
                  poset << (v->cell(k).name) << "   " << (Entry[j]->name) << "\n";
               }
            }
         }
         poset << (Entry[j]->name) << "   " << (Entry[j]->name) << "\n";
         v = (const IcodeVector *) 0;
         break;


      default:
         stetype_to_string(Entry[j]->type, junk);
         cerr << "\n\n  SymbolTable::algebraic_dump(ostream &) const -> unrecognized symbol table entry type ... \""
              << junk << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch


}   //  for j

poset.close();

sprintf(command, "%s %s > %s", tsort, temp_filename_1, temp_filename_2);


if ( file_exists(temp_filename_2) )  {

   if ( unlink(temp_filename_2) < 0 )  {

      cerr << "\n\n  SymbolTable::algebraic_dump(ostream &) const -> unable to remove temp file \""
           << temp_filename_2 << "\" ... "
           // << sys_errlist[errno]
           << strerror(errno)
           << "\n\n";

      exit ( 1 );

   }

}


// cout << "Executing command \"" << command << "\"\n\n";

status = system(command);

if ( status )  {

   cerr << "\n\n  SymbolTable::algebraic_dump(ostream &) const -> command \""
        << command << "\" failed!\n\n";

   exit ( 1 );

}

(void) unlink(temp_filename_1);

   //
   //  print the table entries in order
   //

in.open(temp_filename_2);

if ( !in )  {

   cerr << "\n\n  SymbolTable::algebraic_dump(ostream &) const -> unable to open temp file \""
        << temp_filename_2 << "\"\n\n";

   exit ( 1 );

}

out << "\n\n";

while ( in.getline(line, sizeof(line)) )  {

   e = find(line);

   if ( !e )  {

      cerr << "\n\n  SymbolTable::algebraic_dump(ostream &) const -> unable find symbol table entry for \""
           << line << "\"\n\n";

      exit ( 1 );

   }

   algebraic_dump_entry(out, e);

   out << "\n\n";

}





in.close();

   //
   //  done
   //

(void) unlink(temp_filename_2);

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_entry(ostream & out, const SymbolTableEntry * e) const

{

char junk[128];


switch ( e->type )  {

   case ste_integer:
      break;

   case ste_double:
      break;

   case ste_variable:
      algebraic_dump_variable(out, e);
      break;

   case ste_pwl:
      algebraic_dump_pwl(out, e);
      break;

   case ste_function:
      algebraic_dump_function(out, e);
      break;

   case ste_array:
      algebraic_dump_array(out, e);
      break;

   default:
      stetype_to_string(e->type, junk);
      cerr << "\n\n  SymbolTable::algebraic_dump_entry(ostream &) const -> unrecognized type \""
           << junk << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_variable(ostream & out, const SymbolTableEntry * e) const

{

out << (e->name) << " = ";

algebraic_dump_icv(out, *(e->v), (const IdentifierArray *) 0);

out << ";\n" << flush;



return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_pwl(ostream & out, const SymbolTableEntry * e) const

{

int j;
char junk[128];



out << (e->name) << " = { ";

out.setf(ios::fixed);

out.precision(4);

for (j=0; j<(e->pl->n_points()); ++j)  {

   out << "( ";

   sprintf(junk, "%.10f", e->pl->x(j));

   strip_trailing_zeroes(junk);

   out << junk << ", ";

   sprintf(junk, "%.10f", e->pl->y(j));

   strip_trailing_zeroes(junk);

   out << junk << " ) ";

}


out << "};\n" << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_function(ostream & out, const SymbolTableEntry * e) const

{

int j, n;
IdentifierArray & a = *(e->local_vars);


n = a.n_elements();


out << (e->name) << "(";


for (j=0; j<(a.n_elements()); ++j)  {

   out << a[j].name;

   if ( (n > 1) && (j < (n - 1)) )  out << ", ";

}


out << ") = ";


algebraic_dump_icv(out, *(e->v), e->local_vars);

out << ";\n" << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_array(ostream & out, const SymbolTableEntry * e) const

{

int j, n;


out << (e->name);

for (j=0; j<(e->ai->dim()); ++j)  {

   out << "[";

   out << (e->ai->size(j));

   out << "]";

}


out << " = [ ";


n = e->ai->n_alloc();


for (j=0; j<n; ++j)  {

   algebraic_dump_icv(out, *(e->ai->get(j)), (const IdentifierArray *) 0 );

   if ( (n > 1) && (j < (n - 1)) )   out << ", ";

}

out << " ];";



out.flush();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTable::algebraic_dump_icv(ostream & out, const IcodeVector & v, const IdentifierArray * local_vars) const

{

int j, n;
int pos;
AlgLine a, b;
AlgLineStack s;
char junk[256];
const IcodeCell * cell = (const IcodeCell *) 0;
SymbolTableEntry * e = (SymbolTableEntry *) 0;


pos = 0;

while ( pos < v.length() )  {

   cell = &( v.cell(pos) );

   switch ( cell->type )  {

      case integer:
         sprintf(junk, "%d", cell->val);
         a.append(junk);
         a.prec = 1000;
         s.push(a);
         break;

      case floating_point:
         sprintf(junk, "%.10f", cell->d);
         strip_trailing_zeroes(junk);
         a.append(junk);
         a.prec = 1000;
         s.push(a);
         break;

      case builtin_func:
         n = binfo[cell->val].n_vars;
         for (j=0; j<n; ++j)  {
            b = s.pop();
            if ( (n > 1) && (j > 0) )  b.append(", ");
            a.prepend(b);
         }
         a.parenthesize();
         a.prepend(binfo[cell->val].name);
         a.prec = 1000;
         s.push(a);
         break;



      case identifier:

            //
            //  is it a local variable?
            //

         if ( local_vars && local_vars->has(cell->name) )  {
            a.append(cell->name);
            a.prec = 1000;
            s.push(a);
         } else {

               //
               //  if not, look it up
               //

            e = find(cell->name);

            if ( !e )  {
               cerr << "\n\n  SymbolTable::algebraic_dump_icv() -> symbol \"" << (cell->name) << "\" not found!\n\n";
               exit ( 1 );
            }

            switch ( e->type )  {

               case ste_array:
                  for (j=0; j<(e->ai->dim()); ++j)  {
                     b = s.pop();
                     b.prepend("[");
                     b.append("]");
                     a.prepend(b);
                  }
                  a.prepend(e->name);
                  a.prec = 1000;
                  s.push(a);
                  break;

               case ste_pwl:
                  a = s.pop();
                  a.parenthesize();
                  a.prepend(cell->name);
                  a.prec = 1000;
                  s.push(a);
                  break;



               case ste_function:
                  n = e->local_vars->n_elements();
                  for (j=0; j<n; ++j)  {
                     b = s.pop();
                     if ( (n > 1) && (j > 0) )  b.append(", ");
                     a.prepend(b);
                  }
                  a.parenthesize();
                  a.prepend(cell->name);
                  a.prec = 1000;
                  s.push(a);
                  break;


               case ste_variable:
                  a.append(e->name);
                  a.prec = 1000;
                  s.push(a);
                  break;



               default:
                  stetype_to_string(e->type, junk);
                  cerr << "\n\n  SymbolTable::algebraic_dump_icv() -> symbol \"" << (e->name) << "\" has bad type"
                       << " ... \"" << junk << "\""
                       << "\n\n";
                  exit ( 1 );
                  break;

            }   //  switch









/*

            if ( (e->local_vars) && (e->local_vars->has(cell->name)) )  {
               a.append(cell->name);
               a.prec = 1000;
               if ( e->type == ste_variable )  {
                  a.append(cell->name);
                  a.prec = 1000;
               } else if ( e->type == ste_function )  {
                  n = e->local_vars->n_elements();
                  for (j=0; j<n; ++j)  {
                     b = s.pop();
                     if ( (n > 1) && (j > 0) )  b.append(", ");
                     a.prepend(b);
                  }
                  a.parenthesize();
                  a.prepend(cell->name);
                  a.prec = 1000;
               } else if ( e->type == ste_pwl )  {
                  a = s.pop();
                  a.parenthesize();
                  a.prepend(cell->name);
                  a.prec = 1000;
               } else {
                  celltype_to_string(cell->type, junk);
                  cerr << "\n\n  SymbolTable::algebraic_dump_icv() -> symbol \"" << (cell->name) << "\" has bad type"
                       << " ... \"" << junk << "\""
                       << "\n\n";
                  exit ( 1 );
               }
            }
            if ( e->ai )  {
               for (j=0; j<(e->ai->dim()); ++j)  {
                  b = s.pop();
                  b.prepend("[");
                  b.append("]");
                  a.prepend(b);
               }
               a.prepend(e->name);
               s.push(a);
            }
*/

         }   //  else



         break;

            //
            //  prec = 1
            //

      case op_add:
         b = s.pop();
         a = s.pop();
         a.append(" + ");
         a.append(b);
         a.prec = 1;
         s.push(a);
         break;

      case op_subtract:
         b = s.pop();
         a = s.pop();
         a.append(" - ");
         a.append(b);
         a.prec = 1;
         s.push(a);
         break;

            //
            //   prec = 2
            //

      case op_multiply:
         b = s.pop();
         if ( b.prec < 2 )  b.parenthesize();
         a = s.pop();
         if ( a.prec < 2 )  a.parenthesize();
         a.append("*");
         a.append(b);
         a.prec = 2;
         s.push(a);
         break;

      case op_divide:
         b = s.pop();
         if ( b.prec < 2 )  b.parenthesize();
         a = s.pop();
         if ( a.prec < 2 )  a.parenthesize();
         a.append("/");
         a.append(b);
         a.prec = 2;
         s.push(a);
         break;

            //
            //  prec = 3
            //

      case op_power:
         b = s.pop();
         if ( b.prec < 3 )  b.parenthesize();
         a = s.pop();
         if ( a.prec < 3 )  a.parenthesize();
         a.append("^");
         a.append(b);
         a.prec = 3;
         s.push(a);
         break;

      case op_square:
         a = s.pop();
         if ( a.prec < 3 )  a.parenthesize();
         a.append("^2");
         a.prec = 3;
         s.push(a);
         break;


            //
            //  prec = 4;
            //

      case op_negate:
         a = s.pop();
         if ( a.prec < 4 )  a.parenthesize();
         a.prepend("-");
         a.prec = 4;
         s.push(a);
         break;

         break;



      default:
         celltype_to_string(cell->type, junk);
         cerr << "\n\n  SymbolTable::algebraic_dump_icv(ostream &) const -> unrecognized icode cell type ... \""
              << junk << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

   ++pos;

   a.clear();
   b.clear();

   e = (SymbolTableEntry *) 0;

}   //  while


if ( s.depth() != 1 )  {

   cerr << "\n\n  SymbolTable::algebraic_dump_icv(ostream &, const IcodeVector &) const -> bad stack depth ... "
        << (s.depth()) << "\n\n";

   while ( s.depth() > 0 )  {

      a = s.pop();

      cerr << "---> " << a << "\n\n";

   }

   exit ( 1 );

}

a = s.pop();

   //
   //  done
   //

out << a;

return;

}


////////////////////////////////////////////////////////////////////////


const SymbolTableEntry * const SymbolTable::entry(int k) const

{

if ( (k < 0) || (k >= Nentries) )  {

   cerr << "\n\n  const SymbolTableEntry & SymbolTable::entry(int) const -> range check error\n\n";

   exit ( 1 );

}


return ( (Entry[k]) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SymbolTableStack
   //


////////////////////////////////////////////////////////////////////////


SymbolTableStack::SymbolTableStack()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SymbolTableStack::~SymbolTableStack()

{

clear();

if ( Table )   { delete [] Table;  Table = (SymbolTable **) 0; }

}


////////////////////////////////////////////////////////////////////////


SymbolTableStack::SymbolTableStack(const SymbolTableStack & sts)

{

init_from_scratch();

assign(sts);

}


////////////////////////////////////////////////////////////////////////


SymbolTableStack & SymbolTableStack::operator=(const SymbolTableStack & sts)

{

if ( this == &sts )  return ( * this );

assign(sts);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::init_from_scratch()

{

Table = (SymbolTable **) 0;

Ntables = Nalloc = 0;

extend(1);

int j;

for (j=0; j<Nalloc; ++j)  {

   Table[j] = (SymbolTable *) 0;

}

Table[0] = new SymbolTable;

if ( !(Table[0]) )  {

   cerr << "\n\n  void SymbolTableStack::init_from_scratch() -> memory allocation error!\n\n";

   exit ( 1 );

}

Ntables = 1;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::clear()

{

int j;

for (j=1; j<Nalloc; ++j)  {   //  j starts at ONE here, NOT zero

   Table[j] = (SymbolTable *) 0;

}

Table[0] ->clear();

Ntables = 1;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::assign(const SymbolTableStack & sts)

{

clear();

extend(sts.Ntables);

int j;

*(Table[0]) = *(sts.Table[0]);

for (j=1; j<(sts.Ntables); ++j)  {   //  j starts at ONE here, NOT zero

   Table[j] = sts.Table[j];

}

Ntables = sts.Ntables;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::extend(int n)

{

if ( n <= Nalloc )  return;

n = (n + symtab_stack_alloc_jump - 1)/symtab_alloc_jump;

n *= symtab_alloc_jump;

SymbolTable **u = (SymbolTable **) 0;

u = new SymbolTable * [n];

if ( !u )  {

   cerr << "\n\n  SymbolTableStack::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<Ntables; ++j)  u[j] = Table[j];

for (j=Ntables; j<n; ++j)  u[j] = (SymbolTable *) 0;

delete [] Table;  Table = (SymbolTable **) 0;

Table = u;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::push(SymbolTable *s)

{

extend(Ntables + 1);

Table[Ntables] = s;

++Ntables;

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::pop()

{

   //
   //  DON'T deallocate the symbol table!!
   //

if ( Ntables <= 1 )  {

   cerr << "\n\n  SymbolTableStack::pop() -> can't pop off the last table!\n\n";

   exit ( 1 );

}

--Ntables;

Table[Ntables] = (SymbolTable *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


SymbolTableEntry * SymbolTableStack::find(const char *var) const

{

int j;
SymbolTableEntry *u = (SymbolTableEntry *) 0;

for (j=(Ntables - 1); j>=0; --j)  {

   if ( Table[j] )  u = Table[j]->find(var);

   if ( u )  return ( u );

}

return ( (SymbolTableEntry *) 0 );

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::store(const SymbolTableEntry & e)

{

Table[Ntables - 1]->store(e);

return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::store_as_int(const char * Name, int i)

{

Table[Ntables - 1]->store_as_int(Name, i);

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::store_as_double(const char * Name, double d)

{

Table[Ntables - 1]->store_as_double(Name, d);

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::dump(ostream & out, int indent_depth) const

{

int j;
Indent prefix;


prefix.depth = indent_depth;


out << "\n\n";


if ( Ntables == 0 )  {

   out << prefix << "SymbolTableStack is empty\n";

   return;

} else if ( Ntables == 1 )  {

   out << prefix << "There is one SymbolTable on the SymbolTableStack.\n";

} else  {

   out << prefix << "There are " << Ntables << " SymbolTables on the SymbolTableStack.\n";

}




for (j=0; j<Ntables; ++j)  {

   out << prefix << "SymbolTable # " << j << "\n";

   Table[j]->dump(out, indent_depth + 1);

}




   //
   //  done
   //

out.flush();


return;

}


////////////////////////////////////////////////////////////////////////


void SymbolTableStack::algebraic_dump(ostream & out) const

{

int j;



for (j=0; j<Ntables; ++j)  {

   Table[j]->algebraic_dump(out);

}




return;

}


////////////////////////////////////////////////////////////////////////


const SymbolTable & SymbolTableStack::table(int k) const

{

if ( (k < 0) || (k >= Ntables) )  {

   cerr << "\n\n  const SymbolTable & SymbolTableStack::table(int) const -> range check error\n\n";

   exit ( 1 );

}



return ( *(Table[k]) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void strip_trailing_zeroes(char * s)

{

int j;
int n = strlen(s);

for (j=(n - 1); j>=0; --j)  {

   if ( s[j] != '0' )  break;

   s[j] = (char) 0;

}

n = strlen(s);

if ( s[n - 1] == '.' )  {

   s[n] = '0';

}



return;

}


////////////////////////////////////////////////////////////////////////


void translate_string(ostream & out, const char * text)

{

int j, n;

n = strlen(text);


for (j=0; j<n; ++j)  {

   switch ( text[j] )  {

      case '\n':  out << "\\n";   break;
      case '\t':  out << "\\t";   break;
      case '\b':  out << "\\b";   break;
      case '\"':  out << "\\\"";  break;
      case '\\':  out << "\\\\";  break;

      default:
      out.put(text[j]);
      break;

   }

}   //  for j


return;

}


////////////////////////////////////////////////////////////////////////






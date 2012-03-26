

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "indent.h"
#include "bool_to_string.h"
#include "empty_string.h"

#include "dictionary.h"
#include "configobjecttype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DictionaryEntry
   //


////////////////////////////////////////////////////////////////////////


DictionaryEntry::DictionaryEntry()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


DictionaryEntry::~DictionaryEntry()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DictionaryEntry::DictionaryEntry(const DictionaryEntry & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


DictionaryEntry & DictionaryEntry::operator=(const DictionaryEntry & e)

{

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::init_from_scratch()

{

Text = (ConcatString *) 0;

Dict = (Dictionary *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::clear()

{

Type = no_config_object_type;

Ival = 0;
Dval = 0.0;
Bval = false;

if ( Text )  { delete Text;  Text = (ConcatString *) 0; }

if ( Dict )  { delete Dict;  Dict = (Dictionary *) 0; }

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::assign(const DictionaryEntry & e)

{

clear();

switch ( e.Type )  {

   case IntegerType:
      set_int(e.Name, e.Ival);
      break;

   case FloatType:
      set_double(e.Name, e.Dval);
      break;

   case BooleanType:
      set_boolean(e.Name, e.Bval);
      break;

   case StringType:
      set_string(e.Name, *(e.Text));
      break;

   case DictionaryType:
      set_dict(e.Name, *(e.Dict));
      break;

   case ArrayType:
      set_array(e.Name, *(e.Dict));
      break;

   default:
      cerr << "\n\n  DictionaryEntry::assign(const DictionaryEntry &) -> bad object type ... \""
           << configobjecttype_to_string(e.Type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Name  = " << Name.contents() << "\n";

out << prefix << "Type  = " << configobjecttype_to_string(Type) << "\n";

switch ( Type )  {

   case IntegerType:
      out << prefix << "Value = " << Ival << "\n";
      break;

   case FloatType:
      out << prefix << "Value = " << Dval << "\n";
      break;

   case BooleanType:
      out << prefix << "Value = " << bool_to_string(Bval) << "\n";
      break;

   case StringType:
      out << prefix << "Value = \"" << (*Text) << "\"\n";
      break;

   case DictionaryType:   //  fall through
   case ArrayType:        //  fall through
      out << prefix << "Value = \n";
      Dict->dump(out, depth + 1);
      break;

   default:
      cerr << "\n\n  DictionaryEntry::dump(const DictionaryEntry &) -> bad object type ... \""
           << configobjecttype_to_string(Type) << "\"\n\n";
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


void DictionaryEntry::set_name (const char * _name)

{

if ( empty(_name) )  {

   cerr << "\n\n  DictionaryEntry::set_name (const char *) -> empty string!\n\n";

   exit ( 1 );

}

Name = _name;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_int (const char * _name, int k)

{

clear();

Type = IntegerType;

set_name(_name);

Ival = k;


return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_double (const char * _name, double x)

{

clear();

Type = FloatType;

set_name(_name);

Dval = x;


return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_string (const char * _name, const char * _text)

{

clear();

Type = StringType;

set_name(_name);

if ( empty(_text) )  {

   cerr << "\n\n  DictionaryEntry::set_string() -> empty string!\n\n";

   exit ( 1 );

}

Text = new ConcatString;

*Text = _text;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_boolean(const char * _name, bool tf)

{

clear();

Type = BooleanType;

set_name(_name);

Bval = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_dict(const char * _name, const Dictionary & d)

{

clear();

Type = DictionaryType;

set_name(_name);

Dict = new Dictionary;

*Dict = d;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_array(const char * _name, const Dictionary & d)

{

clear();

Type = ArrayType;

set_name(_name);

Dict = new Dictionary;

*Dict = d;

return;

}


////////////////////////////////////////////////////////////////////////


int DictionaryEntry::i_value() const

{

if ( Type != IntegerType )  {

   cerr << "\n\n  DictionaryEntry::i_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Ival );

}


////////////////////////////////////////////////////////////////////////


double DictionaryEntry::d_value() const

{

if ( Type != FloatType )  {

   cerr << "\n\n  DictionaryEntry::d_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Dval );

}


////////////////////////////////////////////////////////////////////////


bool DictionaryEntry::b_value() const

{

if ( Type != BooleanType )  {

   cerr << "\n\n  DictionaryEntry::b_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Bval );

}


////////////////////////////////////////////////////////////////////////


const ConcatString * DictionaryEntry::string_value() const

{

if ( Type != StringType )  {

   cerr << "\n\n  DictionaryEntry::string_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Text );

}


////////////////////////////////////////////////////////////////////////


Dictionary * DictionaryEntry::dictionary_value() const

{

if ( Type != DictionaryType )  {

   cerr << "\n\n  DictionaryEntry::dictionary_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Dict );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Dictionary
   //


////////////////////////////////////////////////////////////////////////


Dictionary::Dictionary()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Dictionary::~Dictionary()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Dictionary::Dictionary(const Dictionary & d)

{

init_from_scratch();

assign(d);

}


////////////////////////////////////////////////////////////////////////


Dictionary & Dictionary::operator=(const Dictionary & d)

{

if ( this == &d )  return ( * this );

assign(d);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Dictionary::init_from_scratch()

{

e = (DictionaryEntry **) 0;

Parent = (const Dictionary *) 0;

Nentries = 0;

Nalloc = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::clear()

{

if ( e )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( e[j] )  { delete e[j];  e[j] = (DictionaryEntry *) 0; }

   }

   delete [] e;  e = (DictionaryEntry **) 0;

}

Nentries = 0;

Nalloc = 0;

Parent = (const Dictionary *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::assign(const Dictionary & d)

{

clear();

if ( d.Nentries == 0 )  return;

extend(d.Nentries);

int j;

for (j=0; j<(d.Nentries); ++j)  {

   store( *(d.e[j]) );

}

Parent = d.Parent;

patch_parents();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nentries = " << Nentries << "\n";

out << prefix << "Parent   = " << (void *) Parent << "\n";

int j;

for (j=0; j<Nentries; ++j)  {

   out << prefix << "Entry[" << j << "] ...\n";

   e[j]->dump(out, depth + 1);

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::extend(int n)

{

if ( Nalloc >= n )  return;

n = dictionary_alloc_inc*((n + dictionary_alloc_inc - 1)/dictionary_alloc_inc);

int j;
DictionaryEntry ** u = (DictionaryEntry **) 0;

u = new DictionaryEntry * [n];

for (j=0; j<n; ++j)  u[j] = (DictionaryEntry *) 0;

if ( e )  {

   for (j=0; j<Nentries; ++j)  u[j] = e[j];

   delete [] e;  e = (DictionaryEntry **) 0;

}

e = u;   u = (DictionaryEntry **) 0;

Nalloc = n;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::store(const DictionaryEntry & entry)

{


   //
   //  first see if we've already got an entry by that name
   //

int j;
bool found = false;

for (j=0; j<Nentries; ++j)  {

   if ( e[j]->Name == entry.Name )  {

      // *(e[j]) = entry;

      found = true;

      // return;

      break;

   }

}

if ( found )  {

   if ( e[j]->is_dictionary() && entry.is_dictionary() )  {

      e[j]->dictionary_value()->store( *(entry.dictionary_value()) );

   } else {

      *(e[j]) = entry;

   }

   return;

}

   //
   //  nope, new entry
   //

extend(Nentries + 1);

e[Nentries] = new DictionaryEntry;

*(e[Nentries]) = entry;

++Nentries;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::store(const Dictionary & d)

{

int j;

for (j=0; j<(d.n_entries()); ++j)  {

   store( *(d[j]) );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * Dictionary::operator[](int n) const

{

if ( (n < 0) || (n >= Nentries) )  {

   cerr << "\n\n  Dictionary::operator[](int) const -> range check error\n\n";

   exit ( 1 );


}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * Dictionary::lookup(const char * name) const

{

int j;

   //
   //  try current dictionary
   //

for (j=0; j<Nentries; ++j)  {

   if ( e[j]->Name == name )  {

      return ( e[j] );

   }

}

   //
   //  try parent
   //

const DictionaryEntry * E = (const DictionaryEntry *) 0;

if ( Parent )  E = Parent->lookup(name);

   //
   //  done
   //

return ( E );

}


////////////////////////////////////////////////////////////////////////


void Dictionary::set_parent(const Dictionary * D)

{

if ( D == this )  {

   cerr << "\n\n  Dictionary::set_parent(const Dictionary *) -> dictionary can't be it's own parent!\n\n";

   exit ( 1 );

}

Parent = D;

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::patch_parents() const

{

int j;
Dictionary * d = (Dictionary *) 0;

for (j=0; j<Nentries; ++j)  {

   if ( e[j]->type() != DictionaryType )  continue;

   d = e[j]->dictionary_value();

   d->set_parent(this);

   d->patch_parents();

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DictionaryStack
   //


////////////////////////////////////////////////////////////////////////


DictionaryStack::DictionaryStack()

{

// init_from_scratch();

cerr << "\n\n  DictionaryStack::DictionaryStack() -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


DictionaryStack::DictionaryStack(Dictionary & d)

{

init_from_scratch();

D[0] = &d;

Nelements = 1;

}


////////////////////////////////////////////////////////////////////////


DictionaryStack::~DictionaryStack()

{

clear();

}


////////////////////////////////////////////////////////////////////////


DictionaryStack::DictionaryStack(const DictionaryStack & s)

{

// init_from_scratch();
// 
// assign(s);

cerr << "\n\n  DictionaryStack::DictionaryStack(const DictionaryStack &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


DictionaryStack & DictionaryStack::operator=(const DictionaryStack & s)

{

// if ( this == &s )  return ( * this );
// 
// assign(s);

cerr << "\n\n  DictionaryStack::operator=(const DictionaryStack &) -> should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::init_from_scratch()

{

int j;

for (j=0; j<max_dictionary_depth; ++j)  {

   D[j] = (Dictionary *) 0;

}

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::clear()

{

int j;

for (j=1; j<max_dictionary_depth; ++j)  {   //  j starts at one, here

   if ( D[j] )  { delete D[j];  D[j] = (Dictionary *) 0; }

}

Nelements = 0;

return;

}


////////////////////////////////////////////////////////////////////////

/*
void DictionaryStack::assign(const DictionaryStack & s)

{

clear();

if ( s.Nelements == 0 )  return;

int j;

for (j=0; j<(s.Nelements); ++j)  {

   D[j] = new Dictionary;

   *(D[j]) = *(s.D[j]);

}

Nelements = s.Nelements;

return;

}
*/

////////////////////////////////////////////////////////////////////////


void DictionaryStack::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << '\n';

int j;

for (j=0; j<Nelements; ++j)  {

   out << prefix << "Element [" << j << "] ...\n";

   D[j]->dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


const Dictionary * DictionaryStack::top() const

{

if ( Nelements == 0 )  return ( (const Dictionary *) 0 );

return ( D[Nelements - 1] );

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::erase_top()

{

if ( Nelements <= 1 )  {

   cerr << "\n\n  DictionaryStack::erase_top() -> can't erase bottom-level dictionary!\n\n";

   exit ( 1 );

}

delete D[Nelements - 1];   D[Nelements - 1] = (Dictionary *) 0;

--Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::push()

{

if ( Nelements >= max_dictionary_depth )  {

   cerr << "\n\n  DictionaryStack::push() -> stack full!\n\n";

   exit ( 1 );

}

D[Nelements++] = new Dictionary;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::pop(const char * name)

{

if ( Nelements < 2 )  {

   cerr << "\n\n  DictionaryStack::pop() -> stack empty!\n\n";

   exit ( 1 );

}

DictionaryEntry entry;

entry.set_dict(name, *(D[Nelements - 1]));

D[Nelements - 2]->store(entry);

delete D[Nelements - 1];  D[Nelements - 1] = (Dictionary *) 0;

--Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::store(const DictionaryEntry & entry)

{

if ( Nelements == 0 )  push();

D[Nelements - 1]->store(entry);

return;

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * DictionaryStack::lookup(const char * name) const

{

if ( Nelements == 0 )  return ( (const DictionaryEntry *) 0 );

int j;
const DictionaryEntry * e = (const DictionaryEntry *) 0;

for (j=(Nelements - 1); j>=0; --j)  {

   e = D[j]->lookup(name);

   if ( e )  return ( e );

}

   //
   //  nope
   //

return ( (const DictionaryEntry *) 0 );

}


////////////////////////////////////////////////////////////////////////





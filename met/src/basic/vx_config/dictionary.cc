// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "indent.h"
#include "vx_log.h"
#include "nint.h"
#include "bool_to_string.h"
#include "empty_string.h"
#include "is_bad_data.h"

#include "dictionary.h"
#include "configobjecttype_to_string.h"


////////////////////////////////////////////////////////////////////////


static const char config_tab [] = "  ";


////////////////////////////////////////////////////////////////////////


static ConcatString config_prefix(int depth);


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


DictionaryEntry::DictionaryEntry(const DictionaryEntry & entry)

{

init_from_scratch();

assign(entry);

}


////////////////////////////////////////////////////////////////////////


DictionaryEntry & DictionaryEntry::operator=(const DictionaryEntry & entry)

{

if ( this == &entry )  return ( * this );

assign(entry);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::init_from_scratch()

{

Text = (ConcatString *) 0;

Dict = (Dictionary *) 0;

Thresh = (SingleThresh *) 0;

PWL = (PiecewiseLinear *) 0;

v = (IcodeVector *) 0;

Nargs = 0;

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

if ( Thresh )  { delete Thresh;  Thresh = (SingleThresh *) 0; }

if ( PWL )  { delete PWL;  PWL = (PiecewiseLinear *) 0; }

if ( v )  { delete v;  v = (IcodeVector *) 0; }

Nargs = 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::assign(const DictionaryEntry & entry)

{

clear();

switch ( entry.Type )  {
  
   case IntegerType:
      set_int(entry.Name, entry.Ival);
      break;

   case FloatType:
      set_double(entry.Name, entry.Dval);
      break;

   case BooleanType:
      set_boolean(entry.Name, entry.Bval);
      break;

   case StringType:
     set_string(entry.Name, entry.Text->c_str());
      break;

   case DictionaryType:
      set_dict(entry.Name, *(entry.Dict));
      break;

   case ArrayType:
      // set_array(entry.Name, *(entry.Dict));
      set_dict(entry.Name, *(entry.Dict));
      break;

   case ThresholdType:
      set_threshold(entry.Name, *(entry.Thresh));
      break;

   case PwlFunctionType:
     set_pwl(entry.Name, *(entry.PWL));
      break;

   // case VariableType:
   //    set_variable(entry.Name, *(entry.v));
   //    break;

   case UserFunctionType:
     set_user_function(entry.Name, *(entry.v), entry.Nargs);
      break;

   default:
      mlog << Error
           << "\n\n  DictionaryEntry::assign(const DictionaryEntry &) -> bad object type ... \""
           << configobjecttype_to_string(entry.Type) << "\"\n\n";
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
      out << prefix << "Integer Value = " << Ival << "\n";
      break;

   case FloatType:
      out << prefix << "Float Value = " << Dval << "\n";
      break;

   case BooleanType:
      out << prefix << "Boolean Value = " << bool_to_string(Bval) << "\n";
      break;

   case StringType:
      out << prefix << "String Value = ";
      if ( Text->empty() )  out << "(nul)\n";
      else                  out << '\"' << (*Text) << "\"\n";
      break;

   case DictionaryType:   //  fall through
   case ArrayType:        //  fall through
      out << prefix << "Dict/Array Value = \n";
      Dict->dump(out, depth + 1);
      break;

   case ThresholdType:
      out << prefix << "Thresh Value = \n";
      Thresh->dump(out, depth + 1);
      break;


   case PwlFunctionType:
      out << prefix << "Function Value = \n";
      PWL->dump(out, depth + 1);
      break;


   case UserFunctionType:
      out << prefix << "N args = " << Nargs << "\n";
      if ( depth <= 2 )  {
         out << prefix << "Code ... \n";
         v->dump(out, depth + 1);
      }
      break;


   default:
      mlog << Error
           << "\nDictionaryEntry::dump(const DictionaryEntry &) -> "
           << "bad object type ... \""
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


void DictionaryEntry::dump_config_format(ostream & out, int depth) const

{

int k;
const ConcatString s = config_prefix(depth);

out << s;

if ( Name.nonempty() )  out << Name << " = ";

switch ( Type )  {

   case IntegerType:
      out << Ival << ";\n";
      break;

   case FloatType:
      out << Dval << ";\n";
      break;

   case BooleanType:
      out << bool_to_string(Bval) << ";\n";
      break;

   case StringType:
      if ( Text->empty() )  out << "(nul);\n";
      else                  out << '\"' << (*Text) << "\";\n";
      break;


   case DictionaryType:
      out << "{\n";
      Dict->dump_config_format(out, depth + 1);
      out << s << "}\n";
      break;

   case ArrayType:
      out << "[\n";
      Dict->dump_config_format(out, depth + 1);
      out << s << "];\n";
      break;

   case ThresholdType:
      switch ( Thresh->get_type() )  {

         case thresh_lt:  out << "<";   break;
         case thresh_le:  out << "<=";  break;
         case thresh_eq:  out << "==";  break;
         case thresh_ne:  out << "!=";  break;
         case thresh_gt:  out << ">";   break;
         case thresh_ge:  out << ">=";  break;

         case thresh_na:  out << na_str;  break;

         default:
         mlog << Error
              << "DictionaryEntry::dump_config_format() -> bad threshold type ... " << Thresh->get_type() << "\n";
         exit ( 1 );
         break;

      }  //  switch
      if ( Thresh->get_type() != thresh_na )  out << Thresh->get_value();
      out << ";\n";
      break;

   case PwlFunctionType:
      out << "( ";
      for (k=0; k<(PWL->n_points()); ++k)  {

         out << '(' << (PWL->x(k)) << ", " << (PWL->y(k)) << ')';

         if ( k < (PWL->n_points() - 1) )  out << ' ';

      }
      out << " );\n";
      break;

   default:
      mlog << Error
           << "\nDictionaryEntry::dump_config_format(const DictionaryEntry &) -> "
           << "bad object type ... \""
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


void DictionaryEntry::set_name (const std::string _name)

{

  if ( _name.empty() )  {

   mlog << Error
        << "\nDictionaryEntry::set_name (const char *) -> "
        << "empty string!\n\n";

   exit ( 1 );

}

Name = _name;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_icodevector (const IcodeVector & _icv)

{

if ( v )  { delete v;  v = 0; }

v = new IcodeVector;

(*v) = _icv;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_int (const std::string _name, int k)

{

clear();

Type = IntegerType;

if ( _name != "" )   set_name(_name);

Ival = k;


return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_variable (const char * _name, const IcodeVector & _icv)

{

set_name(_name);

set_icodevector(_icv);

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_user_function (const std::string _name, const IcodeVector & _icv, int _n_args)

{

set_name(_name);

set_icodevector(_icv);

Nargs = _n_args;

Type = UserFunctionType;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_double (const std::string _name, double x)

{

clear();

Type = FloatType;

if ( _name != "" )  set_name(_name);

Dval = x;


return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_string (const std::string _name, const std::string _text)

{

clear();

Type = StringType;

if ( _name != "" )  set_name(_name);

Text = new ConcatString;

if ( _text != "" )  *Text = _text;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_boolean(const std::string _name, bool tf)

{

clear();

Type = BooleanType;

if ( _name != "" )  set_name(_name);

Bval = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_dict(const std::string _name, const Dictionary & d)

{

clear();

if ( d.is_array() )  Type = ArrayType;
else                 Type = DictionaryType;

if ( _name != "" )  set_name(_name);

Dict = new Dictionary;

*Dict = d;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_pwl(const std::string _name, const PiecewiseLinear & _pwl)

{

clear();

Type = PwlFunctionType;

if ( _name != "" )  set_name(_name);

PWL = new PiecewiseLinear;

PWL->set_name(_name);

*PWL = _pwl;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryEntry::set_threshold(const std::string _name, const SingleThresh & t)

{

clear();

Type = ThresholdType;

if ( _name != "" )  set_name(_name);

Thresh = new SingleThresh;

*Thresh = t;


return;

}


////////////////////////////////////////////////////////////////////////

/*
void DictionaryEntry::set_array(const char * _name, const Dictionary & d)

{

clear();

Type = ArrayType;

set_name(_name);

Dict = new Dictionary;

*Dict = d;

   //
   //  zero out parents for dictionaries in array
   //

int j;
const DictionaryEntry * E = (const DictionaryEntry *) 0;
Dictionary * D = (Dictionary *) 0;
Dictionary & DD = *Dict;

for (j=0; j<(Dict->n_entries()); ++j)  {

   E = DD[j];

   if ( ! (E->is_dictionary()) )  continue;

   D = E->dict_value();

   D->set_parent(0);

}


   //
   //  done
   //

return;

}
*/

////////////////////////////////////////////////////////////////////////


int DictionaryEntry::i_value() const

{

if ( Type != IntegerType )  {

   mlog << Error
        << "\nDictionaryEntry::i_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Ival );

}


////////////////////////////////////////////////////////////////////////


double DictionaryEntry::d_value() const

{

if ( Type != FloatType )  {

   mlog << Error
        << "\nDictionaryEntry::d_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Dval );

}


////////////////////////////////////////////////////////////////////////


bool DictionaryEntry::b_value() const

{

if ( Type != BooleanType )  {

   mlog << Error
        << "\nDictionaryEntry::b_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Bval );

}


////////////////////////////////////////////////////////////////////////


const ConcatString * DictionaryEntry::string_value() const

{

if ( Type != StringType )  {

   mlog << Error
        << "\nDictionaryEntry::string_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Text );

}


////////////////////////////////////////////////////////////////////////


Dictionary * DictionaryEntry::dict_value() const

{

if ( Type != DictionaryType )  {

   mlog << Error
        << "\nDictionaryEntry::dict_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Dict );

}


////////////////////////////////////////////////////////////////////////


Dictionary * DictionaryEntry::array_value() const

{

if ( Type != ArrayType )  {

   mlog << Error
        << "\nDictionaryEntry::array_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Dict );

}


////////////////////////////////////////////////////////////////////////


SingleThresh * DictionaryEntry::thresh_value() const

{

if ( Type != ThresholdType )  {

   mlog << Error
        << "\nDictionaryEntry::thresh_value() -> bad type\n\n";

   exit ( 1 );

}


return ( Thresh );

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear * DictionaryEntry::pwl_value() const

{

if ( Type != PwlFunctionType )  {

   mlog << Error
        << "\nDictionaryEntry::pwl_value() -> bad type\n\n";

   exit ( 1 );

}


return ( PWL );

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

Parent = (Dictionary *) 0;


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

IsArray = false;

Parent = (Dictionary *) 0;

LastLookupStatus = false;

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::assign(const Dictionary & d)

{

clear();

if ( d.Nentries > 0 )  {

   extend(d.Nentries);

   int j;

   for (j=0; j<(d.Nentries); ++j)  {

      store( *(d.e[j]) );

   }

}

IsArray = d.IsArray;

Parent = d.Parent;

LastLookupStatus = d.LastLookupStatus;

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

out << prefix << "Address  = " << (void *) this   << "\n";

out << prefix << "Parent   = " << (void *) Parent << "\n";
// if ( Parent )  out << "   (" << Parent->name() << ")\n";

out << prefix << "IsArray  = " << bool_to_string(IsArray) << "\n";

int j;

for (j=0; j<Nentries; ++j)  {

   out << prefix << "Dictionary Entry[" << j << "] ...\n";

   e[j]->dump(out, depth + 1);

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::dump_config_format(ostream & out, int depth) const

{

int j;

for (j=0; j<Nentries; ++j)  {

   e[j]->dump_config_format(out, depth + 1);

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

      e[j]->dict_value()->store( *(entry.dict_value()) );

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

   mlog << Error
        << "\nDictionary::operator[](int) const -> "
        << "range check error\n\n";

   exit ( 1 );


}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * Dictionary::lookup(const std::string name)

{


if ( Nentries == 0 )  {

   LastLookupStatus = false;

   return ( (const DictionaryEntry *) 0 );
}

int j;
StringArray scope;
ConcatString Name = name;
const DictionaryEntry * E = (const DictionaryEntry *) 0;
Dictionary * D = this;


   //
   //  resolve scope, if needed
   //

scope = Name.split(".");

if ( scope.n_elements() == 1 )  {

   return ( lookup_simple(name) );

}

for (j=0; j<(scope.n_elements() - 1); ++j)  {

  E = D->lookup(scope[j].c_str());

   if ( !E )  {

      LastLookupStatus = false;

      return ( (const DictionaryEntry *) 0 );
   }

   if ( ! E->is_dictionary() )  {

      LastLookupStatus = false;

      return ( (const DictionaryEntry *) 0 );
   }

   D = E->dict_value();

}

   //
   //  try current dictionary
   //

 const char * stub = scope[scope.n_elements() - 1].c_str();

E = D->lookup_simple(stub);

if ( E )  {

   LastLookupStatus = (E != 0);

   return ( E );
}

   //
   //  try parent
   //

E = (const DictionaryEntry *) 0;

if ( Parent )  E = Parent->lookup(name);

   //
   //  done
   //

LastLookupStatus = (E != 0);

return ( E );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * Dictionary::lookup_simple(const std::string name)

{

if ( Nentries == 0 )  {

   LastLookupStatus = false;

   return ( (const DictionaryEntry *) 0 );
}

int j;

for (j=0; j<Nentries; ++j)  {

   if ( e[j]->Name == name )  {

      LastLookupStatus = (e[j] != 0);

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

LastLookupStatus = (E != 0);

return ( E );

}


////////////////////////////////////////////////////////////////////////


void Dictionary::set_parent(Dictionary * D)

{

if ( D == this )  {

   mlog << Error
       << "\nDictionary::set_parent(const Dictionary *) -> "
       << "dictionary can't be it's own parent!\n\n";

   exit ( 1 );

}

Parent = D;

return;

}


////////////////////////////////////////////////////////////////////////


void Dictionary::patch_parents()

{

int j;
Dictionary * d = (Dictionary *) 0;
ConfigObjectType t = no_config_object_type;


for (j=0; j<Nentries; ++j)  {

   d = (Dictionary *) 0;

   t = e[j]->type();

   switch ( t )  {

      case DictionaryType:
         d = e[j]->dict_value();
         break;

      case ArrayType:
         d = e[j]->array_value();
         break;

      default:
        break;

   }   //  switch

   if ( !d )  continue;

   d->set_parent(this);

   d->patch_parents();

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool Dictionary::lookup_bool(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == BooleanType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_bool() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( false );

}

return ( Entry->b_value() );

}


////////////////////////////////////////////////////////////////////////


int Dictionary::lookup_int(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == IntegerType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_int() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( bad_data_int );

}

return ( Entry->i_value() );

}


////////////////////////////////////////////////////////////////////////


double Dictionary::lookup_double(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;
double v;

if ( Entry )  is_correct_type = (Entry->type() == FloatType ||
                                 Entry->type() == IntegerType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_double() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( bad_data_double );

}

   //
   //  Store the number as a double
   //

     if ( Entry->type() == FloatType   )  v = Entry->d_value();
else if ( Entry->type() == IntegerType )  v = (double) Entry->i_value();

return ( v );

}


////////////////////////////////////////////////////////////////////////


NumArray Dictionary::lookup_num_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
ConfigObjectType Type = no_config_object_type;
bool is_correct_type = false;
NumArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType   ||
                                 Entry->type() == IntegerType ||
                                 Entry->type() == FloatType   ||
                                 Entry->type() == BooleanType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_num_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Store a single integer and return
   //

if ( Entry->type() == IntegerType )  {

   array.add( Entry->i_value() );

   return ( array );

}

   //
   //  Store a single double and return
   //

if ( Entry->type() == FloatType )  {

   array.add( Entry->d_value() );

   return ( array );

}

   //
   //  Store a single boolean and return
   //

if ( Entry->type() == BooleanType )  {

   array.add( Entry->b_value() );

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //

Dict = Entry->array_value();

   //  Check the array type.
   //  Populate the NumArray with integers, doubles, or booleans
   //

for (int i=0; i<Dict->n_entries(); i++)  {

   Type = (*Dict)[i]->type();

   if( Type != IntegerType &&
       Type != FloatType   &&
       Type != BooleanType )  {

      mlog << Error
           << "\nDictionary::lookup_num_array() -> "
           << "array \"" << name
           << "\" does not contain numeric entries.\n\n";

      exit ( 1 );

   }

        if( Type == FloatType   )  array.add((*Dict)[i]->d_value());
   else if( Type == IntegerType )  array.add((*Dict)[i]->i_value());
   else if( Type == BooleanType )  array.add((*Dict)[i]->b_value());

}

return ( array );

}


////////////////////////////////////////////////////////////////////////


IntArray Dictionary::lookup_int_array(const char * name, bool error_out)

{

IntArray array;
NumArray num_array = lookup_num_array(name, error_out);

for (int i=0; i<num_array.n_elements(); i++)
   array.add( nint(num_array[i]) );

return ( array );

}

////////////////////////////////////////////////////////////////////////


ConcatString Dictionary::lookup_string(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == StringType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_string() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   ConcatString s;

   return ( s );

}

return ( *(Entry->string_value()) );

}


////////////////////////////////////////////////////////////////////////


StringArray Dictionary::lookup_string_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
bool is_correct_type = false;
StringArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType ||
                                 Entry->type() == StringType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_string_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Store a single string and return
   //

if ( Entry->type() == StringType )  {

   array.add( *(Entry->string_value()) );

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //

Dict = Entry->array_value();

   //
   //  Check the array type.
   //  Error out if array contains unexpected type.
   //

if ( Dict->n_entries() > 0 )  {

   if( (*Dict)[0]->type() != StringType )  {

      mlog << Error
           << "\nDictionary::lookup_string_array() -> "
           << "array \"" << name
           << "\" does not contain StringType.\n\n";

      exit ( 1 );

   }
}

   //
   //  Populate the StringArray
   //

for (int i=0; i<Dict->n_entries(); i++)  {

   array.add(*(*Dict)[i]->string_value());

}

return ( array );

}


////////////////////////////////////////////////////////////////////////


SingleThresh Dictionary::lookup_thresh(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == ThresholdType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_thresh() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   SingleThresh s;

   return ( s );

}

return ( *(Entry->thresh_value()) );

}


////////////////////////////////////////////////////////////////////////


ThreshArray Dictionary::lookup_thresh_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
bool is_correct_type = false;
ThreshArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType ||
                                 Entry->type() == ThresholdType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_thresh_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Store a single threshold and return
   //

if ( Entry->type() == ThresholdType )  {

   array.add( *(Entry->thresh_value()) );

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //

Dict = Entry->array_value();

   //
   //  Check the array type.
   //  Error out if array contains unexpected type.
   //

if ( Dict->n_entries() > 0 )  {

   if( (*Dict)[0]->type() != ThresholdType )  {

      mlog << Error
           << "\nDictionary::lookup_thresh_array() -> "
           << "array \"" << name
           << "\" does not contain ThreshType.\n\n";

      exit ( 1 );

   }
}

   //
   //  Populate the ThreshArray
   //

for (int i=0; i<Dict->n_entries(); i++)  {

   array.add(*(*Dict)[i]->thresh_value());

}

return ( array );

}


////////////////////////////////////////////////////////////////////////


Dictionary *Dictionary::lookup_dictionary(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == DictionaryType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_dictionary() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( (Dictionary *) 0 );

}

return ( Entry->dict_value() );

}


////////////////////////////////////////////////////////////////////////


Dictionary *Dictionary::lookup_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( (Dictionary *) 0 );

}

return ( Entry->array_value() );

}


////////////////////////////////////////////////////////////////////////
//
// Seconds may be stored as an integer or as an HH[MMSS] timestring.
//
////////////////////////////////////////////////////////////////////////


int Dictionary::lookup_seconds(const char * name, bool error_out)

{

ConcatString cs = lookup_string(name, false);

if ( LastLookupStatus )  {
   if ( cs.empty() )  return ( bad_data_int );
   else               return ( timestring_to_sec( cs.c_str() ) );
}

return ( lookup_int(name, error_out) );

}


////////////////////////////////////////////////////////////////////////


IntArray Dictionary::lookup_seconds_array(const char * name, bool error_out)

{

StringArray sa = lookup_string_array(name, error_out);
IntArray ia;

int j;

 for (j=0; j<sa.n_elements(); ++j)  ia.add( timestring_to_sec( sa[j].c_str() ));

return ( ia );

}

////////////////////////////////////////////////////////////////////////


unixtime Dictionary::lookup_unixtime(const char * name, bool error_out)

{

ConcatString cs = lookup_string(name, error_out);

if ( cs.empty() )  return ( (unixtime) 0 );
 else               return ( timestring_to_unix( cs.c_str() ) );

}

////////////////////////////////////////////////////////////////////////


TimeArray Dictionary::lookup_unixtime_array(const char * name, bool error_out)

{

StringArray sa = lookup_string_array(name, error_out);
TimeArray ta;

int j;

 for (j=0; j<sa.n_elements(); ++j)  ta.add( timestring_to_unix( sa[j].c_str() ));

return ( ta );

}


////////////////////////////////////////////////////////////////////////


PiecewiseLinear *Dictionary::lookup_pwl(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == PwlFunctionType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nDictionary::lookup_pwl() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( (PiecewiseLinear *) 0 );

}

return ( Entry->pwl_value() );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class DictionaryStack
   //


////////////////////////////////////////////////////////////////////////


DictionaryStack::DictionaryStack()

{

// init_from_scratch();

mlog << Error
     << "\nDictionaryStack::DictionaryStack() -> "
     << "should never be called!\n\n";

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

mlog << Error
     << "\nDictionaryStack::DictionaryStack(const DictionaryStack &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


DictionaryStack & DictionaryStack::operator=(const DictionaryStack & s)

{

// if ( this == &s )  return ( * this );
//
// assign(s);

mlog << Error
     << "\nDictionaryStack::operator=(const DictionaryStack &) -> "
     << "should never be called!\n\n";

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

   out << prefix << "Stack Element [" << j << "] ...\n";

   D[j]->dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::dump_config_format(ostream & out, int depth) const

{

int j;
const ConcatString s = config_prefix(depth);

for (j=0; j<Nelements; ++j)  {

   out << s << "Stack Element " << j << "...\n\n";

   D[j]->dump_config_format(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::set_top_is_array(bool tf)

{

if ( Nelements == 0 )  {

   mlog << Error
        << "\n\n  DictionaryStack::set_top_is_array() -> stack empty!\n\n";

   exit ( 1 );

}


if ( Nelements == 1 )  {

   mlog << Error
        << "\n\n  DictionaryStack::set_top_is_array() -> can't set bottom-level dictionary to array!\n\n";

   exit ( 1 );

}


D[Nelements - 1]->set_is_array(true);

return;

}


////////////////////////////////////////////////////////////////////////


bool DictionaryStack::top_is_array() const

{

if ( Nelements == 0 )  {

   mlog << Error
        << "\n\n  DictionaryStack::top_is_array() -> stack empty!\n\n";

   exit ( 1 );

}

return ( D[Nelements - 1]->is_array() );

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

   mlog << Error
        << "\nDictionaryStack::erase_top() -> "
        << "can't erase bottom-level dictionary!\n\n";

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

   mlog << Error
        << "\nDictionaryStack::push() -> stack full!\n\n";

   exit ( 1 );

}

D[Nelements++] = new Dictionary;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::push_array()

{

push();

D[Nelements - 1]->set_is_array(true);

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::pop_dict(const std::string name)

{

if ( Nelements < 2 )  {

   mlog << Error
        << "\nDictionaryStack::pop_dict() -> stack empty!\n\n";

   exit ( 1 );

}

DictionaryEntry entry;

entry.set_dict (name, *(D[Nelements - 1]));

D[Nelements - 2]->store(entry);

delete D[Nelements - 1];  D[Nelements - 1] = (Dictionary *) 0;

--Nelements;

return;

}


////////////////////////////////////////////////////////////////////////


void DictionaryStack::pop_element(const char * name)

{

if ( Nelements < 2 )  {

   mlog << Error
        << "\nDictionaryStack::pop_element() -> stack empty!\n\n";

   exit ( 1 );

}

if ( D[Nelements - 1]->n_entries() != 1 )  {

   mlog << Error
        << "\nDictionaryStack::pop_element() -> toplevel dictionary has more than one element!\n\n";

   exit ( 1 );

}

DictionaryEntry E = *(D[Nelements - 1]->operator[](0));

E.set_name(name);

D[Nelements - 2]->store(E);

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


const DictionaryEntry * DictionaryStack::lookup(const std::string name) const

{

if ( Nelements == 0 )  return ( (const DictionaryEntry *) 0 );

int j;
const DictionaryEntry * E = (const DictionaryEntry *) 0;

for (j=(Nelements - 1); j>=0; --j)  {

   E = D[j]->lookup(name);

   if ( E )  return ( E );

}

   //
   //  nope
   //

return ( (const DictionaryEntry *) 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ConcatString config_prefix(int depth)

{


int j;
ConcatString s;

if ( depth == 0 )   { s << ' ';  return ( s ); }

for (j=0; j<depth; ++j)  {

   s << config_tab;

}

return ( s );

}


////////////////////////////////////////////////////////////////////////





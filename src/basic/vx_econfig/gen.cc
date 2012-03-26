

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


static const char copyright_filename [] = "src/tools/dev_utils/copyright_notice.txt";   //  relative to MET_BASE_DIR


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <cmath>

#include "vx_log.h"
#include "gen.h"
#include "stetype_to_string.h"


////////////////////////////////////////////////////////////////////////


extern const char * program_name;


////////////////////////////////////////////////////////////////////////


static void insert_copyright(ofstream &);


////////////////////////////////////////////////////////////////////////


   //
   //  names of temporary variables used by the member functions
   //

const char * temp_result_varname = "_temp_result";

const char * temp_cell_varname   = "_cell";

   //
   //  misc other stuff
   //

const char * result = "Result";   //  class name of generic return type

const char * tab = "      ";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CodeGenerator
   //


////////////////////////////////////////////////////////////////////////


CodeGenerator::CodeGenerator()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CodeGenerator::~CodeGenerator()

{

clear();

if ( sep )  { delete [] sep;  sep = (const char *) 0; }

}


////////////////////////////////////////////////////////////////////////


CodeGenerator::CodeGenerator(const CodeGenerator & g)

{

mlog << Error << "\nCodeGenerator::CodeGenerator(const CodeGenerator &) -> should never be called\n\n";

exit ( 1 );

// init_from_scratch();

// assign(g);

}


////////////////////////////////////////////////////////////////////////


CodeGenerator & CodeGenerator::operator=(const CodeGenerator & g)

{

mlog << Error << "\nCodeGenerator::operator=(const CodeGenerator &) -> should never be called\n\n";

exit ( 1 );


// if ( this == &g )  return ( * this );

// assign(g);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::init_from_scratch()

{

const int sep_len = 80;
char * u = (char *) 0;


ClassName = (char *) 0;

FilePrefix = (char *) 0;

ConfigFileName = (char *) 0;

u = new char [sep_len];

memset(u, 0, sep_len);

memset(u, '/', 72);

u[72] = '\n';

sep = u;  u = (char *) 0;

zone_name = (const char *) 0;

HH = false;

Panic = true;

AllowMultipleReads = false;


clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::clear()

{

machine.clear();

if ( ClassName )  { delete [] ClassName;  ClassName = (char *) 0; }

if ( FilePrefix )  { delete [] FilePrefix;  FilePrefix = (char *) 0; }

if ( ConfigFileName )  { delete [] ConfigFileName;  ConfigFileName = (char *) 0; }

zone_name = (const char *) 0;

HH = false;

Panic = true;

AllowMultipleReads = false;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_string_value(char * & var, const char * text)

{

if ( var )  { delete [] var;  var = (char *) 0; }

int n = strlen(text);

var = new char [1 + n];

memset(var, 0, 1 + n);

strcpy(var, text);


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_class_name(const char * text)

{

set_string_value(ClassName, text);

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_file_prefix(const char * text)

{

set_string_value(FilePrefix, text);

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_config_filename(const char * text)

{

set_string_value(ConfigFileName, text);

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_hh()

{

HH = true;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_nopanic()

{

Panic = false;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::set_multiple_reads(bool tf)

{

AllowMultipleReads = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::process(const char * config_filename)

{

set_config_filename(config_filename);

if ( !ClassName )  {

   mlog << Error << "\nCodeGenerator::process() -> no class name set\n\n";

   exit ( 1 );

}


if ( !FilePrefix )  {

   mlog << Error << "\nCodeGenerator::process() -> no file prefix set\n\n";

   exit ( 1 );

}

   //
   //  load the config file into the machine
   //

machine.read(ConfigFileName);

   //
   //  do the code generation
   //

GenerationTime = time(0);

if ( is_dst(GenerationTime) )  {

   GenerationTime -= 6*3600;

   zone_name = "MDT";

} else {

   GenerationTime -= 7*3600;

   zone_name = "MST";

}

// mlog << Debug(1) << "\n";

do_header();

// mlog << Debug(1) << "\n";

do_source();

// mlog << Debug(1) << "\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::warning(ostream & out)

{

int month, day, year, hour, minute, second;
char junk[256];
unixtime t;
const char * ampm = (const char *) 0;


t = GenerationTime;

   //
   //  round to the nearest minute
   //

t = 60*((t + 30)/60);

unix_to_mdyhms(t, month, day, year, hour, minute, second);

if ( hour >= 12 )  ampm = "pm";
else               ampm = "am";

hour = 1 + (hour + 11)%12;

sprintf(junk, "%s %d, %d    %d:%02d %s  %s",
               month_name[month], day, year, hour, minute,
               ampm, zone_name);


out << "\n\n"
    << "   //\n"
    << "   //  Warning:\n"
    << "   //\n"
    << "   //     This file is machine generated\n"
    << "   //\n"
    << "   //     Do not edit by hand\n"
    << "   //\n"
    << "   //\n"
    << "   //     Created by " << program_name << " from config file \"" << ConfigFileName << "\"\n"
    << "   //\n"
    << "   //     on " << junk << "\n"
    << "   //\n"
    << "\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_header()

{

int j, n;
ofstream out;
char output_filename[PATH_MAX];
char * pound_define = (char *) 0;
const char * suffix = (const char *) 0;


if ( machine.sts.n_tables() != 1 )  {

   mlog << Error << "\nCodeGenerator::do_header() -> wrong number of symbol tables on the stack ... "
        << machine.sts.n_tables() << "\n\n";

   exit ( 1 );

}

const SymbolTable & symtab = machine.sts.table(0);
const SymbolTableEntry * entry = (const SymbolTableEntry *) 0;

if ( HH )  suffix = "hh";
else       suffix = "h";

   //
   //  make pound_define
   //

n = strlen(ClassName) + 20;   //  better safe than sorry

pound_define = new char [n];

memset(pound_define, 0, n);

sprintf(pound_define, "__%s_%s__", ClassName, suffix);

n = strlen(pound_define);

for (j=0; j<n; ++j)  {

   char c;

   c = pound_define[j];

   if ( (c >= 'a') && (c <= 'z') )  {

      pound_define[j] = c - ('a' - 'A');

   }

}

sprintf(output_filename, "%s.%s", FilePrefix, suffix);

out.open(output_filename);

if ( !out )  {

   mlog << Error << "\nCodeGenerator::do_header() -> unable to open output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

insert_copyright(out);

mlog << Debug(1) << program_name << ":  Generating header file \"" << output_filename << "\"\n";

out << "\n\n"
    << sep;

out << "\n\n"
    << "#ifndef  " << pound_define << "\n"
    << "#define  " << pound_define << "\n"
    << "\n\n"
    << sep;

warning(out);
out << sep;

out << "\n\n"
    << "#include \"machine.h\"\n"
    << "#include \"result.h\"\n"
    << "\n\n"
    << sep;

   //
   //  begin the class declaration
   //

out << "\n\n"
    << "class " << ClassName << " {\n"
    << "\n"
    << "   private:\n"
    << "\n"
    << "      void init_from_scratch();\n"
    << "\n"
    << "      // void assign(const " << ClassName << " &);\n"
    << "\n\n"
    << "      " << ClassName << "(const " << ClassName << " &);\n"
    << "      " << ClassName << " & operator=(const " << ClassName << " &);\n"
    << "\n";


out << "         //\n"
    << "         //  symbol table entries for variables (not allocated)\n"
    << "         //\n"
    << "\n\n";


for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);

   out << tab << "const SymbolTableEntry * _" << (entry->name) << "_entry;\n\n";

}

out << "\n\n";

out << "         //\n"
    << "         //  the machine that \"runs\" the config file\n"
    << "         //\n"
    << "\n\n"
    << "      Machine _m;\n"
    << "\n\n"
    << "   public:\n"
    << "\n"
    << "      " << ClassName << "();\n"
    << "     ~" << ClassName << "();\n"
    << "\n"
    << "      void clear();\n"
    << "\n"
    << "      void read(const char * config_filename);\n"
    << "\n"
    << "      void st_dump(ostream &, int = 0) const;   //  dump machine symbol table\n"
    << "\n";

      //
      //  symbol presence
      //

if ( !Panic )  {


   out << "         //\n"
       << "         //  Symbol Presence\n"
       << "         //\n"
       << "\n";

   for (j=0; j<(symtab.n_entries()); ++j)  {

      entry = symtab.entry(j);

      out << "      bool has_" << (entry->name) << "() const;\n\n";

   }   //  for j


}   //  if !Panic

      //
      //  symbol table access
      //

out << "         //\n"
    << "         //  Symbol Access\n"
    << "         //\n";


for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);

   // if ( j > 0 )  out << "\n";

   switch ( entry->type )  {

      case ste_integer:
         out << "\n";
         out << tab << "int " << (entry->name) << "();\n";
         break;


      case ste_double:
         out << "\n";
         out << tab << "double " << (entry->name) << "();\n";
         break;


      case ste_variable:
         out << "\n";
         out << tab << result << " " << (entry->name) << "();\n";
         break;


      case ste_array:
         out << "\n";
         do_header_array_dec(out, entry);
         out << "\n"
             << tab << "int n_" << (entry->name) << "_elements(";
         if ( entry->ai->dim() > 1 )  out << "int";
         out << ");\n";
         break;


      case ste_function:
         out << "\n";
         do_header_function_dec(out, entry);
         break;


      case ste_pwl:
         out << "\n";
         do_header_pwl_dec(out, entry);
         break;


      default:
         mlog << Error << "\nCodeGenerator::do_header() -> "
              << "don't know how to handle symbol table entries of type \""
              << stetype_to_string(entry->type) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j


   //
   //  finish off the class declaration
   //

out << "\n"
    << "};\n"
    << "\n\n"
    << sep;

if ( !Panic )  {

   out << "\n\n";

   for (j=0; j<(symtab.n_entries()); ++j)  {

      entry = symtab.entry(j);

      out << "inline bool " << ClassName << "::has_" << (entry->name) << "() const "
          << "{ return ( _" << (entry->name) << "_entry != 0 ); }"
          << "\n\n";

   }   //  for j

   out << "\n"
       << sep;

}

   //
   //  finish off the header file
   //

out << "\n\n"
    << "#endif   /*  " << pound_define << "  */\n"
    << "\n\n"
    << sep;

out << "\n\n";

   //
   //  done
   //

out.close();

if ( pound_define )  { delete [] pound_define;  pound_define = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_header_array_dec(ostream & out, const SymbolTableEntry * entry)

{

int j;
const ArrayInfo * a = entry->ai;

out << tab << result << " " << (entry->name) << "(";

for (j=0; j<(a->dim()); ++j)  {

   out << "int";

   if ( j != (a->dim() - 1) )  out << ", ";

}


out << ");";

out << "   //  " << (a->dim()) << "-dimensional array, indices from ";

if ( a->dim() > 1 )  out << "(";

for (j=0; j<(a->dim()); ++j)  {

   out << "0";

   if ( j != (a->dim() - 1) )  out << ", ";

}

if ( a->dim() > 1 )  out << ")";

out << " to ";

if ( a->dim() > 1 )  out << "(";

for (j=0; j<(a->dim()); ++j)  {

   out << a->size(j);

   if ( j != (a->dim() - 1) )  out << ", ";

}


if ( a->dim() > 1 )  out << ")";

out << "\n";


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_header_function_dec(ostream & out, const SymbolTableEntry * entry)

{

int j;
const int nvars = entry->local_vars->n_elements();


out << tab << result << " " << (entry->name) << "(";

for (j=0; j<nvars; ++j)  {

   out << "const " << result << " &";

   if ( j != (nvars - 1) )  out << ", ";

}


out << ");";

out << "   //  function of " << nvars << " ";

if ( nvars > 1 )  out << "variables";
else              out << "variable";

out << "\n";

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_header_pwl_dec(ostream & out, const SymbolTableEntry * entry)

{


out << tab << result << " " << (entry->name) << "(const " << result << " &);   //  pwl function\n";

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_source()

{

int j, k;
ofstream out;
char output_filename[PATH_MAX];
const SymbolTable & symtab = machine.sts.table(0);
const SymbolTableEntry * entry = (const SymbolTableEntry *) 0;
int max_len;


sprintf(output_filename, "%s.cc", FilePrefix);

out.open(output_filename);

if ( !out )  {

   mlog << Error << "\nCodeGenerator::do_header() -> unable to open output file \""
        << output_filename << "\"\n\n";

   exit ( 1 );

}

insert_copyright(out);

mlog << Debug(1) << program_name << ":  Generating source file \"" << output_filename << "\"\n";

out << "\n\n"
    << sep;

warning(out);
out << sep;

out << "\n\n"
    << "using namespace std;\n"
    << "\n\n"
    << "#include <iostream>\n"
    << "#include <unistd.h>\n"
    << "#include <stdlib.h>\n"
    << "#include <string.h>\n"
    << "#include <cmath>\n"
    << "\n"
    << "#include \"" << FilePrefix << "." << (HH ? "hh" : "h") << "\"\n"
    << "#include \"icodecell_to_result.h\"\n"
    << "#include \"vx_log.h\"\n"
    << "\n\n"
    << sep;

// out << "\n\n"
//     << "static const bool Panic = " << ( Panic ? "true" : "false" ) << ";\n"
//     << "\n\n"
//     << sep;

out << "\n\n"
    << "   //\n"
    << "   //  Code for class " << ClassName << "\n"
    << "   //\n"
    << "\n\n"
    << sep;

   //
   //  default constructor
   //

out << "\n\n"
    << ClassName << "::" << ClassName << "()\n"
    << "\n"
    << "{\n"
    << "\n"
    << "init_from_scratch();\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

   //
   //  destructor
   //

out << "\n\n"
    << ClassName << "::~" << ClassName << "()\n"
    << "\n"
    << "{\n"
    << "\n"
    << "clear();\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

   //
   //  copy constructor
   //

/*
out << "\n\n"
    << ClassName << "::" << ClassName << "(const " << ClassName << " & a)\n"
    << "\n"
    << "{\n"
    << "\n"
    << "init_from_scratch();\n"
    << "\n"
    << "assign(a);\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;
*/

out << "\n\n"
    << ClassName << "::" << ClassName << "(const " << ClassName << " &)\n"
    << "\n"
    << "{\n"
    << "\n"
    << "mlog << Error << \"\\n" << ClassName << "::" << ClassName << "(const " << ClassName << " &) -> should never be called!\\n\\n\";\n"
    << "\n"
    << "exit ( 1 );\n"
    << "\n"
    << "//  init_from_scratch();\n"
    << "\n"
    << "//  assign(a);\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

   //
   //  assignment operator
   //

/*
out << "\n\n"
    << ClassName << " & " << ClassName << "::operator=(const " << ClassName << " & a)\n"
    << "\n"
    << "{\n"
    << "\n"
    << "if ( this == &a )  return ( * this );\n"
    << "\n"
    << "assign(a);\n"
    << "\n"
    << "return ( * this );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;
*/

out << "\n\n"
    << ClassName << " & " << ClassName << "::operator=(const " << ClassName << " &)\n"
    << "\n"
    << "{\n"
    << "\n"
    << "mlog << Error << \"\\n" << ClassName << "::" << "operator=(const " << ClassName << " &) -> should never be called!\\n\\n\";\n"
    << "\n"
    << "exit ( 1 );\n"
    << "\n"
    << "// if ( this == &a )  return ( * this );\n"
    << "\n"
    << "// assign(a);\n"
    << "\n"
    << "return ( * this );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


   //
   //  init_from_scratch
   //

out << "\n\n"
    << "void " << ClassName << "::init_from_scratch()\n"
    << "\n"
    << "{\n"
    << "\n"
    << "clear();\n"
    << "\n"
    << "return;\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


   //
   //  clear
   //

max_len = 0;

for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);

   k = strlen(entry->name);

   if ( k > max_len )  max_len = k;

}

out << "\n\n"
    << "void " << ClassName << "::clear()\n"
    << "\n"
    << "{\n"
    << "\n";


for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);

   for (k=strlen(entry->name); k<max_len; ++k)  out.put(' ');

   out << "_" << (entry->name) << "_entry = (const SymbolTableEntry *) 0;\n\n";

}

out << "\n"
    << "_m.clear();\n"
    << "\n"
    << "return;\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

   //
   //  read
   //

do_source_read(out, symtab);

   //
   //  machine symbol table dump
   //

do_st_dump(out);

   //
   //  member functions for symbols
   //

for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);

   do_member_for_symbol(out, entry);

}



   //
   //  done
   //

out << "\n\n";

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_source_read(ostream & out, const SymbolTable & symtab)

{

int j;
const SymbolTableEntry * entry = (const SymbolTableEntry *) 0;


out << "\n\n"
    << "void " << ClassName << "::read(const char * _config_filename)\n"
    << "\n"
    << "{\n"
    << "\n";

if ( AllowMultipleReads )  out << "//  clear();   //  allow multiple reads\n";
else                       out << "clear();\n";

out << "\n";

if ( Panic )  out << "const SymbolTableEntry * _e = (const SymbolTableEntry *) 0;\n";

out << "\n"
    << "   //\n"
    << "   //  read the config file into the machine\n"
    << "   //\n"
    << "\n"
    << "_m.read(_config_filename);\n"
    << "\n";


out << "   //\n"
    << "   //  lookup the entries in the symbol table\n"
    << "   //\n"
    << "\n";

for (j=0; j<(symtab.n_entries()); ++j)  {

   entry = symtab.entry(j);


   if ( Panic )  {

      out << "_e = _m.find(\"" << (entry->name) << "\");\n"
          << "\n"
          << "if ( !_e )  {\n"
          << "\n"
          << "   mlog << Error << \"\\n" << ClassName << "::read(const char *) -> can't get symbol table entry for variable \\\"" << (entry->name) << "\\\"\\n\\n\";\n"
          << "\n"
          << "   exit ( 1 );\n"
          << "\n"
          << "}\n"
          << "\n"
          << "_" << (entry->name) << "_entry = _e;\n"
          << "\n\n";

   } else {

      out << '_' << (entry->name) << "_entry = _m.find(\"" << (entry->name) << "\");\n\n";

   }

}


out << "\n"
    << "   //\n"
    << "   //  done\n"
    << "   //\n"
    << "\n"
    << "return;\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_st_dump(ostream & out)

{


out << "\n\n"
    << "void " << ClassName << "::st_dump(ostream & _out, int _depth) const\n"
    << "\n"
    << "{\n"
    << "\n"
    << "_m.st_dump(_out, _depth);\n"
    << "\n"
    << "return;\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_member_for_symbol(ostream & out, const SymbolTableEntry * entry)

{


switch ( entry->type )  {

   case ste_integer:
      do_integer_member(out, entry);
      break;


   case ste_double:
      do_double_member(out, entry);
      break;


   case ste_variable:
      do_variable_member(out, entry);
      break;


   case ste_pwl:
      do_pwl_member(out, entry);
      break;


   case ste_function:
      do_function_member(out, entry);
      break;


   case ste_array:
      do_array_member(out, entry);
      do_array_nelements_member(out, entry);
      break;


   default:
      mlog << Error << "\nCodeGenerator::do_member_for_symbol() -> "
           << "don't know how to generate code for symbols of type \""
           << stetype_to_string(entry->type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch







return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_integer_member(ostream & out, const SymbolTableEntry * entry)

{


out << "\n\n"
    << "int " << ClassName << "::" << (entry->name) << "()\n"
    << "\n"
    << "{\n"
    << "\n"
    << "return ( " << (entry->i) << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_double_member(ostream & out, const SymbolTableEntry * entry)

{


out << "\n\n"
    << "double " << ClassName << "::" << (entry->name) << "()\n"
    << "\n"
    << "{\n"
    << "\n"
    << "return ( " << (entry->d) << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_variable_member(ostream & out, const SymbolTableEntry * entry)

{

out << "\n\n"
    << result << " " << ClassName << "::" << (entry->name) << "()\n"
    << "\n"
    << "{\n"
    << "\n";

out << "if ( !_" << (entry->name) << "_entry )  {\n"
    << "\n"
    << "   mlog << Error << \"\\n" << ClassName << "::" << (entry->name) << "() -> no symbol table entry found for variable \\\"" << (entry->name) << "\\\"!\\n\\n\";\n"
    << "\n"
    << "   exit ( 1 );\n"
    << "\n"
    << "}\n"
    << "\n";

out << "\n"
    << result << " " << temp_result_varname << ";\n"
    << "IcodeCell " << temp_cell_varname << ";\n"
    << "\n";

out << "_m.run( *_" << (entry->name) << "_entry );\n"
    << "\n";

out << temp_cell_varname << " = _m.pop();\n"
    << "\n";

out << "icodecell_to_result(" << temp_cell_varname << ", " << temp_result_varname << ");\n";

out << "\n\n"
    << "return ( " << temp_result_varname << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;

return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_pwl_member(ostream & out, const SymbolTableEntry * entry)

{

out << "\n\n"
    << result << " " << ClassName << "::" << (entry->name) << "(const " << result << " & _r)\n"
    << "\n"
    << "{\n"
    << "\n";

out << "if ( !_" << (entry->name) << "_entry )  {\n"
    << "\n"
    << "   mlog << Error << \"\\n" << ClassName << "::" << (entry->name) << "() -> no symbol table entry found for piecewise-linear function \\\"" << (entry->name) << "\\\"!\\n\\n\";\n"
    << "\n"
    << "   exit ( 1 );\n"
    << "\n"
    << "}\n"
    << "\n";

out << "\n"
    << result << " " << temp_result_varname << ";\n"
    << "double _x, _y;\n"
    << "const PiecewiseLinear & _pwl_f = *(_" << (entry->name) << "_entry->pl);\n"
    << "\n\n";


out << "_x = _r.dval();\n\n"
    << "_y = _pwl_f(_x);\n\n";

out << temp_result_varname << ".set_double(_y);\n\n";

out << "return ( " << temp_result_varname << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_function_member(ostream & out, const SymbolTableEntry * entry)

{

int j;
const int nvars = entry->local_vars->n_elements();


out << "\n\n"
    << result << " " << ClassName << "::" << (entry->name) << "(";

for (j=0; j<nvars; ++j)  {

   out << "const " << result << " & _r_" << j;

   if ( j != (nvars - 1) )  out << ", ";

}

out << ")\n"
    << "\n"
    << "{\n"
    << "\n";

out << "if ( !_" << (entry->name) << "_entry )  {\n"
    << "\n"
    << "   mlog << Error << \"\\n" << ClassName << "::" << (entry->name) << "() -> no symbol table entry found for function \\\"" << (entry->name) << "\\\"!\\n\\n\";\n"
    << "\n"
    << "   exit ( 1 );\n"
    << "\n"
    << "}\n"
    << "\n";

out << "\n"
    << result << " " << temp_result_varname << ";\n"
    << "IcodeCell " << temp_cell_varname << ";\n"
    << "\n";

   //
   //  push the arguments onto the stack
   //

out << "\n"
    << "   //\n"
    << "   //  push the arguments onto the stack\n"   //  didn't we just say that?
    << "   //\n"
    << "\n";

for (j=0; j<nvars; ++j)  {

   out << "result_to_icodecell(_r_" << j << ", " << temp_cell_varname << ");\n"
       << "\n"
       << "_m.push(" << temp_cell_varname << ");\n";

   if ( j != (nvars - 1) )  out << "\n";

}


out << "\n"
    << "   //\n"
    << "   //  run the program for the function\n"
    << "   //\n"
    << "\n";


out << "_m.run( *_" << (entry->name) << "_entry );\n";

out << "\n"
    << "   //\n"
    << "   //  get the function's return value\n"
    << "   //\n"
    << "\n";

out << temp_cell_varname << " = _m.pop();\n"
    << "\n";

out << "icodecell_to_result(" << temp_cell_varname << ", " << temp_result_varname << ");\n";

out << "\n"
    << "   //\n"
    << "   //  done\n"
    << "   //\n";

out << "\n"
    << "return ( " << temp_result_varname << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_array_member(ostream & out, const SymbolTableEntry * entry)

{

int j;
const ArrayInfo * a = entry->ai;
const int nvars = a->dim();


out << "\n\n"
    << result << " " << ClassName << "::" << (entry->name) << "(";

for (j=0; j<nvars; ++j)  {

   out << "int _i" << j;

   if ( j != (nvars - 1) )  out << ", ";

}

out << ")\n"
    << "\n"
    << "{\n"
    << "\n";

out << "if ( !_" << (entry->name) << "_entry )  {\n"
    << "\n"
    << "   mlog << Error << \"\\n" << ClassName << "::" << (entry->name) << "() -> no symbol table entry found for array \\\"" << (entry->name) << "\\\"!\\n\\n\";\n"
    << "\n"
    << "   exit ( 1 );\n"
    << "\n"
    << "}\n"
    << "\n";

out << "\n"
    << result << " " << temp_result_varname << ";\n"
    << "IcodeCell " << temp_cell_varname << ";\n"
    << "const IcodeVector * _v = (const IcodeVector *) 0;\n"
    << "int _indices[max_array_dim];\n"
    << "const ArrayInfo * _a = _" << (entry->name) << "_entry->ai;\n"
    << "\n";

out << "\n"
    << "   //\n"
    << "   //  load up the indices\n"
    << "   //\n"
    << "\n";

for (j=0; j<nvars; ++j)  {

   out << "_indices[" << j << "] = _i" << j << ";\n";

}

   //
   //  the ArrayInfo::get() function will do range checking on the indices
   //

out << "\n\n"
    << "_v = _a->get(_indices);\n"
    << "\n";


out << "\n"
    << "_m.run( *_v );\n"
    << "\n";

out << "\n"
    << temp_cell_varname << " = _m.pop();\n"
    << "\n";

out << "icodecell_to_result(" << temp_cell_varname << ", " << temp_result_varname << ");\n";

out << "\n"
    << "   //\n"
    << "   //  done\n"
    << "   //\n";

out << "\n"
    << "return ( " << temp_result_varname << " );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


void CodeGenerator::do_array_nelements_member(ostream & out, const SymbolTableEntry * entry)

{

const ArrayInfo * a = entry->ai;
const int nvars = a->dim();

out << "\n\n"
    << "int " << ClassName << "::n_" << (entry->name) << "_elements(";

if ( nvars > 1 )  out << "int _i";

out << ")\n"
    << "\n"
    << "{\n"
    << "\n";

out << "if ( !_" << (entry->name) << "_entry )  {\n"
    << "\n"
    << "   mlog << Error << \"\\n" << ClassName << "::" << (entry->name) << "() -> no symbol table entry found for array n_elements function \\\"" << (entry->name) << "\\\"!\\n\\n\";\n"
    << "\n"
    << "   exit ( 1 );\n"
    << "\n"
    << "}\n"
    << "\n";

out << "\n"
    << "int _n;\n"
    << "const ArrayInfo * _a = _" << (entry->name) << "_entry->ai;\n"
    << "\n";


   //
   //  the ArrayInfo::size() function will do range checking for us
   //

out << "\n"
    << "_n = _a->size(";

if ( nvars > 1 )  out << "_i";
else              out << "0";

out << ");\n"
    << "\n";


out << "\n"
    << "   //\n"
    << "   //  done\n"
    << "   //\n";

out << "\n"
    << "return ( _n );\n"
    << "\n"
    << "}\n"
    << "\n\n"
    << sep;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void insert_copyright(ofstream & out)

{

ifstream in;
char c;
char path[1024];

snprintf(path, sizeof(path), "%s/%s", MET_BASE_DIR, copyright_filename);

in.open(path);

if ( !in )  {

   cerr << "\ninsert_copyright(ofstream &) -> unable to open copyright file \""
        << path << "\"\n\n";

   exit ( 1 );

}

while ( in.get(c) )  out.put(c);

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////






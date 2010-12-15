

////////////////////////////////////////////////////////////////////////


#ifndef  __ECONFIG_CODE_GENERATOR_H__
#define  __ECONFIG_CODE_GENERATOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "machine.h"

#include "cal.h"


////////////////////////////////////////////////////////////////////////


class CodeGenerator {

   private:

      void init_from_scratch();

      void assign(const CodeGenerator &);

      CodeGenerator(const CodeGenerator &);
      CodeGenerator & operator=(const CodeGenerator &);

      void set_config_filename(const char *);

      void set_string_value(char * &, const char *);

      void do_header();
      void do_header_array_dec       (ostream &, const SymbolTableEntry *);
      void do_header_function_dec    (ostream &, const SymbolTableEntry *);
      void do_header_pwl_dec         (ostream &, const SymbolTableEntry *);

      void do_source();
      void do_source_read            (ostream &, const SymbolTable &);
      void do_st_dump                (ostream &);
      void do_member_for_symbol      (ostream &, const SymbolTableEntry *);
      void do_integer_member         (ostream &, const SymbolTableEntry *);
      void do_double_member          (ostream &, const SymbolTableEntry *);
      void do_variable_member        (ostream &, const SymbolTableEntry *);
      void do_pwl_member             (ostream &, const SymbolTableEntry *);
      void do_function_member        (ostream &, const SymbolTableEntry *);
      void do_array_member           (ostream &, const SymbolTableEntry *);
      void do_array_nelements_member (ostream &, const SymbolTableEntry *);

      void warning(ostream &);


      Machine machine;

      char * ClassName;

      char * FilePrefix;

      char * ConfigFileName;

      const char * sep;

      Unixtime GenerationTime;

      const char * zone_name;   //  not allocated

      bool HH;

      bool Panic;

   public:

      CodeGenerator();
     ~CodeGenerator();

      void clear();

      void set_class_name(const char *);

      void set_file_prefix(const char *); 

      void set_hh();

      void set_nopanic();

      void process(const char * config_filename);

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __ECONFIG_CODE_GENERATOR_H__  */


////////////////////////////////////////////////////////////////////////




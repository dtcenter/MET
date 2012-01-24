

////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_SYMTAB_H__
#define  __CONFIG_SYMTAB_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


#ifndef  __ICODE_H__

#include "icode.h"

#endif


#include "array.h"
#include "pwl.h"
#include "idstack.h"
#include "builtin.h"


////////////////////////////////////////////////////////////////////////


class IcodeVector;   //  forward reference


////////////////////////////////////////////////////////////////////////


static const int symtab_alloc_jump       = 100;

static const int symtab_stack_alloc_jump = 100;


////////////////////////////////////////////////////////////////////////


enum SteType {


   ste_integer, 

   ste_double, 


   ste_variable, 

   ste_pwl,

   ste_function,

   ste_array, 

   no_ste_type

};


////////////////////////////////////////////////////////////////////////


class SymbolTable;   //  forward reference


////////////////////////////////////////////////////////////////////////


class SymbolTableEntry {

   private:

      void init_from_scratch();

      void assign(const SymbolTableEntry &);


      void assign_variable(const SymbolTableEntry &);

      void assign_function(const SymbolTableEntry &);

      void assign_pwl(const SymbolTableEntry &);

      void assign_array(const SymbolTableEntry &);




      void set_icodevector(const IcodeVector &);

      void set_pwl(const PiecewiseLinear &);

      void set_symboltable(const SymbolTable &);




   public:

      SymbolTableEntry();
     ~SymbolTableEntry();
      SymbolTableEntry(const SymbolTableEntry &);
      SymbolTableEntry & operator=(const SymbolTableEntry &);



      int i;

      double d;

      IcodeVector * v;               //  allocated if needed

      SymbolTable * st;              //  allocated if needed

      PiecewiseLinear * pl;          //  allocated if needed

      IdentifierArray * local_vars;  //  allocated if needed

      ArrayInfo * ai;                //  allocated if needed

      char * text;                   //  allocated if needed



      char * name;                   //  allocated ... symbol table entries ALWAYS have a name



      SteType type;



      void clear();

      void dump(ostream &, int depth = 0) const;


      void set_name(const char *);
      void set_text(const char *);


      void set_integer (const char *, int);
      void set_double  (const char *, double);

      void set_variable(const char *, const IcodeVector &);

      void set_pwl     (const char *, const PiecewiseLinear &);

      void set_function(const char *, const IdentifierArray &, const IcodeVector &);


};


////////////////////////////////////////////////////////////////////////


class SymbolTable {

   private:

      void init_from_scratch();

      void assign(const SymbolTable &);

   protected:

      SymbolTableEntry **Entry;

      int Nentries;

      int Nalloc;

      void extend(int);

      void algebraic_dump_entry(ostream &, const SymbolTableEntry *) const;

      void algebraic_dump_variable(ostream &, const SymbolTableEntry *) const;
      void algebraic_dump_pwl(ostream &, const SymbolTableEntry *)      const;
      void algebraic_dump_function(ostream &, const SymbolTableEntry *) const;

      void algebraic_dump_array(ostream &, const SymbolTableEntry *) const;

      void algebraic_dump_icv(ostream &, const IcodeVector &, const IdentifierArray *) const;

   public:

      SymbolTable();
     ~SymbolTable();
      SymbolTable(const SymbolTable &);
      SymbolTable & operator=(const SymbolTable &);

      SymbolTableEntry * find(const char *) const;

      const SymbolTableEntry * entry(int) const;

      int n_entries() const;

      void store(const SymbolTableEntry &);

      void store_as_int   (const char *,    int);
      void store_as_double(const char *, double);

      void remove(const SymbolTableEntry *);

      void clear();

      void dump(ostream &, int depth = 0) const;

      void algebraic_dump(ostream &) const;

};


////////////////////////////////////////////////////////////////////////


inline int SymbolTable::n_entries() const { return ( Nentries ); }


////////////////////////////////////////////////////////////////////////


class SymbolTableStack {

   private:

      void init_from_scratch();

      void assign(const SymbolTableStack &);

   protected:

      SymbolTable **Table;   //  Only the first table is allocated

      int Ntables;

      int Nalloc;

      void extend(int);

   public:

      SymbolTableStack();
     ~SymbolTableStack();

      SymbolTableStack(const SymbolTableStack &);
      SymbolTableStack & operator=(const SymbolTableStack &);

      void clear();

      void push(SymbolTable *);

      void pop();

      SymbolTableEntry * find(const char *) const;

      const SymbolTable & table(int) const;


      int n_tables() const;

      void store(const SymbolTableEntry &);   //  stores entry into top-level symbol table

      void store_as_int   (const char *,    int);
      void store_as_double(const char *, double);

      void dump(ostream &, int depth = 0) const;

      void algebraic_dump(ostream &) const;

};


////////////////////////////////////////////////////////////////////////


inline int SymbolTableStack::n_tables() const { return ( Ntables ); }


////////////////////////////////////////////////////////////////////////


extern void strip_trailing_zeroes(char *);

extern void translate_string(ostream & out, const char * text);


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_SYMTAB_H__  */


////////////////////////////////////////////////////////////////////////





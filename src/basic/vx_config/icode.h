

////////////////////////////////////////////////////////////////////////


#ifndef  __CALCULATOR_ICODE_H__
#define  __CALCULATOR_ICODE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class IcodeCell;     //  forward reference
class IcodeVector;   //  forward reference


////////////////////////////////////////////////////////////////////////


#include "scanner_stuff.h"



#ifndef  __CONFIG_SYMTAB_H__

// #include "symtab.h"

#endif


////////////////////////////////////////////////////////////////////////


// class SymbolTableEntry;   //  forward reference


////////////////////////////////////////////////////////////////////////


enum CellType {

   integer, 

   floating_point, 

   boolean, 

   cell_mark, 

   op_add, 
   op_multiply, 
   op_divide, 
   op_subtract, 

   op_power, 
   op_square, 

   op_negate, 

   op_store, 
   op_recall, 

   identifier, 

   user_func, 
   builtin_func, 

   character_string,


   no_cell_type

};


////////////////////////////////////////////////////////////////////////


class IcodeCell {

   private:

      void init_from_scratch();

      void assign(const IcodeCell &);

   public:

      IcodeCell();
     ~IcodeCell();
      IcodeCell(const IcodeCell &);
      IcodeCell & operator=(const IcodeCell &);

      void clear();

      void set_integer (int);
      void set_double  (double);
      void set_boolean (bool);

      void set_identifier(const char *);

      void set_builtin(int);

      void set_string(const char *);


      int val;

      double d;

      char * name;   //  allocated if needed
      char * text;   //  allocated if needed

      CellType type;

      int is_numeric() const;

      int is_mark() const;

      int as_int() const;

      double as_double() const;

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int IcodeCell::is_numeric() const { return ( (type == integer) || (type == floating_point) ); }


////////////////////////////////////////////////////////////////////////


class IcodeVector {

   private:

      IcodeCell * Cell;

      int Ncells;

      int Nalloc;

      void assign(const IcodeVector &);

      void init_from_scratch();

      void extend(int);

   public:

      IcodeVector();
     ~IcodeVector();
      IcodeVector(const IcodeVector &);
      IcodeVector & operator=(const IcodeVector &);

      void clear();

      const IcodeCell & operator[](int) const;

      const IcodeCell & cell(int) const;

      int length() const;

      void add(const IcodeCell &);

      void add(const IcodeVector &);

      void dump(ostream &, int depth = 0) const;

      int is_mark() const;

};


////////////////////////////////////////////////////////////////////////


inline int IcodeVector::length() const { return ( Ncells ); }


////////////////////////////////////////////////////////////////////////


static const int cell_stack_size = 50;


////////////////////////////////////////////////////////////////////////


class CellStack {

   private:

      IcodeCell cell [cell_stack_size];

      int Depth;

      void assign(const CellStack &);

   public:

      CellStack();
     ~CellStack();
      CellStack(const CellStack &);
      CellStack & operator=(const CellStack &);

      void push(const IcodeCell &);

      IcodeCell pop();

      const IcodeCell & peek() const;

      CellType peek_cell_type() const;

      void clear();

      int depth() const;

      void dump_cell(ostream &, int n, int depth = 0) const;

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int CellStack::depth() const { return ( Depth ); }


////////////////////////////////////////////////////////////////////////


static const int icv_stack_size = 30;


////////////////////////////////////////////////////////////////////////


class ICVStack {

   private:

      IcodeVector * v [icv_stack_size];

      int Depth;

      void assign(const ICVStack &);

   public:

      ICVStack();
     ~ICVStack();
      ICVStack(const ICVStack &);
      ICVStack & operator=(const ICVStack &);

      void push(const IcodeVector &);

      IcodeVector pop();

      IcodeVector * peek();

      int depth() const;

      void clear();

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int ICVStack::depth() const { return ( Depth ); }


////////////////////////////////////////////////////////////////////////


class ICVQueue {

   private:

      IcodeVector * v [icv_stack_size];

      int Nelements;

      void assign(const ICVQueue &);

      void init_from_scratch();

   public:

      ICVQueue();
     ~ICVQueue();
      ICVQueue(const ICVQueue &);
      ICVQueue & operator=(const ICVQueue &);

      void push(const IcodeVector &);

      IcodeVector pop();

      int n_elements() const;

      void clear();

};


////////////////////////////////////////////////////////////////////////


inline int ICVQueue::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


static const int icv_array_size = 30;


class ICVArray {

   private:

      IcodeVector v [icv_array_size];

      int Nelements;

      void assign(const ICVArray &);

      void init_from_scratch();

   public:

      ICVArray();
     ~ICVArray();
      ICVArray(const ICVArray &);
      ICVArray & operator=(const ICVArray &);


      int n_elements() const;

      void clear();


      IcodeVector & operator[](int);

      void add(const IcodeVector &);

};


////////////////////////////////////////////////////////////////////////


inline int ICVArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CALCULATOR_ICODE_H__  */


////////////////////////////////////////////////////////////////////////



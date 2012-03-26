

////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_DICTIONARY_H__
#define  __CONFIG_DICTIONARY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "object_types.h"

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


class Dictionary;   //  forward reference


////////////////////////////////////////////////////////////////////////


class DictionaryEntry {

      friend class Dictionary;

   private:

      void init_from_scratch();

      void assign(const DictionaryEntry &);



      ConfigObjectType Type;

      ConcatString Name;

      int Ival;
      double Dval;
      bool Bval;

      ConcatString * Text;   //  allocated

      Dictionary * Dict;     //  allocated


   public:

      DictionaryEntry();
     ~DictionaryEntry();
      DictionaryEntry(const DictionaryEntry &);
      DictionaryEntry & operator=(const DictionaryEntry &);

      void clear();

      void dump(ostream & out, int = 0) const;

         //
         //  set stuff
         //

      void set_int        (const char * _name, int);
      void set_double     (const char * _name, double);
      void set_boolean    (const char * _name, bool);
      void set_string     (const char * _name, const char * _text);
      void set_dictionary (const char * _name, const Dictionary &);

      void set_name       (const char *);

         //
         //  get stuff
         //

      ConfigObjectType type() const;

      ConcatString name() const;

      int     i_value() const;
      double  d_value() const;
      bool    b_value() const;

      const ConcatString * string_value () const;

      // const Dictionary * dictionary_value () const;
      Dictionary * dictionary_value () const;

         //
         //  do stuff
         //

      bool is_number     () const;
      bool is_dictionary () const;

};


////////////////////////////////////////////////////////////////////////


inline ConfigObjectType DictionaryEntry::type() const { return ( Type ); }

inline ConcatString DictionaryEntry::name() const { return ( Name ); }

inline bool DictionaryEntry::is_number() const { return ( (Type == IntegerType) || (Type == FloatType) ); }

inline bool DictionaryEntry::is_dictionary() const { return ( Type == DictionaryType ); }


////////////////////////////////////////////////////////////////////////


static const int dictionary_alloc_inc = 100;


////////////////////////////////////////////////////////////////////////


class Dictionary {

   private:

      void init_from_scratch();

      void assign(const Dictionary &);

      void extend(int);


      int Nentries;

      int Nalloc;

      DictionaryEntry ** e;   // allocated

      const Dictionary * Parent;   //  not allocated

   public:

      Dictionary();
     ~Dictionary();
      Dictionary(const Dictionary &);
      Dictionary & operator=(const Dictionary &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_parent(const Dictionary *);

         //
         //  get stuff
         //

      int n_entries() const;

      const DictionaryEntry * operator[](int) const;

      const Dictionary * parent() const;

         //
         //  do stuff
         //

      void store(const DictionaryEntry &);

      void store(const Dictionary &);

      const DictionaryEntry * lookup(const char * name) const;

};


////////////////////////////////////////////////////////////////////////


inline int Dictionary::n_entries() const { return ( Nentries ); }


////////////////////////////////////////////////////////////////////////


static const int max_dictionary_depth = 50;   //  should be plenty big enough


////////////////////////////////////////////////////////////////////////


class DictionaryStack {

   private:

      void init_from_scratch();

      void assign(const DictionaryStack &);

      DictionaryStack();
      DictionaryStack(const DictionaryStack &);
      DictionaryStack & operator=(const DictionaryStack &);

        //
        //  all but the first one is allocated
        //

      Dictionary * D [max_dictionary_depth];

      int Nelements;

   public:

      DictionaryStack(Dictionary &);
     ~DictionaryStack();

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

         //
         //  get stuff
         //

     int n_elements() const;

     // const Dictionary * top() const;

         //
         //  do stuff
         //

      void push();

      void pop(const char * name);

      void store(const DictionaryEntry &);

      const DictionaryEntry * lookup(const char * name) const;



};


////////////////////////////////////////////////////////////////////////


inline int DictionaryStack::n_elements () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_DICTIONARY_H__  */


////////////////////////////////////////////////////////////////////////



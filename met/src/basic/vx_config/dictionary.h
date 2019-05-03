// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __CONFIG_DICTIONARY_H__
#define  __CONFIG_DICTIONARY_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "object_types.h"

#include "num_array.h"
#include "int_array.h"
#include "threshold.h"
#include "thresh_array.h"
#include "vx_cal.h"
#include "vx_log.h"
#include "pwl.h"


////////////////////////////////////////////////////////////////////////


static const bool default_dictionary_error_out = true;


////////////////////////////////////////////////////////////////////////


class Dictionary;        //  forward reference


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

      ConcatString * Text;           //  allocated

      Dictionary * Dict;             //  allocated
                                     //  also used for arrays

      SingleThresh * Thresh;         //  allocated

      PiecewiseLinear * PWL;         //  allocated

   public:

      DictionaryEntry();
     ~DictionaryEntry();
      DictionaryEntry(const DictionaryEntry &);
      DictionaryEntry & operator=(const DictionaryEntry &);

      void clear();

      void dump(ostream & out, int = 0) const;

      void dump_config_format(ostream & out, int = 0) const;

         //
         //  set stuff
         //

      void set_int          (const char * _name, int);
      void set_double       (const char * _name, double);
      void set_boolean      (const char * _name, bool);
      void set_string       (const char * _name, const char * _text);
      void set_dict         (const char * _name, const Dictionary &);
      void set_array        (const char * _name, const Dictionary &);
      void set_threshold    (const char * _name, const SingleThresh &);
      void set_pwl          (const char * _name, const PiecewiseLinear &);

      void set_name         (const char *);

         //
         //  get stuff
         //

      ConfigObjectType type() const;

      ConcatString name() const;

      int     i_value() const;
      double  d_value() const;
      bool    b_value() const;

      const ConcatString * string_value () const;

      Dictionary * dict_value () const;

      Dictionary * array_value () const;

      SingleThresh * thresh_value() const;

      PiecewiseLinear * pwl_value() const;

         //
         //  do stuff
         //

      bool is_number     () const;
      bool is_dictionary () const;
      bool is_array      () const;

};


////////////////////////////////////////////////////////////////////////


inline ConfigObjectType DictionaryEntry::type() const { return ( Type ); }

inline ConcatString DictionaryEntry::name() const { return ( Name ); }

inline bool DictionaryEntry::is_number() const { return ( (Type == IntegerType) || (Type == FloatType) ); }

inline bool DictionaryEntry::is_dictionary() const { return ( Type == DictionaryType ); }

inline bool DictionaryEntry::is_array() const { return ( Type == ArrayType ); }


////////////////////////////////////////////////////////////////////////


static const int dictionary_alloc_inc = 100;

static const bool default_dict_error_out = true;


////////////////////////////////////////////////////////////////////////


class Dictionary {

   protected:

      void init_from_scratch();

      void assign(const Dictionary &);

      void extend(int);

      void patch_parents();

      virtual const DictionaryEntry * lookup_simple(const char * name);   //  no scope


      int Nentries;

      int Nalloc;

      bool IsArray;

      DictionaryEntry ** e;   // allocated

      Dictionary * Parent;   //  not allocated

      bool LastLookupStatus;      

   public:

      Dictionary();
      virtual ~Dictionary();
      Dictionary(const Dictionary &);
      Dictionary & operator=(const Dictionary &);

      void clear();

      virtual void dump(ostream &, int = 0) const;

      virtual void dump_config_format(ostream & out, int = 0) const;

         //
         //  set stuff
         //

      void set_parent(Dictionary *);

      void set_is_array(bool = true);

         //
         //  get stuff
         //

      virtual int n_entries() const;

      virtual const DictionaryEntry * operator[](int) const;

      virtual const Dictionary * parent() const;

      virtual bool is_array() const;

      bool last_lookup_status () const;      

         //
         //  do stuff
         //

      virtual void store(const DictionaryEntry &);

      virtual void store(const Dictionary &);

      virtual const DictionaryEntry * lookup(const char * name);

         //
         //  convenience functions
         //

      bool         lookup_bool           (const char * name, bool error_out = default_dictionary_error_out);
      int          lookup_int            (const char * name, bool error_out = default_dictionary_error_out);
      double       lookup_double         (const char * name, bool error_out = default_dictionary_error_out);
      NumArray     lookup_num_array      (const char * name, bool error_out = default_dictionary_error_out);
      IntArray     lookup_int_array      (const char * name, bool error_out = default_dictionary_error_out);
      ConcatString lookup_string         (const char * name, bool error_out = default_dictionary_error_out);
      StringArray  lookup_string_array   (const char * name, bool error_out = default_dictionary_error_out);
      SingleThresh lookup_thresh         (const char * name, bool error_out = default_dictionary_error_out);
      ThreshArray  lookup_thresh_array   (const char * name, bool error_out = default_dictionary_error_out);
      int          lookup_seconds        (const char * name, bool error_out = default_dictionary_error_out);
      IntArray     lookup_seconds_array  (const char * name, bool error_out = default_dictionary_error_out);
      unixtime     lookup_unixtime       (const char * name, bool error_out = default_dictionary_error_out);
      TimeArray    lookup_unixtime_array (const char * name, bool error_out = default_dictionary_error_out);

         //
         //  return value not allocated
         //

      Dictionary *      lookup_dictionary (const char * name, bool error_out = default_dictionary_error_out);
      Dictionary *      lookup_array      (const char * name, bool error_out = default_dictionary_error_out);
      PiecewiseLinear * lookup_pwl        (const char * name, bool error_out = default_dictionary_error_out);
};


////////////////////////////////////////////////////////////////////////


inline int Dictionary::n_entries() const { return ( Nentries ); }

inline const Dictionary * Dictionary::parent() const { return ( Parent ); }

inline void Dictionary::set_is_array(bool __tf) { IsArray = __tf;  return; }

inline bool Dictionary::is_array() const { return ( IsArray ); }

inline bool Dictionary::last_lookup_status() const { return ( LastLookupStatus ); }


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

      void dump_config_format(ostream & out, int = 0) const;

         //
         //  set stuff
         //

      void set_top_is_array(bool);

         //
         //  get stuff
         //

      int n_elements() const;

      const Dictionary * top() const;

      bool top_is_array() const;

         //
         //  do stuff
         //

      void push();
      void push_array();

      void pop_dict    (const char * name);

      void pop_element (const char * name);

      void erase_top();

      void store(const DictionaryEntry &);

      const DictionaryEntry * lookup(const char * name) const;

};


////////////////////////////////////////////////////////////////////////


inline int DictionaryStack::n_elements () const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CONFIG_DICTIONARY_H__  */


////////////////////////////////////////////////////////////////////////



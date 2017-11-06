

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_CONFIG_FUNCTION_INTERFACES_H__
#define  __VX_CONFIG_FUNCTION_INTERFACES_H__


////////////////////////////////////////////////////////////////////////


#include "is_number.h"
#include "dictionary.h"
#include "icode.h"


////////////////////////////////////////////////////////////////////////


class UserFunc_1Arg {

   private:

      void init_from_scratch();

      void assign(const UserFunc_1Arg &);


      const DictionaryEntry * entry;   //  not allocated

      const IcodeVector * v;           //  not allocated

   public:

      UserFunc_1Arg();
     ~UserFunc_1Arg();
      UserFunc_1Arg(const UserFunc_1Arg &);
      UserFunc_1Arg & operator=(const UserFunc_1Arg &);

         //
         //  set stuff
         //

      void set(const DictionaryEntry *);

         //
         //  get stuff
         //

      ConcatString name() const;

      int n_args () const;   //  should be 1

      const IcodeVector * program() const;

         //
         //  do stuff
         //


      double operator()(double) const;


};


////////////////////////////////////////////////////////////////////////


inline ConcatString UserFunc_1Arg::name() const { return ( entry ? (entry->name()) : ConcatString("(nul)") ); }

inline int UserFunc_1Arg::n_args() const { return ( entry ? (entry->n_args()) : (-1) ); }

inline const IcodeVector * UserFunc_1Arg::program() const { return ( v ); }


////////////////////////////////////////////////////////////////////////


class UserFunc_MultiArg {

   private:

      void init_from_scratch();

      void assign(const UserFunc_MultiArg &);


      const DictionaryEntry * entry;   //  not allocated

      const IcodeVector * v;           //  not allocated

   public:

      UserFunc_MultiArg();
     ~UserFunc_MultiArg();
      UserFunc_MultiArg(const UserFunc_MultiArg &);
      UserFunc_MultiArg & operator=(const UserFunc_MultiArg &);

         //
         //  set stuff
         //

      void set(const DictionaryEntry *);

         //
         //  get stuff
         //

      ConcatString name() const;

      int n_args () const;

      const IcodeVector * program() const;

         //
         //  do stuff
         //

      Number operator()(const Number *) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_CONFIG_FUNCTION_INTERFACES_H__  */


////////////////////////////////////////////////////////////////////////



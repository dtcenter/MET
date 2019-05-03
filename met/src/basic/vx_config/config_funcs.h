

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

      ConcatString Name;
      int          NArgs;
      IcodeVector  V;

   public:

      UserFunc_1Arg();
     ~UserFunc_1Arg();
      UserFunc_1Arg(const UserFunc_1Arg &);
      UserFunc_1Arg & operator=(const UserFunc_1Arg &);

      void clear();

         //
         //  set stuff
         //

      void set(const DictionaryEntry *);

         //
         //  get stuff
         //

      ConcatString name() const;

      int n_args () const;   //  should be 1

      bool is_set() const;

      const IcodeVector * program() const;

         //
         //  do stuff
         //

      double operator()(double) const;

};


////////////////////////////////////////////////////////////////////////


inline ConcatString UserFunc_1Arg::name() const { return ( Name ); }

inline int UserFunc_1Arg::n_args() const { return ( NArgs ); }

inline bool UserFunc_1Arg::is_set() const { return ( NArgs >= 0 ); };

inline const IcodeVector * UserFunc_1Arg::program() const { return ( &V ); }


////////////////////////////////////////////////////////////////////////


class UserFunc_MultiArg {

   private:

      void init_from_scratch();

      void assign(const UserFunc_MultiArg &);

      ConcatString Name;
      int          NArgs;
      IcodeVector  V;

   public:

      UserFunc_MultiArg();
     ~UserFunc_MultiArg();
      UserFunc_MultiArg(const UserFunc_MultiArg &);
      UserFunc_MultiArg & operator=(const UserFunc_MultiArg &);

      void clear();

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



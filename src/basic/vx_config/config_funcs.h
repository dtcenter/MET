

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

      void set(const DictionaryEntry *);

      double operator()(double) const;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_CONFIG_FUNCTION_INTERFACES_H__  */


////////////////////////////////////////////////////////////////////////



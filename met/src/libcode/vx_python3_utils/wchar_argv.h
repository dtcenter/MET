

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __WCHAR_ARGV_H__
#define  __WCHAR_ARGV_H__


////////////////////////////////////////////////////////////////////////


#include "string_array.h"


////////////////////////////////////////////////////////////////////////


class Wchar_Argv {

   private:

      void init_from_scratch();

      // void assign(const Wchar_Argv &);

      Wchar_Argv(const Wchar_Argv &);
      Wchar_Argv & operator=(const Wchar_Argv &);


      wchar_t * W_Buf;        //  allocated

      wchar_t ** W_Argv;      //  allocated

      int Argc;

   public:

      Wchar_Argv();
     ~Wchar_Argv();

      void clear();

         //
         //  set stuff
         //

      void set(int _argc, char ** _argv);

      void set(const StringArray &);

         //
         //  get stuff
         //

      int wargc() const;

      wchar_t ** wargv() const;

         //
         //  do stuff
         //

};


////////////////////////////////////////////////////////////////////////


inline int Wchar_Argv::wargc() const { return ( Argc ); }

inline wchar_t ** Wchar_Argv::wargv() const { return ( W_Argv ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __WCHAR_ARGV_H__  */


////////////////////////////////////////////////////////////////////////


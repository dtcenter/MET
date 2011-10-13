

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __VERIF_COMMAND_LINE_H__
#define  __VERIF_COMMAND_LINE_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "string_array.h"


////////////////////////////////////////////////////////////////////////


typedef void (*CLSetFunction)(const StringArray &);   //  command-line set function

typedef void (*UsageFunction)();   //  usage function


////////////////////////////////////////////////////////////////////////


class CLOptionInfo {   //  command-line option info

   private:

      void init_from_scratch();

      void assign(const CLOptionInfo &);

   public:

      CLOptionInfo();
     ~CLOptionInfo();
      CLOptionInfo(const CLOptionInfo &);
      CLOptionInfo & operator=(const CLOptionInfo &);

      void clear();

      void dump(ostream &, int = 0) const;


      ConcatString option_text;

      int Nargs;

      CLSetFunction f;   //  not allocated

};


////////////////////////////////////////////////////////////////////////


class CLOptionInfoArray {

   private:

      void init_from_scratch();

      void assign(const CLOptionInfoArray &);

      void extend(int);


      int Nelements;

      int Nalloc;

      int AllocInc;

      CLOptionInfo * e;


   public:

      CLOptionInfoArray();
     ~CLOptionInfoArray();
      CLOptionInfoArray(const CLOptionInfoArray &);
      CLOptionInfoArray & operator=(const CLOptionInfoArray &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_alloc_inc(int = 0);   //  0 means default value (16)

      int n_elements() const;

      void add(const CLOptionInfo &);
      void add(const CLOptionInfoArray &);

      CLOptionInfo & operator[](int) const;

      int lookup(const char *) const;

};


////////////////////////////////////////////////////////////////////////


inline int CLOptionInfoArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


class CommandLine {

   private:

      void init_from_scratch();

      void assign(const CommandLine &);


      void do_help();

      StringArray args;

      CLOptionInfoArray options;

      ConcatString ProgramName;

      UsageFunction Usage;

      bool AllowNumbers;   //  default: true

   public:

      CommandLine();
     ~CommandLine();
      CommandLine(const CommandLine &);
      CommandLine & operator=(const CommandLine &);

      void clear();

      void dump(ostream &, int depth = 0) const;

         //
         //  set stuff
         //

      void set(int argc, char ** argv);   //  includes argv[0]

      void set_usage(UsageFunction);

      void set_allow_numbers(bool);

         //
         //  get stuff
         //

      int n() const;  //  # of elements

      int max_length() const;

      int length(int) const;   //  length of jth arg

      // bool has_option(int & index) const;

      int next_option() const;   //  -1 if no option found, else index into array

         //
         //  do stuff
         //

      void shift_down(int pos, int k);

      const char * operator[](int) const;

      void parse();

      void add(CLSetFunction, const char * text, int n_args);   //  n_args not including switch

};


////////////////////////////////////////////////////////////////////////


inline int CommandLine::n() const { return ( args.n_elements() ); }

inline int CommandLine::max_length() const { return ( args.max_length() ); }

inline void CommandLine::set_usage(UsageFunction f) { Usage = f;  return; }

inline void CommandLine::set_allow_numbers(bool tf) { AllowNumbers = tf;  return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VERIF_COMMAND_LINE_H__  */


////////////////////////////////////////////////////////////////////////



// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __COMMAND_LINE_H__
#define  __COMMAND_LINE_H__


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

      int Nargs;   //  -1 for arbitrary number

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

      int lookup(const string &) const;

};


////////////////////////////////////////////////////////////////////////


inline int CLOptionInfoArray::n_elements() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


class CommandLine {

   private:

      void init_from_scratch();

      void assign(const CommandLine &);


      void do_help() const;

      void show_version() const;

      void get_n_args(StringArray &, const int Nargs,
                      const string & switch_name, const int pos);

      int  get_unlimited_args(StringArray &, const int pos);

      StringArray args;

      CLOptionInfoArray options;

      ConcatString ProgramName;

      UsageFunction Usage;

      bool AllowNumbers;   //  default: false

      bool AllowUnrecognizedSwitches;   //  default: false

      bool is_switch(const string &) const;

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

      void allow_numbers();

      void allow_unrecognized_switches();

         //
         //  get stuff
         //

      int n() const;  //  # of elements

      int max_length() const;

      int length(int) const;   //  length of jth arg

      bool has_option(const string &) const;

      int next_option(int & index) const;   //  -1 if no option found

         //
         //  do stuff
         //

      void shift_down(int pos, int k);

      const string operator[](int) const;

      void parse();

      void add(CLSetFunction, const string & text, int n_args);   //  n_args not including switch

};


////////////////////////////////////////////////////////////////////////


inline int CommandLine::n() const { return ( args.n_elements() ); }

inline int CommandLine::max_length() const { return ( args.max_length() ); }

inline void CommandLine::set_usage(UsageFunction f) { Usage = f;  return; }

inline void CommandLine::allow_numbers() { AllowNumbers = true;  return; }

inline void CommandLine::allow_unrecognized_switches() { AllowUnrecognizedSwitches = true;  return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __COMMAND_LINE_H__  */


////////////////////////////////////////////////////////////////////////



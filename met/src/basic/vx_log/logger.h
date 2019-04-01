// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


//////////////////////////////////////////////////////////////////


#ifndef __LOGGER_H__
#define __LOGGER_H__


//////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>

#include "concat_string.h"
#include "indent.h"


//////////////////////////////////////////////////////////////////


class MsgLevel
{
   private:

         //
         // MsgLevel Value must be -1 or greater.
         // MsgLevel Value -1 implies an ERROR message and will be written to cerr.
         // MsgLevel Value 0 implies an WARNING message and will be written to cerr.
         // MsgLevel Value 1 or greater is a DEBUG message and will be written to cout.
         //
      int Value;

      void init_from_scratch();

      void assign(const MsgLevel &);

   public:

      MsgLevel();
      ~MsgLevel();
      MsgLevel(int);
      MsgLevel(const MsgLevel &);
      MsgLevel & operator=(const MsgLevel &);

      void clear();

         //
         // get stuff
         //

      int value() const;

         //
         // set stuff
         //

         //
         // do stuff
         //

      operator const int & () const;

};


//////////////////////////////////////////////////////////////////


inline int MsgLevel::value() const { return (Value); }

inline MsgLevel::operator const int & () const { return (Value); }


//////////////////////////////////////////////////////////////////


extern MsgLevel Global_Level;


//////////////////////////////////////////////////////////////////


class LoggerError {
   private:

   public:
      int err;

      LoggerError();
      ~LoggerError();
      LoggerError(const LoggerError &);
      LoggerError & operator=(const LoggerError &);

};


//////////////////////////////////////////////////////////////////


class LoggerWarning {
   private:

   public:
      int warn;
      bool ExitOnWarning;
      bool NeedToExit;

      LoggerWarning();
      ~LoggerWarning();
      LoggerWarning(const LoggerWarning &);
      LoggerWarning & operator=(const LoggerWarning &);

};


//////////////////////////////////////////////////////////////////


class LoggerDebug {
   private:

         //
         // LoggerDebug Value must be -1 or greater.
         // LoggerDebug Value -1 implies an ERROR message and will be written to cerr.
         // LoggerDebug Value 0 implies an WARNING message and will be written to cerr.
         // LoggerDebug Value 1 or greater is a DEBUG message and will be written to cout.
         //
      int Value;

      void init_from_scratch();

      void assign(const LoggerDebug &);

   public:

      LoggerDebug();
      ~LoggerDebug();
      LoggerDebug(int);
      LoggerDebug(const LoggerDebug &);
      LoggerDebug & operator=(const LoggerDebug &);

      void clear();

         //
         // get stuff
         //

      int value() const;

         //
         // set stuff
         //

         //
         // do stuff
         //

      operator int () const;

};


//////////////////////////////////////////////////////////////////


inline int LoggerDebug::value() const { return (Value); }

inline LoggerDebug::operator int () const { return (Value); }


//////////////////////////////////////////////////////////////////


class Logger
{
   friend MsgLevel & level(const int);
   // friend LoggerDebug & Debug(const int);
   friend MsgLevel & Debug(const int);

   private:

      Logger(const Logger &);
      Logger & operator=(const Logger &);

   protected:

      bool need_to_output_type;

      MsgLevel message_level;

         //
         // VerbosityLevel must be 0 or greater.
         // This is the level entered by the user on the command line.
         // If zero is given, then only ERROR and WARNING messages will
         // be output to cerr.
         // Values of 1 or more will print DEBUG messages to cout if the
         // message level is less than or equal to the verbosity level.
         // The verbosity level defaults to 2.
         //
      int VerbosityLevel;

         //
         // If LogFilename is set, then ERROR, WARNING, and DEBUG messages
         // will be written to the this output file also.
         //
      ConcatString LogFilename;

      std::ofstream * out;  // allocated

         //
         // do stuff
         //

      void init_from_scratch();

         // don't need since we don't have the copy constructor or assignment operator
//      void assign(const Logger &);

      void write_msg_type();

   public:

      Logger();
      ~Logger();

      void clear();

      void dump (std::ostream &, int = 1) const;

         //
         // get stuff
         //

      ConcatString log_filename() const;

      int verbosity_level() const;

      bool is_open() const;

         //
         // set stuff
         //

      void set_verbosity_level(const int);

      void set_exit_on_warning(bool);

         //
         // do stuff
         //

      void open_log_file(const ConcatString);

      Logger & operator<<(const char *);
      Logger & operator<<(const std::string);
      Logger & operator<<(const int);
      Logger & operator<<(const unsigned int);
      Logger & operator<<(const long);
      Logger & operator<<(const unsigned long);
      Logger & operator<<(const long long);
      Logger & operator<<(const unsigned long long);
      Logger & operator<<(const double);
      Logger & operator<<(const char);
      Logger & operator<<(const bool);
      Logger & operator<<(const Indent &);
      Logger & operator<<(const MsgLevel &);
      Logger & operator<<(const LoggerError);
      Logger & operator<<(const LoggerWarning);
      Logger & operator<<(const LoggerDebug);

};


//////////////////////////////////////////////////////////////////


inline ConcatString Logger::log_filename() const { return (LogFilename); }

inline int Logger::verbosity_level() const { return (VerbosityLevel); }

inline bool Logger::is_open() const { return (out != 0); }


//////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////


extern MsgLevel & level(const int);
extern MsgLevel & Debug(const int);
extern LoggerError Error;
extern LoggerWarning Warning;


//////////////////////////////////////////////////////////////////


   //
   // global instantiation of the logger class
   //
extern Logger mlog;


//////////////////////////////////////////////////////////////////


#endif  //  __LOGGER_H__


//////////////////////////////////////////////////////////////////



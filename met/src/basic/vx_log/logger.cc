// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


using namespace std;


//////////////////////////////////////////////////////////////////


#include "logger.h"


//////////////////////////////////////////////////////////////////


Logger mlog;


//////////////////////////////////////////////////////////////////


static const int MinimumMessageLevel   = -1;

static const int DefaultMessageLevel   = -1;

static const int DefaultVerbosityLevel =  2;

static const int ErrorMessageLevel     = -1;

static const int WarningMessageLevel   =  0;

static const bool DefaultExitOnWarning =  false;

   //
   //  these need external linkage, do not make static or extern
   //

MsgLevel Global_Level;

LoggerError Error;
LoggerWarning Warning;


//////////////////////////////////////////////////////////////////


   //
   // Code for class MsgLevel
   //


//////////////////////////////////////////////////////////////////


MsgLevel::MsgLevel()
{
   init_from_scratch();

}


//////////////////////////////////////////////////////////////////


MsgLevel::~MsgLevel()
{
   clear();

}


//////////////////////////////////////////////////////////////////


MsgLevel::MsgLevel(int i)
{
   init_from_scratch();

   if (i < MinimumMessageLevel)
   {
      cerr << "\n\n  void MsgLevel::MsgLevel(int i) -> The message level must be "
           << MinimumMessageLevel << " or greater\n\n";
      exit (1);
   }

   Value = i;

}


//////////////////////////////////////////////////////////////////


MsgLevel::MsgLevel(const MsgLevel & m)
{
   init_from_scratch();

   assign(m);

}


//////////////////////////////////////////////////////////////////


MsgLevel & MsgLevel::operator=(const MsgLevel & m)
{
   if (this == &m)
      return (*this);

   assign(m);

   return (*this);

}


//////////////////////////////////////////////////////////////////


void MsgLevel::init_from_scratch()
{
   clear();

}


//////////////////////////////////////////////////////////////////


void MsgLevel::clear()
{
   Value = DefaultMessageLevel;

}


//////////////////////////////////////////////////////////////////


void MsgLevel::assign(const MsgLevel & m)
{
   clear();

   Value = m.Value;

}


//////////////////////////////////////////////////////////////////


   //
   // Code for class LoggerError
   //


//////////////////////////////////////////////////////////////////


LoggerError::LoggerError()
{
   err = ErrorMessageLevel;

}


//////////////////////////////////////////////////////////////////


LoggerError::~LoggerError()
{
}


//////////////////////////////////////////////////////////////////


LoggerError::LoggerError(const LoggerError & le)
{
   err = le.err;
}


//////////////////////////////////////////////////////////////////


LoggerError & LoggerError::operator=(const LoggerError & le)
{
   err = le.err;

   return (*this);

}


//////////////////////////////////////////////////////////////////


   //
   // Code for class LoggerWarning
   //


//////////////////////////////////////////////////////////////////


LoggerWarning::LoggerWarning()
{
   warn = WarningMessageLevel;

   ExitOnWarning = DefaultExitOnWarning;

   NeedToExit = false;

}


//////////////////////////////////////////////////////////////////


LoggerWarning::~LoggerWarning()
{
   if(Warning.NeedToExit)
   {
      mlog << Error
           << "\nExiting since exit_on_warning = TRUE;\n\n";
      exit(1);
   }

}


//////////////////////////////////////////////////////////////////


LoggerWarning::LoggerWarning(const LoggerWarning & lw)
{
   warn = lw.warn;

   ExitOnWarning = lw.ExitOnWarning;

   NeedToExit = lw.NeedToExit;
}


//////////////////////////////////////////////////////////////////


LoggerWarning & LoggerWarning::operator=(const LoggerWarning & lw)
{
   warn = lw.warn;

   ExitOnWarning = lw.ExitOnWarning;

   NeedToExit = lw.NeedToExit;

   return (*this);

}


//////////////////////////////////////////////////////////////////


   //
   // Code for class LoggerDebug
   //


//////////////////////////////////////////////////////////////////


LoggerDebug::LoggerDebug()
{
   init_from_scratch();

}


//////////////////////////////////////////////////////////////////


LoggerDebug::~LoggerDebug()
{
   clear();

}


//////////////////////////////////////////////////////////////////


LoggerDebug::LoggerDebug(int i)
{
   init_from_scratch();

   if (i < MinimumMessageLevel)
   {
      cerr << "\n\n  void LoggerDebug::LoggerDebug(int i) -> The debug level must be "
           << MinimumMessageLevel << " or greater\n\n";
      exit (1);
   }

   Value = i;

}


//////////////////////////////////////////////////////////////////


LoggerDebug::LoggerDebug(const LoggerDebug & m)
{
   init_from_scratch();

   assign(m);

}


//////////////////////////////////////////////////////////////////


LoggerDebug & LoggerDebug::operator=(const LoggerDebug & m)
{
   if (this == &m)
      return (*this);

   assign(m);

   return (*this);

}


//////////////////////////////////////////////////////////////////


void LoggerDebug::init_from_scratch()
{
   clear();

}


//////////////////////////////////////////////////////////////////


void LoggerDebug::clear()
{
   Value = DefaultMessageLevel;

}


//////////////////////////////////////////////////////////////////


void LoggerDebug::assign(const LoggerDebug & m)
{
   clear();

   Value = m.Value;

}


//////////////////////////////////////////////////////////////////


   //
   // Code for class Logger
   //


//////////////////////////////////////////////////////////////////


Logger::Logger()
{
   init_from_scratch();

}


//////////////////////////////////////////////////////////////////


Logger::~Logger()
{
   clear();

}


//////////////////////////////////////////////////////////////////


Logger::Logger(const Logger & l)
{
   cerr << "\n\n  Logger(const Logger & l) -> This function should never be called\n\n";
   exit (1);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator=(const Logger & l)
{
   cerr << "\n\n  operator=(const Logger & l) -> This function should never be called\n\n";
   exit (1);

   return (*this);  // left in to keep the compiler quiet

}


//////////////////////////////////////////////////////////////////


void Logger::init_from_scratch()
{
   out = (ofstream *) 0;

   clear();

}


//////////////////////////////////////////////////////////////////


void Logger::clear()
{
   if (out)
   {
      out->flush();

      out->close();

      delete out;

      out = (ofstream *) 0;

   }

   message_level.clear();

   VerbosityLevel = DefaultVerbosityLevel;

   LogFilename.clear();

}


//////////////////////////////////////////////////////////////////


void Logger::dump(ostream & dump_out, int depth) const
{
   Indent prefix(depth);

   dump_out << prefix << "MsgLevel = \"" << message_level.value() << "\"\n";

   dump_out << prefix << "VerbosityLevel = \"" << VerbosityLevel << "\"\n";

   dump_out << prefix << "LogFilename = ";

   if (LogFilename.length() == 0)
      dump_out << "(nul)\n";
   else
      dump_out << '\"' << LogFilename << "\"\n";
   
   dump_out.flush();

}


//////////////////////////////////////////////////////////////////


void Logger::set_verbosity_level(const int i)
{
      //
      // VerbosityLevel must be -1 or greater.
      // This is the level entered by the user on the command line.
      // If -1 or 0 is given, then only ERROR and WARNING messages will
      // be output to cerr.
      // Values of 1 or more will print DEBUG messages to cout if the
      // message level is less than or equal to the verbosity level.
      //
   VerbosityLevel = (i < MinimumMessageLevel) ? DefaultVerbosityLevel : i;

}


//////////////////////////////////////////////////////////////////


void Logger::set_exit_on_warning(bool b)
{
      //
      // if true, exit after writing the first warning message
      //
   Warning.ExitOnWarning = b;

}



//////////////////////////////////////////////////////////////////


void Logger::open_log_file(const ConcatString s)
{
      //
      // check if the filename is empty
      //
   if (s.length() == 0)
   {
      cerr << "\n\n  void Logger::open_log_file() -> no filename given!\n\n";
      exit (1);
   }

   LogFilename = s;

      //
      // allocate the out pointer
      //
   out = new ofstream;

      //
      // open file and check for error
      //

   out->open(LogFilename.c_str());

   if (!(*out))
   {
      cerr << "\n\n  void Logger::open_log_file() -> unable to open file \""
           << LogFilename << "\"\n\n";
      exit (1);
   }

}

//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const string s)
{
   ConcatString msg;
   StringArray messages;
   int i, len, msg_len;
   

   if (s.empty())
   {
      //
      // if s is null or the length of s is zero, then print "(nul)"
      //
     messages.add((string)"(nul)");
   }
   else
   {
     //     StringArray temp = msg.split("\n");
     
     //     messages.add(temp);

     len = s.length();
      
         //
         // Search through s, looking for newline characters. Copy each
         // non_newline character to msg. When we reach a newline character
         // put it in msg, then put that msg into the StringArray messages,
         // clear msg, and continue. When we have found all the sub-messages
         // in s, then write them out to the appropriate places putting a
         // message type header at the beginning of each line.
         //

      for (i = 0; i < len; i++)
      {
            //
            // put the next character into the ConcatString msg
            //
	//         tmp[0] = s[i];
         msg.add(s[i]);

         if (s[i] == '\n')
         {
               //
               // this was a newline, so
               // put msg into the StringArray messages
               //
	   messages.add((string)msg);

               //
               // clear msg, and continue checking s
               //
            msg.clear();

         }

      }
         //
         // Check if the last string did not end in '\n'.
         // If it did not then we need to add it to messages.
         //

      if (s.length() > 0)
      {
         if (s[s.length() - 1] != '\n')
         {
	   messages.add((string)msg);
            msg.clear();

         }

      }

   }  // end of else s is not empty

      //
      // now we've broken the string s up into substrings each of
      // which ends with a single newline or no newline if at the
      // end of s, so we will write each substring out prepending
      // the "ERROR:", "WARNING:", or "DEBUG num:" string in front
      // if needed.
      //
   for (i = 0; i < messages.n_elements(); i++)
   {
         //
         // prepend the message type if needed
         //
      if (need_to_output_type)
      {
         write_msg_type();
         need_to_output_type = false;
      }

         //
         // if the message level is -1, then this is an ERROR type message,
         // so write it to cerr
         //
      if (message_level == ErrorMessageLevel)
      {
         cerr << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
            (*out) << messages[i] << flush;

      }
         //
         // else if the message level is 0, then this is a WARNING type message,
         // so write it to cerr
         //
      else if (message_level == WarningMessageLevel)
      {
         cerr << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
            (*out) << messages[i] << flush;

      }
         //
         // else if the message level is greater than 0, then this is a DEBUG
         // type message, so write it to cout, but only if it is less than or
         // equal to the verbosity level
         //
      else
      {
         if (message_level <= VerbosityLevel)
         {
            cout << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
               (*out) << messages[i] << flush;

         }
      }

         //
         // only want to set this to true if there is a newline at the end
         // of this message.
         //
      msg = messages[i];
      msg_len = msg.length();

      if (msg[msg_len - 1] == '\n')
         need_to_output_type = true;

   }

   return (*this);

}



//////////////////////////////////////////////////////////////////

Logger & Logger::operator<<(const char * s)
{
   ConcatString msg;
   StringArray messages;
   char tmp[2];
   int i, len, msg_len;
   
   memset(tmp, 0, sizeof(tmp));

   if (!s || !*s)
   {
      //
      // if s is null or the length of s is zero, then print "(nul)"
      //
      messages.add("(nul)");
   }
   else
   {

      len = strlen(s);
      
         //
         // Search through s, looking for newline characters. Copy each
         // non_newline character to msg. When we reach a newline character
         // put it in msg, then put that msg into the StringArray messages,
         // clear msg, and continue. When we have found all the sub-messages
         // in s, then write them out to the appropriate places putting a
         // message type header at the beginning of each line.
         //
      for (i = 0; i < len; i++)
      {
            //
            // put the next character into the ConcatString msg
            //
         tmp[0] = s[i];
         msg.add(tmp);

         if (s[i] == '\n')
         {
               //
               // this was a newline, so
               // put msg into the StringArray messages
               //
            messages.add(msg);

               //
               // clear msg, and continue checking s
               //
            msg.clear();

         }

      }

         //
         // Check if the last string did not end in '\n'.
         // If it did not then we need to add it to messages.
         //
      if (msg.length() > 0)
      {
         if (msg[msg.length() - 1] != '\n')
         {
            messages.add(msg);
            msg.clear();

         }

      }

   }  // end of else s is not empty

      //
      // now we've broken the string s up into substrings each of
      // which ends with a single newline or no newline if at the
      // end of s, so we will write each substring out prepending
      // the "ERROR:", "WARNING:", or "DEBUG num:" string in front
      // if needed.
      //
   for (i = 0; i < messages.n_elements(); i++)
   {
         //
         // prepend the message type if needed
         //
      if (need_to_output_type)
      {
         write_msg_type();
         need_to_output_type = false;
      }

         //
         // if the message level is -1, then this is an ERROR type message,
         // so write it to cerr
         //
      if (message_level == ErrorMessageLevel)
      {
         cerr << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
            (*out) << messages[i] << flush;

      }
         //
         // else if the message level is 0, then this is a WARNING type message,
         // so write it to cerr
         //
      else if (message_level == WarningMessageLevel)
      {
         cerr << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
            (*out) << messages[i] << flush;

      }
         //
         // else if the message level is greater than 0, then this is a DEBUG
         // type message, so write it to cout, but only if it is less than or
         // equal to the verbosity level
         //
      else
      {
         if (message_level <= VerbosityLevel)
         {
            cout << messages[i] << flush;

            //
            // if the file is open, then also write it to the log file
            //
         if (is_open())
               (*out) << messages[i] << flush;

         }
      }

         //
         // only want to set this to true if there is a newline at the end
         // of this message.
         //
      msg = messages[i];
      msg_len = msg.length();

      if (msg[msg_len - 1] == '\n')
         need_to_output_type = true;

   }

   return (*this);

}


Logger & Logger::operator<<(const int n)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << n << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << n << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << n << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const unsigned int n)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << n << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << n << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << n << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << n << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const long l)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << l << flush;

      }
   }

   return (*this);

}

//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const unsigned long l)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << l << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const long long l)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << l << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const unsigned long long l)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << l << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << l << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << l << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const double d)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << d << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << d << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << d << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << d << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << d << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << d << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const char c)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << c << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << c << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << c << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << c << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << c << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << c << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const bool b)
{
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << b << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << b << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << b << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << b << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << b << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << b << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const Indent & i)
{
   int j, jmax;
   ConcatString tmp_str;

   tmp_str.erase();

      //
      // set size of indentation
      //
   jmax = (i.delta) * (i.depth);

      //
      // fill the temporary string with the correct type of characters
      //
   for (j = 0; j < jmax; j++)
   {
      if ((j % (i.delta)) == 0)
         tmp_str << i.on_char;
      else
         tmp_str << i.off_char;
   }

      //
      // write the indentation string out
      //
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << tmp_str << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << tmp_str << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << tmp_str << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << tmp_str << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << tmp_str << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << tmp_str << flush;

      }
   }

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const MsgLevel & m)
{
      //
      // set this logger's message_level value from m
      //
   message_level = m;

   write_msg_type();

   need_to_output_type = false;

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const LoggerError e)
{
   (*this) << level(ErrorMessageLevel);

   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const LoggerWarning w)
{  
   (*this) << level(WarningMessageLevel);
   
   if (Warning.ExitOnWarning) Warning.NeedToExit = true;
   
   return (*this);

}


//////////////////////////////////////////////////////////////////


Logger & Logger::operator<<(const LoggerDebug d)
{
   (*this) << level(d.value());

   return (*this);

}


//////////////////////////////////////////////////////////////////


void Logger::write_msg_type()
{  
      //
      // if the message level is -1, then this is an ERROR type message,
      // so write it to cerr
      //
   if (message_level == ErrorMessageLevel)
   {
      cerr << "ERROR  : " << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << "ERROR  : " << flush;

   }
      //
      // else if the message level is 0, then this is a WARNING type message,
      // so write it to cerr
      //
   else if (message_level == WarningMessageLevel)
   {
      cerr << "WARNING: " << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
         (*out) << "WARNING: " << flush;

   }
      //
      // else if the message level is greater than 0, then this is a DEBUG
      // type message, so write it to cout, but only if it is less than or
      // equal to the verbosity level
      //
   else
   {
      if (message_level <= VerbosityLevel)
      {
         cout << "DEBUG " << message_level << ": " << flush;

         //
         // if the file is open, then also write it to the log file
         //
      if (is_open())
            (*out) << "DEBUG " << message_level << ": " << flush;

      }
   }

}


//////////////////////////////////////////////////////////////////


   //
   // Code for Miscellaneous Functions
   //


//////////////////////////////////////////////////////////////////


MsgLevel & level(const int n)
{
      //
      // create a MsgLevel object and initialize it to n
      //
   MsgLevel m(n);

      //
      // assign the new object to the one sent in
      //
   Global_Level = m;

      //
      // return a reference to the MsgLevel object
      //
   return (Global_Level);

}


//////////////////////////////////////////////////////////////////


MsgLevel & Debug(const int n)

{
      //
      // create a MsgLevel object and initialize it to n
      //
   MsgLevel m(n);

      //
      // assign the new object to the one sent in
      //
   Global_Level = m;

      //
      // return a reference to the MsgLevel object
      //
   return (Global_Level);

}


//////////////////////////////////////////////////////////////////



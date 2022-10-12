// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 2022 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   main.cc
//
//   Description:
//      MET has only one main(). The common features like logging with user ID
//      for STIG and the signal handling are implemented one place whcih
//      prevents from copying or calling the same APIs at the multiple palces.
//      The existing "main" at MET tools should be renamed to "met_main" and
//      three APIs (get_tool_name, initialize, and process_command_line) must
//      be added. Most MET tools have initialize and process_command_line.
//
//      int main(int argc, char *argv[])
//      ==>
//      int met_main(int argc, char *argv[])
//
//      string get_tool_name() { return "mode"; }
//      void initialize();
//      void process_command_line(int argc, char **argv);
//
//   Mod#   Date      Name        Description
//   ----   ----      ----        -----------
//   000    07-06-22  Soh         New
//   001    09-06-22  Prestopnik  MET #2227 Remove namespace std from header files
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <csignal>
#include <pwd.h>

#include "main.h"
#include "concat_string.h"
#include "memory.h"
#include "logger.h"


////////////////////////////////////////////////////////////////////////

static string met_cmdline = "";

static int met_start_time;
static int met_end_time;
static uid_t met_user_id;
static string met_user_name;
static string met_tool_name;

////////////////////////////////////////////////////////////////////////

extern string get_tool_name();

extern int met_main(int argc, char *argv[]);

void do_post_process();
void do_pre_process(int argc, char *argv[]);
void set_handlers();
void set_user_id();
void store_arguments(int argc, char **argv);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   do_pre_process(argc, argv);

   int return_code = met_main(argc, argv);

   do_post_process();

   return return_code;

}

////////////////////////////////////////////////////////////////////////

void do_pre_process(int argc, char *argv[]) {
   ConcatString msg, msg2;

   store_arguments(argc, argv);

   set_user_id();
   met_tool_name = get_tool_name();

   msg << "Start " << met_tool_name << " by " << met_user_name
       << "(" << met_user_id << ") at " << get_current_time();
   msg2 << "  cmd: " << met_cmdline;
   mlog << Debug(1) << msg << msg2 << "\n";

   set_handlers();
}

////////////////////////////////////////////////////////////////////////

void do_post_process() {
   ConcatString msg;
   msg << "Finish " << met_tool_name << " by " << met_user_name
       << "(" << met_user_id << ") at " << get_current_time();
   mlog << Debug(1) << msg << "\n";
}

////////////////////////////////////////////////////////////////////////

string get_current_time() {
   time_t curr_time;
   tm * curr_tm;
   char date_string[MET_BUF_SIZE];

   time(&curr_time);
   curr_tm = gmtime (&curr_time);

   strftime(date_string, MET_BUF_SIZE, "%Y-%m-%d %TZ", curr_tm);

   return string(date_string);
}

////////////////////////////////////////////////////////////////////////

/* not working at Docker
// based on blog at http://www.alexonlinux.com/how-to-handle-sigsegv-but-also-generate-core-dump
// NOTE:  that comments on the blog indicate the core file generated on red hat or on multi-threaded programs
//        might contain unhelpful information.
void segv_handler(int signum) {
   char cwdbuffer[MET_BUF_SIZE+1];
   string timebuffer = get_current_time();

   getcwd(cwdbuffer,MET_BUF_SIZE+1);

   fprintf(stderr, "FATAL ERROR (SEGFAULT): Process %d got signal %d @ local time = %s\n", getpid(), signum, timebuffer);
   fprintf(stderr, "FATAL ERROR (SEGFAULT): Look for a core file in %s\n",cwdbuffer);
   fprintf(stderr, "FATAL ERROR (SEGFAULT): Process command line: %s\n",met_cmdline.c_str());
   signal(signum, SIG_DFL);
   kill(getpid(), signum);
}
*/

////////////////////////////////////////////////////////////////////////
// Need signal handlers for SIGINT, SIGHUP, SIGTERM, SIGPIPE, and SIGSEGV
//  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
//  PORTsignal(SIGSEGV, segv_handler);

void set_handlers() {

  set_new_handler(oom);
}

////////////////////////////////////////////////////////////////////////

void set_user_id() {
   met_user_id = geteuid ();
   register struct passwd *pw;
   pw = getpwuid (met_user_id);
   if (pw) met_user_name = string(pw->pw_name);
}

////////////////////////////////////////////////////////////////////////

void store_arguments(int argc, char **argv) {
   for (int ix = 0; ix < argc; ix++){
      met_cmdline += argv[ix];
      met_cmdline += " ";
   }
}

////////////////////////////////////////////////////////////////////////

void tidy_and_exit(int signal) {
   printf("Exiting %d\n", signal);
   exit(signal);
}

////////////////////////////////////////////////////////////////////////

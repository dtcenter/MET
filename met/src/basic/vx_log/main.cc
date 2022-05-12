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
//      The main() should be inplemented here and the exisyting main() should be renamed to met_main.

#include "main.h"
#include <unistd.h>
//#include <pwd.h>

////////////////////////////////////////////////////////////////////////

static const int tmp_buf_size = 512;

////////////////////////////////////////////////////////////////////////

string get_current_time()
{
   time_t curr_time;
   tm * curr_tm;
   char date_string[tmp_buf_size];

   time(&curr_time);
   curr_tm = gmtime (&curr_time);

   strftime(date_string, tmp_buf_size, "%Y-%m-%d %TZ", curr_tm);
   string time_str(date_string);

   return time_str;
}

////////////////////////////////////////////////////////////////////////

uid_t get_user_id()
{
   register uid_t uid = geteuid ();
   /*
   register struct passwd *pw;
   pw = getpwuid (uid);
   if (pw) {
      puts (pw->pw_name);
      exit (EXIT_SUCCESS);
   }
   fprintf (stderr,"%s: cannot find username for UID %u\n",
       _PROGRAM_NAME, (unsigned) uid);
   exit (EXIT_FAILURE);
   */
   return uid;
}

////////////////////////////////////////////////////////////////////////

void do_pre_process(uid_t user_id, const char *tool_name) {
   cout << "  Started " << tool_name << " by user " << user_id << " at " << get_current_time() << "\n";
   cout << "  Do signal handling here and other pre-processing\n";
}


////////////////////////////////////////////////////////////////////////

void do_post_preocess(uid_t user_id, const char *tool_name) {
   cout << "  Do post-processing\n";
   cout << "  Done  " << tool_name << " by user " << user_id << " at " << get_current_time() << "\n";
}

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   uid_t user_id = get_user_id();
   const char * tool_name = get_tool_name();

   do_pre_process(user_id, tool_name);

   int return_code = met_main(argc, argv);

   do_post_preocess(user_id, tool_name);

   return return_code;

}
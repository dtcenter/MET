// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   // chk4copyright.cc
   //
   // This program takes a directory to recursively search and checks
   // each .h and .cc file in each directory in the tree for the
   // copyright notice. If the file does not have one, then it puts
   // the copyright notice in. The copyright notice filename can
   // either be given on the command line or set in the environment
   // variable COPYRIGHT_NOTICE. Also, the user can turn off the
   // output for what the program is doing by using the -quiet
   // option. The user may also use the environment variable TMPDIR
   // to set a temporary directory instead of the temporary directory
   // /tmp.
   //


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/file.h>
#include <dirent.h>
#include <cmath>

#include "command_line.h"
#include "string_fxns.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static ConcatString top_dir;

static ConcatString copyright_notice_filename;

static bool output_info = true;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_top_dir(const StringArray &);
static void set_copyright_notice_filename(const StringArray &);
static void set_quiet(const StringArray &);

static void process_directory(const char *);

static int get_line(int &, ConcatString &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
   CommandLine cline;

      //
      // begin execution
      //
   program_name = get_short_name(argv[0]);

      //
      // check for no arguments
      //
   if (argc == 1)
      usage();

      //
      // parse the command line into tokens
      //
   cline.set(argc, argv);

      //
      // set the usage function
      //
   cline.set_usage(usage);

      //
      // add the options function calls
      //
   cline.add(set_top_dir, "-dir", 1);
   cline.add(set_copyright_notice_filename, "-notice", 1);
   cline.add(set_quiet, "-quiet", 0);

      //
      // parse the command line
      //
   cline.parse();

      //
      // check that we have a directory path
      //
   if (top_dir.length() == 0)
      usage();

      //
      // check for leftover arguments
      //
   if (cline.n() > 0)
      usage();

      //
      // Did the user give a filename for the copyright notice file?
      // If not, check the environment variable, COPYRIGHT_NOTICE.
      //
   if (copyright_notice_filename.length() == 0)
   {
      get_env("COPYRIGHT_NOTICE", copyright_notice_filename);

      if(copyright_notice_filename.empty())
      {
         mlog << Error << "\nno copyright notice filename given on "
              << "command line or set in the environment variable "
              << "COPYRIGHT_NOTICE!\n\n";
         usage();
      }
   }

      //
      // process this directory
      //
   process_directory(top_dir.c_str());

   exit (0);

}


////////////////////////////////////////////////////////////////////////


void usage()
{
   mlog << Error << "\nUsage: " << program_name << "\n"
        << "          -dir DirectoryPath\n"
        << "          [-notice CopyrightNoticeFilename ]\n"
        << "          [-quiet ]\n\n";
   exit (1);

}


////////////////////////////////////////////////////////////////////////


void set_top_dir(const StringArray &a)
{
   top_dir = a[0];

}


////////////////////////////////////////////////////////////////////////


void set_copyright_notice_filename(const StringArray &a)
{
   copyright_notice_filename = a[0];

}


////////////////////////////////////////////////////////////////////////


void set_quiet(const StringArray &)
{
   output_info = false;

}


////////////////////////////////////////////////////////////////////////


void process_directory(const char * dir_name)
{
   DIR *dir = (DIR *) 0;  // pointer to a DIR structure
   dirent *pde = (dirent *) 0;  // pointer to a portable directory entry
   ConcatString new_directory;
   ConcatString tmp_directory;
   ConcatString new_filename;
   ConcatString tmp_filename;
   ConcatString entry_name;
   ConcatString line;
   ConcatString command;
   int fd;
   int pid;
   bool notice_found = false;

      //
      // set the temporary directory and temporary filename
      //

   pid = getpid();
   
        if(get_env("TMPDIR",      tmp_directory)) {}
   else if(get_env("MET_TMP_DIR", tmp_directory)) {}
   else
   {
      tmp_directory = "/tmp";

         //
         // the TMPDIR environment variable is not set so check if /tmp exists
         //
      dir = met_opendir(tmp_directory.c_str());
      if (!dir)
      {
         mlog << Error << "\nprocess_directory() -> the directory \"/tmp\" does not exist.\n\n";
         exit (1);
      }
      else {
         met_closedir(dir);
      }

   }

   tmp_filename << cs_erase << tmp_directory << '/' << "copyright_" << pid;

      //
      // open this directory
      //
   dir = met_opendir(dir_name);
   if (!dir)
   {
      mlog << Error << "\nprocess_directory() -> can't open directory \"" << dir_name << "\"\n\n";
      exit (1);
   }

   if (output_info)
   {
      mlog << Debug(1) << "\n\nSearching files in directory \"" << dir_name << "\"\n";
   }

      //
      // now for each entry in this directory, check if it is a file
      // ending in either .h or .cc. If it is, then do the check for
      // the copyright notice. If it is any other type of file, then
      // just skip it. If it is a directory, then call this function
      // with the directory name.
      //
   while ((pde = readdir(dir)) != NULL)
   {
         //
         // check if this is the directory "." or ".." and also skip
         // the .svn directories
         //
      if ((strcmp(pde->d_name, ".") == 0) || (strcmp(pde->d_name, "..") == 0) ||
          (strcmp(pde->d_name, ".svn") == 0))
         continue;  // don't loop forever

         //
         // if this is a directory then call process_directory on it
         //
      if (pde->d_type == DT_DIR)
      {
         new_directory << cs_erase << dir_name << '/' << pde->d_name;
         process_directory(new_directory.c_str());
         continue;
      }

         //
         // check if this is a regular file
         //
      if (pde->d_type == DT_REG)
      {
            //
            // save the entry name to check the filename
            //
         entry_name = pde->d_name;

            //
            // check suffix of entry name
            //
         if ((entry_name.endswith(".h")) || (entry_name.endswith(".cc")))
         {
            if (output_info)
            {
               mlog << Debug(1) << "  Examining file \"" << entry_name << "\"\n";
            }

               //
               // create the new filename with the directory path
               //
            new_filename << cs_erase << dir_name << '/' << entry_name;

               //
               // Check if the copyright notice is there and add it if not.
               // To do this, open the file, then check each line to see
               // if it contains the substring with the copyright line
               // "// ** Copyright UCAR (c) 19". If it reads the whole file
               // without finding it, then we need to add it.
               //
            fd = open(new_filename.c_str(), O_RDONLY, 0);

            if (fd == -1)
            {
               mlog << Error << "\nprocess_directory() -> unable to open file \""
                    << new_filename << "\"\n\n";
               exit (1);
            }

               //
               // while there are lines to read in the file,
               // read each line and check if it contains the
               // copyright notice
               //
            notice_found = false;
            line << cs_erase;

            while (get_line(fd, line) != 0)
            {
                  //
                  // first check for an empty string
                  //
               if (line.empty())
                  continue;

               if (strstr(line.c_str(), "// ** Copyright UCAR (c) 19") != NULL)
               {
                  notice_found = true;
                  break;
               }

               line << cs_erase;

            }  // end of while there are lines to read and notice not found

               //
               // close the file
               //
            close(fd);

               //
               // if the copyright notice was not found, then put it in
               //
            if (!notice_found)
            {
               if (output_info)
               {
                  mlog << Debug(1) << "    Adding copyright notice to file \"" << entry_name << "\"\n";
               }

                  //
                  // create the command to copy the copyright notice file
                  // to a temporary file
                  //
               command << cs_erase << "cp " << copyright_notice_filename << ' ' << tmp_filename;
               system(command.c_str());

                  //
                  // create the command to append the .h or .cc file
                  // to the temporary file
                  //
               command << cs_erase << "cat " << new_filename << " >> " << tmp_filename;
               system(command.c_str());

                  //
                  // create the command to copy the temporary file
                  // back to the name of the .h or .cc file
                  //
               command << cs_erase << "cp " << tmp_filename  << ' ' << new_filename;
               system(command.c_str());

                  //
                  // remove the temporary file
                  //
               command << cs_erase << "rm " << tmp_filename;
               system(command.c_str());
            }

         }  // end of if this is a .h or .cc file

      }  // end of if this is a regular file

   }  // end of for each entry in this directory

      //
      // close the directory
      //
   
   if(dir) met_closedir(dir);

}


////////////////////////////////////////////////////////////////////////


int get_line(int & fd, ConcatString & s)
{
   char buf[2];
   int n_bytes_to_read = 1;

   memset(buf, 0, sizeof(buf));

      //
      // if not at the end of the file and the character read in
      // is not a newline, then add the character read in to the
      // line.
      //
   while (read(fd, buf, n_bytes_to_read) > 0)
   {
      if (buf[0] == '\n')
         return (1);

      s.add(buf);

   }

      //
      // end of file
      //
   return (0);

}


////////////////////////////////////////////////////////////////////////



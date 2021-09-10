// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include "logger.h"
#include "str_wrappers.h"


////////////////////////////////////////////////////////////////////////

int m_strlen(const char *str) {
   int str_len = -1;
   if (str) str_len = strlen(str);

   return str_len;
}

////////////////////////////////////////////////////////////////////////
// to_str should be allocated before calling this.

void m_strcpy(char *to_str, const char *from_str, const char *method_name,
              const char *extra_msg) {

   int str_len = sizeof(to_str);
   m_strncpy(to_str, from_str, str_len, method_name, extra_msg);

}

////////////////////////////////////////////////////////////////////////
// to_string should not allocated. This allocates and return to_str after copying

char *m_strcpy2(const char *from_str, const char *method_name, const char *extra_msg) {
   char *to_str = (char *) 0;
   if (from_str) {
      int str_len = m_strlen(from_str);

      to_str = new char[str_len + 1];

      if(!to_str) {
         mlog << Error << "\n" << method_name
              << "memory allocation error (m_strcpy)"
              << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
         exit(1);
      }

      m_strncpy(to_str, from_str, str_len, method_name, extra_msg);
   }
   else {
      mlog << Error << "\n" << method_name 
           << " Do not copy the string because a from_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }

   return to_str;
}

////////////////////////////////////////////////////////////////////////

void m_strncpy(char *to_str, const char *from_str, const int buf_len,
               const char *method_name, const char *extra_msg, bool truncate) {
   if (!from_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a from_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else if (!to_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a to_string is NULL. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else {   // (from_str && to_str)
      int str_len = m_strlen(from_str);
      if (str_len > buf_len) str_len = buf_len;

      memset(to_str, 0, str_len);
      string temp_str = from_str;
      temp_str.copy(to_str, str_len);
      to_str[str_len] = 0;

      if (!truncate && strcmp(from_str, to_str)) {
         mlog << Warning << "\n" << method_name
              << " truncated a string " << (extra_msg == 0 ? " " : extra_msg)
              << " from \"" << from_str << "\" to \"" << to_str << "\"\n\n";
      }
   }

}



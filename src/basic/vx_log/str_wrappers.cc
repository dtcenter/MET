// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

bool is_whitespaces(char cur_char) {
   return (' ' == cur_char || '\t' == cur_char || '\n' == cur_char || '\r' == cur_char);
}

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

   // Note: recommend to use m_strncpy because there are some cases that
   // sizeof(to_str) returns 8 which is not the allocated buffer size.
   int str_len = m_strlen(from_str);
   m_strncpy(to_str, from_str, str_len, method_name, extra_msg);

}

////////////////////////////////////////////////////////////////////////
// to_string should not allocated. This allocates and return to_str after copying

char *m_strcpy2(const char *from_str, const char *method_name, const char *extra_msg) {
   char *to_str = (char *) nullptr;
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
           << " Do not copy the string because a from_string is nullptr. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }

   return to_str;
}

////////////////////////////////////////////////////////////////////////

void m_strncpy(char *to_str, const char *from_str, const int buf_len,
               const char *method_name, const char *extra_msg, bool truncate) {
   if (!from_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a from_string is nullptr. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else if (!to_str){
      mlog << Warning << "\n" << method_name 
           << " Do not copy the string because a to_string is nullptr. " 
           << (extra_msg == 0 ? "" : extra_msg) << "\n\n";
   }
   else {   // (from_str && to_str)
      int str_len = m_strlen(from_str);
      if (str_len > buf_len) str_len = buf_len;

      memset(to_str, 0, str_len);
      // Kludge: there were cases that sizeof returns 8 instead of the real size.
      // Use sizeof only if it's not 8.
      int to_buf_size = sizeof(to_str);
      if (to_buf_size != 8) {
         if (str_len < to_buf_size) memset(to_str, str_len, to_buf_size);
         if (str_len > to_buf_size) str_len = to_buf_size;  // truncate
      }

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

////////////////////////////////////////////////////////////////////////

bool m_replace_char(char *str_buf, char from_ch, char to_ch, bool all_instances) {
   bool replaced = false;
   int str_len = m_strlen(str_buf);
   for(int idx=0; idx<str_len; idx++) {
      if (from_ch == str_buf[idx]) {
         replaced = true;
         str_buf[idx] = to_ch;
         if (!all_instances) break;
      }
   }
   return replaced;
}

////////////////////////////////////////////////////////////////////////

void m_rstrip(char *str_buf, int buf_len, bool find_white_ch) {
   // Make sure it's nullptr terminated
   if (buf_len >= 0) str_buf[buf_len] = '\0';
   // Change the trailing blank space to a null
   int str_len = m_strlen(str_buf);
   for(int idx=str_len-1; idx>=0; idx--) {
      if(is_whitespaces(str_buf[idx])) {
         str_buf[idx] = '\0';
         if((idx > 0) && !is_whitespaces(str_buf[idx-1])) break;
      }
      else if (!find_white_ch) break;
   }
}

////////////////////////////////////////////////////////////////////////

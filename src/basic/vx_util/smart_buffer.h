

////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SMART_BUFFER_H__
#define  __VX_SMART_BUFFER_H__


#include <vector>


////////////////////////////////////////////////////////////////////////


class SmartBuffer {


   protected:

      void init_from_scratch();

      void assign (const SmartBuffer &);


      std::vector<unsigned char> Buf;



   public:

      SmartBuffer();
     ~SmartBuffer();
      SmartBuffer(const SmartBuffer &);
      SmartBuffer & operator=(const SmartBuffer &);

      void clear();   //  deallocates

         //
         //  set stuff
         //


         //
         //  get stuff
         //

     int size () const;

     bool is_empty () const;

         //
         //  do stuff
         //

     operator unsigned char * () const;

     operator void * () const;

     void extend (const int bytes);


     int read  (const int fd, const int bytes);         //  wrapper for read(2)

     int write (const int fd, const int bytes) const;   //  wrapper for write(2)


           //
           //  these use memcpy
           //

     void read_from_buf  (void * other_buf, const int bytes, const int pos = 0);         //  position in THIS buffer

     void write_to_buf   (void * other_buf, const int bytes, const int pos = 0) const;   //  position in THIS buffer


};


////////////////////////////////////////////////////////////////////////


inline int  SmartBuffer::size()  const { return (int) Buf.size(); }

inline bool SmartBuffer::is_empty() const { return Buf.empty(); }

inline      SmartBuffer::operator unsigned char * () const { return const_cast<unsigned char *>(Buf.data()); }

inline      SmartBuffer::operator void * () const { return const_cast<unsigned char *>(Buf.data()); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SMART_BUFFER_H__  */


////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_SMART_BUFFER_H__
#define  __VX_SMART_BUFFER_H__


////////////////////////////////////////////////////////////////////////


class SmartBuffer {


   protected:

      void init_from_scratch();

      void assign (const SmartBuffer &);


      unsigned char * Buf;   //  allocated

      int Size;   //  bytes


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

     bool is_empty () const;   //  Buf is zero

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


inline int  SmartBuffer::size()  const { return ( Size ); }

inline bool SmartBuffer::is_empty() const { return ( Buf == 0 ); }

inline      SmartBuffer::operator unsigned char * () const { return ( Buf ); }

inline      SmartBuffer::operator void * () const { return ( Buf ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_SMART_BUFFER_H__  */


////////////////////////////////////////////////////////////////////////



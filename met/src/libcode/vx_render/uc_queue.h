// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __UNSIGNED_CHAR_QUEUE_H__
#define  __UNSIGNED_CHAR_QUEUE_H__


////////////////////////////////////////////////////////////////////////


static const int ucqueue_size    = 256;


////////////////////////////////////////////////////////////////////////


class UCQueue {

   private:

      unsigned char data[ucqueue_size];

      int NElements;

      void assign(const UCQueue &);

      int calc_run_count() const;

   public:

      UCQueue();
     ~UCQueue();
      UCQueue(const UCQueue &);
      UCQueue & operator=(const UCQueue &);


      int is_empty() const;

      int n_elements() const;

      int last_char() const;

      int run_count() const;

      void enqueue(unsigned char);

      unsigned char dequeue();

      void clear();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __UNSIGNED_CHAR_QUEUE_H__


////////////////////////////////////////////////////////////////////////



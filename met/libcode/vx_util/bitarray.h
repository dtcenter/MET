// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VERIF_BIT_ARRATY_H__
#define  __VERIF_BIT_ARRATY_H__


////////////////////////////////////////////////////////////////////////


class BitArray {

   private:

      void init_from_scratch();

      void assign(const BitArray &);

      unsigned char * u;

      int Nbits;

      int Nalloc;   //  Nalloc should not be any larger than needed
                    //    in other words, Nalloc should always equal
                    //    (Nbits + 7)/8

   public:

      BitArray();
     ~BitArray();
      BitArray(const BitArray &);
      BitArray & operator=(const BitArray &);

      void clear();   //  deallocate

      void dump(ostream &, int depth = 0) const;

      void set_all_zeroes();
      void set_all_ones();

      void set_size(int);   //  set # of bits

      void set_bit(int bitno, int onoff);

      int operator[](int) const;  //  get bit

      int nbits() const;

      int n_bits_on() const;

};


////////////////////////////////////////////////////////////////////////


inline int BitArray::nbits() const { return ( Nbits ); }


////////////////////////////////////////////////////////////////////////


extern int bob(const unsigned char);   //  how many bits are on


////////////////////////////////////////////////////////////////////////


#endif   //  __VERIF_BIT_ARRATY_H__


////////////////////////////////////////////////////////////////////////



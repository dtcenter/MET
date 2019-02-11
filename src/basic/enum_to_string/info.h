// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ENUM_INFO_H__
#define  __ENUM_INFO_H__


////////////////////////////////////////////////////////////////////////


static const int enuminfo_alloc_increment = 100;


////////////////////////////////////////////////////////////////////////


class EnumInfo {

      friend ostream & operator<<(ostream &, const EnumInfo &);

   private:

      char ** s;

      char * Name;

      char * LowerCaseName;

      char * Scope;

      char * U_Scope;

      char * Header;

      int Nids;

      int Nalloc;

      void assign(const EnumInfo &);

      void init_from_scratch();

      void extend(int);

   public:

      EnumInfo();
     ~EnumInfo();
      EnumInfo(const EnumInfo &);
      EnumInfo & operator=(const EnumInfo &);

      void add_id(const char *);

      int n_ids() const;

      const char * id(int) const;

      int max_id_length() const;

      void clear();

      const char * name() const;

      const char * lowercase_name() const;

      const char * scope() const;

      const char * u_scope() const;

      const char * header() const;

      void set_name(const char *);

      void set_scope(const char *);

      void set_header(const char *);

};


////////////////////////////////////////////////////////////////////////


inline int EnumInfo::n_ids() const { return ( Nids ); }

inline const char * EnumInfo::name() const { return ( Name ); }

inline const char * EnumInfo::lowercase_name() const { return ( LowerCaseName ); }

inline const char * EnumInfo::scope() const { return ( Scope ); }

inline const char * EnumInfo::u_scope() const { return ( U_Scope ); }

inline const char * EnumInfo::header() const { return ( Header ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const EnumInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ENUM_INFO_H__  */


////////////////////////////////////////////////////////////////////////




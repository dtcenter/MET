// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ENUM_INFO_H__
#define  __ENUM_INFO_H__


#include <string>
#include <vector>


////////////////////////////////////////////////////////////////////////


class EnumInfo {

      friend std::ostream & operator<<(std::ostream &, const EnumInfo &);

   private:

      std::vector<std::string> s;

      std::string Name;

      std::string LowerCaseName;

      std::string Scope;

      std::string U_Scope;

      std::string Header;

      void assign(const EnumInfo &);

      void init_from_scratch();


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


inline int EnumInfo::n_ids() const { return (int) s.size(); }

   //
   //  These return nullptr rather than "" when unset, because that is the
   //  contract the generator relies on: code.cc tests "if ( e.scope() )" to
   //  decide whether to emit a "Scope::" qualifier at all. Returning c_str()
   //  unconditionally would make that test always true and prefix every
   //  generated type with a bare "::".
   //

inline const char * EnumInfo::name() const { return Name.empty() ? nullptr : Name.c_str(); }

inline const char * EnumInfo::lowercase_name() const { return LowerCaseName.empty() ? nullptr : LowerCaseName.c_str(); }

inline const char * EnumInfo::scope() const { return Scope.empty() ? nullptr : Scope.c_str(); }

inline const char * EnumInfo::u_scope() const { return U_Scope.empty() ? nullptr : U_Scope.c_str(); }

inline const char * EnumInfo::header() const { return Header.empty() ? nullptr : Header.c_str(); }


////////////////////////////////////////////////////////////////////////


extern std::ostream & operator<<(std::ostream &, const EnumInfo &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ENUM_INFO_H__  */


////////////////////////////////////////////////////////////////////////




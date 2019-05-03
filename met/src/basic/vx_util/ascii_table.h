// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __ASCII_TABLE_H__
#define  __ASCII_TABLE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <vector>
#include <string>

#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  justification settings
   //


enum AsciiTableJust {

   RightJust,    //   right-justified entry
   LeftJust,     //    left-justified entry
   CenterJust    //  center-justified entry

};


////////////////////////////////////////////////////////////////////////


   //
   //  default character for padding table entries
   //


static const char default_table_pad_char = ' ';


   //
   //  default column separation character
   //


static const char default_col_sep_char = ' ';


   //
   //  default justification value for table entries
   //


static const AsciiTableJust default_justification = RightJust;


////////////////////////////////////////////////////////////////////////


static const int default_ics = 1;   //  default value of inter-column space
                                    //  default is one, not zero

static const int default_irs = 0;   //  default value of inter-row space


////////////////////////////////////////////////////////////////////////


static const int max_ics = 100;   //  maximum value of inter-column space
                                  //  should be plenty big enough

static const int max_irs = 100;   //  maximum value of inter-row space
                                  //  should be plenty big enough


////////////////////////////////////////////////////////////////////////


static const int default_table_indent =   0;   //  default value of table indentation

static const int max_table_indent     = 500;   //  maximum value of table indentation
                                               //  should be plenty big enough


////////////////////////////////////////////////////////////////////////


   //
   //  should we fill empty rows with blanks?
   //

static const bool default_fill_blank = false;

   //
   //  default floating-point decimal places
   //

static const int ascii_table_default_precision         = 2;

static const int ascii_table_max_precision             = 12;

static const double ascii_table_default_bad_data_value = -9999.0;


////////////////////////////////////////////////////////////////////////


class AsciiTable {

   protected:

      void init_from_scratch();

      void assign(const AsciiTable &);

      int rc_to_n(int r, int c) const;

      int Nrows;
      int Ncols;

      char ColSepChar;   //  column separator character

      char PadChar;      //  pad character for table entries

      int TableIndent;   //  how many blank spaces before each line of the table
                         //  this has no effect on TableWidth

      bool FillBlank;    //  fill empty lines in the output with blanks?

      std::vector<std::string> e;   //  table entries
                   //  this is really a two-dimensional
                   //  array

      std::vector<AsciiTableJust> Just;    //  justification values for table entries
                                //  this is really a two-dimensional
                                //  array

      std::vector<int> ColWidth;           //  array of column widths
      std::vector<int> InterColumnSpace;   //  array of inter-column spaces
      std::vector<int> InterRowSpace;      //  array of inter-column spaces

         //
         //  floating-point stuff
         //

      int    Precision;

      double BadDataValue;

      std::string BadDataStr;

      char   f_FloatFormat[16];
      char   g_FloatFormat[16];


      bool   DoCommaString;   //  do comma string?

      bool   DeleteTrailingBlankRows;

      bool   ElimTrailingWhitespace;   //  default: true

      bool   DecimalPointsAligned;

   public:

      AsciiTable();
      virtual ~AsciiTable();
      AsciiTable(const AsciiTable &);
      AsciiTable & operator=(const AsciiTable &);

      void clear();
      void erase();


         //
         //  set stuff
         //


      virtual void set_size(const int NR, const int NC);
      virtual void add_rows(const int NR);

      virtual void set_entry(const int r, const int c, const char*);
      virtual void set_entry(const int r, const int c, const ConcatString &);
      virtual void set_entry(const int r, const int c, int);
      virtual void set_entry(const int r, const int c, char);
      virtual void set_entry(const int r, const int c, double);


      virtual void set_table_just  (const AsciiTableJust);                             //  whole table
      virtual void set_column_just (const int c, const AsciiTableJust);                //  whole column
      virtual void set_row_just    (const int r, const AsciiTableJust);                //  whole row
      virtual void set_entry_just  (const int r, const int c, const AsciiTableJust);   //  specific entry


      virtual void set_ics(int value);          //  inter-column space
      virtual void set_ics(int c, int value);   //  inter-column space between columns c and c + 1

      virtual void set_irs(int value);          //  inter-row space
      virtual void set_irs(int r, int value);   //  inter-row space between rows r and r + 1


      virtual void set_table_indent  (int value);

      virtual void set_col_sep_char  (const char);

      virtual void set_pad_char      (const char);

      virtual void set_precision     (int);

      virtual void set_bad_data_value(double);

      virtual void set_bad_data_str  (const std::string);


      virtual void set_fill_blank                 (bool);

      virtual void set_comma_string               (bool);

      virtual void set_delete_trailing_blank_rows (bool);

      virtual void set_elim_trailing_whitespace   (bool);


         //
         //  get stuff
         //


      virtual int nrows() const;

      virtual int ncols() const;


      virtual int inter_column_space(int col_left) const;

      virtual int inter_row_space(int row_top) const;


      virtual int col_width(const int c) const;

      virtual int max_col_width() const;


      virtual int table_width  () const;
      virtual int table_height () const;

      virtual int table_indent () const;

      virtual bool fill_blank() const;


      virtual char col_sep_char() const;

      virtual char pad_char() const;


      virtual AsciiTableJust entry_just(const int r, const int c) const;

      virtual bool comma_string() const;

      virtual bool row_is_blank(int r) const;

         //  add some pad characters on the right side of an entry,
         //    eg, to line up the decimal points

      virtual void pad_entry_right(const int r, const int c, const int n, const char = ' ');

      virtual void line_up_decimal_points();


         //
         //  retrieve individual entries
         //


      virtual ConcatString padded_entry(const int r, const int c) const;

      virtual const ConcatString operator()(const int r, const int c) const;

         //
         //  retrieve a whole row
         //

      virtual ConcatString padded_row(const int r) const;

         //
         //  misc
         //

      virtual int precision() const;

      virtual const char * f_float_format() const;
      virtual const char * g_float_format() const;

      virtual bool delete_trailing_blank_rows() const;

      virtual bool elim_trailing_whitespace() const;

      virtual bool decimal_points_aligned() const;

      virtual void underline_row(const int row, const char);

};


////////////////////////////////////////////////////////////////////////


inline int  AsciiTable::nrows() const { return ( Nrows ); }

inline int  AsciiTable::ncols() const { return ( Ncols ); }

inline char AsciiTable::col_sep_char() const { return ( ColSepChar ); }

inline char AsciiTable::pad_char() const { return ( PadChar ); }

inline bool AsciiTable::fill_blank () const { return ( FillBlank ); }

inline void AsciiTable::set_fill_blank (bool tf) { FillBlank = tf;  return; }

inline int  AsciiTable::table_indent () const { return ( TableIndent ); }

inline int  AsciiTable::precision() const { return ( Precision ); }

inline const char * AsciiTable::f_float_format() const { return ( f_FloatFormat ); }
inline const char * AsciiTable::g_float_format() const { return ( g_FloatFormat ); }

inline bool AsciiTable::comma_string() const { return ( DoCommaString ); }

inline bool AsciiTable::delete_trailing_blank_rows() const { return ( DeleteTrailingBlankRows ); }

inline bool AsciiTable::elim_trailing_whitespace() const { return ( ElimTrailingWhitespace ); }

inline bool AsciiTable::decimal_points_aligned() const { return ( DecimalPointsAligned ); }


////////////////////////////////////////////////////////////////////////

   //
   //  externs
   //

extern ostream & operator<<(ostream &, AsciiTable &);

extern void justified_item(const char * text, const int field_width, const char pad, const AsciiTableJust just, char * out);

extern void justified_item(const char * text, const int field_width, const AsciiTableJust just, char * out);

extern void copy_ascii_table_row(const AsciiTable &at_from, const int r_from, AsciiTable &at_to, const int r_to);

extern void justify_met_at(AsciiTable &at, const int n_hdr_cols);

extern ConcatString check_hdr_str(const ConcatString,
                                  bool space_to_underscore = false);


////////////////////////////////////////////////////////////////////////


#endif   /*  __ASCII_TABLE_H__  */


////////////////////////////////////////////////////////////////////////



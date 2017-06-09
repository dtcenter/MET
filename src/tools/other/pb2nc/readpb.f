C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2013
C*      ** University Corporation for AtmospheriC*Research (UCAR)
C*      ** National Center for AtmospheriC*Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C-----------------------------------------------------------------------
C-----------------------------------------------------------------------
        SUBROUTINE READPB  ( lunit, iret, cnlev, chdr, cevns )
C
C*      This subroutine will read and combine the mass and wind subsets
C*      of the next station report in the prepbufr file.  It is styled
C*      after function IREADNS, and it only requires the prepbufr file
C*      to be opened for reading with OPENBF.  The combined station
C*      report is returned to the caller in COMMON /PREPBC/.
C*      This common area contains the number of levels in the report,
C*      a one dimensional array with the header information, and a four
C*      dimensional array containing all events from the variables POB,
C*      QOB, TOB, ZOB, UOB, and VOB for the report.
C*
C*      The header array contains the following list of mnemonics:
C*
C*      SID XOB YOB DHR ELV TYP T29 ITP
C*
C*      The 4-D array of data, EVNS ( ii, lv, jj, kk ), is indexed
C*      as follows:
C*
C*      "ii" indexes the event data types; these consist of:
C*          1) OBservation
C*          2) Quality Mark
C*          3) Program Code
C*          4) Reason Code
C*          5) ForeCast value
C*          6) ANalysed value
C*          7) office note CATegory
C*      "lv" indexes the levels of the report
C*      "jj" indexes the event stacks
C*      "kk" indexes the variable types (p,q,t,z,u,v)
C*
C*      Note that the structure of this array is identical to one
C*      returned from UFBEVN, with an additional (4th) dimension to
C*      include the six variable types into the same array.
C*
C*      The return codes are as follows:
C*      iret =  0 - normal return
C*           =  1 - the station report within COMMON /PREPBC/ contains the
C*                  last available subset from within the prepbufr file
C*           = -1 - there are no more subsets available from within the
C*                  prepbufr file        
C*
        INCLUDE       'readpb.prm'
C*
        INTEGER       cnlev
        REAL*8        chdr ( MXR8PM )
        REAL*8        cevns ( MXR8PM, MXR8LV, MXR8VN, MXR8VT )
C*
        CHARACTER*(MXSTRL) head
     +          / 'SID XOB YOB DHR ELV TYP T29 ITP' /
C*
        CHARACTER*(MXSTRL) ostr ( MXR8VT )
     +         / 'POB PQM PPC PRC PFC PAN CAT',
     +            'QOB QQM QPC QRC QFC QAN CAT',
     +            'TOB TQM TPC TRC TFC TAN CAT',
     +            'ZOB ZQM ZPC ZRC ZFC ZAN CAT',
     +            'UOB WQM WPC WRC UFC UAN CAT',
     +            'VOB WQM WPC WRC VFC VAN CAT' /
C*
        REAL*8      r8sid, r8sid2
C*
        CHARACTER*8 csid, csid2
C*
        EQUIVALENCE ( r8sid, csid ), ( r8sid2, csid2 )
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      IREADNS should be called by the caller to advance the subset pointer to the next subset.
C*
C*      Read the HDR and EVNS data for the subset that is currently
C*      being pointed to.
C*
        CALL UFBINT  ( lunit, hdr, MXR8PM, 1, jret, head )
C
        DO ii = 1, MXR8VT
           CALL UFBEVN  ( lunit, evns ( 1, 1, 1, ii ), MXR8PM, MXR8LV,
     +                    MXR8VN, nlev, ostr (ii) )
        END DO
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
   20   cnlev = nlev
C
        DO ii = 1, MXR8PM
           chdr ( ii ) = hdr ( ii )
        END DO
C
        DO lv = 1, nlev
           DO kk = 1, MXR8VT
              DO jj = 1, MXR8VN
                 DO ii = 1, MXR8PM
                    cevns ( ii, lv, jj, kk ) = evns ( ii, lv, jj, kk )
                 END DO
              END DO
           END DO
        END DO
C
        RETURN
        END

        
        SUBROUTINE READPBINT  ( lunit, iret, cnlev, cobs, ostr, olen )
C
C*      This subroutine will read and combine the mass and wind subsets
C*      of the next station report in the prepbufr file.  It is styled
C*      after function IREADNS, and it only requires the prepbufr file
C*      to be opened for reading with OPENBF.  The combined station
C*      report is returned to the caller in COMMON /PREPBC/.
C*      This common area contains the number of levels in the report,
C*      a one dimensional array with the header information, and a four
C*      dimensional array containing all events from the variables POB,
C*      QOB, TOB, ZOB, UOB, and VOB for the report.
C*
C*      The 2-D array of data, cobs ( ii, lv ), is indexed
C*      as follows:
C*
C*      "ii" indexes ForeCast value
C*      "lv" indexes the levels of the report
C*
C*      The return codes are as follows:
C*      iret =  0 - normal return
C*           =  1 - the station report within COMMON /PREPBC/ contains the
C*                  last available subset from within the prepbufr file
C*           = -1 - there are no more subsets available from within the
C*                  prepbufr file        
C*
        INCLUDE       'readpb.prm'
C*
        INTEGER       cnlev, olen
        REAL*8        cobs ( MXR8PM, MXR8LV )
C*
        CHARACTER*(MXSTRL) ostr
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      Read the HDR and EVNS data for the subset that is currently
C*      being pointed to.
C*
        CALL UFBINT(lunit,obsi,MXR8PM,MXR8LV,nlev,ostr(1:olen))
C
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
   20   cnlev = nlev
C
        DO lv = 1, nlev
           DO ii = 1, MXR8PM
              cobs ( ii, lv ) = obsi ( ii, lv )
           END DO
        END DO
C
        RETURN
        END
        
        
        SUBROUTINE READPBEVT  ( lunit, iret, cnlev, cobs, ostr, olen )
C
C*      This subroutine will read and combine the mass and wind subsets
C*      of the next station report in the prepbufr file.  It is styled
C*      after function IREADNS, and it only requires the prepbufr file
C*      to be opened for reading with OPENBF.  The combined station
C*      report is returned to the caller in COMMON /PREPBC/.
C*      This common area contains the number of levels in the report,
C*      a one dimensional array with the header information, and a four
C*      dimensional array containing all events from the variables POB,
C*      QOB, TOB, ZOB, UOB, and VOB for the report.
C*
C*      The header array contains the following list of mnemonics:
C*
C*      SID XOB YOB DHR ELV TYP T29 ITP
C*
C*      The 4-D array of data, EVNS ( ii, lv, jj, kk ), is indexed
C*      as follows:
C*
C*      "ii" indexes the event data types; these consist of:
C*          1) OBservation
C*          2) Quality Mark
C*          3) Program Code
C*          4) Reason Code
C*          5) ForeCast value
C*          6) ANalysed value
C*          7) office note CATegory
C*      "lv" indexes the levels of the report
C*      "jj" indexes the event stacks
C*      "kk" indexes the variable types (p,q,t,z,u,v)
C*
C*      Note that the structure of this array is identical to one
C*      returned from UFBEVN, with an additional (4th) dimension to
C*      include the six variable types into the same array.
C*
C*      The return codes are as follows:
C*      iret =  0 - normal return
C*           =  1 - the station report within COMMON /PREPBC/ contains the
C*                  last available subset from within the prepbufr file
C*           = -1 - there are no more subsets available from within the
C*                  prepbufr file        
C*
        INCLUDE       'readpb.prm'
C*
        INTEGER       cnlev, olen
        REAL*8        cobs ( MXR8PM, MXR8LV, MXR8VN )
C*
        CHARACTER*(MXSTRL) ostr
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      Read the one obs data for the subset that is currently
C*      being pointed to.
C*
        CALL UFBEVN(lunit,obse,MXR8PM,MXR8LV,MXR8VN,nlev,ostr(1:olen))
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
   20   cnlev = nlev
C
        DO lv = 1, nlev
           DO jj = 1, MXR8VN
              DO ii = 1, MXR8PM
                 cobs ( ii, lv, jj ) = obse( ii, lv, jj )
              END DO
           END DO
        END DO
C
        RETURN
        END
        
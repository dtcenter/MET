C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2018
C*      ** University Corporation for AtmospheriC*Research (UCAR)
C*      ** National Center for AtmospheriC*Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C-----------------------------------------------------------------------
C-----------------------------------------------------------------------
        SUBROUTINE READPB (lunit,iret,cnlev,chdr,cevns,reqlev )
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
        INTEGER       cnlev, max_lvl, reqlev
        REAL*8        chdr ( MXR8PM )
        REAL*8        cevns ( MXR8PM, MXR8LV, MXR8VN, MXR8VT )
        LOGICAL       small_buf, medium_buf, tiny_buf
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
        tiny_buf = .FALSE.
        small_buf = .FALSE.
        medium_buf = .FALSE.
        IF (reqlev .LE. MXR8LV_T) THEN
           tiny_buf = .TRUE.
        ELSE IF (reqlev .LE. MXR8LV_S) THEN
           small_buf = .TRUE.
        ELSE IF (reqlev .LE. MXR8LV_M) THEN
           medium_buf = .TRUE.
        END IF
        DO ii = 1, MXR8VT
           IF (small_buf) THEN
              CALL UFBEVN  ( lunit, evns_s ( 1, 1, 1, ii ), MXR8PM,
     +                       MXR8LV_S, MXR8VN, nlev, ostr (ii) )
           ELSE IF (medium_buf) THEN
              CALL UFBEVN  ( lunit, evns_m ( 1, 1, 1, ii ), MXR8PM,
     +                       MXR8LV_M, MXR8VN, nlev, ostr (ii) )
           ELSE IF (tiny_buf) THEN
              CALL UFBEVN  ( lunit, evns_t ( 1, 1, 1, ii ), MXR8PM,
     +                       MXR8LV_T, MXR8VN, nlev, ostr (ii) )
           ELSE
              CALL UFBEVN  ( lunit, evns ( 1, 1, 1, ii ), MXR8PM,
     +                       MXR8LV, MXR8VN, nlev, ostr (ii) )
           END IF
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
                    IF (small_buf) THEN
                       cevns ( ii, lv, jj, kk ) = evns_s(ii,lv,jj,kk)
                    ELSE IF (medium_buf) THEN
                       cevns ( ii, lv, jj, kk ) = evns_m(ii,lv,jj,kk)
                    ELSE IF (tiny_buf) THEN
                       cevns ( ii, lv, jj, kk ) = evns_t(ii,lv,jj,kk)
                    ELSE
                       cevns ( ii, lv, jj, kk ) = evns( ii, lv, jj, kk )
                    END IF
                 END DO
              END DO
           END DO
        END DO
        IF (cnlev .NE. nlev) THEN
           PRINT *,'   === WARN === at READPB, '
     *            ,'The vertical level was overridden!!!'
     *            ,cnlev, ' should be ', nlev
        END IF
C
        RETURN
        END

        
        SUBROUTINE READPB_HDR (lunit,iret,chdr)
C
C*      This subroutine will read the header information of the next
C*      station report in the prepbufr file.  It is styled after function
C*      IREADNS, and it only requires the prepbufr file to be opened for
C*      reading with OPENBF. The combined station report is returned to the
C*      caller in COMMON /PREPBC/.
C*      This common area contains a one dimensional array with the header
C*      information for the report.
C*
C*      The header array contains the following list of mnemonics:
C*
C*      SID XOB YOB DHR ELV TYP T29 ITP
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
        REAL*8        chdr ( MXR8PM )
C*
        CHARACTER*(MXSTRL) head
     +          / 'SID XOB YOB DHR ELV TYP T29 ITP' /
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      IREADNS should be called by the caller to advance the subset
C*      pointer to the next subset.
C*
C*      Read the HDR data for the subset that is currently being pointed to.
C*
        CALL UFBINT  ( lunit, hdr, MXR8PM, 1, jret, head )
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
        DO ii = 1, MXR8PM
           chdr ( ii ) = hdr ( ii )
        END DO

        RETURN
        END
        
        SUBROUTINE READPBINT (lunit,iret,cnlev,cobs,ostr,olen,reqlev)
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
        INTEGER       cnlev, olen, reqlev, max_nlev
        REAL*8        cobs ( MXR8PM, MXR8LV )
        LOGICAL       small_buf, medium_buf, tiny_buf
C*
        CHARACTER*(MXSTRL) ostr
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      Read the HDR and EVNS data for the subset that is currently
C*      being pointed to.
C*
        tiny_buf = .FALSE.
        small_buf = .FALSE.
        medium_buf = .FALSE.
        IF (reqlev .LE. MXR8LV_T) THEN
           tiny_buf = .TRUE.
           CALL UFBINT(lunit,obsi_t,MXR8PM,MXR8LV_T,nlev,ostr(1:olen))
        ELSE IF (reqlev .LE. MXR8LV_S) THEN
           small_buf = .TRUE.
           CALL UFBINT(lunit,obsi_s,MXR8PM,MXR8LV_S,nlev,ostr(1:olen))
        ELSE IF (reqlev .LE. MXR8LV_M) THEN
           medium_buf = .TRUE.
           CALL UFBINT(lunit,obsi_m,MXR8PM,MXR8LV_M,nlev,ostr(1:olen))
        ELSE
           CALL UFBINT(lunit,obsi,MXR8PM,MXR8LV,nlev,ostr(1:olen))
        END IF
C
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
   20   cnlev = nlev
C
        max_nlev = nlev
        if (max_nlev .GT. reqlev) max_nlev = reqlev
        DO lv = 1, max_nlev
           DO ii = 1, MXR8PM
              IF (small_buf) THEN
                 cobs ( ii, lv ) = obsi_s ( ii, lv )
              ELSE IF (medium_buf) THEN
                 cobs ( ii, lv ) = obsi_m ( ii, lv )
              ELSE IF (tiny_buf) THEN
                 cobs ( ii, lv ) = obsi_t ( ii, lv )
              ELSE
                 cobs ( ii, lv ) = obsi ( ii, lv )
              END IF
           END DO
        END DO
C
        RETURN
        END
        
        
        SUBROUTINE READPBEVT(lunit,iret,cnlev,cobs,ostr,olen,reqlev)
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
        INTEGER       cnlev, olen, reqlev, max_nlev
        REAL*8        cobs ( MXR8PM, MXR8LV, MXR8VN )
        LOGICAL       small_buf, medium_buf, tiny_buf
C*
        CHARACTER*(MXSTRL) ostr
C*
C-----------------------------------------------------------------------
        iret = 0
C*
C*      Read the one obs data for the subset that is currently
C*      being pointed to.
C*
        tiny_buf = .FALSE.
        small_buf = .FALSE.
        medium_buf = .FALSE.
        IF (reqlev .LE. MXR8LV_T) THEN
          tiny_buf = .TRUE.
          CALL UFBEVN(lunit,obse_t,MXR8PM,MXR8LV_T,MXR8VN,nlev,
     +                ostr(1:olen))
        ELSEIF (reqlev .LE. MXR8LV_S) THEN
          small_buf = .TRUE.
          CALL UFBEVN(lunit,obse_s,MXR8PM,MXR8LV_S,MXR8VN,nlev,
     +                ostr(1:olen))
        ELSE IF (reqlev .LE. MXR8LV_M) THEN
          medium_buf = .TRUE.
          CALL UFBEVN(lunit,obse_m,MXR8PM,MXR8LV_M,MXR8VN,nlev,
     +                ostr(1:olen))
        ELSE
          CALL UFBEVN(lunit,obse,MXR8PM,MXR8LV,MXR8VN,nlev,ostr(1:olen))
        END IF
C
C*      Prior to returning, copy the contents COMMON block PREPBC into 
C*      variables passed to the subroutine.
C
   20   cnlev = nlev
C
        max_nlev = nlev
        if (max_nlev .GT. reqlev) max_nlev = reqlev
        DO jj = 1, MXR8VN
           DO lv = 1, max_nlev
              DO ii = 1, MXR8PM
                 IF (small_buf) THEN
                    cobs ( ii, lv, jj ) = obse_s( ii, lv, jj )
                 ELSE IF (medium_buf) THEN
                    cobs ( ii, lv, jj ) = obse_m( ii, lv, jj )
                 ELSE IF (tiny_buf) THEN
                    cobs ( ii, lv, jj ) = obse_t( ii, lv, jj )
                 ELSE
                    cobs ( ii, lv, jj ) = obse( ii, lv, jj )
                 END IF
              END DO
           END DO
        END DO
C
        IF (cnlev .NE. nlev) THEN
           PRINT *,'   === WARN === at READPBEVT, '
     *            ,'The vertical level was overridden!!!'
     *            ,cnlev, ' should be ', nlev
        END IF
        RETURN
        END
        

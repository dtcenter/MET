C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2026
C*      ** University Corporation for Atmospheric Research (UCAR)
C*      ** National Center for Atmospheric Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE DUMPPB ( PBFILE, FID, OUTDIR, LEN1,
     +                      PREFIX, LEN2, FLAG )
C*
        INCLUDE   'readpb.prm'
C*
        INTEGER, PARAMETER :: NFILO = 18

        ! -- Arguments
        ! Use fixed-length strings since C++ passes no hidden lengths
        CHARACTER(LEN=FILEMXSTRL), intent(in) :: PBFILE
        INTEGER,                   intent(in) :: FID
        CHARACTER(LEN=FILEMXSTRL), intent(in) :: OUTDIR
        INTEGER,                   intent(in) :: LEN1
        CHARACTER(LEN=FILEMXSTRL), intent(in) :: PREFIX
        INTEGER,                   intent(in) :: LEN2
        INTEGER,                   intent(in) :: FLAG(NFILO)

        ! -- Local variables
        CHARACTER(LEN=MXSTRL) :: outstg
        CHARACTER(LEN=8)      :: subset
        INTEGER               :: idate, ierrpb, nlev
        INTEGER               :: ii, iuno, kk, jj, mm, lv
        LOGICAL               :: found
        REAL(dp)              :: hdr(MXR8PM)
        REAL(dp), ALLOCATABLE :: evns(:,:,:,:)

        ! initialize var and iunso and FILO consistent with original
        CHARACTER var ( MXR8VT )
     +          /'P','Q','T','Z','U','V'/

        INTEGER iunso ( NFILO )
     +          /   51,   52,   53,   54,   55,
     +              56,   57,   58,   59,   60,
     +              61,   62,   63,   64,   65,
     +              66,   67,   68  /
C*
        CHARACTER(LEN=6) FILO ( NFILO )
     +          / 'ADPUPA', 'AIRCAR', 'AIRCFT', 'ADPSFC', 'ERS1DA',
     +            'GOESND', 'GPSIPW', 'MSONET', 'PROFLR', 'QKSWND',
     +            'RASSDA', 'SATEMP', 'SATWND', 'SFCBOG', 'SFCSHP',
     +            'SPSSMI', 'SYNDAT', 'VADWND' /

C-----------------------------------------------------------------------
C
C*      Open the output files.
C
        DO ii = 1, NFILO
           IF ( FLAG ( ii ) .eq. 1 ) THEN
              OPEN  ( UNIT = iunso ( ii ),
     +           FILE = OUTDIR(1:LEN1) // '/dump_' //
     +                  PREFIX(1:LEN2) // '_' // FILO(ii) // '.txt' )
           END IF
        END DO
C
C*      Open the input file.
C
        OPEN  ( UNIT = FID, FILE = PBFILE, FORM = 'UNFORMATTED' )
        CALL OPENBF  ( FID, 'IN', FID )
        CALL DATELEN  ( 10 )
        ALLOCATE ( evns ( MXR8PM, MXR8LV, MXR8VN, MXR8VT ) )
C
C*      Get the next station report from the input file.
C
  10    CALL READPBDUMP  ( FID, NFILO, FILO, FLAG, subset, idate,
     +                     hdr, evns, nlev, ierrpb )
        IF ( ierrpb .eq. -1 )  THEN
            GO TO 20
        END IF
C
C*      Set the appropriate output file unit number.
C
        ii = 1
        found = .false.
        DO WHILE  ( ( .not. found ) .and. ( ii .le. NFILO ) )
            IF  ( ( subset (1:6) .eq. FILO ( ii ) ) .and.
     +            ( FLAG ( ii )  .eq. 1 ) )  THEN
                found = .true.
                iuno = iunso ( ii )
            ELSE
                ii = ii + 1
            END IF
        END DO
        IF  ( .not. found )  THEN
            IF  ( ierrpb .eq. 0 )  THEN
                GO TO 10
            ELSE
                GO TO 20
            END IF
        END IF
C
C*      Print the HDR data for this station report.
C
        WRITE  ( UNIT = iuno,
     +            FMT = '( /, A8, 1X, 2F7.2, 1X, F7.3, 1X, 2F8.1, '  //
     +                  '1X, F7.1, 1X, F6.1 )' )
     +            ( hdr (ii), ii = 1, 8 )
C
C*      Print the EVNS data for this station report.
C
        DO lv = 1, nlev
           WRITE  ( UNIT = iuno, FMT = '( 80("-") )' )
           WRITE  ( UNIT = iuno, FMT = '( "lev ", I4, 7A9 )' )
     +        lv, 'ob', 'qm', 'pc', 'rc', 'fc', 'an', 'cat'
           WRITE  ( UNIT = iuno, FMT = '( 80("-") )' )
           DO kk = 1, MXR8VT
              DO jj = 1, MXR8VN
                 WRITE  ( UNIT = outstg, FMT = '( A8, 7(1X,F8.1) )' )
     +              var (kk), ( evns ( ii, lv, jj, kk ), ii = 1, 7 )
                 DO mm = 1, MXSTRL
                    IF  ( outstg (mm:mm) .eq. '*' )  THEN
                       outstg (mm:mm) = ' '
                    END IF
                 END DO
                 IF  ( outstg (9:64) .ne. ' ' )  THEN
                    WRITE  ( UNIT = iuno, FMT = '(A80)' )  outstg
                 ENDIF
              END DO
           END DO
        END DO
C
        IF  ( ierrpb .eq. 0 )  THEN
           GO TO 10
        END IF
C
C*      Close the output files.
C
  20    DO ii = 1, NFILO
           IF ( FLAG ( ii ) .eq. 1 ) THEN
              CLOSE ( UNIT = iunso ( ii ) )
           END IF
        END DO
        DEALLOCATE ( evns )
C
C*      Close the input file
C
        CALL CLOSBF  ( FID )
        CLOSE  ( UNIT = FID )
C
        RETURN
        END SUBROUTINE DUMPPB

C-----------------------------------------------------------------------
C-----------------------------------------------------------------------
        SUBROUTINE READPBDUMP  ( lunit, nfilo, filo, flag, subset,
     +                           idate, hdr, evns, nlev, iret )
C
C*      This subroutine will read and combine the mass and wind subsets
C*      of the next station report in the prepbufr file.  It is styled
C*      after function IREADNS, and it only requires the prepbufr file
C*      to be opened for reading with OPENBF.  The combined station
C*      report is returned to the caller in the HDR, EVNS, and NLEV
C*      arguments: a one dimensional array with the header information,
C*      a four dimensional array containing all events from the
C*      variables POB, QOB, TOB, ZOB, UOB, and VOB for the report, and
C*      the number of levels in the report.
C*
C*      Data is only read for message types in FILO whose FLAG is 1.
C*      For other message types, only SUBSET and IDATE are returned.
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
C*          7) office note CATegory (data level CATegory)
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
C*           =  1 - the returned station report contains the last
C*                  available subset from within the prepbufr file
C*           = -1 - there are no more subsets available from within the
C*                  prepbufr file
C*
        INCLUDE       'readpb.prm'
C*
! -- Arguments
        INTEGER, INTENT(IN)          :: lunit
        INTEGER, INTENT(IN)          :: nfilo
        CHARACTER(LEN=*), INTENT(IN) :: filo(nfilo)
        INTEGER, INTENT(IN)          :: flag(nfilo)
        CHARACTER(LEN=*), INTENT(OUT):: subset
        INTEGER, INTENT(OUT)         :: idate
        REAL(dp), INTENT(OUT)        :: hdr(MXR8PM)
        REAL(dp), INTENT(OUT)        :: evns(MXR8PM, MXR8LV, MXR8VN,
     +                                       MXR8VT)
        INTEGER, INTENT(OUT)         :: nlev
        INTEGER, INTENT(OUT)         :: iret

! -- Local arrays and temporaries
        CHARACTER(LEN=MXSTRL) :: head
        CHARACTER(LEN=MXSTRL) :: ostr(MXR8VT)
        REAL(dp)              :: hdr2(MXR8PM)
        REAL(dp), ALLOCATABLE :: evns2(:,:,:,:)
        REAL(dp)              :: pob2
        CHARACTER(LEN=8)      :: csid, csid2, subst2
        LOGICAL               :: match = .TRUE.
        LOGICAL               :: lvfound, keep
        INTEGER               :: ii, lv, lv2, kk, jj
        INTEGER               :: idate2, jret
        INTEGER               :: nlev2

        DATA head / 'SID XOB YOB DHR ELV TYP T29 ITP' /
        DATA ostr
     +          / 'POB PQM PPC PRC PFC PAN CAT',
     +            'QOB QQM QPC QRC QFC QAN CAT',
     +            'TOB TQM TPC TRC TFC TAN CAT',
     +            'ZOB ZQM ZPC ZRC ZFC ZAN CAT',
     +            'UOB WQM WPC WRC UFC UAN CAT',
     +            'VOB WQM WPC WRC VFC VAN CAT'  /

        SAVE match, subst2, idate2, evns2
C-----------------------------------------------------------------------
        iret = 0
C*
C*      If the previous call to this subroutine did not yield matching
C*      mass and wind subsets, then IREADNS is already pointing at an
C*      unmatched subset.  Otherwise, call IREADNS to advance the subset
C*      pointer to the next subset.
C*
        IF ( match )  THEN
           IF  ( IREADNS  ( lunit, subset, idate ) .ne. 0 )  THEN
              iret = -1
              RETURN
           END IF
        ELSE
           subset = subst2
           idate = idate2
        END IF
C*
C*      Skip subsets whose message type will not be dumped, since
C*      UFBEVN is expensive.  Matching mass and wind subsets share a
C*      message type, so this cannot split a station report.
C*
        keep = .false.
        DO ii = 1, nfilo
           IF  ( ( subset (1:6) .eq. filo ( ii ) ) .and.
     +           ( flag ( ii ) .eq. 1 ) )  keep = .true.
        END DO
        IF  ( .not. keep )  THEN
           nlev = 0
           match = .true.
           RETURN
        END IF
C*
C*      Read the HDR and EVNS data for the subset that is currently
C*      being pointed to.
C*
        CALL UFBINT  ( lunit, hdr, MXR8PM, 1, jret, head )
        DO ii = 1, MXR8VT
           CALL UFBEVN  ( lunit, evns ( 1, 1, 1, ii ), MXR8PM, MXR8LV,
     +                    MXR8VN, nlev, ostr (ii) )
        END DO
C
C*      Now, advance the subset pointer to the following subset and
C*      read its HDR data.  Store its message type and date in SUBST2
C*      and IDATE2 so that the next call can use them if this subset
C*      does not match.
C
        IF ( IREADNS ( lunit, subst2, idate2 ) .ne. 0 )  THEN
           iret = 1
C
C*         Reset for the next file.
C
           match = .true.
           RETURN
        END IF
        CALL UFBINT ( lunit, hdr2, MXR8PM, 1, jret, head )
C
C*      Check whether these two subsets have identical message types
C*      and SID, YOB, XOB, ELV, and DHR values.  If so, then they are
C*      matching mass and wind subsets for a single station report.
C
        match = .true.
C
        IF  ( subset .ne. subst2 )  THEN
           match = .false.
           RETURN
        END IF
C
C*      The 8-character SID is stored in the bytes of a REAL*8 value.
C
        csid = TRANSFER ( hdr (1), csid )
        csid2 = TRANSFER ( hdr2 (1), csid2 )
        IF  ( csid .ne. csid2 )  THEN
           match = .false.
           RETURN
        END IF
C
        DO ii = 2, 5
           IF  ( hdr (ii) .ne. hdr2 (ii) )  THEN
              match = .false.
              RETURN
           END IF
        END DO
C
C*      Read the EVNS data for the second of the two matching subsets.
C
        IF  ( .not. ALLOCATED ( evns2 ) )  THEN
           ALLOCATE ( evns2 ( MXR8PM, MXR8LV, MXR8VN, MXR8VT ) )
        END IF
        DO ii = 1, MXR8VT
           CALL UFBEVN  ( lunit, evns2 ( 1, 1, 1, ii ), MXR8PM, MXR8LV,
     +                    MXR8VN, nlev2, ostr (ii) )
        ENDDO
C
C*      Combine the EVNS data for the two matching subsets into a
C*      single 4-D array.  Do this by merging the EVNS2 array into
C*      the EVNS array.
C
        DO lv2 = 1, nlev2
           pob2 = evns2 ( 1, lv2, 1, 1 )
           lvfound = .false.
           DO lv = 1, nlev
              IF  ( evns ( 1, lv, 1, 1 ) .eq. pob2 )  THEN
C
C*            This pressure level from the second subset also exists
C*            in the first subset, so overwrite any "missing" piece
C*            of data for this pressure level in the first subset
C*            with the corresponding piece of data from the second
C*            subset (since this results in no net loss of data!).
C
              DO kk = 1, MXR8VT
                 DO jj = 1, MXR8VN
                    DO ii = 1, MXR8PM
                       IF  ( evns ( ii, lv, jj, kk ) .eq. R8BFMS ) THEN
                          evns ( ii, lv, jj, kk ) =
     +                       evns2 ( ii, lv2, jj, kk )
                       END IF
                    END DO
                 END DO
              END DO
              lvfound = .true.
              EXIT
              END IF
           END DO
C
C*         This pressure level from the second subset does not exist
C*         in the first subset, so add it to the end of the EVNS array.
C
           IF  ( ( .not. lvfound ) .and. ( nlev .lt. MXR8LV ) )  THEN
              nlev = nlev + 1
              DO kk = 1, MXR8VT
                 DO jj = 1, MXR8VN
                    DO ii = 1, MXR8PM
                       evns ( ii, nlev, jj, kk ) =
     +                    evns2 ( ii, lv2, jj, kk )
                    END DO
                 END DO
              END DO
           END IF
        END DO
C*
        RETURN
        END SUBROUTINE READPBDUMP

        SUBROUTINE DUMP_TBL( PBFILE, FID, TBL_NAME, LEN1 )
C*
        INCLUDE   'readpb.prm'
C*
        ! Use fixed-length strings since C++ passes no hidden lengths
        CHARACTER(LEN=FILEMXSTRL), INTENT(IN) :: PBFILE
        INTEGER, INTENT(IN)                   :: FID
        CHARACTER(LEN=FILEMXSTRL), INTENT(IN) :: TBL_NAME
        INTEGER, INTENT(IN)                   :: LEN1

        INTEGER :: TID
C-----------------------------------------------------------------------
C
C*      Open the output files.
C
        TID = 98
        OPEN  ( UNIT = TID, FILE = TBL_NAME(1:LEN1) )

C
C*      Dump BUFR table.
C
        CALL DXDUMP  ( FID, TID )
C
C*      Close the input file
C
        CLOSE  ( UNIT = TID )
C
        RETURN
        END SUBROUTINE DUMP_TBL


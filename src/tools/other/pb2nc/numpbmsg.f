C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2025
C*      ** University Corporation for Atmospheric Research (UCAR)
C*      ** National Center for Atmospheric Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE NUMPBMSG ( FID, NMSG ) BIND(C)
C*
        INCLUDE    'readpb.prm'
C*
        INTEGER, INTENT(IN)  :: FID
        INTEGER, INTENT(OUT) :: NMSG
        INTEGER :: FID2
        REAL(dp), DIMENSION(1,1) :: R8ARR
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        FID2 = -FID
        CALL UFBTAB  ( FID2, R8ARR, 1, 1, NMSG, ' ' )
C*
        END SUBROUTINE NUMPBMSG


        SUBROUTINE NUMPBMSG_NEW ( PBFILE, FID, NMSG )
C*
        INCLUDE    'readpb.prm'
C*
        CHARACTER(LEN=FILEMXSTRL), INTENT(IN) :: PBFILE
        INTEGER, INTENT(IN)  :: FID
        INTEGER, INTENT(OUT) :: NMSG
        INTEGER :: FID2
        REAL(dp), DIMENSION(1,1) :: R8ARR
        INTEGER :: IOS
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        OPEN  ( UNIT = FID, FILE = PBFILE, FORM = 'UNFORMATTED',
     &          ACTION='READ', IOSTAT=ios )
        IF (ios /= 0) THEN
           ! Could not open file; return NMSG = 0 to indicate failure/no messages
           NMSG = 0
           RETURN
        ENDIF
        FID2 = -FID
        CALL UFBTAB  ( FID2, R8ARR, 1, 1, NMSG, ' ' )
        CLOSE ( UNIT = FID )
C*
        END SUBROUTINE NUMPBMSG_NEW


        SUBROUTINE GET_TMIN ( FID, TMIN ) BIND(C)
C*
        INTEGER, INTENT(IN)  :: FID
        INTEGER, INTENT(OUT) :: TMIN
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        TMIN = IUPVS01(FID,'MINU')
C*
        END SUBROUTINE GET_TMIN


C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2023
C*      ** University Corporation for Atmospheric Research (UCAR)
C*      ** National Center for Atmospheric Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE NUMPBMSG ( FID, NMSG )
C*
        INCLUDE    'readpb.prm'
C*
        INTEGER    FID, FID2
        INTEGER    NMSG
        REAL*8     R8ARR(1, 1)
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        FID2 = -FID
        CALL UFBTAB  ( FID2, R8ARR, 1, 1, NMSG, ' ' )
C*
        END


        SUBROUTINE NUMPBMSG_NEW ( PBFILE, FID, NMSG )
C*
        INCLUDE    'readpb.prm'
C*
        CHARACTER  PBFILE*(FILEMXSTRL)
        INTEGER    FID, FID2
        INTEGER    NMSG
        REAL*8     R8ARR(1, 1)
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        OPEN  ( UNIT = FID, FILE = PBFILE, FORM = 'UNFORMATTED' )
        FID2 = -FID
        CALL UFBTAB  ( FID2, R8ARR, 1, 1, NMSG, ' ' )
        CLOSE ( UNIT = FID )
C*
        END


        SUBROUTINE GET_TMIN ( FID, TMIN )
C*
        INTEGER    FID, TMIN
C*
C-----------------------------------------------------------------------
C*
C*      Call UFBTAB to figure out how many messages the PrepBufr file
C*      attached to FID contains
C*
        FID2 = -FID
        TMIN = IUPVS01(FID,'MINU')
C*
        END
        

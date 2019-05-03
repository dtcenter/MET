C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2018
C*      ** University Corporation for AtmospheriC*Research (UCAR)
C*      ** National Center for AtmospheriC*Research (NCAR)
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
        
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2023
C*      ** University Corporation for Atmospheric Research (UCAR)
C*      ** National Center for Atmospheric Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE CLOSEPB ( FID )
C*
        INCLUDE    'readpb.prm'
C*
        INTEGER    FID
C*
C-----------------------------------------------------------------------
C*
C*      Close the file.
C*
        CALL CLOSBF  ( FID )
        CLOSE ( UNIT = FID )
C*
        END

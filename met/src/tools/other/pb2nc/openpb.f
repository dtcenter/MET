C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2018
C*      ** University Corporation for AtmospheriC*Research (UCAR)
C*      ** National Center for AtmospheriC*Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE OPENPB ( PBFILE, FID )
C*
        INCLUDE    'readpb.prm'
C*
        CHARACTER  PBFILE*(FILEMXSTRL)
        INTEGER    FID
C*
C-----------------------------------------------------------------------
C*
C*      Open the input file.
C*
        OPEN  ( UNIT = FID, FILE = PBFILE, FORM = 'UNFORMATTED' )
        CALL OPENBF  ( FID, 'IN', FID )
        CALL DATELEN  ( 10 )
C*
        END

C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
C*      ** Copyright UCAR (c) 1992 - 2026
C*      ** University Corporation for Atmospheric Research (UCAR)
C*      ** National Center for Atmospheric Research (NCAR)
C*      ** Research Applications Lab (RAL)
C*      ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
C*      *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

        SUBROUTINE OPENPB ( PBFILE, FID )
C*
        INCLUDE    'readpb.prm'
C*
        CHARACTER(len=FILEMXSTRL), INTENT(IN) :: PBFILE
        INTEGER, INTENT(IN)                   :: FID
C*
C-----------------------------------------------------------------------
C*
C*      Open the input file.
C*
        OPEN  ( UNIT = FID, FILE = PBFILE, FORM = 'UNFORMATTED',
     &          ACTION='read' )
        CALL OPENBF  ( FID, 'IN', FID )
        CALL DATELEN  ( 10 )
C*
        END SUBROUTINE OPENPB

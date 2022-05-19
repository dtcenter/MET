       SUBROUTINE CALPBL(T,Q,P,Z,U,V,MZBL,HPBL,jpbl)
C
C --------------------------------------------------------------------
C  ATMOSPHERIC BOUNDARY-LAYER HEIGHT CALCULATION
C --------------------------------------------------------------------
C  REFERENCES:
C ------------
C  TROEN, I. AND L. MAHRT, 1986:  A SIMPLE MODEL OF THE ATMOSPHERIC
C    BOUNDARY LAYER:  SENSITIVITY TO SURFACE EVAPORATION.  BOUND.-
C    LAYER METEOROL., 37, 129-148.
C ------------
C  VOGELEZANG, D. H. P. AND A. A. M. HOLTSLAG, 1996:  EVALUATION AND
C    MODEL IMPACTS OF ALTERNATIVE BOUNDARY-LAYER HEIGHT FORMULAITONS.
C    BOUND.-LAYER METEOROL., 81, 245-269.   
C --------------------------------------------------------------------
C  VARIABLE  UNITS  DEFINITION
C  --------  -----  ----------
C  BETA      -      COEFFICIENT (SEE VOGELEZANG AND HOLTSLAG, 1996)
C  EVAP      M/S    KINEMATIC MOISTURE FLUX
C  GRAV      M/S2   GRAVITY
C  HEAT      K M/S  KINEMATIC HEAT FLUX
C  HPBL      M      ATMOSPHERIC BOUNDARY-LAYER DEPTH
C  JPBL      -      BOUNDARY-LAYER TOP (LEVEL NUMBER - INTEGER)
C  MZBL      -      TOP OF MODEL DOMAIN (LEVEL NUMBER - INTEGER)
C  LVL       -      CURRENT ATMOSPHERIC LEVEL
C  LVL3      -      3RD MODEL LEVEL (=2ND ATMOS LEVEL)
C  LVLP      -      PREVIOUS ATMOSPHERIC LEVEL
C  PR        -      PRANDTL NUMBER
C  Q         KG/KG  SPECIFIC HUMIDITY
C  RIF       -      FLUX RICHARDSON NUMBER
C  RIB       -      BULK RICHARDSON NUMBER FOR A GIVEN LAYER
C  RIBP      -      PREVIOUS BULK RICHARDSON NUMBER FOR A GIVEN LAYER
C  RICR      -      CRITICAL RICHARDSON NUMBER
C                     =0.25 FOR FINE (VERTICAL) RESOLUTION MODELS
C                     =1.00 FOR COURSE (VERTICAL) RESOLUTION MODELS
C  TH        K      POTENTIAL TEMPERATURE
C  TH2V      K      VIRTUAL POTENTIAL TEMPERATURE AT 2ND MODEL LEVEL
C                      (=1ST ATMOS LEVEL)
C  U         M/S    U-COMPONENT
C  USTAR     M/S    FRICTION VELOCITY
C  V         M/S    V-COMPONENT
C  WIND      M/S    WIND SPEED AT 2ND MODEL LEVEL (=1ST ATMOS LEVEL)
C  WIND2     M/S    SQUARE OF WIND
C  WINDL2    M/S    SUM OF SQUARE OF WIND COMPONENTS AT LVL
C  WDL2      M/S    SUM OF SQUARE OF WIND COMPONENT DIFFERENCE BETWEEN
C                     LVL AND 2ND MODEL LEVEL (=1ST ATMOS LEVEL) +
C                     WMIN**2
C  WMIN      M/S    MINIMUM WIND SPEED (FOR NUMERICAL STABILITY)
C  Z         M      HEIGHT
C                     1=1ST MODEL LEVEL (=SURFACE=0M)
C                     2=2ND MODEL LEVEL (=1ST ATMOS LEVEL, E.G. 20M)
C                     3=3RD MODEL LEVEL (=2ND ATMOS LEVEL), ETC
C --------------------------------------------------------------------
                             P A R A M E T E R
     & (H10E5=100000.E0
     &, EPSQ=2.E-12
     &, G=9.8E0,CP=1004.6E0,CAPA=0.28589641E0,ROG=287.04/G)
        PARAMETER (NLEV=256)
        DIMENSION Z(NLEV),U(NLEV),V(NLEV),TH(NLEV),Q(NLEV),
     &            P(NLEV),T(NLEV)

        DATA BETA,GRAV,WMIN,RICR /100.,9.806,0.01,0.25/
c==     DATA BETA,GRAV,WMIN,RICR /100.,9.806,0.01,1.00/

        do k=1,nlev
        th(k)=t(k)*(H10E5/p(k))**capa
        enddo

C --------------------------------------------------------------------
C  DETERMINE 1ST ATMOSPHERIC MODEL LEVEL VIRTUAL POTENTIAL TEMPERTURE
C  (TH2V) AND WIND SPEED (WINDL), AND SURFACE VIRTUAL HEAT FLUX
C  (HEATV) AND FLUX RICHARDSON NUMBER (RIF).  SET THE PREVIOUS BULK
C  RICHARDSON NUMBER (RIBP) EQUAL TO THE FLUX RICHARDSON NUMBER (RIF).
C --------------------------------------------------------------------
c==     TH2V  = TH(2)*(1.+0.608*Q(2))
        TH2V  = TH(1)*(1.+0.608*Q(1))
c==     WIND2 = U(2)**2+V(2)**2
        WIND2 = U(1)**2+V(1)**2
        WIND  = SQRT(WIND2)
c       HEATV = HEAT*(1.+0.608*Q(2))+0.608*TH(2)*EVAP
c       RIF   = -GRAV*Z(2)/(TH2V*WIND)*HEATV*PR/(USTAR**2)
c       RIBP  = RIF
        HEATV = 0.
        RIF   = 0.
        RIBP  = RIF
        USTAR = 0.1
C --------------------------------------------------------------------
C  USE FLUX RICHARDSON NUMBER FOR THE LOWEST LAYER.
C  IF FLUX RICHARDSON NUMBER (RIF) IS GREATER THAN CRITICAL RICHARDSON
C  NUMBER (RICR) THEN THE BOUNDARY-LAYER DEPTH IS THE LOWEST MODEL
C  LEVEL IN THE ATMOSPHERE (E.G. 20 M).
C --------------------------------------------------------------------
        IF (RIF .GT. RICR) THEN
c==       HPBL=Z(2)
c==       JPBL=2
          HPBL=Z(1)
          JPBL=1
        ELSE
C --------------------------------------------------------------------
C  BOUNDARY-LAYER DEPTH HIGHER THAN LOWEST MODEL LEVEL IN THE
C  ATMOSPHERE.  USE BULK RICHARDSON NUMBER FOR HIGHER LAYERS.
C --------------------------------------------------------------------
c==       LVLP=2
c==       LVL3=3
          LVLP=1
          LVL3=2
          DO 100 LVL=LVL3,MZBL
            THVL = TH(LVL)*(1.+0.608*Q(LVL))
C  FOLLOWING TROEN AND MAHRT (1986):
c-          WINDL2 = U(LVL)**2+V(LVL)**2
c-          RIB=(GRAV/TH2V)*(THVL-TH2V)*Z(LVL)/WINDL2
C  FOLLOWING VOGELEZANG AND HOLTSLAG (1996):
C            WDL2 = (U(LVL)-U(2))**2 + (V(LVL)-V(2))**2 + WMIN**2
C            RIB=(GRAV/TH2V)*(THVL-TH2V)*(Z(LVL)-Z(2)/
C     .       (WDL2+BETA*(USTAR**2))
            WDL2 = (U(LVL)-U(1))**2 + (V(LVL)-V(1))**2 + WMIN**2
            RIB=(GRAV/TH2V)*(THVL-TH2V)*(Z(LVL)-Z(1))/
     .       (WDL2+BETA*(USTAR**2))
C --------------------------------------------------------------------
C  IF BULK RICHARDSON NUMBER (RIB) EXCEEDS THE CRITICAL RICHARDSON
C  NUMBER (RICR), DETERMINE ABL HEIGHT USING LINEAR INTERPOLATION
C  BETWEEN HEIGHTS, AND PREVIOUS (RIBP) AND CURRENT (RIB) BULK
C  RICHARDSON NUMBERS.  LVL IS BOUNDARY-LAYER TOP LEVEL NUMBER.
C --------------------------------------------------------------------
            IF (RIB .GE. RICR) THEN
C  FOLLOWING TROEN AND MAHRT (1986):
c-            HPBL = Z(LVLP) + (Z(LVL)-Z(LVLP))*(RICR-RIBP)/(RIB-RIBP)
C  FOLLOWING VOGELEZANG AND HOLTSLAG (1996):
C              HBPL = Z(2) + Z(LVLP) + (Z(LVL)-Z(LVLP))*(RICR-RIBP)/
C     .          (RIB-RIBP)
c-            HPBL = Z(1) + Z(LVLP) + (Z(LVL)-Z(LVLP))*(RICR-RIBP)/
              HPBL = Z(LVLP) + (Z(LVL)-Z(LVLP))*(RICR-RIBP)/
     .          (RIB-RIBP)
              JPBL = LVL
C --------------------------------------------------------------------
C  BOUNDARY-LAYER DEPTH CALCULATION COMPLETE.
C --------------------------------------------------------------------
              GOTO 999   
            ENDIF
C --------------------------------------------------------------------
C  IF BULK RICHARDSON NUMBER (RIB) DOES NOT EXCEED THE CRITICAL
C  RICHARDSON NUMBER (RICR), SET PREVIOUS BULK RICHARDSON NUMBERS
C  (RIBP) AND LEVEL (LVL) EQUAL TO CURRENT VALUES, THEN INCREMENT LVL.
C --------------------------------------------------------------------
            RIBP = RIB
            LVLP = LVL

 100      CONTINUE
C --------------------------------------------------------------------
C  BOUNDARY-LEVEL DEPTH AT TOP OF MODEL DOMAIN
C --------------------------------------------------------------------
          HPBL = Z(MZBL)
          JPBL = MZBL

        ENDIF

 999    RETURN
        END

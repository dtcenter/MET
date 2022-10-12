C     ------------------------------------------------------------
      SUBROUTINE BTCLIP5(BASIN,IYR,IMO,IDY,IHR,NBT,BMO,BDY,BHR,
     *                   BLAT,BLON,BWS,CLAT,CLON,SWND)
C
C     Returns best-track cliper forecast.  Output is at 12, 24, 36,
C     48, 60, 72, 84, 96, 108, and 120 h.
C     Valid for Atlantic and East Pacific.
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     NBT           Number of elements in the best track data arrays.
C     BMO,BDY,BHR   Arrays of best track dates/times.
C     BLAT(NBT)     BT latitude (deg N).
C     BLON(NBT)     BT longitude (deg E).
C     BWS(NBT)      BT wind speed (kt).
C     CLAT(NVTX)    Returned array of BT CLIPER lats.
C     CLON(NVTX)    Returned array of BT CLIPER lons.
C     SWND(NVTX)    Returned array of BT SHIFOR winds.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION BMO(NBT), BDY(NBT), BHR(NBT)
      DIMENSION BLAT(NBT), BLON(NBT), BWS(NBT)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION FPCLIP(2,NVTX), IWSHIF(NVTX), SWND(NVTX)
      CHARACTER*2 BASIN
C
C
C
      RLAT0 = -9999.
      RLON0 = -9999.
      RLAT12= -9999.
      RLON12= -9999.
      DIR0  = -9999.
      SPD0  = -9999.
      WS0   = -9999.
      WS12  = -9999.
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
C
C     Compute input parameters for the CLIPER subroutine.
C     Start by checking for a BT match to desired time.
C     ---------------------------------------------------
      DO 110 L = 1,NBT
         IF (IMO.EQ.NINT(BMO(L)).AND.IDY.EQ.NINT(BDY(L))
     *       .AND.IHR.EQ.NINT(BHR(L)))
     *   THEN
            L0 = L
            GOTO 200
            ENDIF
110      CONTINUE
C
C     Could not find a best track entry for the requested time
C     --------------------------------------------------------
      GOTO 800
C
C
C     Found a valid match at index L0.  Get initial position.
C     -------------------------------------------------------
200   RLAT0 = BLAT(L0)
      RLON0 = -BLON(L0)
      WS0 = BWS(L0)
C
C     Get previous, next positions to estimate current speed
C     ------------------------------------------------------
      LB = L0-1
      LF = L0+1
      IF (LB.LT.1) LB=1
      IF (LB.EQ.LF) LF=LF+1
      IF (LF.GT.NBT) LF = NBT
      IF (LB.EQ.LF) GOTO 800
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR0)
      SPD0 = DIST/DELTAH
C
C
C     Get additional parameters for SHIFOR
C     ------------------------------------
      LB = L0-2
      LF = L0
      IF (LB.LT.1) LB=1
      IF (LB.EQ.LF) LF=LF+1
      IF (LF.GT.NBT) LF = NBT
      IF (LB.EQ.LF) GOTO 800
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR12)
      SPD12 = DIST/DELTAH
      WS12 = BWS(LB)
C
C
C     Get CLIPER forecast.
C     -----------------------------------------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL CLIP_5(IMO,IDY,IHR,RLAT0,RLON0,WS0,DIR0,SPD0,BASIN,FPCLIP)
C
C
C     Run best-track SHIFOR
C     ---------------------
      IF (DIR12.EQ.-9999. .OR. SPD12.EQ.-9999.) GOTO 800
      IF (WS12.EQ.-9999.) GOTO 800
      UCMP = DIR12
      VCMP = SPD12
      CALL UVCOMP(UCMP,VCMP)
      UCMP = -UCMP
      VCMP = -VCMP
      IF (BASIN.EQ.'AL') CALL ATSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
      IF (BASIN.EQ.'EP') CALL EPSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 550 K=1,NVTX
         CLAT(K) = FPCLIP(1,K)
         CLON(K) = -FPCLIP(2,K)
         IF (IWSHIF(K).GT.0) SWND(K) = IWSHIF(K)
550      CONTINUE
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE BTCLIPD5(BASIN,IYR,IMO,IDY,IHR,NBT,BMO,BDY,BHR,
     *                   BLAT,BLON,BWS,CLAT,CLON,SWND)
C
C     Returns best-track cliper/decay SHIFOR forecast.
C     Output is at 12, 24, 36, 48, 60, 72, 84, 96, 108, and 120 h.
C     Valid for Atlantic and East Pacific.
C
C     Decay SHIFOR differs from original in that it uses the
C     CLIPER track and Mark DeMaria's decay rate to decay the
C     intensity forecast over land.  There is also a new lower
C     limit of 15 kt imposed (limit on SHIFOR was 1 kt).
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     NBT           Number of elements in the best track data arrays.
C     BMO,BDY,BHR   Arrays of best track dates/times.
C     BLAT(NBT)     BT latitude (deg N).
C     BLON(NBT)     BT longitude (deg E).
C     BWS(NBT)      BT wind speed (kt).
C     CLAT(NVTX)    Returned array of BT CLIPER lats.
C     CLON(NVTX)    Returned array of BT CLIPER lons.
C     SWND(NVTX)    Returned array of BT SHIFOR winds.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION BMO(NBT), BDY(NBT), BHR(NBT)
      DIMENSION BLAT(NBT), BLON(NBT), BWS(NBT)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION FPCLIP(2,NVTX), IWSHIF(NVTX), SWND(NVTX)
      DIMENSION FTIME(NVTX+1), RLAT(NVTX+1), RLON(NVTX+1)
      DIMENSION VMAX(NVTX+1), VMAXD(NVTX+1), DLAND(NVTX+1)
      CHARACTER*2 BASIN
C
C
C
      DT = 1.0
      RCRAD = 110.
      RLAT0 = -9999.
      RLON0 = -9999.
      RLAT12= -9999.
      RLON12= -9999.
      DIR0  = -9999.
      SPD0  = -9999.
      WS0   = -9999.
      WS12  = -9999.
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
C
C     Compute input parameters for the CLIPER subroutine.
C     Start by checking for a BT match to desired time.
C     ---------------------------------------------------
      DO 110 L = 1,NBT
         IF (IMO.EQ.NINT(BMO(L)).AND.IDY.EQ.NINT(BDY(L))
     *       .AND.IHR.EQ.NINT(BHR(L)))
     *   THEN
            L0 = L
            GOTO 200
            ENDIF
110      CONTINUE
C
C     Could not find a best track entry for the requested time
C     --------------------------------------------------------
      GOTO 800
C
C
C     Found a valid match at index L0.  Get initial position.
C     -------------------------------------------------------
200   RLAT0 = BLAT(L0)
      RLON0 = -BLON(L0)
      WS0 = BWS(L0)
C
C     Get previous, next positions to estimate current speed
C     ------------------------------------------------------
      LB = L0-1
      LF = L0+1
      IF (LB.LT.1) LB=1
      IF (LB.EQ.LF) LF=LF+1
      IF (LF.GT.NBT) LF = NBT
      IF (LB.EQ.LF) GOTO 800
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR0)
      SPD0 = DIST/DELTAH
C
C
C     Get additional parameters for SHIFOR
C     ------------------------------------
      LB = L0-2
      LF = L0
      IF (LB.LT.1) LB=1
      IF (LB.EQ.LF) LF=LF+1
      IF (LF.GT.NBT) LF = NBT
      IF (LB.EQ.LF) GOTO 800
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR12)
      SPD12 = DIST/DELTAH
      WS12 = BWS(LB)
C
C
C     Get CLIPER forecast.
C     -----------------------------------------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL CLIP_5(IMO,IDY,IHR,RLAT0,RLON0,WS0,DIR0,SPD0,BASIN,FPCLIP)
C
C
C     Run best-track SHIFOR
C     ---------------------
      IF (DIR12.EQ.-9999. .OR. SPD12.EQ.-9999.) GOTO 800
      IF (WS12.EQ.-9999.) GOTO 800
      UCMP = DIR12
      VCMP = SPD12
      CALL UVCOMP(UCMP,VCMP)
      UCMP = -UCMP
      VCMP = -VCMP
      IF (BASIN.EQ.'AL') CALL ATSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
      IF (BASIN.EQ.'EP') CALL EPSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 550 K=1,NVTX
         CLAT(K) = FPCLIP(1,K)
         CLON(K) = -FPCLIP(2,K)
         IF (IWSHIF(K).GT.0) SWND(K) = IWSHIF(K)
550      CONTINUE
C
C
C
C     Decay the SHIFOR forecast
C     -------------------------
      FTIME(1) = 0.
      RLAT(1) = RLAT0
      RLON(1) = RLON0
      VMAX(1) = WS0
      DO 600 K=1,NVTX
         FTIME(K+1) = FLOAT(K*12)
      RLAT(K+1) = CLAT(K)
      RLON(K+1) = -CLON(K)
      VMAX(K+1) = SWND(K)
      IF (VMAX(K+1).LT.15.) VMAX(K+1) = 15.
600   CONTINUE
      CALL DECAY(FTIME,RLAT,RLON,VMAX,VMAXD,DT,RCRAD,DLAND,1)
      DO 620 K=1,NVTX
         SWND(K) = VMAXD(K+1)
      IF (SWND(K).LT.15.) SWND(K) = 15.
620   CONTINUE
C
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE OCLIP5(BASIN,IYR,IMO,IDY,IHR,
     *                  RLAT0,RLON0E,WS0,DIR0,SPD0,
     *                  RLAT12,RLON12E,WS12,DIR12,SPD12,
     *                  CLAT,CLON,SWND)
C
C     Reruns operational 5-day cliper/shifor forecast.
C     Output is at 12, 24, 36, 48, 60, 72, 84, 96, 108, and 120 h.
C     Valid for Atlantic and East Pacific.
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     RLAT0         Initial lat
C     RLON0E        Initial long (deg E)
C     WS0           Initial wind speed
C     DIR0          Initial heading
C     SPD0          Initial forward speed
C     RLAT12        t-12h lat
C     RLON12E       t-12h long (deg E)
C     WS12          t-12h wind speed
C     DIR12         t-12h heading
C     SPD12         t-12h forward speed
C     CLAT(NVTX)    Returned array of CLIPER lats.
C     CLON(NVTX)    Returned array of CLIPER lons.
C     SWND(NVTX)    Returned array of SHIFOR winds.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION FPCLIP(2,NVTX), IWSHIF(NVTX), SWND(NVTX)
      CHARACTER*2 BASIN
C
C
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
      RLON0 = -9999.
      RLON12 = -9999.
      IF (RLON0E.NE.-9999.) RLON0 = -RLON0E
      IF (RLON12E.NE.-9999.) RLON12 = -RLON12E
C
C
C     Get CLIPER forecast.
C     --------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL CLIP_5(IMO,IDY,IHR,RLAT0,RLON0,WS0,DIR0,SPD0,BASIN,FPCLIP)
C
C
C     Get SHIFOR forecast.
C     --------------------
      IF (RLAT12.EQ.-9999. .OR. RLON12.EQ.-9999.) GOTO 800
      IF (WS12.EQ.-9999.) GOTO 800
      RLATB = RLAT12
      RLONB = RLON12
      RLATF = RLAT0
      RLONF = RLON0
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIRL12)
      SPDL12 = DIST/12.
      IF (DIRL12.EQ.-9999. .OR. SPDL12.EQ.-9999.) GOTO 800
      UCMP = DIRL12
      VCMP = SPDL12
      CALL UVCOMP(UCMP,VCMP)
      UCMP = -UCMP
      VCMP = -VCMP
      IF (BASIN.EQ.'AL') CALL ATSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
      IF (BASIN.EQ.'EP') CALL EPSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 550 K=1,NVTX
         CLAT(K) = FPCLIP(1,K)
         CLON(K) = -FPCLIP(2,K)
         IF (IWSHIF(K).GT.0) SWND(K) = IWSHIF(K)
550      CONTINUE
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE OCLIPD5(BASIN,IYR,IMO,IDY,IHR,
     *                  RLAT0,RLON0E,WS0,DIR0,SPD0,
     *                  RLAT12,RLON12E,WS12,DIR12,SPD12,
     *                  CLAT,CLON,SWND)
C
C     Reruns operational 5-day cliper/decay shifor forecast.
C     Output is at 12, 24, 36, 48, 60, 72, 84, 96, 108, and 120 h.
C     Valid for Atlantic and East Pacific.
C
C     Decay SHIFOR differs from original in that it uses the
C     CLIPER track and Mark DeMaria's decay rate to decay the
C     intensity forecast over land.  There is also a new lower
C     limit of 15 kt imposed (limit on SHIFOR was 1 kt).
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     RLAT0         Initial lat
C     RLON0E        Initial long (deg E)
C     WS0           Initial wind speed
C     DIR0          Initial heading
C     SPD0          Initial forward speed
C     RLAT12        t-12h lat
C     RLON12E       t-12h long (deg E)
C     WS12          t-12h wind speed
C     DIR12         t-12h heading
C     SPD12         t-12h forward speed
C     CLAT(NVTX)    Returned array of CLIPER lats.
C     CLON(NVTX)    Returned array of CLIPER lons.
C     SWND(NVTX)    Returned array of DECAY SHIFOR winds.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION FPCLIP(2,NVTX), IWSHIF(NVTX), SWND(NVTX)
      DIMENSION FTIME(NVTX+1), RLAT(NVTX+1), RLON(NVTX+1)
      DIMENSION VMAX(NVTX+1), VMAXD(NVTX+1), DLAND(NVTX+1)
      CHARACTER*2 BASIN
C
C
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
      RLON0 = -9999.
      RLON12 = -9999.
      IF (RLON0E.NE.-9999.) RLON0 = -RLON0E
      IF (RLON12E.NE.-9999.) RLON12 = -RLON12E
      DT = 1.0
      RCRAD = 110.
C
C
C     Get CLIPER forecast.
C     --------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL CLIP_5(IMO,IDY,IHR,RLAT0,RLON0,WS0,DIR0,SPD0,BASIN,FPCLIP)
C
C
C     Get SHIFOR forecast.
C     --------------------
      IF (RLAT12.EQ.-9999. .OR. RLON12.EQ.-9999.) GOTO 800
      IF (WS12.EQ.-9999.) GOTO 800
      RLATB = RLAT12
      RLONB = RLON12
      RLATF = RLAT0
      RLONF = RLON0
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIRL12)
      SPDL12 = DIST/12.
      IF (DIRL12.EQ.-9999. .OR. SPDL12.EQ.-9999.) GOTO 800
      UCMP = DIRL12
      VCMP = SPDL12
      CALL UVCOMP(UCMP,VCMP)
      UCMP = -UCMP
      VCMP = -VCMP
      IF (BASIN.EQ.'AL') CALL ATSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
      IF (BASIN.EQ.'EP') CALL EPSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 550 K=1,NVTX
         CLAT(K) = FPCLIP(1,K)
         CLON(K) = -FPCLIP(2,K)
         IF (IWSHIF(K).GT.0) SWND(K) = IWSHIF(K)
550      CONTINUE
C
C
C     Decay the SHIFOR forecast
C     -------------------------
      FTIME(1) = 0.
      RLAT(1) = RLAT0
      RLON(1) = -RLON0E
      VMAX(1) = WS0
      DO 600 K=1,NVTX
         FTIME(K+1) = FLOAT(K*12)
      RLAT(K+1) = CLAT(K)
      RLON(K+1) = -CLON(K)
      VMAX(K+1) = SWND(K)
      IF (VMAX(K+1).LT.15.) VMAX(K+1) = 15.
600   CONTINUE
      CALL DECAY(FTIME,RLAT,RLON,VMAX,VMAXD,DT,RCRAD,DLAND,1)
      DO 620 K=1,NVTX
         SWND(K) = VMAXD(K+1)
      IF (SWND(K).LT.15.) SWND(K) = 15.
620   CONTINUE
C
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE OCLIPD5_TEST(BASIN,IYR,IMO,IDY,IHR,
     *                  RLAT0,RLON0E,WS0,DIR0,SPD0,
     *                  RLAT12,RLON12E,WS12,DIR12,SPD12,
     *                  CLAT,CLON,SWND)
C
C     Reruns operational 5-day cliper/decay shifor forecast.
C     Output is at 12, 24, 36, 48, 60, 72, 84, 96, 108, and 120 h.
C     Valid for Atlantic and East Pacific.
C
C     Decay SHIFOR differs from original in that it uses the
C     CLIPER track and Mark DeMaria's decay rate to decay the
C     intensity forecast over land.  There is also a new lower
C     limit of 15 kt imposed (limit on SHIFOR was 1 kt).
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     RLAT0         Initial lat
C     RLON0E        Initial long (deg E)
C     WS0           Initial wind speed
C     DIR0          Initial heading
C     SPD0          Initial forward speed
C     RLAT12        t-12h lat
C     RLON12E       t-12h long (deg E)
C     WS12          t-12h wind speed
C     DIR12         t-12h heading
C     SPD12         t-12h forward speed
C     CLAT(NVTX)    Returned array of CLIPER lats.
C     CLON(NVTX)    Returned array of CLIPER lons.
C     SWND(NVTX)    Returned array of DECAY SHIFOR winds.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION FPCLIP(2,NVTX), IWSHIF(NVTX), SWND(NVTX)
      DIMENSION FTIME(NVTX+1), RLAT(NVTX+1), RLON(NVTX+1)
      DIMENSION VMAX(NVTX+1), VMAXD(NVTX+1), DLAND(NVTX+1)
      CHARACTER*2 BASIN
C
C
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
      RLON0 = -9999.
      RLON12 = -9999.
      IF (RLON0E.NE.-9999.) RLON0 = -RLON0E
      IF (RLON12E.NE.-9999.) RLON12 = -RLON12E
      DT = 1.0
      RCRAD = 0.0
C
C
C     Get CLIPER forecast.
C     --------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL CLIP_5(IMO,IDY,IHR,RLAT0,RLON0,WS0,DIR0,SPD0,BASIN,FPCLIP)
C
C
C     Get SHIFOR forecast.
C     --------------------
      IF (RLAT12.EQ.-9999. .OR. RLON12.EQ.-9999.) GOTO 800
      IF (WS12.EQ.-9999.) GOTO 800
      RLATB = RLAT12
      RLONB = RLON12
      RLATF = RLAT0
      RLONF = RLON0
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIRL12)
      SPDL12 = DIST/12.
      IF (DIRL12.EQ.-9999. .OR. SPDL12.EQ.-9999.) GOTO 800
      UCMP = DIRL12
      VCMP = SPDL12
      CALL UVCOMP(UCMP,VCMP)
      UCMP = -UCMP
      VCMP = -VCMP
      IF (BASIN.EQ.'AL') CALL ATSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
      IF (BASIN.EQ.'EP') CALL EPSHIF5D(IYR,IMO,IDY,RLAT0,RLON0,
     *                        UCMP,VCMP,WS0,WS12,IWSHIF)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 550 K=1,NVTX
         CLAT(K) = FPCLIP(1,K)
         CLON(K) = -FPCLIP(2,K)
         IF (IWSHIF(K).GT.0) SWND(K) = IWSHIF(K)
550      CONTINUE
C
C
C     Decay the SHIFOR forecast
C     -------------------------
      FTIME(1) = 0.
      RLAT(1) = RLAT0
      RLON(1) = -RLON0E
      VMAX(1) = WS0
      DO 600 K=1,NVTX
         FTIME(K+1) = FLOAT(K*12)
      RLAT(K+1) = CLAT(K)
      RLON(K+1) = -CLON(K)
      VMAX(K+1) = SWND(K)
      IF (VMAX(K+1).LT.15.) VMAX(K+1) = 15.
600   CONTINUE
      CALL DECAY(FTIME,RLAT,RLON,VMAX,VMAXD,DT,RCRAD,DLAND,1)
      DO 620 K=1,NVTX
         SWND(K) = VMAXD(K+1)
      IF (SWND(K).LT.15.) SWND(K) = 15.
620   CONTINUE
C
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C
C     ------------------------------------------------------------------
      subroutine clip_5(imo,ida,ihr,latcur,loncur,wndcur,dircur,spdcur,
     *                  basin,fpclip)
C
C     written by Sim Aberson 1-APR-1998
C     NOAA/AOML/Hurricane Research Division
C     Modified by Jim Gross 98/04/24 by changing arthmitic to floating
C         point, finding an error, and combining the Atlantic and east
C         Pacific into one 5_day CLIPER model
C
C     Made operational by Jim Gross 2001/05/17
C
C     Note: output longitudes are in degrees E (0-360).
C
C     Modified by James Franklin, July 2002, to better integrate with
C     verify_model.f.
C
C     Modified 26 April 2005 to use new coefficient files based on
C     1931-2004 dependent data (in the Atlantic).  Also added date
C     limits on initial day.
C     ------------------------------------------------------------------
c
      parameter (nvaratl=27, nvargulf=27, nvar=nvaratl)
c
      dimension fpclip(2,10)
      real latcur, loncur, wind, rdir, days, ucmp, vcmp, jday(12),
     *     acon(40), coef(40,nvar), x(nvar), disp(20)
c
      character*2  basin
      character*20 alfile, gmfile, epfile
c
      data jday /  1.0,  32.0,  60.0,  91.0, 121.0, 152.0,
     *           182.0, 213.0, 244.0, 274.0, 305.0, 335.0 /
c
c
c     Current files
c     -------------
      alfile = 'clp5_al3104coeff.dat'
      gmfile = 'clp5_gm3104coeff.dat'
      epfile = 'clp5_ep4904coeff.dat'
c
c     Original Aberson files
c     ----------------------
c     alfile = 'clp5_al6100coeff.dat'
c     gmfile = 'clp5_gm6100coeff.dat'
c     epfile = 'clp5_ep6100coeff.dat'
c
      degrad=atan(1.0)/45.0
C
c
c     Open, and read regression coefficients file
c     -------------------------------------------
      if ( basin .eq. 'al' .or. basin .eq. 'AL') then
         open (21,file=alfile,status='old',iostat=ios,err=1010)
         open (22,file=gmfile,status='old',iostat=ios,err=1020)
         do i = 1, 20
            read (21,1,iostat=ios,err=1030)
     *            acon(i), (coef(i,j),j=1,nvaratl)
 1          format (f11.6,4x,4e15.7,/,5(5e15.7,/))
            read (22,1,iostat=ios,err=1030)
     *            acon(i+20),(coef(i+20,j),j=1,nvargulf)
         enddo
         close(21)
         close(22)
      else
         open (21,file=epfile,status='old',iostat=ios,err=1040)
         do i = 1,20
            read (21,1,iostat=ios,err=1030)
     *             acon(i), (coef(i,j),j=1,nvaratl)
         enddo
         close(21)
      endif
c
c
      days = jday(imo) + real(ida) + real(ihr)/24.0
c
      if (basin .eq. 'al' .or. basin .eq. 'AL') then
         if (days.lt.152.0) days = 152.0
         else
         if (days.lt.135.0) days = 135.0
         endif
      if (days .gt. 334.0) days = 334.0
c
      wind = wndcur*111.1*1000.0/(60.0*3600.0)
      rdir = dircur + 180.0
      if ( rdir .ge. 360.0 ) rdir = rdir - 360.0
      rspd = spdcur*111.1*1000.0/(60.0*3600.0)
c
      ucmp = rdir
      vcmp = rspd
      call uvcomp(ucmp,vcmp)
c
      ucmp = -ucmp
c
      x(1) = latcur
      x(2) = loncur
      x(3) = wind
      x(4) = days
      x(5) = vcmp
      x(6) = ucmp
c
      klij = 6
c
      do ijkl = 1, 6
         do jkli = ijkl, 6
            klij = klij + 1
            x(klij) = x(ijkl)*x(jkli)
         enddo
      enddo
c
      do i = 1, 20
         disp(i) = acon(i)
         if (basin.eq.'al' .or. basin.eq.'AL') then
            if ( latcur .lt. loncur - 64.0) disp(i) = acon(i + 20)
         endif
      enddo
c
      do i = 1, 20
         do j = 1, nvar
            if (basin.eq.'al' .or. basin.eq.'AL') then
               if ( latcur .ge. loncur - 64.0 ) then
                  disp(i) = disp(i) + x(j)*coef(i,j)
               else
                  disp(i) = disp(i) + x(j)*coef(i+20,j)
               endif
            else
               disp(i) = disp(i) + x(j)*coef(i,j)
            endif
c
        enddo
      enddo
c
      fpclip(1,1) = latcur + disp(1)
      do i = 2, 10
         fpclip(1,i) = fpclip(1,i - 1) + disp(i)
      enddo
c
      fpclip(2,1) = loncur + disp(11)/
     *                 cos((latcur + fpclip(1,1))*
     *                 degrad/2.0 )
      do i = 2, 10
         fpclip(2,i) = fpclip(2,i - 1) + disp(i + 10)/
     *                    cos((fpclip(1,i - 1) + fpclip(1,i))*
     *                    degrad/2.0)
      enddo
C
C
c     Round to the nearest tenth
c     --------------------------
      DO K = 1, 10
         fpclip(1,k) = float(nint(fpclip(1,k)*10.0))/10.
         fpclip(2,k) = float(nint(fpclip(2,k)*10.0))/10.
      enddo
c
c
c     Check hemisphere
c     ----------------
      do k = 1, 10
         IF ( FPCLIP( 2, k ) .LT. 0.0 ) THEN
            FPCLIP( 2, k ) = 360.0 + FPCLIP( 2, k )
            ENDIF
         enddo
c
c
c     All done, return
c     ----------------
      return
c
c
c     Error messages
c     --------------
 1010 print *, ' error opening alcoff.dat = ',ios
      stop
c
 1020 print *, ' error opening gfcoff.dat = ',ios
      stop
c
 1030 print *, ' error reading coefficient file = ',ios
      stop
c
 1040 print *, ' error opening epcoff.dat = ',ios
      stop
c
      end
C
C
C
      subroutine atshif5d(iyear,imonth,iday,alat,alon,ucmp12,vcmp12,
     *                    vel,vel12,iwnd)
c
c     This subroutine calculates tropical cyclone intensities through 120
c     hours based upon climatology and persistence using the years
c     1967-1999.  The model was created using the total change in intensity
c     for each period (12-hr,....120-hr) from intial conditions as the
c     predictand and 35 predictors including and derived from
c     julian day, latitude, longitude, zonal speed, meridional speed,
c     current intensity the past 12-hour intensity trend.
c
c     In the formulation of the model linear terms are first put into the
c     model using a forward stepping approach for the 12-hour forecast.
c     The linear predictors chosen in this forward stepping process
c     are then forced into the model and exposed to the 2nd order terms,
c     which at this point are allowed to come into the model in a
c     stepwise fashion.  A backward step is then performed to remove
c     predictors that are no longer significant.  Then a final stepwise
c     stepping proceedure is performed possibly adding a removing predictors
c
c
c     Following the 12-hour forecast the predictors chosen for the previous
c     forecast period are then given preference in the selection process.
c     Again, the predictors chosen in this forward stepping process
c     are then forced into the model and exposed to the 2nd order terms,
c     which at this point are allowed to come into the model in a
c     stepwise fashion.  A backward pass through the data is then performed
c     to remove predictors that are no longer significant. Followed by
c     a final step that is stepwise.
c
c     J. Knaff (04/05/2001)
c
c     Modified by James Franklin 7/2002 for compatibility with
c     verify_model.f
c
c     Modified by James Franklin 3/2006 to avoid losing forecasts of winds
c     less than zero.  Before, such forecasts were tossed, resulting in a loss of official
c     forecasts in homogeneous comparisons.  Now, if the model forecasts a negative
c     windspeed, then the output value is set to 1 kt.
c
      common /coefats/ scoef(10,36), avg(10,36), sdev(10,36)
c
c     dimension coeficients.
c
      parameter(nc=36)
      real p(36), forecast(10)
      double precision  dv (10)
      dimension iwnd(10)
c
c     dimension input.
c
      real alat, alon, vel, vel12
c
c     intialize to zero
c
      rad = 3.14159/180.
      do i=1,10
         dv(i)=0.0
         iwnd(i)=0
      end do
c
c
c     check for system intensity requirements.
c
      if (vel.lt.15.0.OR.vel12.lt.15.0) return
c
c     create predictor pool (first order terms, squares, and
c     co-variances terms)
c
c     p1 = julian day - 253
c     p2 = lat
c     p3 = lon
c     p4 = u ! zonal speed of the storm over the last 12 hours
c     p5 = v ! meridional speed of the storm over the last 12 hours
c     p6 = vmax
c     p7 = delta vmax
c
c     calculate julian day
c
      call julian_day(imonth,iday,iyear,julday)
c
c     assign predictor values from the input data
c
      p(1) = dble(julday-253)
      p(2) = dble(alat)
      p(3) =dble(alon)
c     avglat=(alat+alat12)/2.0
c     p(4) =dble((alon-alon12)* (-60.0)/ 12.0 *
c    .     COS(rad*avglat))
c     p(5)=dble((alat-alat12)*60./12.)
      p(4)=dble(ucmp12)
      p(5)=dble(vcmp12)
      p(6)=dble(vel)
      p(7)=dble(vel-vel12)
      p(8)=p(1)**2                !p1*p1
      p(9)=p(1)*p(2)              !p1*p2
      p(10)=p(1)*p(3)             !p1*p3
      p(11)=p(1)*p(4)             !etc....
      p(12)=p(1)*p(5)
      p(13)=p(1)*p(6)
      p(14)=p(1)*p(7)
      p(15)=p(2)**2
      p(16)=p(2)*p(3)
      p(17)=p(2)*p(4)
      p(18)=P(2)*p(5)
      p(19)=p(2)*p(6)
      p(20)=p(2)*p(7)
      p(21)=p(3)**2
      p(22)=p(3)*p(4)
      p(23)=p(3)*p(5)
      p(24)=p(3)*p(6)
      p(25)=p(3)*p(7)
      p(26)=p(4)**2
      p(27)=p(4)*p(5)
      p(28)=p(4)*p(6)
      p(29)=p(4)*p(7)
      p(30)=p(5)**2
      p(31)=p(5)*p(6)
      p(32)=p(5)*p(7)
      p(33)=p(6)**2
      p(34)=p(6)*p(7)
      p(35)=p(7)**2
      p(36)=vel
c
c     calculate the predicted incremental change in velocity
c
      do i=1,10
         dv(i)=0.0 ! intitialize array to zero.
         do j=1,35
            dv(i)=dv(i)+dble(scoef(i,j)*((p(j)-avg(i,j))/sdev(i,j)))
         end do
         dv(i)=dv(i)*dble(sdev(i,36)) + dble(avg(i,36))
      end do
c
c
c     construct forecast intensities
c
      forecast(1)=p(36)+dv(1)

      do i=1,10
         forecast(i)= p(36)+sngl(dv(i))
      end do
      do i=1,10
c         if (forecast(i).lt.0.0)forecast(i)=0.0
c
c        Modification to avoid losing forecasts - JLF 3/13/06
c        ----------------------------------------------------
         if (forecast(i).lt.0.5)forecast(i)=1.0
         iwnd(i)=nint(forecast(i))
      end do
c
      return
      end
c
c
c
      block data atlshifor_data
c
c     block data for the standardized coeficients to the 5-day
c     Atlantic SHIFOR
c
c     These are used by alshif5d and passed via a common block
c     initialized in this subprogram.  The common block is not passed
c     to the main program.
c
c     scoef    are the standardized coeficients
c     avg      are the averages
c     sdev     are the standard deviations.
c
c
      common /coefats/ scoef(10,36), avg(10,36), sdev(10,36)
c
      data (scoef( 1,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.9632107E-01, 0.2161774E+00, 0.0000000E+00,-0.2598887E+00,
     . 0.6619257E+00,-0.6903946E-01, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.3840485E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.2655196E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 1,j),j=1,36) / 0.6924230E+00, 0.2501092E+02,
     . 0.6001072E+02,-0.2517827E+01, 0.4779600E+01, 0.5636170E+02,
     . 0.2587011E+01, 0.1030277E+04, 0.1775327E+02, 0.4254318E+02,
     . 0.3506200E+02,-0.1217535E+02, 0.1154956E+03,-0.1672390E+02,
     . 0.6990111E+03, 0.1512090E+04,-0.2923416E+01, 0.1318952E+03,
     . 0.1450323E+04, 0.5226493E+02, 0.3901107E+04,-0.1370406E+03,
     . 0.2816506E+03, 0.3394556E+04, 0.1688047E+03, 0.9963890E+02,
     .-0.9566036E-01,-0.1150216E+03,-0.1612303E+02, 0.4871619E+02,
     . 0.2915584E+03, 0.1450799E+02, 0.3791872E+04, 0.1823332E+03,
     . 0.8294604E+02, 0.2005495E+01/
      data (sdev( 1,j),j=1,36) / 0.3209314E+02, 0.8571875E+01,
     . 0.1731675E+02, 0.9659969E+01, 0.5086838E+01, 0.2480592E+02,
     . 0.8733048E+01, 0.1864341E+04, 0.8357047E+03, 0.2234280E+04,
     . 0.2879513E+03, 0.2154462E+03, 0.1749152E+04, 0.3003741E+03,
     . 0.4446778E+03, 0.6461969E+03, 0.2631858E+03, 0.1692676E+03,
     . 0.8461565E+03, 0.2281288E+03, 0.2127881E+04, 0.5451351E+03,
     . 0.3145661E+03, 0.1852198E+04, 0.5793061E+03, 0.1301999E+03,
     . 0.7732862E+02, 0.5875003E+03, 0.9362686E+02, 0.7837893E+02,
     . 0.3577816E+03, 0.6779141E+02, 0.3409085E+04, 0.6547117E+03,
     . 0.1612456E+03, 0.9235003E+01/
      data (scoef( 2,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.1233280E+00, 0.2059798E+00, 0.0000000E+00,-0.2399135E+00,
     . 0.7064717E+00,-0.1008747E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.1276496E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.3583669E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.3909434E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 2,j),j=1,36) / 0.9079197E+00, 0.2451084E+02,
     . 0.5893240E+02,-0.3085352E+01, 0.4501705E+01, 0.5650076E+02,
     . 0.2622774E+01, 0.9678882E+03, 0.2445280E+02, 0.7167412E+02,
     . 0.3633494E+02,-0.1068501E+02, 0.1269655E+03,-0.1328003E+02,
     . 0.6709643E+03, 0.1464025E+04,-0.2066254E+02, 0.1185758E+03,
     . 0.1428452E+04, 0.5308390E+02, 0.3755662E+04,-0.1573582E+03,
     . 0.2618678E+03, 0.3339356E+04, 0.1592436E+03, 0.9354892E+02,
     .-0.6658569E+01,-0.1436312E+03,-0.1608299E+02, 0.4283905E+02,
     . 0.2740909E+03, 0.1404197E+02, 0.3795095E+04, 0.1763668E+03,
     . 0.7905362E+02, 0.3726980E+01/
      data (sdev( 2,j),j=1,36) / 0.3110060E+02, 0.8378330E+01,
     . 0.1681333E+02, 0.9167630E+01, 0.4751629E+01, 0.2455349E+02,
     . 0.8496373E+01, 0.1768235E+04, 0.7934583E+03, 0.2132501E+04,
     . 0.2678891E+03, 0.1941161E+03, 0.1705528E+04, 0.2929895E+03,
     . 0.4255700E+03, 0.6396952E+03, 0.2369832E+03, 0.1480606E+03,
     . 0.8353543E+03, 0.2164446E+03, 0.2038278E+04, 0.5116897E+03,
     . 0.2930400E+03, 0.1768569E+04, 0.5501223E+03, 0.1166377E+03,
     . 0.6353190E+02, 0.5536301E+03, 0.8621957E+02, 0.6173920E+02,
     . 0.3307446E+03, 0.5880326E+02, 0.3329759E+04, 0.6239390E+03,
     . 0.1531375E+03, 0.1532608E+02/
      data (scoef( 3,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.1334133E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.6672866E+00,-0.1210680E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.2524992E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.5450201E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.1718453E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.4152353E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 3,j),j=1,36) / 0.9616966E+00, 0.2404943E+02,
     . 0.5779779E+02,-0.3558863E+01, 0.4308050E+01, 0.5671110E+02,
     . 0.2737719E+01, 0.9210535E+03, 0.2779111E+02, 0.8878762E+02,
     . 0.3805056E+02,-0.1038823E+02, 0.1297228E+03,-0.1325298E+02,
     . 0.6464169E+03, 0.1417209E+04,-0.3392802E+02, 0.1093273E+03,
     . 0.1409962E+04, 0.5483954E+02, 0.3607683E+04,-0.1723524E+03,
     . 0.2469319E+03, 0.3293460E+04, 0.1577506E+03, 0.9111264E+02,
     .-0.1095332E+02,-0.1681860E+03,-0.1828591E+02, 0.3958164E+02,
     . 0.2626406E+03, 0.1389569E+02, 0.3815360E+04, 0.1803387E+03,
     . 0.7714023E+02, 0.4966674E+01/
      data (sdev( 3,j),j=1,36) / 0.3033690E+02, 0.8249646E+01,
     . 0.1634490E+02, 0.8857997E+01, 0.4585509E+01, 0.2448144E+02,
     . 0.8346269E+01, 0.1721265E+04, 0.7590904E+03, 0.2051263E+04,
     . 0.2579913E+03, 0.1808050E+03, 0.1664844E+04, 0.2869225E+03,
     . 0.4123503E+03, 0.6362572E+03, 0.2202684E+03, 0.1374561E+03,
     . 0.8296448E+03, 0.2083708E+03, 0.1948476E+04, 0.4887198E+03,
     . 0.2799759E+03, 0.1721974E+04, 0.5274450E+03, 0.1109338E+03,
     . 0.5723892E+02, 0.5324178E+03, 0.8228619E+02, 0.5533614E+02,
     . 0.3181781E+03, 0.5351896E+02, 0.3303091E+04, 0.6109392E+03,
     . 0.1502984E+03, 0.2018737E+02/
      data (scoef( 4,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.1179411E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.5688543E+00,-0.1263167E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.3047682E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.6389252E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.1818729E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.3632210E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 4,j),j=1,36) / 0.8890815E+00, 0.2363174E+02,
     . 0.5672563E+02,-0.3959007E+01, 0.4171206E+01, 0.5699431E+02,
     . 0.2956177E+01, 0.8737126E+03, 0.2832931E+02, 0.9426695E+02,
     . 0.3968130E+02,-0.1056524E+02, 0.1285152E+03,-0.1212280E+02,
     . 0.6245836E+03, 0.1374201E+04,-0.4443031E+02, 0.1024761E+03,
     . 0.1394454E+04, 0.5930000E+02, 0.3471278E+04,-0.1841963E+03,
     . 0.2358062E+03, 0.3260008E+04, 0.1639787E+03, 0.9053625E+02,
     .-0.1423454E+02,-0.1899361E+03,-0.2018777E+02, 0.3704388E+02,
     . 0.2550115E+03, 0.1441322E+02, 0.3847742E+04, 0.1924855E+03,
     . 0.7592820E+02, 0.5805893E+01/
      data (sdev( 4,j),j=1,36) / 0.2954891E+02, 0.8132701E+01,
     . 0.1592306E+02, 0.8653384E+01, 0.4432809E+01, 0.2448550E+02,
     . 0.8197918E+01, 0.1667888E+04, 0.7266108E+03, 0.1972593E+04,
     . 0.2514733E+03, 0.1682396E+03, 0.1624908E+04, 0.2809930E+03,
     . 0.4014341E+03, 0.6329095E+03, 0.2091046E+03, 0.1281973E+03,
     . 0.8236214E+03, 0.2017373E+03, 0.1863299E+04, 0.4736634E+03,
     . 0.2677355E+03, 0.1698982E+04, 0.5058230E+03, 0.1072277E+03,
     . 0.5339523E+02, 0.5209970E+03, 0.8075079E+02, 0.5054678E+02,
     . 0.3098015E+03, 0.5224454E+02, 0.3286008E+04, 0.5994625E+03,
     . 0.1427729E+03, 0.2406040E+02/
      data (scoef( 5,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.1066351E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.4361093E+00,-0.1229526E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.3588182E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.7048543E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.7875312E-01,
     . 0.0000000E+00,-0.1810176E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.2748392E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 5,j),j=1,36) / 0.7411598E+00, 0.2323304E+02,
     . 0.5587768E+02,-0.4325152E+01, 0.4093352E+01, 0.5715955E+02,
     . 0.3132390E+01, 0.8297267E+03, 0.2728141E+02, 0.9450549E+02,
     . 0.4206970E+02,-0.1141570E+02, 0.1228300E+03,-0.1365545E+02,
     . 0.6040857E+03, 0.1336892E+04,-0.5336236E+02, 0.9788639E+02,
     . 0.1375858E+04, 0.6265878E+02, 0.3368683E+04,-0.1953185E+03,
     . 0.2288874E+03, 0.3232563E+04, 0.1698837E+03, 0.9092222E+02,
     .-0.1654315E+02,-0.2097693E+03,-0.2138794E+02, 0.3543734E+02,
     . 0.2507907E+03, 0.1472942E+02, 0.3870560E+04, 0.2028622E+03,
     . 0.7680764E+02, 0.6461952E+01/
      data (sdev( 5,j),j=1,36) / 0.2879951E+02, 0.8020582E+01,
     . 0.1569833E+02, 0.8499159E+01, 0.4322857E+01, 0.2456659E+02,
     . 0.8186252E+01, 0.1617134E+04, 0.6940360E+03, 0.1907284E+04,
     . 0.2470698E+03, 0.1595705E+03, 0.1592168E+04, 0.2760713E+03,
     . 0.3922771E+03, 0.6302870E+03, 0.2006715E+03, 0.1216372E+03,
     . 0.8166120E+03, 0.1994494E+03, 0.1810311E+04, 0.4624083E+03,
     . 0.2593056E+03, 0.1694290E+04, 0.4982435E+03, 0.1037749E+03,
     . 0.5123920E+02, 0.5120408E+03, 0.8149457E+02, 0.4752918E+02,
     . 0.3031102E+03, 0.5243453E+02, 0.3284298E+04, 0.5991281E+03,
     . 0.1446465E+03, 0.2705362E+02/
      data (scoef( 6,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.8026610E-01, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.3887290E+00,-0.1167581E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.3965225E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.7556319E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.9791975E-01,
     . 0.0000000E+00,-0.1792372E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.2562578E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 6,j),j=1,36) / 0.5282349E+00, 0.2288561E+02,
     . 0.5509222E+02,-0.4616736E+01, 0.4066473E+01, 0.5720878E+02,
     . 0.3269119E+01, 0.7833530E+03, 0.2451029E+02, 0.8987844E+02,
     . 0.4654677E+02,-0.1255986E+02, 0.1154485E+03,-0.1190449E+02,
     . 0.5868539E+03, 0.1304664E+04,-0.5985332E+02, 0.9588982E+02,
     . 0.1357026E+04, 0.6501071E+02, 0.3277084E+04,-0.2018239E+03,
     . 0.2255694E+03, 0.3203238E+04, 0.1741666E+03, 0.9196463E+02,
     .-0.1768923E+02,-0.2261230E+03,-0.2270596E+02, 0.3445014E+02,
     . 0.2489540E+03, 0.1535398E+02, 0.3879045E+04, 0.2116815E+03,
     . 0.7734914E+02, 0.6904808E+01/
      data (sdev( 6,j),j=1,36) / 0.2798798E+02, 0.7945013E+01,
     . 0.1555665E+02, 0.8406735E+01, 0.4233170E+01, 0.2462512E+02,
     . 0.8165998E+01, 0.1559906E+04, 0.6625800E+03, 0.1841586E+04,
     . 0.2441215E+03, 0.1501844E+03, 0.1544518E+04, 0.2655307E+03,
     . 0.3854558E+03, 0.6318023E+03, 0.1945790E+03, 0.1180420E+03,
     . 0.8090012E+03, 0.1964395E+03, 0.1767535E+04, 0.4526402E+03,
     . 0.2536928E+03, 0.1695497E+04, 0.4891541E+03, 0.1022458E+03,
     . 0.5073444E+02, 0.5048276E+03, 0.8129501E+02, 0.4514432E+02,
     . 0.2981933E+03, 0.5209768E+02, 0.3278063E+04, 0.5976022E+03,
     . 0.1443764E+03, 0.2930631E+02/
      data (scoef( 7,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.1047530E+00,-0.8868639E-01, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.4587812E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.8218156E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.1123108E+00,
     . 0.0000000E+00,-0.1655864E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 7,j),j=1,36) / 0.4207630E+00, 0.2255040E+02,
     . 0.5443819E+02,-0.4917055E+01, 0.4062362E+01, 0.5703888E+02,
     . 0.3388848E+01, 0.7403136E+03, 0.2312487E+02, 0.9094167E+02,
     . 0.4966651E+02,-0.1176302E+02, 0.1092751E+03,-0.4587674E+01,
     . 0.5698082E+03, 0.1275754E+04,-0.6694496E+02, 0.9490926E+02,
     . 0.1333167E+04, 0.6673709E+02, 0.3206201E+04,-0.2110588E+03,
     . 0.2246610E+03, 0.3166486E+04, 0.1796703E+03, 0.9265514E+02,
     .-0.1852151E+02,-0.2425760E+03,-0.2346312E+02, 0.3375825E+02,
     . 0.2468534E+03, 0.1563206E+02, 0.3860909E+04, 0.2147707E+03,
     . 0.7540719E+02, 0.7243947E+01/
      data (sdev( 7,j),j=1,36) / 0.2721044E+02, 0.7830068E+01,
     . 0.1558119E+02, 0.8276644E+01, 0.4154732E+01, 0.2465152E+02,
     . 0.7996647E+01, 0.1486568E+04, 0.6332492E+03, 0.1784579E+04,
     . 0.2395884E+03, 0.1394451E+03, 0.1492160E+04, 0.2504226E+03,
     . 0.3766640E+03, 0.6321689E+03, 0.1879339E+03, 0.1158340E+03,
     . 0.7986561E+03, 0.1904583E+03, 0.1749101E+04, 0.4422272E+03,
     . 0.2496878E+03, 0.1696971E+04, 0.4733990E+03, 0.1008219E+03,
     . 0.4979871E+02, 0.4965642E+03, 0.7946046E+02, 0.4403391E+02,
     . 0.2927683E+03, 0.5060949E+02, 0.3274097E+04, 0.5828798E+03,
     . 0.1414711E+03, 0.3087317E+02/
      data (scoef( 8,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.9537594E-01, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.4518728E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.8478302E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.1335201E+00,
     . 0.0000000E+00,-0.1486502E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 8,j),j=1,36) / 0.4769039E+00, 0.2223271E+02,
     . 0.5392975E+02,-0.5245444E+01, 0.4073866E+01, 0.5679109E+02,
     . 0.3439451E+01, 0.6933816E+03, 0.2537599E+02, 0.9890241E+02,
     . 0.5022994E+02,-0.1020454E+02, 0.1035189E+03,-0.2918435E+01,
     . 0.5532652E+03, 0.1250069E+04,-0.7521220E+02, 0.9437025E+02,
     . 0.1308384E+04, 0.6672680E+02, 0.3154948E+04,-0.2238049E+03,
     . 0.2245567E+03, 0.3128242E+04, 0.1821682E+03, 0.9341725E+02,
     .-0.1931823E+02,-0.2613367E+03,-0.2460387E+02, 0.3332844E+02,
     . 0.2451973E+03, 0.1544944E+02, 0.3841154E+04, 0.2145339E+03,
     . 0.7453392E+02, 0.7796921E+01/
      data (sdev( 8,j),j=1,36) / 0.2633330E+02, 0.7680913E+01,
     . 0.1570454E+02, 0.8119729E+01, 0.4091335E+01, 0.2482301E+02,
     . 0.7920240E+01, 0.1413855E+04, 0.6058992E+03, 0.1721314E+04,
     . 0.2337595E+03, 0.1330247E+03, 0.1450492E+04, 0.2424808E+03,
     . 0.3661280E+03, 0.6296606E+03, 0.1804536E+03, 0.1136799E+03,
     . 0.7890709E+03, 0.1860230E+03, 0.1750739E+04, 0.4317303E+03,
     . 0.2462065E+03, 0.1700974E+04, 0.4660102E+03, 0.1003206E+03,
     . 0.4966600E+02, 0.4872369E+03, 0.7847066E+02, 0.4366581E+02,
     . 0.2870837E+03, 0.5009062E+02, 0.3295655E+04, 0.5789249E+03,
     . 0.1388171E+03, 0.3218217E+02/
      data (scoef( 9,j),j=1,36) / 0.0000000E+00, 0.3089123E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.1055703E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.5977988E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.1598280E+00,
     . 0.1132629E+00,-0.2142111E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.2412702E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 9,j),j=1,36) / 0.4476055E+00, 0.2192138E+02,
     . 0.5340811E+02,-0.5579272E+01, 0.4054054E+01, 0.5657278E+02,
     . 0.3548601E+01, 0.6483376E+03, 0.2617615E+02, 0.1013527E+03,
     . 0.5210487E+02,-0.9421763E+01, 0.9515031E+02,-0.3575154E+01,
     . 0.5370754E+03, 0.1224067E+04,-0.8324326E+02, 0.9253274E+02,
     . 0.1284906E+04, 0.6745268E+02, 0.3103078E+04,-0.2374746E+03,
     . 0.2228494E+03, 0.3088268E+04, 0.1870104E+03, 0.9521982E+02,
     .-0.2048142E+02,-0.2797499E+03,-0.2673219E+02, 0.3287624E+02,
     . 0.2417376E+03, 0.1522926E+02, 0.3831306E+04, 0.2196652E+03,
     . 0.7287435E+02, 0.8194879E+01/
      data (sdev( 9,j),j=1,36) / 0.2546458E+02, 0.7520314E+01,
     . 0.1583576E+02, 0.8007618E+01, 0.4055698E+01, 0.2512221E+02,
     . 0.7765975E+01, 0.1336978E+04, 0.5802391E+03, 0.1659712E+04,
     . 0.2310691E+03, 0.1320454E+03, 0.1407886E+04, 0.2287508E+03,
     . 0.3549326E+03, 0.6250199E+03, 0.1747647E+03, 0.1116726E+03,
     . 0.7815645E+03, 0.1800884E+03, 0.1754649E+04, 0.4243386E+03,
     . 0.2447865E+03, 0.1705707E+04, 0.4525801E+03, 0.1010436E+03,
     . 0.5000002E+02, 0.4821107E+03, 0.7772745E+02, 0.4336179E+02,
     . 0.2819421E+03, 0.5012947E+02, 0.3344774E+04, 0.5673912E+03,
     . 0.1336848E+03, 0.3292386E+02/
      data (scoef(10,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.9469541E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.1078930E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     .-0.1727341E+00, 0.0000000E+00,-0.6476331E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.6957146E+00, 0.0000000E+00, 0.1605409E+00,
     . 0.1261314E+00,-0.1576631E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg(10,j),j=1,36) / 0.3980477E+00, 0.2159967E+02,
     . 0.5280331E+02,-0.5925649E+01, 0.4006236E+01, 0.5633189E+02,
     . 0.3727766E+01, 0.6011931E+03, 0.2631410E+02, 0.1027855E+03,
     . 0.5542253E+02,-0.9485357E+01, 0.8738937E+02,-0.7033623E+00,
     . 0.5208379E+03, 0.1195026E+04,-0.9081781E+02, 0.8989330E+02,
     . 0.1261422E+04, 0.7061524E+02, 0.3041471E+04,-0.2519473E+03,
     . 0.2191908E+03, 0.3042551E+04, 0.1953049E+03, 0.9791512E+02,
     .-0.2154251E+02,-0.2975466E+03,-0.2856389E+02, 0.3216120E+02,
     . 0.2370388E+03, 0.1545336E+02, 0.3819031E+04, 0.2332690E+03,
     . 0.7262690E+02, 0.8647505E+01/
      data (sdev(10,j),j=1,36) / 0.2452266E+02, 0.7370310E+01,
     . 0.1591915E+02, 0.7926909E+01, 0.4014973E+01, 0.2541850E+02,
     . 0.7665672E+01, 0.1239007E+04, 0.5538984E+03, 0.1587020E+04,
     . 0.2307229E+03, 0.1327658E+03, 0.1352833E+04, 0.2263261E+03,
     . 0.3441646E+03, 0.6174752E+03, 0.1699011E+03, 0.1090849E+03,
     . 0.7755641E+03, 0.1731434E+03, 0.1751020E+04, 0.4186579E+03,
     . 0.2430095E+03, 0.1705991E+04, 0.4421535E+03, 0.1029045E+03,
     . 0.4997415E+02, 0.4806690E+03, 0.7786277E+02, 0.4229101E+02,
     . 0.2772892E+03, 0.4837127E+02, 0.3400294E+04, 0.5601602E+03,
     . 0.1343539E+03, 0.3347443E+02/
       end
c
c
c
      subroutine epshif5d(iyear,imonth,iday,elat,elon,ucmp12,vcmp12,
     *                    vel,vel12,iwnd)
c
c     This subroutine calculates tropical cyclone in the eastern Pacific
c     intensities through 120 hours based upon climatology and persistence
c     using tropical cyclone data during the years 1975-1999 for development.
c     Tropical cyclones used in this developmental dataset had initial
c     positions south of 35N and east of 160W and were 50km from any
c     coastline.  The linear regression model (one for each forecast time)
c     was created using the total change in intensity for each period
c     (12-hr,....120-hr) from intial conditions as the predictand and
c     35 predictors including and derived from julian day, latitude,
c     longitude, zonal speed, meridional speed, current intensity the
c     past 12-hour intensity trend.
c
c     In the formulation of the model linear terms are first put into the
c     model using a forward stepping approach for the 12-hour forecast.
c     The linear predictors chosen in this forward stepping process
c     are then forced into the model and exposed to the 2nd order terms,
c     which at this point are allowed to come into the model in a
c     stepwise fashion.  A backward step is then performed to remove
c     predictors that are no longer significant.  Then a final stepwise
c     stepping proceedure is performed possibly adding a removing predictors
c
c
c     Following the 12-hour forecast the predictors chosen for the previous
c     forecast period are then given preference in the selection process.
c     Again, the predictors chosen in this forward stepping process
c     are then forced into the model and exposed to the 2nd order terms,
c     which at this point are allowed to come into the model in a
c     stepwise fashion.  A backward pass through the data is then performed
c     to remove predictors that are no longer significant. Followed by
c     a final step that is stepwise. Probabilities were set at .000000001%.
c
c     J. Knaff (04/12/2001)
c
c     Modified by James Franklin 7/2002 for compatibility with
c     verify_model.f
c
c     Modified by James Franklin 3/2006 to avoid losing forecasts of winds
c     less than zero.  Before, such forecasts were tossed, resulting in a loss of official
c     forecasts in homogeneous comparisons.  Now, if the model forecasts a negative
c     windspeed, then the output value is set to 1 kt.
c
c
      common /coefeps/ scoef(10,36), avg(10,36), sdev(10,36)
c
c     dimension coeficients.
c
      parameter(nc=36)
      real p(36), forecast(10)
      double precision  dv (10)
      dimension iwnd(10)
c
c     dimension input.
c
      real elat, elon, vel, vel12
c
c     intialize to zero
c
      rad = 3.14159/180.
      do i=1,10
         dv(i)=0.0
         iwnd(i)=0
      end do
c
c
c     check for system intensity requirements.
c
      if (vel.lt.15.0.OR.vel12.lt.15.0) return
c
c     create predictor pool (first order terms, squares, and
c     co-variances terms)
c
c     p1 = absolute value of (julian day - 238)
c     p2 = lat
c     p3 = lon
c     p4 = u ! zonal speed of the storm over the last 12 hours
c     p5 = v ! meridional speed of the storm over the last 12 hours
c     p6 = vmax
c     p7 = delta vmax
c
c     calculate julian day
c
      call julian_day(imonth,iday,iyear,julday)
c
c     assign predictor values from the input data
c
      p(1) = dble(abs(julday-238))
      p(2) = dble(elat)
      p(3) = dble(elon)
c     avglat=(elat+elat12)/2.0
c     p(4) =dble((elon-elon12)* (-60.0)/ 12.0 *
c    .     COS(rad*avglat))
c     p(5)=dble((elat-elat12)*60./12.)
      p(4)=dble(ucmp12)
      p(5)=dble(vcmp12)
      p(6)=dble(vel)
      p(7)=dble(vel-vel12)
      p(8)=p(1)**2                !p1*p1
      p(9)=p(1)*p(2)              !p1*p2
      p(10)=p(1)*p(3)             !p1*p3
      p(11)=p(1)*p(4)             !etc....
      p(12)=p(1)*p(5)
      p(13)=p(1)*p(6)
      p(14)=p(1)*p(7)
      p(15)=p(2)**2
      p(16)=p(2)*p(3)
      p(17)=p(2)*p(4)
      p(18)=P(2)*p(5)
      p(19)=p(2)*p(6)
      p(20)=p(2)*p(7)
      p(21)=p(3)**2
      p(22)=p(3)*p(4)
      p(23)=p(3)*p(5)
      p(24)=p(3)*p(6)
      p(25)=p(3)*p(7)
      p(26)=p(4)**2
      p(27)=p(4)*p(5)
      p(28)=p(4)*p(6)
      p(29)=p(4)*p(7)
      p(30)=p(5)**2
      p(31)=p(5)*p(6)
      p(32)=p(5)*p(7)
      p(33)=p(6)**2
      p(34)=p(6)*p(7)
      p(35)=p(7)**2
      p(36)=vel
c
c     calculate the predicted incremental change in velocity
c
c
      do i=1,10
         dv(i)=0.0 ! intitialize array to zero.
         do j=1,35
            dv(i)=dv(i)+dble(scoef(i,j)*((p(j)-avg(i,j))/sdev(i,j)))
         end do
         dv(i)=dv(i)*dble(sdev(i,36)) + dble(avg(i,36))
      end do
c
c
c     construct forecast intensities
c
      forecast(1)=p(36)+dv(1)

      do i=1,10
         forecast(i)= p(36)+sngl(dv(i))
      end do
      do i=1,10
c         if (forecast(i).lt.0.0)forecast(i)=0.0
c
c        Modification to avoid losing forecasts - JLF 3/13/06
c        ----------------------------------------------------
         if (forecast(i).lt.0.5)forecast(i)=1.0
         iwnd(i)=nint(forecast(i))
      end do
c
c
      return
      end
C
C
C
      block data epacshifor_data
c
c     This subprogram contains the standardized coefficients, means and
c     standard deviations of the predictors used in the eastern Pacific
c     version of the Statistical Hurricane Intensity Forecast.  These
c     data are used in subroutine epshif5d and are passed via a common
c     block.
c
c     scoef    are the standardized coefficients
c     avg      are the averages
c     sdev     are the standard deviations
c
c
      common /coefeps/ scoef(10,36), avg(10,36), sdev(10,36)
c
      data (scoef( 1,j),j=1,36) / 0.0000000E+00,-0.1823524E+00,
     .-0.6025346E-01, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.7614017E+00, 0.0000000E+00,-0.9648295E-01, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2989867E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2441196E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 1,j),j=1,36) / 0.3409045E+02, 0.1647778E+02,
     . 0.1194100E+03,-0.7575877E+01, 0.2703427E+01, 0.6359764E+02,
     . 0.8415730E+00, 0.1709157E+04, 0.5391900E+03, 0.3987097E+04,
     .-0.2346845E+03, 0.8988298E+02, 0.1862021E+04, 0.3237899E+02,
     . 0.2843427E+03, 0.1978615E+04,-0.1227261E+03, 0.4757833E+02,
     . 0.9302553E+03,-0.9259438E+00, 0.1446544E+05,-0.9215050E+03,
     . 0.3176249E+03, 0.6665289E+04, 0.5694735E+02, 0.8057443E+02,
     .-0.1971690E+02,-0.4274374E+03,-0.7634565E+01, 0.1694525E+02,
     . 0.1703698E+03, 0.4083034E+01, 0.3833088E+04, 0.7290146E+02,
     . 0.1182917E+03,-0.6213483E-01/
      data (sdev( 1,j),j=1,36) / 0.2338930E+02, 0.3581487E+01,
     . 0.1437713E+02, 0.4814886E+01, 0.3104484E+01, 0.2800452E+03,
     . 0.1084420E+02, 0.2377903E+04, 0.3472305E+03, 0.2704805E+04,
     . 0.2426896E+03, 0.1372968E+03, 0.1545159E+04, 0.4306967E+03,
     . 0.1266151E+03, 0.5390896E+03, 0.8227717E+02, 0.6035736E+02,
     . 0.5446276E+03, 0.1866596E+03, 0.3603070E+04, 0.6238622E+03,
     . 0.3774387E+03, 0.3333336E+04, 0.1306984E+04, 0.7470361E+02,
     . 0.2940368E+02, 0.4714602E+03, 0.9879948E+02, 0.2882326E+02,
     . 0.3413755E+03, 0.4834698E+02, 0.3712495E+04, 0.2072621E+04,
     . 0.2253573E+03, 0.1101109E+02/
      data (scoef( 2,j),j=1,36) / 0.0000000E+00,-0.2201037E+00,
     .-0.1125746E+01, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.7669960E+00, 0.0000000E+00,-0.1205761E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.3805355E+00, 0.9819010E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.3280108E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 2,j),j=1,36) / 0.3388374E+02, 0.1621464E+02,
     . 0.1190225E+03,-0.7737213E+01, 0.2666646E+01, 0.6608255E+02,
     . 0.1375492E+01, 0.1694369E+04, 0.5275113E+03, 0.3950797E+04,
     .-0.2393237E+03, 0.8834578E+02, 0.1907476E+04, 0.5036208E+02,
     . 0.2745901E+03, 0.1940280E+04,-0.1240043E+03, 0.4571776E+02,
     . 0.9503964E+03, 0.8596223E+01, 0.1436974E+05,-0.9367046E+03,
     . 0.3124024E+03, 0.6867745E+04, 0.1217556E+03, 0.8150956E+02,
     .-0.2046919E+02,-0.4495492E+03,-0.1176072E+02, 0.1598613E+02,
     . 0.1734764E+03, 0.5824803E+01, 0.4036648E+04, 0.9347293E+02,
     . 0.1184358E+03,-0.3540846E+00/
      data (sdev( 2,j),j=1,36) / 0.2337367E+02, 0.3417155E+01,
     . 0.1426190E+02, 0.4652715E+01, 0.2979298E+01, 0.2928763E+03,
     . 0.1079621E+02, 0.2404624E+04, 0.3395607E+03, 0.2700929E+04,
     . 0.2367192E+03, 0.1330576E+03, 0.1568953E+04, 0.4286487E+03,
     . 0.1185794E+03, 0.5170034E+03, 0.7846817E+02, 0.5543961E+02,
     . 0.5550459E+03, 0.1833025E+03, 0.3568410E+04, 0.6010082E+03,
     . 0.3588002E+03, 0.3373653E+04, 0.1297714E+04, 0.7369955E+02,
     . 0.2833205E+02, 0.4820746E+03, 0.9938897E+02, 0.2604367E+02,
     . 0.3510014E+03, 0.4619587E+02, 0.3774183E+04, 0.2161811E+04,
     . 0.2182329E+03, 0.1938118E+02/
      data (scoef( 3,j),j=1,36) / 0.0000000E+00,-0.2303735E+00,
     .-0.1473419E+01, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.7107538E+00, 0.0000000E+00,-0.1389359E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.4188341E+00, 0.1300377E+01, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.6600728E-01,
     . 0.0000000E+00, 0.0000000E+00,-0.3934926E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 3,j),j=1,36) / 0.3368899E+02, 0.1595987E+02,
     . 0.1186078E+03,-0.7856443E+01, 0.2622831E+01, 0.6849837E+02,
     . 0.2040401E+01, 0.1680311E+04, 0.5164424E+03, 0.3916439E+04,
     .-0.2425100E+03, 0.8671753E+02, 0.1946771E+04, 0.7170458E+02,
     . 0.2654435E+03, 0.1902633E+04,-0.1242365E+03, 0.4395626E+02,
     . 0.9662881E+03, 0.2062413E+02, 0.1426791E+05,-0.9467457E+03,
     . 0.3062754E+03, 0.7042070E+04, 0.2034481E+03, 0.8239523E+02,
     .-0.2073621E+02,-0.4684744E+03,-0.1724166E+02, 0.1511958E+02,
     . 0.1754523E+03, 0.7855410E+01, 0.4233243E+04, 0.1224204E+03,
     . 0.1156651E+03,-0.9270607E+00/
      data (sdev( 3,j),j=1,36) / 0.2335459E+02, 0.3275283E+01,
     . 0.1414661E+02, 0.4546905E+01, 0.2870793E+01, 0.3072913E+03,
     . 0.1056016E+02, 0.2435531E+04, 0.3325807E+03, 0.2700756E+04,
     . 0.2341398E+03, 0.1308612E+03, 0.1594228E+04, 0.4169710E+03,
     . 0.1119010E+03, 0.4971324E+03, 0.7578139E+02, 0.5153394E+02,
     . 0.5683723E+03, 0.1760623E+03, 0.3533794E+04, 0.5863201E+03,
     . 0.3425258E+03, 0.3432086E+04, 0.1268322E+04, 0.7296887E+02,
     . 0.2785581E+02, 0.4960866E+03, 0.9846447E+02, 0.2392651E+02,
     . 0.3619989E+03, 0.4294414E+02, 0.3852650E+04, 0.2257165E+04,
     . 0.2120755E+03, 0.2645941E+02/
      data (scoef( 4,j),j=1,36) / 0.0000000E+00,-0.6944878E+00,
     .-0.1674557E+01, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.6274028E+00, 0.0000000E+00,-0.1514233E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.5581393E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.4049150E+00, 0.1250209E+01, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.7471567E-01,
     . 0.0000000E+00, 0.0000000E+00,-0.4392256E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 4,j),j=1,36) / 0.3350910E+02, 0.1571008E+02,
     . 0.1181326E+03,-0.7945663E+01, 0.2557487E+01, 0.7075982E+02,
     . 0.2670429E+01, 0.1667663E+04, 0.5056566E+03, 0.3882390E+04,
     .-0.2444269E+03, 0.8436892E+02, 0.1976468E+04, 0.9222919E+02,
     . 0.2567841E+03, 0.1864842E+04,-0.1238269E+03, 0.4194849E+02,
     . 0.9755518E+03, 0.3153982E+02, 0.1415158E+05,-0.9529552E+03,
     . 0.2972958E+03, 0.7171507E+04, 0.2793818E+03, 0.8310629E+02,
     .-0.2060539E+02,-0.4832446E+03,-0.2235800E+02, 0.1434458E+02,
     . 0.1753482E+03, 0.9751166E+01, 0.4404444E+04, 0.1549809E+03,
     . 0.1150295E+03,-0.1737848E+01/
      data (sdev( 4,j),j=1,36) / 0.2334276E+02, 0.3158935E+01,
     . 0.1401045E+02, 0.4469423E+01, 0.2793746E+01, 0.3236109E+03,
     . 0.1038819E+02, 0.2472404E+04, 0.3255645E+03, 0.2702188E+04,
     . 0.2325489E+03, 0.1293590E+03, 0.1617585E+04, 0.4053358E+03,
     . 0.1064771E+03, 0.4795868E+03, 0.7352919E+02, 0.4875947E+02,
     . 0.5826815E+03, 0.1700602E+03, 0.3490472E+04, 0.5741495E+03,
     . 0.3301058E+03, 0.3499914E+04, 0.1247264E+04, 0.7179615E+02,
     . 0.2745527E+02, 0.5120160E+03, 0.9851597E+02, 0.2227744E+02,
     . 0.3742758E+03, 0.4073142E+02, 0.3942398E+04, 0.2366353E+04,
     . 0.2132211E+03, 0.3194976E+02/
      data (scoef( 5,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.6812104E+00, 0.0000000E+00,-0.1495004E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2898585E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.5135095E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2273493E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.8122859E-01,
     . 0.0000000E+00, 0.0000000E+00,-0.3064732E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 5,j),j=1,36) / 0.3327280E+02, 0.1547144E+02,
     . 0.1176365E+03,-0.8025509E+01, 0.2476860E+01, 0.7294497E+02,
     . 0.3328004E+01, 0.1649972E+04, 0.4945912E+03, 0.3841616E+04,
     .-0.2455439E+03, 0.8077078E+02, 0.1994411E+04, 0.1132713E+03,
     . 0.2486167E+03, 0.1828098E+04,-0.1232210E+03, 0.3979703E+02,
     . 0.9797526E+03, 0.4265947E+02, 0.1403067E+05,-0.9581615E+03,
     . 0.2865943E+03, 0.7265209E+04, 0.3586360E+03, 0.8392738E+02,
     .-0.2025503E+02,-0.4949773E+03,-0.2786637E+02, 0.1355154E+02,
     . 0.1738348E+03, 0.1159021E+02, 0.4546875E+04, 0.1918206E+03,
     . 0.1140813E+03,-0.2786772E+01/
      data (sdev( 5,j),j=1,36) / 0.2330202E+02, 0.3041842E+01,
     . 0.1386926E+02, 0.4418356E+01, 0.2723592E+01, 0.3420756E+03,
     . 0.1015003E+02, 0.2512109E+04, 0.3184644E+03, 0.2702379E+04,
     . 0.2311652E+03, 0.1254157E+03, 0.1632792E+04, 0.3962381E+03,
     . 0.1006864E+03, 0.4603183E+03, 0.7158336E+02, 0.4644048E+02,
     . 0.5971776E+03, 0.1630912E+03, 0.3445141E+04, 0.5664914E+03,
     . 0.3198441E+03, 0.3571135E+04, 0.1213577E+04, 0.7166328E+02,
     . 0.2698355E+02, 0.5301886E+03, 0.9685469E+02, 0.2155422E+02,
     . 0.3892121E+03, 0.3890517E+02, 0.4031562E+04, 0.2486870E+04,
     . 0.2124573E+03, 0.3609719E+02/
      data (scoef( 6,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.6261557E+00, 0.0000000E+00,-0.1497080E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2719499E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.5162974E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2741849E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.7807224E-01,
     . 0.0000000E+00, 0.0000000E+00,-0.2952774E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 6,j),j=1,36) / 0.3309105E+02, 0.1524456E+02,
     . 0.1170965E+03,-0.8085144E+01, 0.2390498E+01, 0.7491747E+02,
     . 0.3891728E+01, 0.1636725E+04, 0.4848110E+03, 0.3805275E+04,
     .-0.2458836E+03, 0.7685264E+02, 0.2000089E+04, 0.1311931E+03,
     . 0.2410369E+03, 0.1792407E+04,-0.1222881E+03, 0.3766839E+02,
     . 0.9766734E+03, 0.5174439E+02, 0.1390026E+05,-0.9611770E+03,
     . 0.2751466E+03, 0.7302574E+04, 0.4254768E+03, 0.8465199E+02,
     .-0.1974076E+02,-0.5023709E+03,-0.3289441E+02, 0.1277484E+02,
     . 0.1704612E+03, 0.1295930E+02, 0.4633386E+04, 0.2234049E+03,
     . 0.1132493E+03,-0.3925989E+01/
      data (sdev( 6,j),j=1,36) / 0.2327682E+02, 0.2939717E+01,
     . 0.1373689E+02, 0.4391593E+01, 0.2657386E+01, 0.3626381E+03,
     . 0.9905671E+01, 0.2560954E+04, 0.3121063E+03, 0.2704248E+04,
     . 0.2283015E+03, 0.1202093E+03, 0.1636206E+04, 0.3908052E+03,
     . 0.9559690E+02, 0.4429090E+03, 0.7007668E+02, 0.4442154E+02,
     . 0.6100070E+03, 0.1564196E+03, 0.3400547E+04, 0.5635516E+03,
     . 0.3103789E+03, 0.3626780E+04, 0.1178258E+04, 0.7209558E+02,
     . 0.2665871E+02, 0.5498324E+03, 0.9543129E+02, 0.2097211E+02,
     . 0.4054575E+03, 0.3702311E+02, 0.4106859E+04, 0.2619981E+04,
     . 0.2103640E+03, 0.3879220E+02/
      data (scoef( 7,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.5853452E+00, 0.0000000E+00,-0.1406750E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2364586E+00, 0.0000000E+00,-0.8097417E-01,
     . 0.0000000E+00,-0.4477453E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.3301889E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.8612577E-01,-0.2660320E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 7,j),j=1,36) / 0.3287039E+02, 0.1503177E+02,
     . 0.1165874E+03,-0.8148692E+01, 0.2312446E+01, 0.7469045E+02,
     . 0.4279135E+01, 0.1621358E+04, 0.4750126E+03, 0.3765667E+04,
     .-0.2458547E+03, 0.7305763E+02, 0.1991023E+04, 0.1410150E+03,
     . 0.2340843E+03, 0.1759192E+04,-0.1215257E+03, 0.3584299E+02,
     . 0.9667653E+03, 0.5768961E+02, 0.1377877E+05,-0.9648518E+03,
     . 0.2649892E+03, 0.7298661E+04, 0.4706489E+03, 0.8548945E+02,
     .-0.1926236E+02,-0.5090874E+03,-0.3672544E+02, 0.1212880E+02,
     . 0.1640531E+03, 0.1378406E+02, 0.4672013E+04, 0.2449278E+03,
     . 0.1147646E+03,-0.4972365E+01/
      data (sdev( 7,j),j=1,36) / 0.2325965E+02, 0.2851655E+01,
     . 0.1364507E+02, 0.4369481E+01, 0.2604390E+01, 0.3572350E+03,
     . 0.9822130E+01, 0.2620210E+04, 0.3063521E+03, 0.2710421E+04,
     . 0.2250002E+03, 0.1148629E+03, 0.1627826E+04, 0.3834453E+03,
     . 0.9109157E+02, 0.4275875E+03, 0.6883896E+02, 0.4284667E+02,
     . 0.6080621E+03, 0.1531289E+03, 0.3367195E+04, 0.5614836E+03,
     . 0.3026992E+03, 0.3670575E+04, 0.1166571E+04, 0.7268656E+02,
     . 0.2630556E+02, 0.5491028E+03, 0.9548569E+02, 0.2073056E+02,
     . 0.3987535E+03, 0.3619340E+02, 0.4166875E+04, 0.2772995E+04,
     . 0.2160317E+03, 0.4040804E+02/
      data (scoef( 8,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.1366803E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2516920E+00, 0.0000000E+00,-0.9944827E-01,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2916846E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.3086647E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 8,j),j=1,36) / 0.3267741E+02, 0.1483578E+02,
     . 0.1161084E+03,-0.8193725E+01, 0.2257631E+01, 0.7162686E+02,
     . 0.4580464E+01, 0.1611289E+04, 0.4663680E+03, 0.3730107E+04,
     .-0.2449940E+03, 0.7036105E+02, 0.1976781E+04, 0.1472987E+03,
     . 0.2277729E+03, 0.1728568E+04,-0.1205942E+03, 0.3454231E+02,
     . 0.9528255E+03, 0.6192274E+02, 0.1366508E+05,-0.9661400E+03,
     . 0.2576956E+03, 0.7274969E+04, 0.5052988E+03, 0.8597862E+02,
     .-0.1888309E+02,-0.5160939E+03,-0.3935513E+02, 0.1167900E+02,
     . 0.1558802E+03, 0.1408596E+02, 0.4682938E+04, 0.2563573E+03,
     . 0.1164193E+03,-0.5997314E+01/
      data (sdev( 8,j),j=1,36) / 0.2331542E+02, 0.2770294E+01,
     . 0.1356292E+02, 0.4341210E+01, 0.2565874E+01, 0.3118126E+03,
     . 0.9770463E+01, 0.2700221E+04, 0.3019180E+03, 0.2728907E+04,
     . 0.2217190E+03, 0.1108056E+03, 0.1614140E+04, 0.3789682E+03,
     . 0.8680218E+02, 0.4127718E+03, 0.6735981E+02, 0.4158892E+02,
     . 0.5858888E+03, 0.1504650E+03, 0.3336576E+04, 0.5575274E+03,
     . 0.2972206E+03, 0.3702202E+04, 0.1157438E+04, 0.7301750E+02,
     . 0.2609112E+02, 0.5195650E+03, 0.9562437E+02, 0.2091177E+02,
     . 0.3571355E+03, 0.3537364E+02, 0.4208102E+04, 0.2944192E+04,
     . 0.2209439E+03, 0.4131228E+02/
      data (scoef( 9,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.1245924E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.2065900E+00, 0.0000000E+00,-0.1119146E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.3103650E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.3156649E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg( 9,j),j=1,36) / 0.3256700E+02, 0.1464551E+02,
     . 0.1155945E+03,-0.8221828E+01, 0.2212730E+01, 0.6724372E+02,
     . 0.4851480E+01, 0.1610180E+04, 0.4590651E+03, 0.3702369E+04,
     .-0.2446719E+03, 0.6839015E+02, 0.1959760E+04, 0.1568171E+03,
     . 0.2217126E+03, 0.1698372E+04,-0.1193639E+03, 0.3350007E+02,
     . 0.9351116E+03, 0.6553180E+02, 0.1354381E+05,-0.9647840E+03,
     . 0.2514245E+03, 0.7223625E+04, 0.5354881E+03, 0.8602952E+02,
     .-0.1838713E+02,-0.5213773E+03,-0.4177164E+02, 0.1129564E+02,
     . 0.1469301E+03, 0.1391820E+02, 0.4667659E+04, 0.2648191E+03,
     . 0.1178677E+03,-0.7068398E+01/
      data (sdev( 9,j),j=1,36) / 0.2344619E+02, 0.2687708E+01,
     . 0.1348207E+02, 0.4293741E+01, 0.2530070E+01, 0.2366000E+03,
     . 0.9713761E+01, 0.2796710E+04, 0.2982126E+03, 0.2756915E+04,
     . 0.2191171E+03, 0.1077691E+03, 0.1599424E+04, 0.3733430E+03,
     . 0.8225807E+02, 0.3977646E+03, 0.6485299E+02, 0.4052077E+02,
     . 0.5554506E+03, 0.1482124E+03, 0.3306920E+04, 0.5491180E+03,
     . 0.2916813E+03, 0.3740634E+04, 0.1145511E+04, 0.7211736E+02,
     . 0.2558451E+02, 0.4751219E+03, 0.9594357E+02, 0.2122226E+02,
     . 0.2924824E+03, 0.3480763E+02, 0.4253586E+04, 0.3133444E+04,
     . 0.2277168E+03, 0.4168740E+02/
      data (scoef(10,j),j=1,36) / 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,-0.6412943E+00,
     . 0.0000000E+00, 0.0000000E+00,-0.1008322E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00,-0.1808974E+00, 0.0000000E+00,-0.9930872E-01,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.0000000E+00, 0.0000000E+00, 0.0000000E+00,
     . 0.0000000E+00, 0.1000000E+01/
      data (avg(10,j),j=1,36) / 0.3244245E+02, 0.1446961E+02,
     . 0.1151464E+03,-0.8219359E+01, 0.2156941E+01, 0.6130843E+02,
     . 0.5112215E+01, 0.1608799E+04, 0.4521713E+03, 0.3675283E+04,
     .-0.2431198E+03, 0.6584370E+02, 0.1934150E+04, 0.1645114E+03,
     . 0.2161679E+03, 0.1671032E+04,-0.1178861E+03, 0.3231959E+02,
     . 0.9146586E+03, 0.6895284E+02, 0.1344060E+05,-0.9605197E+03,
     . 0.2440476E+03, 0.7157191E+04, 0.5657189E+03, 0.8555419E+02,
     .-0.1777317E+02,-0.5241910E+03,-0.4391199E+02, 0.1068508E+02,
     . 0.1363673E+03, 0.1368596E+02, 0.4633374E+04, 0.3485460E+03,
     . 0.1163851E+03,-0.8081436E+01/
      data (sdev(10,j),j=1,36) / 0.2358951E+02, 0.2607800E+01,
     . 0.1348963E+02, 0.4242887E+01, 0.2456545E+01, 0.2957923E+02,
     . 0.9501540E+01, 0.2905742E+04, 0.2946989E+03, 0.2792864E+04,
     . 0.2139260E+03, 0.1052045E+03, 0.1575003E+04, 0.3630870E+03,
     . 0.7811045E+02, 0.3847938E+03, 0.6279043E+02, 0.3821323E+02,
     . 0.5127150E+03, 0.1431177E+03, 0.3302588E+04, 0.5408215E+03,
     . 0.2809234E+03, 0.3784311E+04, 0.1114523E+04, 0.7099328E+02,
     . 0.2340798E+02, 0.4078613E+03, 0.9483244E+02, 0.1876076E+02,
     . 0.1704033E+03, 0.3395292E+02, 0.4303698E+04, 0.8312715E+03,
     . 0.2208521E+03, 0.4166153E+02/
       end
C
C
      subroutine julian_day(imon,iday,iyear,julday)
c
c     This routine calculates the Julian day (julday) from
c     the month (imon), day (iday), and year (iyear). The
c     appropriate correction is made for leap year.
c
      dimension ndmon(12)
c
c     Specify the number of days in each month
      ndmon(1)  = 31
      ndmon(2)  = 28
      ndmon(3)  = 31
      ndmon(4)  = 30
      ndmon(5)  = 31
      ndmon(6)  = 30
      ndmon(7)  = 31
      ndmon(8)  = 31
      ndmon(9)  = 30
      ndmon(10) = 31
      ndmon(11) = 30
      ndmon(12) = 31
c
c     Correct for leap year
      if (mod(iyear,4) .eq. 0) ndmon(2)=29
c
c     Check for illegal input
      if (imon .lt. 1 .or. imon .gt. 12) then
         julday=-1
         return
      endif
c
      if (iday .lt. 1 .or. iday .gt. ndmon(imon)) then
         julday=-1
         return
      endif
c
c     Calculate the Julian day
      julday = iday
      if (imon .gt. 1) then
         do 10 i=2,imon
            julday = julday + ndmon(i-1)
   10    continue
      endif
c
      return
      end
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE BTCLIP(BASIN,IMO,IDY,IHR,NBT,BMO,BDY,BHR,
     *                  BLAT,BLON,BWS,CLAT,CLON,SWND)
C
C     Returns original BT cliper forecast.  Output is 12,24,36,48,
C     60, and 72 h.  Remaining elements of CLAT, CLON are -9999.
C     Valid for the north Atlantic only.
C
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     NBT        Number of elements in the best track data arrays.
C     BMO,BDY,BHR   Arrays of best track dates/times.
C     BLAT(NBT)  BT latitude (deg N).
C     BLON(NBT)  BT longitude (deg E).
C     BWS(NBT)   BT wind speed (kt).
C     CLAT(NVTX) Returned array of BT CLIPER lats.
C     CLON(NVTX) Returned array of BT CLIPER lons.
C     SWND(NVTX) Placeholder for SHIFOR forecast - not used.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION BMO(NBT), BDY(NBT), BHR(NBT)
      DIMENSION BLAT(NBT), BLON(NBT), BWS(NBT)
      DIMENSION CLAT(NVTX), CLON(NVTX), SWND(NVTX)
      DIMENSION DISP(12)
      CHARACTER*2 BASIN
C
C
      RLAT0 = -9999.
      RLON0 = -9999.
      DIR0  = -9999.
      SPD0  = -9999.
      DIR12 = -9999.
      SPD12 = -9999.
      WS0   = -9999.
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
      IF (BASIN.NE.'AL') RETURN
C
C
C     Compute input parameters for the CLIPER subroutine.
C     Start by checking for a BT match to desired time.
C     ---------------------------------------------------
      DO 110 L = 1,NBT
         IF (IMO.EQ.NINT(BMO(L)).AND.IDY.EQ.NINT(BDY(L))
     *       .AND.IHR.EQ.NINT(BHR(L)))
     *   THEN
            L0 = L
            GOTO 200
            ENDIF
110      CONTINUE
C
C     Could not find a best track entry for the requested time
C     --------------------------------------------------------
      GOTO 800
C
C
C     Found a valid match at index L0.  Get initial position.
C     -------------------------------------------------------
200   RLAT0 = BLAT(L0)
      RLON0 = -BLON(L0)
      WS0 = BWS(L0)
C
C     Get previous, next positions to estimate current speed
C     ------------------------------------------------------
      LB = L0-1
      LF = L0+1
      IF (LB.LT.1) LB=1
      IF (LF.GT.NBT) LF = NBT
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR0)
      SPD0 = DIST/DELTAH
C     DIR0 = NINT(DIR0)
C     SPD0 = NINT(SPD0)
C
C
C     Now try to get previous 12 h speed
C     ----------------------------------
      LB = L0-3
      LF = L0-1
      IF (LB.LT.1) LB=1
      IF (LF.LE.LB) LF=LB+1
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR12)
      SPD12 = DIST/DELTAH
C     DIR12 = NINT(DIR12)
C     SPD12 = NINT(SPD12)
C
C
C
C     Generate CLIPER displacements and convert to lat/lon.
C     -----------------------------------------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (DIR12.EQ.-9999. OR. SPD12.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
C      WRITE(1,'(3I2,1X,7F9.1)')
C     *   IMO,IDY,IHR,RLAT0,RLON0,DIR0,SPD0,DIR12,SPD12,WS0
      CALL CLIPER(RLAT0,RLON0,DIR0,SPD0,DIR12,SPD12,WS0,IMO,IDY,DISP)
      GBEAR = 360.
      GSIZE = 1.
      CALL STHGPR(RLAT0,RLON0,GBEAR,GSIZE,0.,0.)
      DO 510 I=1,6
         K=2*I
         J=(2*I)-1
         CALL XY2LLH(DISP(K),DISP(J),CLAT(I),CLON(I))
         CLAT(I) = FLOAT(INT(CLAT(I)*10.+0.5))/10.
         CLON(I) = -FLOAT(INT(CLON(I)*10.+0.5))/10.
510      CONTINUE
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE BTCLIPA(BASIN,IMO,IDY,IHR,NBT,BMO,BDY,BHR,
     *                  BLAT,BLON,BWS,CLAT,CLON,SWND)
C
C     Returns BT cliper forecast, using Sim Aberson version
C     of Charlie Neumann's original model, except raw output is
C     every 12 h out to 120 h.  Valid for the north Atlantic only.
C
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     NBT        Number of elements in the best track data arrays.
C     BMO,BDY,BHR   Arrays of best track dates/times.
C     BLAT(NBT)  BT latitude (deg N).
C     BLON(NBT)  BT longitude (deg E).
C     BWS(NBT)   BT wind speed (kt).
C     CLAT(NVTX) Returned array of BT CLIPER lats.
C     CLON(NVTX) Returned array of BT CLIPER lons.
C     SWND(NVTX) Placeholder for SHIFOR forecast - not used.
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      parameter (nvaratl=165,nvar=nvaratl)
      DIMENSION BMO(NBT), BDY(NBT), BHR(NBT)
      DIMENSION BLAT(NBT), BLON(NBT), BWS(NBT)
      DIMENSION CLAT(NVTX), CLON(NVTX), SWND(NVTX)
      CHARACTER*2 BASIN
      CHARACTER*20 ALFILE
C
      real rlat(11),rlon(11),jday(12),
     1     acon(20),coef(20,nvar),x(nvar),disp(20)
      data jday /1.,32.,60.,91.,121.,152.,182.,213.,244.,274.,305.,335./
C
C
      RLAT0 = -9999.
      RLON0 = -9999.
      DIR0  = -9999.
      SPD0  = -9999.
      DIR12 = -9999.
      SPD12 = -9999.
      WS0   = -9999.
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
      IF (BASIN.NE.'AL') RETURN
C
C
C     Compute input parameters for the CLIPER subroutine.
C     Start by checking for a BT match to desired time.
C     ---------------------------------------------------
      DO 110 L = 1,NBT
         IF (IMO.EQ.NINT(BMO(L)).AND.IDY.EQ.NINT(BDY(L))
     *       .AND.IHR.EQ.NINT(BHR(L)))
     *   THEN
            L0 = L
            GOTO 200
            ENDIF
110      CONTINUE
C
C     Could not find a best track entry for the requested time
C     --------------------------------------------------------
      GOTO 800
C
C
C     Found a valid match at index L0.  Get initial position.
C     -------------------------------------------------------
200   RLAT0 = BLAT(L0)
      RLON0 = -BLON(L0)
      WS0 = BWS(L0)
C
C     Get previous, next positions to estimate current speed
C     ------------------------------------------------------
      LB = L0-1
      LF = L0+1
      IF (LB.LT.1) LB=1
      IF (LF.GT.NBT) LF = NBT
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR0)
      SPD0 = DIST/DELTAH
C     DIR0 = NINT(DIR0)
C     SPD0 = NINT(SPD0)
C
C
C     Now try to get previous 12 h speed
C     ----------------------------------
      LB = L0-3
      LF = L0-1
      IF (LB.LT.1) LB=1
      IF (LF.LE.LB) LF=LB+1
      CALL DIFTIME(2000,NINT(BMO(LB)),NINT(BDY(LB)),BHR(LB),
     *             2000,NINT(BMO(LF)),NINT(BDY(LF)),BHR(LF),DELTAH)
      RLATB = BLAT(LB)
      RLONB = -BLON(LB)
      RLATF = BLAT(LF)
      RLONF = -BLON(LF)
      CALL LL2DB(RLATB,RLONB,RLATF,RLONF,DIST,DIR12)
      SPD12 = DIST/DELTAH
C     DIR12 = NINT(DIR12)
C     SPD12 = NINT(SPD12)
C
C
C
C     Generate CLIPER displacements and convert to lat/lon.
C     -----------------------------------------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (DIR12.EQ.-9999. OR. SPD12.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
C      WRITE(1,'(3I2,1X,7F9.1)')
C     *   IMO,IDY,IHR,RLAT0,RLON0,DIR0,SPD0,DIR12,SPD12,WS0
C
C     Sim's code begins here, with modifications.
C     -------------------------------------------
      alfile = 'sda_alclpt_coeff.dat'
      open (21,file=alfile,status='old',err=900)
      do i=1,20
         read(21,599,err=900) acon(i),(coef(i,j),j=1,nvaratl)
599      format(33(5e16.9,/),e16.9)
c599     format(33(5e15.7,/),e15.7)
c599     format(23(7f11.4,/),5f11.4)
         end do
      close (21)
      rlat(1) = RLAT0
      rlon(1) = RLON0
      wind=WS0*111.1*1000./(60.*3600.)
      days=jday(IMO)+real(IDY)+real(IHR)/24.
      rdir=DIR0+180.
      if(rdir.ge.360.)rdir=rdir-360.
      rspd=SPD0*111.1*1000./(60.*3600.)
      call uvcomp2(rdir,rspd,ucmp,vcmp)
      rdir=DIR12+180.
      if(rdir.ge.360.)rdir=rdir-360.
      rspd=SPD12*111.1*1000./(60.*3600.)
      call uvcomp2(rdir,rspd,ucmpm12,vcmpm12)
      ucmp=-ucmp
      ucmpm12=-ucmpm12
      x(1)=rlat(1)
      x(2)=rlon(1)
      x(3)=wind
      x(4)=days
      x(5)=vcmp
      x(6)=ucmp
      x(7)=vcmpm12
      x(8)=ucmpm12
c      write(1,*) (x(i),i=1,8)
c      rlat(1) = 17.1
c      rlon(1) = 84.2
c      x(1) = 17.1
c      x(2) = 84.2
c      x(3) = 15.43056
c      x(4) = 155.
c      x(5) = 4.008976
c      x(6) = 2.314584
c      x(7) = 4.899859
c      x(8) = 2.828935
c     write(1,*) (x(i),i=1,8)
      klij=8
      do ijkl=1,8
        do jkli=ijkl,8
          klij=klij+1
          x(klij)=x(ijkl)*x(jkli)
        end do
      end do
      do ijkl=1,8
        do jkli=ijkl,8
          do lijk=jkli,8
             klij=klij+1
            x(klij)=x(ijkl)*x(jkli)*x(lijk)
          end do
        end do
      end do
      do i=1,20
        disp(i)=acon(i)
c        if(rlat(1).lt.rlon(1)-64.)disp(i)=acon(i+20)
      end do
      do i=1,20
        do j=1,nvar
c          if(rlat(1).ge.rlon(1)-64.)then
             disp(i)=disp(i)+x(j)*coef(i,j)
c          else
c            disp(i)=disp(i)+x(j)*coef(i+20,j)
c          endif
        end do
      end do
      do i=2,11
        rlat(i)=rlat(1)+disp(i-1)
      end do
      do i=2,11
        disp(i+9)=disp(i+9)/
     +     cos(real((rlat(1)+rlat(i))/2.)*3.14159/180.0)
      end do
      do i=2,11
        rlon(i)=rlon(1)+disp(i+9)
      end do
c      write(1,101)
c    1             (int(rlat(i)*10.+0.5),int(rlon(i)*10.+0.5),i=2,5),
c     2             int(rlat(7)*10.+0.5),int(rlon(7)*10.+0.5)
c      write(1,101)
c     1             (int(rlat(i)*10.+0.5),int(rlon(i)*10.+0.5),i=7,11)
 101  format(10i4)
C
C
C     Load final array of forecast values.
C     ------------------------------------
      DO 510 I=1,10
         CLAT(I) = FLOAT(INT(RLAT(I+1)*10.+0.5))/10.
         CLON(I) = -FLOAT(INT(RLON(I+1)*10.+0.5))/10.
c         write(1,*) clat(i),clon(i)
510      CONTINUE
C
C
C     All done, go back
C     -----------------
800   RETURN
C
C
C     Error on coefficient file
C     -------------------------
900   PRINT *, ' FATAL ERROR READING CLIPER COEFFICIENTS.'
      STOP
C
      END
C
C
C
C     ------------------------------------------------------------
      SUBROUTINE OCLIP(BASIN,IYR,IMO,IDY,IHR,
     *                 RLAT0,RLON0E,WS0,DIR0,SPD0,
     *                 RLAT12,RLON12E,WS12,DIR12,SPD12,
     *                 CLAT,CLON,SWND)
C
C     Reruns operational 3-day cliper forecast.
C     Output variables formally at 12, 24, 36, 48, 60, 72,
C     84, 96, 108, and 120 h, but only filled for 12, 24, 36, 48,
C     and 72 h.  Valid for Atlantic basin only.
C
C     BASIN         AL or EP.
C     IYR           Year (4 digit)
C     IMO,IDY,IHR   Time to compute a cliper forecast for.
C     RLAT0         Initial lat
C     RLON0E        Initial long (deg E)
C     WS0           Initial wind speed
C     DIR0          Initial heading
C     SPD0          Initial forward speed
C     RLAT12        t-12h lat
C     RLON12E       t-12h long (deg E)
C     WS12          t-12h wind speed
C     DIR12         t-12h heading
C     SPD12         t-12h forward speed
C     CLAT(NVTX)    Returned array of CLIPER lats.
C     CLON(NVTX)    Returned array of CLIPER lons.
C     SWND(NVTX)    Returned array of SHIFOR winds (not yet).
C     ------------------------------------------------------------
C
      PARAMETER (NVTX=10)
      DIMENSION CLAT(NVTX), CLON(NVTX)
      DIMENSION IWSHIF(NVTX), SWND(NVTX)
      DIMENSION FLAT(5),FLON(5)
      CHARACTER*2 BASIN
C
C
      DO 50 J = 1,NVTX
         CLAT(J) = -9999.
         CLON(J) = -9999.
         SWND(J) = -9999.
50       CONTINUE
C
      IF (BASIN.NE.'AL') RETURN
C
      RLON0 = -9999.
      RLON12 = -9999.
      IF (RLON0E.NE.-9999.) RLON0 = -RLON0E
      IF (RLON12E.NE.-9999.) RLON12 = -RLON12E
C
C
C     Get CLIPER forecast.
C     --------------------
500   IF (RLAT0.EQ.-9999. .OR. RLON0.EQ.-9999.) GOTO 800
      IF (DIR0.EQ.-9999. OR. SPD0.EQ.-9999.) GOTO 800
      IF (WS0.EQ.-9999.) GOTO 800
      CALL OPCLIP(RLAT0,RLON0,DIR0,SPD0,DIR12,SPD12,WS0,IMO,IDY,
     *            FLAT,FLON)
C
C
C     Place results in final arrays.
C     ------------------------------
      DO 510 I=1,5
         II = I
         IF (I.EQ.5) II = 6
         CLON(II) = -FLON(I)
         CLAT(II) = FLAT(I)
510      CONTINUE
C
C
C     All done, go back
C     -----------------
800   RETURN
      END
C
C
C
C     ----------------------------------------------------------------
      SUBROUTINE OPCLIP(Y,X,DNOW,SNOW,DM12,SM12,WKTS,MO,KDA,YF,XF)
C
C     THIS IS THE OPERATIONAL SUBROUTINE--NOT THE 'BEST-TRACK' VERSION
C     THIS VERSION CONTAINS GULF OF MEXICO CORRECTION
C
C     Modified to pass lat,lon not displacements.  JLF 1/2004.
C     ----------------------------------------------------------------
C
      LOGICAL SKIP_OUTPUT
      DIMENSION DISP(10),FULL(10),GULF(10),CNS(14,5),CEW(8,5),ND(12),
     1  P(14),XG(5),YG(5),XA(5),YA(5),XF(5),YF(5)
      COMMON /BLK13/DAN
      COMMON /BLK3/ STMNAM(5),IN(10),LIST(5,6),FP(6,6)
C
C REFER TO NOAA TECHNICAL MEMORANDUM NWS SR-62 (C.J. NEUMANN, JAN 1972)
C FOR A DESCRIPTION OF CLIPER SYSTEM.
C
C
C LIST CONSTANTS FOR MERIDIONAL DISPLACEMENTS.
C
      DATA CNS/7.60553,13.59909,-2.575127,-.0001868914,.00460007,
     1.0022623,-.001491833,-.0002678624,-.00006994195,.00004407501,
     2-.0002049626,.00007781249,.1430621,-.00008156078,30.30846,
     322.91538,-2.484599,.004968631,.009297729,.02511378,-.007839641,
     4-.005977559,-.0003504201,.0001557468,-.0009956548,.0004778924,
     5.3879478,-.000671019,67.69324,31.94291,-3.697592,.009672852,
     6.009539232,.06322068,-.01332191,-.01610885,-.0007292257,
     7.0002341562,-.002814067,.001145255,.8940798,-.002181595,120.27143,
     838.94701,-4.380877,.01323013,.02292699,.09532129,-.01664333,
     9-.03200883,-.001216522,.000315372,-.005450743,.001867606,1.666658,
     1-.004353084,263.15653,48.41731,-4.456658,.01126704,.04297187,
     2.16962,-.01748416,-.06485724,-.002215515,.0003599106,-.012677,
     3.003690242,4.121246,-.01102184/
C
C LIST CONSTANTS FOR ZONAL DISPLACEMENTS.
C
      DATA CEW/-3.52591,13.69309,-2.637347,.8151257,.6867776,
     1-.002168753,-.000596625,.1247267,-13.12388,23.30256,-3.215529,
     23.584506,3.949364,-.007860247,-.006764091,.5135556,-28.48156,
     332.37355,-5.342858,8.073875,9.321241,-.01318171,-.02040824,
     41.044624,-44.13759,38.93667,-6.819777,14.10797,16.35476,
     5-.01967039,-.03853289,1.698018,-60.23074,46.26022,-8.8089,
     629.11625,32.91178,-.02181549,-.08553838,3.291178/
C
      DATA ND/0,31,59,90,120,151,181,212,243,273,304,334/
C
C     Skip written output (JLF 1/2004)
      DATA SKIP_OUTPUT/.TRUE./
C
C  CHECK FOR STORM LOCATION AND DETERMINE WEIGHTING FACTORS
      WA=(X-Y-61.)/5.
      WB=(Y-13.)/4.
      IF(WA.GT.1.)WA=1.
      IF(WA.LT.0.)WA=0.
      IF(WB.GT.1.)WB=1.
      IF(WB.LT.0.)WB=0.
      WGULF=WA*WB
      WFULL=1.-WGULF
C COMPUTE DAY NUMBER AND SUBTRACT OFF MEANS
C
      DAN=ND(MO)+KDA
      DANBR=ND(MO)+KDA-248
      IF(DANBR.LT.-95.)DANBR=-95.
      IF(DANBR.GT.82.)DANBR=82.
C
      WMPH=(WKTS*1.15)-71.
      ALAT=Y-24.
      ALON=X-68.
C
C CONVERT PRESENT AND PAST MOTION TO U AND V COMPONENTS.
C
      T=.0174533
      UNOW=SIN(DNOW*T)*SNOW
      VNOW=COS(DNOW*T)*SNOW
      UM12=SIN(DM12*T)*SM12
      VM12=COS(DM12*T)*SM12
      CALL CGULF(Y,X,UNOW,VNOW,UM12,VM12,DAN,WKTS,WGULF,GULF)
C
C SET UP MERIDIONAL PREDICTORS AND COMPUTE MERIDIONAL DISPLACEMENTS.
C
      P(1)=1.0
      P(2)=VNOW
      P(3)=VM12
      P(4)=VNOW*VM12*VM12
      P(5)=WMPH*VM12
      P(6)=VNOW*WMPH
      P(7)=VNOW*VNOW*VM12
      P(8)=ALAT*ALAT*VNOW
      P(9)=DANBR*DANBR*VM12
      P(10)=VNOW*DANBR*DANBR
      P(11)=ALAT*ALAT*DANBR
      P(12)=WMPH*DANBR*VM12
      P(13)=UNOW
      P(14)=DANBR*DANBR
      DO 15 I=1,5
      ZZ=0.
      DO 10 J=1,14
   10 ZZ=ZZ+CNS(J,I)*P(J)
  15  FULL(2*I-1)=ZZ
C
C SET UP ZONAL PREDICTORS AND COMPUTE ZONAL DISPLACEMENTS.
C
      P(2)=UNOW
      P(3)=UM12
      P(4)=ALAT
      P(5)=VNOW
      P(6)=VNOW*VNOW*UM12
      P(7)=ALAT*VNOW*UM12
      P(8)=ALON
      DO 25 I=1,5
      ZZ=0.
      DO 20 J=1,8
   20 ZZ=ZZ+CEW(J,I)*P(J)
C CHANGE SIGN TO DESIGNATE WESTWARD MOTION AS POSITIVE MOTION.
  25  FULL(2*I)=-ZZ
C  COMPUTE COMBINED DISPLACEMENTS AND OUTPUT SUPPLEMENTAL
C  CLIPER MESSAGE
C
C     Skip written output...JLF 1/2004
C
      IF (SKIP_OUTPUT) GOTO 100
      WRITE(6,32)
  32  FORMAT(1H1,15X,39HSUPPLEMENTAL OUTPUT FOR CLIPER FORECAST)
      IF(WFULL.EQ.0.)WRITE(6,33)
      IF(WGULF.EQ.0.)WRITE(6,34)
      IF(WFULL.NE.0.AND.WGULF.NE.0.)WRITE(6,35)
  33  FORMAT(1H0,15X,42HFORECAST BASED ON GULF OF MEXICO EQUATIONS)
  34  FORMAT(1H0,15X,42HFORECAST BASED ON ATLANTIC BASIN EQUATIONS)
  35  FORMAT(1H0,15X,37HFORECAST BASED ON WEIGHTED AVERAGE OF,/,16X,43HA
     1TLANTIC BASIN AND GULF OF MEXICO EQUATIONS)
      WRITE(6,36)STMNAM,DNOW,SNOW,DAN,DM12,SM12,WKTS
  36  FORMAT(1H0,15X,8HRUN FOR ,5A4,/,21X,15HCURRENT MOTION ,F4.0,1H/,F3
     1.0,12H  DAYNUMBER=,F4.0,/,21X,15H12H OLD MOTION ,F4.0,1H/,F3.0,7H
     2 WIND=,F4.0)
      WRITE(6,38)WFULL,WGULF
  38  FORMAT(1H0,15X,48HFCST   FULL BASIN    GULF OF MEXICO    COMPOSITE
     1,/,16X,13HTIME  WEIGHT=,F4.2,12H     WEIGHT=,F4.2)
C
100   DO 40 I=1,10
      DISP(I)=GULF(I)*WGULF+FULL(I)*WFULL
  40  CONTINUE
      DO 42 I=1,5
      YG(I)=GULF(2*I-1)/60.+Y
      XG(I)=GULF(2*I)/COS((YG(I)+Y)*T/2.)/60.+X
      IF(WGULF.EQ.0.)YG(I)=-99.9
      IF(WGULF.EQ.0.)XG(I)=-99.9
      YA(I)=FULL(2*I-1)/60.+Y
      XA(I)=FULL(2*I)/COS((YA(I)+Y)*T/2.)/60.+X
      YF(I)=DISP(2*I-1)/60.+Y
      XF(I)=DISP(2*I)/COS((YF(I)+Y)*T/2.)/60.+X
  42  CONTINUE
C  WRITE POSITIONS
      IF (SKIP_OUTPUT) RETURN
      ITME=0
      WRITE(6,305)ITME,Y,X,Y,X,Y,X
  305 FORMAT(1H ,16X,I2,F7.1,2HN ,F5.1,4HW   ,2(F5.1,2HN ,F5.1,4HW   ))
      DO 44 I=1,5
      ITME=12*I
      IF(I.EQ.5)ITME=72
      WRITE(6,305)ITME,YA(I),XA(I),YG(I),XG(I),YF(I),XF(I)
  44  CONTINUE
*     WRITE(6,310)
* 310 FORMAT(1H0,16X,49H.......... END OF CLIPER FORECAST ..............
*    1.,///)
      RETURN
      END
C
C
C
      SUBROUTINE CGULF(Y,X,U,V,U12,V12,DN,W,WGULF,GULF)
C  THIS SUBROUTINE WAS ADDED JUNE 1980 AND INCLUDES A CLIPER EQUATION
C  SET DEVELOPED FOR THE WESTERN GULF OF MEXICO AND  NORTHWEST CARIBBEAN
C  SEA.  IT IS CALLED BY SUBROUTINE CLIPER AND IS IN THE NHC72 OVERLAY
      DIMENSION GULF(10),X12(37),X24(37),X36(37),X48(37),X72(37),
     1             P(37),Y12(37),Y24(37),Y36(37),Y48(37),Y72(37)
      DATA Y12/       0.0888784,   2.0189338,  -3.0232588,   5.0112631,
     1  -0.4663586,  -2.2601204,   1.6083838,  -0.0002354,   0.0025722,
     2   0.0380934,  -0.0006782,  -0.0451460,   0.0196852,   0.0053249,
     3  -0.0941109,   0.1065220,   0.0371968,  -0.0074174,   0.0114476,
     4   0.0397118,  -0.1690010,  -0.1136812,  -0.0028529,   0.0703457,
     5  -0.0153966,  -0.0423261,   0.1190649,   0.0001786,   0.0059967,
     6  -0.0224450,  -0.0375346,   0.1719109,   0.2308200,  -0.1857073,
     7  -0.0995993,   0.0085389, 138.45727/
      DATA Y24/      -0.1899584,   8.6911949,  -7.7038102,  17.8842992,
     1   1.8958812, -26.6646610,   0.8565066,  -0.0005621,  -0.0008909,
     2   0.0850158,   0.0034124,  -0.1292738,   0.0426631,   0.0168735,
     3  -0.1717200,   0.0842524,   0.0193852,  -0.0207373,   0.1633104,
     4   0.0636077,  -0.5315965,  -0.4433265,  -0.0009782,   0.2091911,
     5   0.1755244,   0.0419906,   0.3086234,  -0.0357209,   0.0119171,
     6  -0.1367056,  -0.0408844,   0.5093530,   0.8909345,  -0.5621746,
     7  -0.3903772,   0.0554313, 408.02599/
      DATA Y36/      -1.2791723,  18.5437755, -21.4965205,  15.7440568,
     1  13.7586086, -56.0136527,  -1.0273326,  -0.0010307,  -0.0173412,
     2  -0.0069898,   0.0192933,  -0.1414892,   0.0882439,   0.0368659,
     3  -0.2220821,   0.1678605,   0.1462510,  -0.0429246,  -0.0773177,
     4   0.1059194,  -1.3342283,  -0.8653569,   0.0101134,   0.3178639,
     5   0.4078366,  -0.0984645,   0.7358741,   0.0350543,   0.0221573,
     6   0.2828421,  -0.1563257,   1.0009459,   1.6636230,  -1.0310655,
     7  -0.6516668,   0.1635486,1203.12003/
      DATA Y48/      -2.2503651,  29.2124117, -40.3642497,  17.0779631,
     1  17.2671773,-104.0852329,  12.0955831,  -0.0017919,  -0.0235381,
     2  -0.0449264,   0.0317965,  -0.2155600,   0.1666921,   0.0817288,
     3   0.2846074,  -0.0195034,   0.2448353,  -0.0474278,  -0.1532889,
     4   0.1664857,  -1.9062156,  -1.2207714,   0.0097954,  -0.1064522,
     5   1.0404667,  -0.2380823,   0.9551006,  -0.0139472,   0.0130756,
     6   0.4827187,  -0.3369744,   0.9455972,   2.3926721,  -0.9962841,
     7  -0.9012016,   0.2849380,2280.38586/
      DATA Y72/      -1.4432551,  69.0610138, -95.5025662, -27.2247875,
     1  29.0697411,-193.8726068,  19.9328988,  -0.0065750,  -0.0171360,
     2  -0.0367320,   0.0388394,  -0.6790639,   0.4813834,   0.2157335,
     3   0.8932160,   0.0837914,   0.4136060,  -0.0491940,  -0.3513719,
     4   0.2171922,  -2.6074776,  -1.9936009,  -0.0604859,  -1.0243483,
     5   2.4907242,  -0.6472058,   1.1425050,  -0.0513188,  -0.0240031,
     6   0.6859001,  -0.3869234,   0.6265983,   3.8490617,  -0.7979159,
     7  -1.4598369,   0.3810998,4739.46017/
      DATA X12/      -0.6704965,   1.4162022,  -4.6100138,   1.9871832,
     1  17.5537836,  -2.1569492,  -5.3402337,   0.0012716,   0.0030864,
     2   0.0278260,   0.0000683,  -0.0343753,   0.0304263,   0.0099592,
     3   0.0788021,  -0.0810208,   0.1417678,   0.0012298,  -0.0018823,
     4  -0.0419311,   0.0747932,  -0.0306750,  -0.0050167,  -0.0070392,
     5   0.0516586,  -0.1393882,   0.0512521,   0.0067553,  -0.0037287,
     6  -0.0017485,   0.0441335,  -0.2081854,   0.1950879,   0.0235002,
     7  -0.1346830,  -0.0285769, 259.47152/
      DATA X24/      -3.2076529,  18.7032497, -51.1117663,   5.6067213,
     1  41.0624529,  -5.9989359, -10.5029546,   0.0062741,   0.0186705,
     2   0.1565185,  -0.0014309,  -0.3151049,   0.3294738,   0.0497310,
     3   0.4094322,  -0.3283910,   0.2947786,   0.0020355,   0.0202198,
     4  -0.1759863,   0.1977871,  -0.0611235,  -0.0238707,  -0.0192411,
     5   0.1737112,  -0.2251423,   0.3112377,  -0.0422888,  -0.0167690,
     6   0.0152779,   0.1156717,  -0.8251600,   0.5527846,   0.0455937,
     7  -0.3788840,  -0.1737905,2398.43577/
      DATA X36/      -6.6781488,  45.4614176,-144.0815478,  -5.8840358,
     1  69.7781262,  -2.2740308,  -4.6481818,   0.0134484,   0.0379086,
     2   0.3815790,  -0.0039753,  -0.7343225,   0.9044577,   0.1020808,
     3   1.2687996,  -0.5415504,   0.4282047,   0.0083583,   0.2549853,
     4  -0.4604174,   0.0714267,  -0.0236414,  -0.0567410,  -0.3394735,
     5   0.3102145,  -0.4093667,   0.5577365,  -0.0197161,  -0.0410260,
     6   0.0190018,   0.1124278,  -1.3487227,   0.5703272,  -0.0312319,
     7  -0.3382746,  -0.4661488,6604.67254/
      DATA X48/     -11.2833695,  67.7041708,-213.4578700,   7.2101560,
     1 103.3901233, -19.5321088, -14.6757722,   0.0229843,   0.0483794,
     2   0.4193408,  -0.0036961,  -0.9820638,   1.3240829,   0.1520701,
     3   2.3483309,  -1.0601856,   0.4061403,   0.0143723,   0.9105408,
     4  -0.9182075,   0.0862826,  -0.1668057,  -0.0868560,  -0.8808472,
     5   0.7276123,  -0.3159147,   0.5817031,  -0.0911473,  -0.0663834,
     6  -0.4230014,   0.3969854,  -2.0183565,   0.7029859,   0.2263893,
     7  -0.3186548,  -0.7828428,9918.98936/
      DATA X72/     -20.0923662, 140.1359479,-364.9217391, 111.3688979,
     1 174.4471782,-107.3250567, -43.3118878,   0.0429588,   0.0710944,
     2   0.5375573,  -0.0133502,  -1.8051834,   2.2847824,   0.2595816,
     3   4.8826019,  -3.0362177,   0.2986331,   0.0258721,   2.1804674,
     4  -1.8921960,   0.2580539,  -0.5958524,  -0.1663261,  -2.3306786,
     5   2.2924075,  -0.7559446,   0.7681488,   0.0810918,  -0.1449324,
     6  -1.4833308,   1.2019937,  -3.7551080,   1.0364360,   0.7572207,
     7  -0.3288727,  -1.4009940,16794.94253/
C  INITIALIZE PREDICTAND ARRAY WITH ZEROS
      DO 105 I=1,10
      GULF(I)=0.
  105 CONTINUE
      IF(WGULF.EQ.0.)RETURN
C  COMPUTE PREDICTORS
      P(1)=DN
      P(2)=Y
      P(3)=X
      P(4)=V
      P(5)=U
      P(6)=V12
      P(7)=U12
      L=7
      DO 110 I=1,7
      K=I
      DO 110 J=1,K
      L=L+1
      P(L)=P(I)*P(J)
  110 CONTINUE
      P(36)=W
      P(37)=1.
C  COMPUTE DISPLACEMENTS
      DO 115 I=1,37
      GULF(1)=GULF(1)+Y12(I)*P(I)
      GULF(2)=GULF(2)-X12(I)*P(I)
      GULF(3)=GULF(3)+Y24(I)*P(I)
      GULF(4)=GULF(4)-X24(I)*P(I)
      GULF(5)=GULF(5)+Y36(I)*P(I)
      GULF(6)=GULF(6)-X36(I)*P(I)
      GULF(7)=GULF(7)+Y48(I)*P(I)
      GULF(8)=GULF(8)-X48(I)*P(I)
      GULF(9)=GULF(9)+Y72(I)*P(I)
      GULF(10)=GULF(10)-X72(I)*P(I)
  115 CONTINUE
      RETURN
      END
C
C
C
      SUBROUTINE CLIPER(Y,X,DNOW,SNOW,DM12,SM12,WKTS,MO,KDA,DISP)

C

C Y AND X ARE CURRENT LAT AND LON OF STORM

C DNOW AND SNOW ARE CURRENT (ESTIMATED INSTANTANEOUS) DIRECTION (DEG)

C SPEED OF STORM (KNOTS)

C DM12 AND SM12 ARE DIRECTION AND SPEED OF STORM 12 HOURS EARLIER. THIS

C COULD BE AN AVERAGE BETWEEN -6 AND -18 HRS.

C WKTS IS STORM INTENSITY IN KNOTS

C MO AND KDA ARE INTEGER VALUES OF CURRENT MONTH AND DAY

C

C DISP RETURNS FORECAST DISPLACEMENTS IN NAUTICAL MILES WHERE...

C DISP(01) AND DISP(02) ARE 12 HR MERIDIONAL AND ZONAL DISPLACEMENTS

C DISP(03) AND DISP(04) ARE 24 HR     "       "    "         "

C DISP(05) AND DISP(06) ARE 36 HR     "       "    "         "

C DISP(07) AND DISP(08) ARE 48 HR     "       "    "         "

C DISP(09) AND DISP(10) ARE 60 HR     "       "    "         "

C DISP(11) AND DISP(12) ARE 72 HR     "       "    "         "

C

C  THIS DATASET IS NOW NAMED' NWS.WD80.CJN.SOURCE1(PGM22)

C

      DIMENSION DISP(12),CNS(14,6),CEW(8,6),ND(12),P(14)

C

C LIST CONSTANTS FOR MERIDIONAL DISPLACEMENTS.

C

      DATA CNS/7.60553,13.59909,-2.575127,-.0001868914,.00460007,

     1.0022623,-.001491833,-.0002678624,-.00006994195,.00004407501,

     2-.0002049626,.00007781249,.1430621,-.00008156078,30.30846,

     322.91538,-2.484599,.004968631,.009297729,.02511378,-.007839641,

     4-.005977559,-.0003504201,.0001557468,-.0009956548,.0004778924,

     5.3879478,-.000671019,67.69324,31.94291,-3.697592,.009672852,

     6.009539232,.06322068,-.01332191,-.01610885,-.0007292257,

     7.0002341562,-.002814067,.001145255,.8940798,-.002181595,120.27143,

     838.94701,-4.380877,.01323013,.02292699,.09532129,-.01664333,

     9-.03200883,-.001216522,.000315372,-.005450743,.001867606,1.666658,

     1-.004353084,186.02612,44.48386,-4.72498,.01074,.03200,.13383,

     2-.01607,-.04866,-.00172,.00036,-.00877,.00271,2.76818,-.00733,

     3263.15653,48.41731,-4.456658,.01126704,.04297187,.16962,

     4-.01748416,-.06485724,-.002215515,.0003599106,-.012677,.003690242,

     54.121246,-.01102184/

C

C LIST CONSTANTS FOR ZONAL DISPLACEMENTS.

C

      DATA CEW/-3.52591,13.69309,-2.637347,.8151257,.6867776,

     1-.002168753,-.000596625,.1247267,-13.12388,23.30256,-3.215529,

     23.584506,3.949364,-.007860247,-.006764091,.5135556,-28.48156,

     332.37355,-5.342858,8.073875,9.321241,-.01318171,-.02040824,

     41.044624,-44.13759,38.93667,-6.819777,14.10797,16.35476,

     5-.01967039,-.03853289,1.698018,-55.80913,43.27097,-7.86100,

     621.27143,24.07252,-.02254,-.05992,2.47757,-60.23074,46.26022,

     7-8.8089,29.11625,32.91178,-.02181549,-.08553838,3.291178/

C

      DATA ND/0,31,59,90,120,151,181,212,243,273,304,334/

C

C COMPUTE DAY NUMBER AND SUBTRACT OFF MEANS

C

      DANBR=ND(MO)+KDA-248

C HOLD CONSTANT ALL DAY NBRS BEFORE JUNE 2 AND AFTER NOV 26

      IF(DANBR.LT.-95.)DANBR=-95.

      IF(DANBR.GT.82.)DANBR=82.

      WMPH=(WKTS*1.15)-71.

      ALAT=Y-24.

      ALON=X-68.

C

C CONVERT PRESENT AND PAST MOTION TO U AND V COMPONENTS.

C

      T=.0174533

      UNOW=SIN(DNOW*T)*SNOW

      VNOW=COS(DNOW*T)*SNOW

      UM12=SIN(DM12*T)*SM12

      VM12=COS(DM12*T)*SM12

C

C SET UP MERIDIONAL PREDICTORS AND COMPUTE MERIDIONAL DISPLACEMENTS.

C SOUTHWARD MOTION IS NEGATIVE.

C

      P(1)=1.0

      P(2)=VNOW

      P(3)=VM12

      P(4)=VNOW*VM12*VM12

      P(5)=WMPH*VM12

      P(6)=VNOW*WMPH

      P(7)=VNOW*VNOW*VM12

      P(8)=ALAT*ALAT*VNOW

      P(9)=DANBR*DANBR*VM12

      P(10)=VNOW*DANBR*DANBR

      P(11)=ALAT*ALAT*DANBR

      P(12)=WMPH*DANBR*VM12

      P(13)=UNOW

      P(14)=DANBR*DANBR

      DO 15 I=1,6

      ZZ=0.

      DO 10 J=1,14

   10 ZZ=ZZ+CNS(J,I)*P(J)

   15 DISP(2*I-1)=ZZ

C

C SET UP ZONAL PREDICTORS AND COMPUTE ZONAL DISPLACEMENTS.

C WESTWARD MOTION IS NEGATIVE.

C

      P(2)=UNOW

      P(3)=UM12

      P(4)=ALAT

      P(5)=VNOW

      P(6)=VNOW*VNOW*UM12

      P(7)=ALAT*VNOW*UM12

      P(8)=ALON

      DO 25 I=1,6

      ZZ=0.

      DO 20 J=1,8

   20 ZZ=ZZ+CEW(J,I)*P(J)

   25 DISP(2*I)=ZZ

      RETURN

      END

C
C
C
      BLOCK DATA CLIPER_DATA

C

C  THIS DATASET IS NOW NAMED' NWS.WD80.CJN.SOURCE1(PGM04)

C

C   ALBION D. TAYLOR, MARCH 19, 1982

C  THE HURRICANE GRID IS BASED ON AN OBLIQUE EQUIDISTANT CYLINDRICAL

C  MAP PROJECTION ORIENTED ALONG THE TRACK OF THE HURRICANE.

C

C    THE X (OR I) COORDINATE XI OF A POINT REPRESENTS THE DISTANCE

C  FROM THAT POINT TO THE GREAT CIRCLE THROUGH THE HURRICANE, IN

C  THE DIRECTION OF MOTION OF THE HURRICANE MOTION.  POSITIVE VALUES

C  REPRESENT DISTANCES TO THE RIGHT OF THE HURRICANE MOTION, NEGATIVE

C  VALUES REPRESENT DISTANCES TO THE LEFT.

C    THE Y (OR J) COORDINATE OF THE POINT REPRESENTS THE DISTANCE

C  ALONG THE GREAT CIRCLE THROUGH THE HURRICANE TO THE PROJECTION

C  OF THE POINT ONTO THAT CIRCLE.  POSITIVE VALUES REPRESENT

C  DISTANCE IN THE DIRECTION OF HURRICANE MOTION, NEGATIVE VALUES

C  REPRESENT DISTANCE IN THE OPPOSITE DIRECTION.

C

C     SCALE DISTANCES ARE STRICTLY UNIFORM IN THE I-DIRECTION ALWAYS.

C  THE SAME SCALE HOLDS IN THE J-DIRECTION ONLY ALONG THE HURRICANE TRAC

C  ELSEWHERE, DISTANCES IN THE J-DIRECTION ARE EXAGERATED BY A FACTOR

C  INVERSELY PROPORTIONAL TO THE COSINE OF THE ANGULAR DISTANCE FROM

C  THE TRACK.  THE SCALE IS CORRECT TO 1 PERCENT WITHIN A DISTANCE OF

C  480 NM OF THE STORM TRACK, 5 PERCENT WITHIN 1090 NM, AND

C  10 PERCENT WITHIN 1550 NM.

C

C  BIAS VALUES ARE ADDED TO THE XI AND YJ COORDINATES FOR CONVENIENCE

C  IN INDEXING.

C

C  A PARTICULAR GRID IS SPECIFIED BY THE USER BY MEANS OF A CALL

C  TO SUBROUTINE STHGPR (SET HURRICANE GRID PARAMETERS)

C  WITH ARGUMENTS (XLATH,XLONH,BEAR,GRIDSZ,XIO,YJO)

C   WHERE

C     XLATH,XLONH = LATITUDE, LONGITUDE OF THE HURRICANE

C     BEAR        = BEARING OF THE HURRICANE MOTION

C     GRIDSZ      = SIZE OF GRID ELEMENTS IN NAUTICAL MILES

C     XIO, YJO    = OFFSETS IN I AND J COORDINATES (OR I AND J

C                     COORDINATES OF HURRICANE)

C    AND WHERE

C     LATITUDES, LONGITUDES AND BEARINGS ARE GIVEN IN DEGREES,

C     POSITIVE VALUES ARE NORTH AND WEST, NEGATIVE SOUTH AND EAST,

C     BEARINGS ARE GIVEN CLOCKWISE FROM NORTH.

C

C  THE CALL TO STHGPR SHOULD BE MADE ONCE ONLY, AND BEFORE REFERENCE

C  TO ANY CALL TO LL2XYH OR XY2LLH.  IN DEFAULT, THE SYSTEM

C  WILL ASSUME A STORM AT LAT,LONG=0.,0., BEARING DUE NORTH,

C  WITH A GRIDSIZE OF 120 NAUTICAL MILES AND OFFSETS OF 0.,0. .

C

C  TO CONVERT FROM GRID COORDINATES XI AND YJ, USE A CALL TO

C    CALL XY2LLH(XI,YJ,XLAT,XLONG)

C  THE SUBROUTINE WILL RETURN THE LATITUDE AND LONGITUDE CORRESPONDING

C  TO THE GIVEN VALUES OF XI AND YJ.

C

C  TO CONVERT FROM LATITUDE AND LONGITUDE TO GRID COORDINATES, USE

C    CALL LL2XYH(XLAT,XLONG,XI,YJ)

C  THE SUBROUTINE WILL RETURN THE I-COORDINATE XI AND Y-COORDINATE

C  YJ CORRESPONDING TO THE GIVEN VALUES OF LATITUDE XLAT AND

C  LONGITUDE XLONG.

      COMMON /HGRPRM/ A(3,3),RADPDG,RRTHNM,DGRIDH,HGRIDX,HGRIDY

      DATA A /0.,-1.,0., 1.,0.,0.,  0.,0.,1./

      DATA RADPDG/1.745 3293 E-2/,RRTHNM /3 440.17/

      DATA DGRIDH/120./

      DATA HGRIDX,HGRIDY/0.,0./

      END

C
C
C
      SUBROUTINE STHGPR(XLATH,XXXXX,BEAR,GRIDSZ,XI0,YJ0)

C   ALBION D. TAYLOR, MARCH 19, 1982

      COMMON /HGRPRM/ A(3,3),RADPDG,RRTHNM,DGRIDH,HGRIDX,HGRIDY

C MAKE CERTAIN THAT INCOMING LONGITUDES CONFORM TO PROGRAM CONVENTION

      XLONH=XXXXX

      IF(XLONH.LT.0.0.AND.XLONH.GE.-180.0)GOTO 10

      IF(XLONH.GT.180.0)XLONH=XLONH-360.

      IF(XLONH.LT.-180.0)XLONH=XLONH+360.

   10 CONTINUE

      CLAT=COS(RADPDG*XLATH)

      SLAT=SIN(RADPDG*XLATH)

      SLON=SIN(RADPDG*XLONH)

      CLON=COS(RADPDG*XLONH)

      SBEAR=SIN(RADPDG*BEAR)

      CBEAR=COS(RADPDG*BEAR)

      A(1,1)=   CLAT*SLON

      A(1,2)=   CLAT*CLON

      A(1,3)=   SLAT

      A(2,1)= - CLON*CBEAR + SLAT*SLON*SBEAR

      A(2,2)=   SLON*CBEAR + SLAT*CLON*SBEAR

      A(2,3)=              - CLAT*     SBEAR

      A(3,1)= - CLON*SBEAR - SLAT*SLON*CBEAR

      A(3,2)=   SLON*SBEAR - SLAT*CLON*CBEAR

      A(3,3)=                CLAT*     CBEAR

      DGRIDH=GRIDSZ

      HGRIDX=XI0

      HGRIDY=YJ0

      RETURN

      END

C
C
C
      SUBROUTINE LL2XYH(XLAT,XXXXX,XI,YJ)

C   ALBION D. TAYLOR, MARCH 19, 1982

      COMMON /HGRPRM/ A(3,3),RADPDG,RRTHNM,DGRIDH,HGRIDX,HGRIDY

      DIMENSION ZETA(3),ETA(3)

      XLONG=XXXXX

      IF(XLONG.LT.0.0.AND.XLONG.GE.-180.0)GOTO 10

      IF(XLONG.GT.180.0)XLONG=XLONG-360.

      IF(XLONG.LT.-180.0)XLONG=XLONG+360.

   10 CONTINUE

      CLAT=COS(RADPDG*XLAT)

      SLAT=SIN(RADPDG*XLAT)

      SLON=SIN(RADPDG*XLONG)

      CLON=COS(RADPDG*XLONG)

      ZETA(1)=CLAT*SLON

      ZETA(2)=CLAT*CLON

      ZETA(3)=SLAT

      DO 20 I=1,3

      ETA(I)=0.

      DO 20 J=1,3

      ETA(I)=ETA(I) + A(I,J)*ZETA(J)

   20 CONTINUE

      R=SQRT(ETA(1)*ETA(1) + ETA(3)*ETA(3))

      XI=HGRIDX+RRTHNM*ATAN2(ETA(2),R)/DGRIDH

      IF(R.LE.0.) GO TO 40

      YJ=HGRIDY+RRTHNM*ATAN2(ETA(3),ETA(1))/DGRIDH

      RETURN

   40 YJ=0.

      RETURN

      END

C
C
C
      SUBROUTINE XY2LLH(XI,YJ,XLAT,XLONG)

C   ALBION D. TAYLOR, MARCH 19, 1982

      COMMON /HGRPRM/ A(3,3),RADPDG,RRTHNM,DGRIDH,HGRIDX,HGRIDY

      DIMENSION ZETA(3),ETA(3)

      CXI=COS(DGRIDH*(XI-HGRIDX)/RRTHNM)

      SXI=SIN(DGRIDH*(XI-HGRIDX)/RRTHNM)

      SYJ=SIN(DGRIDH*(YJ-HGRIDY)/RRTHNM)

      CYJ=COS(DGRIDH*(YJ-HGRIDY)/RRTHNM)

      ETA(1)=CXI*CYJ

      ETA(2)=SXI

      ETA(3)=CXI*SYJ

      DO 20 I=1,3

      ZETA(I)=0.

      DO 20 J=1,3

      ZETA(I)=ZETA(I) + A(J,I)*ETA(J)

   20 CONTINUE

      R=SQRT(ZETA(1)*ZETA(1) + ZETA(2)*ZETA(2))

      XLAT=ATAN2(ZETA(3),R)/RADPDG

      IF(R.LE.0.) GO TO 40

      XLONG=ATAN2(ZETA(1),ZETA(2))/RADPDG

      RETURN

   40 XLONG=0.

      RETURN

      END

C
C
C
      SUBROUTINE LL2DB(XLATO,XXXX1,XLATT,XXXX2,DIST,BEAR)

C      ALBION D. TAYLOR MARCH 18, 1981

      DATA RRTHNM/3 440.17/,RADPDG/1.745 3293 E-2/

C   RRTHNM=RADIUS OF EARTH IN NAUT. MILES, RADPDG==OF RADIANS

C   PER DEGREE

C*---------------------------------------------------------------------*

C* GIVEN AN ORIGIN AT LATITUDE, LONGITUDE=XLATO,XLONO, WILL LOCATE     *

C* A TARGET POINT AT LATITUDE, LONGITUDE = XLATT, XLONT.  RETURNS      *

C* DISTANCE DIST IN NAUTICAL MILES, AND BEARING BEAR (DEGREES CLOCKWISE*

C* FROM NORTH).                                                        *

C*                                                                     *

C* ALL LATITUDES ARE IN DEGREES, NORTH POSITIVE AND SOUTH NEGATIVE.    *

C* ALL LONGITUDES ARE IN DEGREES, WEST POSITIVE AND EAST NEGATIVE.     *

C*                                                                     *

C* NOTE-- WHEN ORIGIN IS AT NORTH OR SOUTH POLE, BEARING IS NO LONGER  *

C* MEASURED FROM NORTH.  INSTEAD, BEARING IS MEASURED CLOCKWISE        *

C* FROM THE LONGITUDE OPPOSITE THAT SPECIFIED IN XLONO.                *

C* EXAMPLE-- IF XLATO=90., XLONO=80., THE OPPOSITE LONGITUDE IS -100.  *

C* (100 EAST), AND A TARGET AT BEARING 30. WILL LIE ON THE -70.        *

C* (70 EAST) MERIDIAN                                                  *

C*---------------------------------------------------------------------*

      XLONO=XXXX1

      IF(XLONO.LT.0.0.AND.XLONO.GE.-180.0)GOTO 10

      IF(XLONO.GT.180.0)XLONO=XLONO-360.

      IF(XLONO.LT.-180.0)XLONO=XLONO+360.

   10 CONTINUE

      XLONT=XXXX2

      IF(XLONT.LT.0.0.AND.XLONT.GE.-180.0)GOTO 15

      IF(XLONT.GT.180.0)XLONT=XLONT-360.

      IF(XLONT.LT.-180.0)XLONT=XLONT+360.

   15 CONTINUE

      CLATO=COS(RADPDG*XLATO)

      SLATO=SIN(RADPDG*XLATO)

      CLATT=COS(RADPDG*XLATT)

      SLATT=SIN(RADPDG*XLATT)

      CDLON=COS(RADPDG*(XLONT-XLONO))

      SDLON=SIN(RADPDG*(XLONT-XLONO))

      Z=SLATT*SLATO + CLATT*CLATO*CDLON

      Y= - CLATT*SDLON

      X=CLATO*SLATT - SLATO*CLATT*CDLON

      R=SQRT(X*X+Y*Y)

      DIST=RRTHNM*ATAN2(R,Z)

      IF (R.LE.0.) GO TO 20

      BEAR=ATAN2(-Y,-X)/RADPDG + 180.

      RETURN

   20 BEAR=0.

      RETURN

      END

C     --------------------------------------------------------
      SUBROUTINE DIFTIME(IYZ,IMZ,IDZ,HRZ,IYX,IMX,IDX,HRX,DHR)
C     --------------------------------------------------------
C
      DIMENSION MODA(12)
      DATA MODA/31,28,31,30,31,30,31,31,30,31,30,31/
C
      DHR=0.0
      IDZZ=IDZ
      IMZZ=IMZ
      HRZZ=HRZ
C
      IF(IYZ.EQ.IYX) GOTO 100
      DHR=24.-HRZZ+FLOAT((MODA(IMZZ)-IDZZ)*24+MAX0(IYX-(IYZ+1),0)*8760)
      IMZZ=MOD(IMZZ+1,12)
      IDZZ=1
      HRZZ=0.0
C
      IF (IMZZ.EQ.1) GOTO 100
      DO 10 I=IMZZ,12
10    DHR=DHR+FLOAT(MODA(I)*24)
      IMZZ=1
C
100   IF (IMZZ.EQ.IMX) GOTO 200
      DHR=DHR+FLOAT((MODA(IMZZ)-IDZZ)*24)+24.-HRZZ
      IMZZ1=IMZZ+1
C
      IF (IMZZ1 .GE. IMX) GOTO 120
      IMX1=IMX-1
      DO 110 I=IMZZ1,IMX1
110   DHR=DHR+FLOAT(MODA(I)*24)
C
120   IMZZ=IMZZ1
      IDZZ=1
      HRZZ=0.0
C
200   DHR=DHR+FLOAT((IDX-IDZZ-1)*24)+24.-HRZZ+HRX
      RETURN
      END
C
c
      SUBROUTINE ALAND ( CLON, CLAT, DIST )
C
c  Input: CLON - Longitude (deg W negative)
c         CLAT - Latitude (deg N positive)
c
c  Output: DIST - distance (km) to nearest coastline. DIST is positive if the
c                 point CLON,CLAT is over water and negative if it is over land.
c
c                 This version is valid for tropical cyclones in the Atlantic,
c                 east, and central North Pacific.
c
c                 The represenation of the coastline and islands is in the file
c                 aland.dat
c
C  SUBROUTINE LAND COMPUTES THE DISTANCE (KM) FROM, AND NORMAL BEARING
C  TO, THE NEAREST COASTLINE TO A CYCLONE CENTRAL AT LATITUDE 'CLAT
C  AND LONGITUDE 'CLON'.  COASTLINES ARE STORED IN THE COMMON BLOCK
C  /COAST/ AND FOR UP TO 20 ISLANDS (NUMBER NISL) OF UP TO 30 COASTAL
C  LINE SEGMENTS.  THE INTIIAL POINTS OF EACH SEGMENT ARE STORED
C  IN COUNTERCLOCKWISE ORDER AROUND THE COAST.  THE LAST INDEX IN
C  'XISL' OR 'XCON' IS 1 FOR LONGITUDE AND 2 FOR LATITUDE.
C
C  THE DISTANCE FROM THE CYCLONE TO THE NEAREST POINT ON EACH COASTAL
C  SEGMENT IS CALCULATED, AND THE MINIMUM ABSOLUTE VALUE IS ACCEPTED
C  AS THE COASTAL DISTANCE FOR THAT PARTICULAR LAND MASS.  NEGATIVE
C  VALUES ARE INLAND.  THE MINIMUM ABSOLUTE VALUE OF ALL COASTAL
C  DISTANCES FOR THE CYCLONE IS RETURNED AS THE COASTAL DISTANCE FOR
C  THIS PARTICULAR CYCLONE.
C
C       SEVERAL MODIFICATIONS WERE MADE TO THE SUBROUTINE LAND
C       TO FACILITATE FUTURE CHANGES TO THE DATA FILE
C       USED TO DETERMINE A TROPICAL CYCLONE'S DISTANCE FROM LAND.
C
C       THE VARIABLES NCONOB AND NISLOB SPECIFY THE LIMITS
C       FOR THE NUMBER OF POINTS WHICH DEFINE A CONTINENT AND ISLAND
C       RESPECTIVELY. THE VARIABLE NCOAST IS THE LIMIT FOR THE NUMBER
C       OF ISLANDS. PROVIDED THESE NUMBERS ARE NOT EXCEEDED ANY CHANGES
C       TO THE DATA FILE LAND.DAT SHOULD BE TRANSPARENT TO THE PROGRAM.
C       5/13/92    J.KAPLAN
C
      PARAMETER (NCOAST=100)
      PARAMETER (NCONOB=800)
      PARAMETER (NISLOB=800)
C
C      REAL*8 CISL(NCOAST)
      REAL*4 XISL(NCOAST,NISLOB,2), DI(NCOAST), BI(NCOAST),
     1       XCON(NCONOB,2), DWRC(NCONOB), BWRC(NCONOB),
     2       DWRK(NISLOB), BWRK(NISLOB)
      INTEGER*4 NPT(NCOAST), LT(NCOAST)
      CHARACTER CISL(NCOAST)*8
      LOGICAL*1 FLAG1, LPR
C
      DATA FLAG1/.FALSE./
      DATA LPR  /.FALSE./
      DATA IOPTN /7/
      DATA LUIN  /55/
      save flag1,xisl,xcon,ncon,nisl,npt,lt,cisl
C
      IF( FLAG1 ) GO TO 409
C
C     T H I S   I S   C O A S L I N E   I N P U T **********
C
      OPEN(LUIN,FILE='aland.dat',
     +     FORM='FORMATTED',STATUS='OLD')

  405 CONTINUE
C
C     READ CONTINENT COASTAL POINTS
C     READ TOTAL NUMBER OF CONTINENT POINTS
C
      READ(LUIN,209,END=995) NCON, CISL(1), LT(1)
C
      IF (NCON.GT.NCONOB) THEN
         WRITE(6,*)'NUMBER OF CONTINENT POINTS EXCEEDS PROGRAM LIMIT
     1              OF ',NCONOB
         STOP
      ENDIF
C
      DO 105 I = 1,NCON
         READ(LUIN,207,END=995) XCON(I,1), XCON(I,2)
  207    FORMAT(F7.1,F6.1)
  105 CONTINUE
C
C     COPY INITIAL POINT INTO NCON+1 ARRAY ADDRESS (WILL BE NEEDED
C     TO COMPUTE DISTANCE AND BEARING FROM LAST POINT IN CLOSED LOOP
C     TO 'NEXT' POINT (WHICH IS THE FIRST POINT ENTERED).
      XCON(NCON+1,1) = XCON(1,1)
      XCON(NCON+1,2) = XCON(1,2)
C
C     OUTPUT THE COASTAL POINTS FOR THE CONTINENT
      IF( LPR ) WRITE(6,305) CISL(1), NCON
  305 FORMAT(/,5X,'COASTLINE OF ',A8,/,5X,'IS DEFINED BY THE FOLLOWING '
     1       ,I3,' POINTS')
C
      IF( LPR )WRITE(6,307) ( I, XCON(I,1), XCON(I,2), I=1,NCON )
  307 FORMAT(6(1X,I3,' (',F6.1,F5.1,')'))
C
C     READ ISLAND NAMES AND COASTAL POINTS
C
      NISL = 0
  200 NISL = NISL + 1
      N= NISL + 1
C
      READ(LUIN,209,END=407) NPT(N), CISL(N), LT(N)
  209 FORMAT(I3,1X,A8,1X,I1)
C
      NPTS = NPT(N)
C
      DO 109 I = 1,NPTS
         READ(LUIN,207,END=995) XISL(N,I,1), XISL(N,I,2)
  109 CONTINUE
C
C     COPY INITIAL POINT INTO NPTS+1 ARRAY SPACE (WILL BE NEEDED TO
C     COMPUTE DISTANCE AND BEARING FROM LAST POINT IN CLOSED LOOP TO
C     'NEXT' POINT (WHICH IS THE FIRST POINT ENTERED).
C
      XISL(N,NPTS+1,1) = XISL(N,1,1)
      XISL(N,NPTS+1,2) = XISL(N,1,2)
C
C     OUTPUT THE NAME AND COASTLINE POINTS FOR THIS ISLAND
C
      IF( LPR ) WRITE(6,305) CISL(N), NPT(N)
      NPTS = NPT(N)
      IF( LPR ) WRITE(6,307) (I, XISL(N,I,1), XISL(N,I,2), I=1,NPTS )
      GO TO 200
C
C     COME HERE IF THERE ARE NO MORE ISLANDS TO ENTER...EOF ENCOUNTE
C     ON READ(LUIN,205) AT THE TOP OF THIS INPUT SEGMENT
C
  407 NISL = NISL - 1
      FLAG1 = .TRUE.
      CLOSE(LUIN)
C
C     T H I S   I S   L A N D   N A V I G A T I O N **********
C
  409 CONTINUE
C
C     PROCESS CONTINENT
C
C     COMPUTE ANGLE OF PREVIOUS SEGMENT IF NEEDED FOR BAY OR PENINSULA
C     COASTLINE DETERMINATION FOR SIGN OF DISTANCE IN LSUB1
      CALL LSUB2(XCON(1,1),XCON(NCON,1),XCON(1,2),XCON(NCON,2),DX0,DY0,
     1           AL0)
C     ANGLE OF PREVIOUS SEGMENT IS DIRECTED WITH ORIGIN AT BEGINNING OF
C     CURRENT SEGMENT
      AL0 = ANGL(AL0+180.)
C
      DO 113 I = 1,NCON
C        COMPUTE DISTANCE FROM BEGINNING POINT TO END POINT OF SEGMENT
C        ( COMPONENTS (DX,DY)  AND FROM BEGINNING POINT TO CYCLONE
C        ( COMPONENTS (DXC,DYC) ).
C
         CALL LSUB2(XCON(I+1,1),XCON(I,1),XCON(I+1,2),XCON(I,2), DX, DY,
     1              AL)
         CALL LSUB2(       CLON,XCON(I,1),       CLAT,XCON(I,2),DXC,DYC,
     1              AC)
C
C        CALL DISTANCE, ANGLE CALCULATION SHELL
C
         CALL LSUB1 ( DX, DY, DXC, DYC, AL0,AL,AC, A, DRA, D )
C
         DWRC(I) = D
         BWRC(I) = A
         AL0 = ANGL(AL+180.)
  113 CONTINUE
C
C     COMPUTE MINIMUM DISTANCE FROM CYCLONE TO CONTINENT
      DM = 1000.
      DO 115 I = 1,NCON
         IF( ABS(DWRC(I)).LT.DM ) IM=I
         DM = ABS(DWRC(IM))
  115 CONTINUE
C
C     ASSIGN CONTINENT DISTANCE TO FIRST ADDRESS IN WORKING ARRAY DI, BI
      DI(1) = DWRC(IM)
      BI(1) = BWRC(IM)
C
C
C     OUTER LOOP FOR NUMBER OF ISLANDS NISL
C
      DO 117 N = 2,NISL+1
         NPTS = NPT(N)
C
C        LOOP THROUGH NPT(N) COASTAL SEGMENTS OF NTH ISLAND.
         CALL LSUB2(XISL(N,1,1),XISL(N,NPTS,1),XISL(N,1,2),
     1              XISL(N,NPTS,2),DX0,DY0,AL0)
         AL0 = ANGL(AL0+180.)
C
         DO 119 I = 1,NPTS
C           COMPUTE DISTANCE FROM BEGINNING POINT TO END POINT OF SEGMENT
C           (DX, DY) AND BEGINNING POINT TO CYCLONE (DXC, DYC)
C
            CALL LSUB2(XISL(N,I+1,1),XISL(N,I,1),XISL(N,I+1,2),
     1                 XISL(N,I,2), DX, DY, AL)
            CALL LSUB2(CLON,XISL(N,I,1),CLAT,XISL(N,I,2)
     1                 ,DXC,DYC, AC)
C
            CALL LSUB1 ( DX, DY, DXC, DYC, AL0,AL,AC, A, DRA, D )
            DWRK(I) = D
            BWRK(I) = A
C
            AL0 = ANGL(AL+180.)
  119   CONTINUE
C
        DM = 1000.
        DO 121 I = 1,NPTS
           IF( ABS(DWRK(I)).LT.DM ) IM = I
           DM = ABS(DWRK(IM))
  121   CONTINUE
C
        DI(N) = DWRK(IM)
        BI(N) = BWRK(IM)
  117 CONTINUE
C
      DMIN = 9999.
      DO 116 N=1,NISL+1
         IF( ABS(DI(N)).GE.DMIN ) GO TO 116
         IMN = N
         DMIN= ABS(DI(N))
  116 CONTINUE
C
      DIST = DI(IMN)*111.12
      BRG = BI(IMN)
      RETURN
C
C     ERROR MESSAGES
C
  995 WRITE(6,3995) N, CISL(N)
 3995 FORMAT(///,5X,'END OF FILE WHILE ATTEMPTING TO READ THE COAST POIN
     1TS FOR',/,5X,'ISLAND N=',I2,' WITH NAME ',A8,/,5X,'PROBABLE CAUSE
     2IS MISMATCHED NUMBER OF POINTS SPECIFIED FOR ISLAND COAST AND',/,5
     3X,'ACTUAL NUMBER OF POINTS ON LIST.')
C
      STOP
      END
      SUBROUTINE LSUB1 ( DX, DY, DXC, DYC, AL0,AL,AC, A, DRA, D )
      DR = 180./3.141592
C
C     COMPUTE CYCLONE DISTANCE TO COASTAL SEGMENT
      ACL = ANGL(AC-AL)
      DL = SQRT(DX*DX + DY*DY)
      DC = SQRT(DXC*DXC + DYC*DYC)
      DNC = -DC*SIN(ACL/DR)
      DAC =  DC*COS(ACL/DR)
      DRA =  DAC/DL
C
C     ASSIGN DISTANCE FROM CYCLONE TO COAST
C     IF DISTANCE RATIO IS IN RANGE 0-1, USE NORMAL DISTANCE
C     IF DISTANCE RANGE OUTSIDE ABOVE, USE DISTANCE FROM CYCLONE
C     TO INITIAL POINT OF SEGMENT, WITH SIGN DEPENDING ON THE ANGLE OF
C     THE COAST AT THAT POINT
C
C     DETERMINE DISTANCE AND WHETHER OVER LAND OR WATER
      SGNA = 1.
      IF( ABS(ACL).GE.1.E-5 ) SGNA = ACL/ABS(ACL)
      AC0 = ANGL(AC-AL0)
      ALL0= ANGL(AL0-AL)
C
      DFLAG = 0.
      IF( DNC.LT.0. ) DFLAG = 180.
      AFC = -90. - AC
      AFR = DFLAG- AL
C
      IF( DRA.GT.1. ) GO TO 405
      IF( DRA.LT.0. ) GO TO 407
C
C     DISTANCE IS ALONG NORMAL TO COAST
      D = DNC
      A = AFR
      GO TO 410
C
C     DISTANCE IS APPROXIMATED BY DISTANCE TO CYCLONE, BUT WILL BE
C     DEFINITELY LONGER THAN, AND THEREFORE SUPERSEDED BY, THE
C     DISTANCE FROM THE INITIAL POINT OF THE NEXT COASTAL SEGMENT
  405 D = -DC*SGNA
      A = AFC
      GO TO 410
C
C     DISTANCE IS THAT FROM INITIAL POINT TO CYCLONE.  POINT IS OVER
C     WATER IF BEARING OF VECTOR TO CYCLONE IS LESS THAN BEARING
C     OF PREVIOUS COASTAL SEGMENT, OR IF TO RIGHT OF CURRENT COASTAL
C     SEGMENT
  407 D = -DC*SGNA
      A = AFC
C
C     DISTANCE OFFSHORE/ONSHORE IS NOW SET UP RELATIVE TO CURRENT COASTAL
C     SEGMENT...NEGATIVE IS ONSHORE, OR TO THE LEFT.  CORRECT DEPENDING
C     UPON ORIENTATION OF PREVIOUS COASTAL SEGMENT
C
      IF( D.LE.0. ) GO TO 409
C     CYCLONE IS OFFSHORE (TO RIGHT)
      IF( AC0.LE.0.AND.ALL0.LE.0. ) D = -D
      GO TO 410
C     CYCLONE IS ONSHORE (TO LEFT)
  409 IF( AC0.GT.0.AND.ALL0.GT.0 )  D = -D
C
C     TRANSFORM BEARING FROM THE -180 TO +180 RANGE USED INTERNALLY
C     TO THE 0 TO 360 RANGE FOR DISPLAY PURPOSES.
  410 IF( A.LT.0. ) A = A + 360.
C
      RETURN
      END
      SUBROUTINE LSUB2( X2, X1, Y2, Y1, DX, DY, A )
      DR = 180./3.141592
      DY =  Y2-Y1
      DX = (X2-X1)*COS((Y2+Y1)/(2.*DR))
      A  = 0.
      DD = ABS(DX)+ABS(DY)
      IF( DD.LE.1.E-5 ) RETURN
      A  = ATAN2(DY,DX)*DR
      RETURN
      END
C
      REAL FUNCTION ANGL(A)
      ANGL = A
      IF( A.LE.-180.) ANGL = A + 360.
      IF( A.GT. 180.) ANGL = A - 360.
      RETURN
      END
C
C
C
      subroutine decay(ftime,rlat,rlon,vmax,vmaxa,dt,rcrad,dland,lulg)
C
C     This routine adjusts a tropical cyclone intensity
C     forecast to account for decay over land. this version is
C     valid for the atlantic basin and was written by M. DeMaria
C     and J. Kaplan of the Hurricane Research Division, May 1994.
C
C     Note: M. DeMaria says it's good for the EPAC and CPAC as well.
C
C     This version was modified 4/10/97 (MDM) to include the
C     New England coefficients. The distance inland correction
C     is disabled in this version (idtlc=0).
C
C     New version created 9/30/2004 that allows for decay proportional
C     to the fraction of the storm circulation over land.
C     (Set rcrad > 0.0 to activate this option). The logic of the code
c     was changed so accomodate this option. The old distance inland
c     correction was completely eliminated with this modification.
C
C     ********** INPUT **********
C
C       ftime: the time in hours (for example, 0.,12.,24. ... 72.)
C              The times need to sequential, but the time interval
C              does not need to be even.
C       rlat:  The storm latitude (deg n) at the times in array ftime
C       rlon:  The storm longitude (deg w positive) at the times in
C              array ftime
C       vmax:  The storm maximum wind (kt) at the times in array ftime.
C              Set vmax=0 for missing forecast times.
C         dt:  Interval (hr) for linearly interpolating track
C              positions.
C       lulg:  Unit number for write statements
C
C     ********** OUTPUT **********
C
C       vmaxa: The storm maximum wind (kt) adjusted for decay over land
C              at the times in array ftime.
C
C       dland: The distance (km) from the storm center (rlat,rlon) to
C              the nearest major land mass. dland is negative if the
C              point is storm center is inland.
C
C     ********** METHOD *********
C
C     The simple exponential decay model developed by M. DeMaria
C     and J. Kaplan at HRD is used to decay the storm intensity for
C     the portions of the track over land.
C
C     In this version, the decay rate is proportional to the fraction
C     of the storm circualtion over land.
C
C     ********** PARAMETER SPECIFICATION **********
C
C     Specify the maximum number of time values.
      parameter (imax=11)
C
C     Specify the time interval (hr) for linearly interpolating
C     the track positions.
C      data dt /1.00/
C
C     Set interp=1 to print out (to unit lulg) all intermediate
C     intensity calculations or else set interp=0 for no print
      data interp /0/
C
C     Specify decay model parameters
C
C     Coefficients for east/gulf coast
      data rf1,a1,vb1,rclat1 /0.9,0.095,26.7,36.0/
C
C     Coefficients for New England
      data rf2,a2,vb2,rclat2 /0.9,0.183,29.6,40.0/
C
C     Specify radius of storm circulation (km) for fractional
C     decay option. Set rcrad to zero to eliminate this option.
C
c      data rcrad / 110.0/
c      data rcrad /   0.0/
c
      common /mparm/ alpha,vb,redfac
C
C     ********** DIMENSION ARRAYS **********
C
      dimension ftime(imax),rlat(imax),rlon(imax)
      dimension vmax(imax),vmaxa(imax),dland(imax)
C
c     Arrays for small time step
      parameter (imaxs=1000)
      dimension ftimes(imaxs),rlats(imaxs),rlons(imaxs)
      dimension vmaxs(imaxs),vmaxas(imaxs)
      dimension dlands(imaxs),flands(imaxs)
c
C     ********** MODEL CODE *********
C
C     Write model message to log file
      if (interp .gt. 0) then
         write(lulg,810) rcrad
  810    format(' Decay model with rcrad= ',f5.0,' km')
      endif
C
c     Initialize vmaxa array to zero
      do i=1,imax
      vmaxa(i) = 0.0
      enddo
c
C     Find the number of valid forecast times
      itimet = 0
      do 10 i=1,imax
         if (vmax(i) .lt. 0.5) go to 1000
         itimet=i
   10 continue
C
 1000 continue
C     There must be at least two valid forecast times
      if (itimet .lt. 2) return
C
C     Check to make sure times are sequential
      itime=0
      do 15 i=2,itimet
         if (ftime(i) .le. ftime(i-1)) go to 1100
         itime=i
   15 continue
C
 1100 continue
      if (itime .lt. 2) return
c
      if (interp .gt. 2) then
         do i=1,itime
            write(6,887) ftime(i),rlat(i),rlon(i),vmax(i)
         enddo
      endif
c
c     Calcuate the time values at the small time interval points
      ntimes = 1 + (ftime(itime)-ftime(1))/dt
      do i=1,ntimes
         ftimes(i) = ftime(1) + dt*float(i-1)
      enddo
c
c     Interpolate the input lat,lon and max winds to the
c     small time interval
c
c       ++Find vmax on small time grid
      iflag=1
      lflag=0
      xi = 0.0
c
      call xint(ftime,vmax,itime,iflag,lflag,xi,fi,ierr)
c
      iflag=0
      do i=1,ntimes
         call xint(ftime,vmax,itime,iflag,lflag,
     +                  ftimes(i),vmaxs(i),ierr)
      enddo
c
c       ++Find lat on small time grid
      iflag=1
      call xint(ftime,rlat,itime,iflag,lflag,xi,fi,ierr)
c
      iflag=0
      do i=1,ntimes
         call xint(ftime,rlat,itime,iflag,lflag,
     +                  ftimes(i),rlats(i),ierr)
      enddo
c
c       ++Find lon on small time grid
      iflag=1
      call xint(ftime,rlon,itime,iflag,lflag,xi,fi,ierr)
c
      iflag=0
      do i=1,ntimes
         call xint(ftime,rlon,itime,iflag,lflag,
     +                  ftimes(i),rlons(i),ierr)
      enddo
c
c     Calcuate distance to land and fractional land at small time points
      do i=1,ntimes
         call aland(-rlons(i),rlats(i),dlands(i))
         call fland(-rlons(i),rlats(i),rcrad,flands(i))
      enddo
c
c     Integrate the decay model over the small time points
      do i=1,ntimes
         if (rcrad .gt. 0.0) then
            call fland(-rlons(i),rlats(i),rcrad,flands(i))
         else
       flands(i) = 1.0
         endif
      enddo
c
      vmaxas(1) = vmaxs(1)
c
      do i=2,ntimes
c        At each step in this loop, the decay model is integrated
c        from t=ftimes(i-1) to t=ftimes(i)
c
c        Calculate decay model parameters at current latitude
         rlatt = rlats(i-1)
         if     (rlatt .ge. rclat2) then
             redfac = rf2
             alpha  = a2
             vb     = vb2
         elseif (rlatt .le. rclat1) then
             redfac = rf1
             alpha  = a1
             vb     = vb1
         else
             w1 = (rclat2-rlatt)/(rclat2-rclat1)
             w2 = (rlatt-rclat1)/(rclat2-rclat1)
C
             redfac = w1*rf1 + w2*rf2
             alpha  = w1*a1  + w2*a2
             vb     = w1*vb1 + w2*vb2
         endif
C
        vmaxt1 = vmaxas(i-1)
c
      if (dlands(i) .ge. 0.0) then
c          ++ This is an over-water point
c
c          Check to see if storm just moved over water.
c          If so, adjust for land/ocean surface roughness differences
           if (dlands(i-1) .lt. 0.) then
              vmaxt1 = vmaxt1/redfac
           endif
c
           vmaxt2 = vmaxt1 + (vmaxs(i)-vmaxs(i-1))
      else
c          ++ This is an over-land point
c
c          Check to see if storm just moved over land.
c          If so, adjust for ocean/land surface roughness differences
           if (dlands(i-1) .ge. 0.) then
              vmaxt1 = redfac*vmaxt1
           endif
c
         t      = ftimes(i)-ftimes(i-1)
         fbar   = 0.5*(flands(i)+flands(i-1))
         vmaxt2 = vb + (vmaxt1-vb)* exp(-fbar*alpha*t)
      endif
c
      vmaxas(i) = vmaxt2
      enddo
c
      if (interp .gt. 2) then
         do i=1,ntimes
       write(6,887) ftimes(i),vmaxs(i),vmaxas(i),rlats(i),rlons(i),
     +                   dlands(i),flands(i)
  887       format(8(f6.1,1x))
         enddo
      endif
c
c     Interpolate decay vmaxas back to original forecast times
      iflag=1
c
      call xint(ftimes,vmaxas,ntimes,iflag,lflag,xi,fi,ierr)
c
      iflag=0
      do i=1,itime
         call xint(ftimes,vmaxas,ntimes,iflag,lflag,
     +                  ftime(i),vmaxa(i),ierr)
      enddo
c
c     Interpolate dlands back to original forecast times
      iflag=1
c
      call xint(ftimes,dlands,ntimes,iflag,lflag,xi,fi,ierr)
c
      iflag=0
      do i=1,itime
         call xint(ftimes,dlands,ntimes,iflag,lflag,
     +                  ftime(i),dland(i),ierr)
      enddo
c
      return
      end
      subroutine fland(slon,slat,rkm,fraction)
c     This routine calcuates the fraction of the circular area
c     centered at the point (slat,slon) that is over land.
c
c     Input: rkm - radius of circle in km
c            slat - center latitude of the circle  (deg N pos)
c            slon - center longitude of the circle (deg W neg)
c
c     Output: fraction - the fraction of the circle over land
c
c     Calculate the distance to nearest land at the circle center
      call aland(slon,slat,d00)
c
c     Check for special cases
      if (rkm .le. 0.0) then
         fraction =  1.0
         return
      endif
c
      if (d00 .gt. rkm) then
         fraction = 0.0
         return
      endif
c
      if (d00 .lt. -rkm) then
         fraction = 1.0
         return
      endif
c
c     Perform area integration
      fraction = 0.0
      dx = 25.0
      if (dx .ge. rkm/4.0) dx = rkm/4.0
      dy = dx
c
      pi = 3.14159
      dtr= pi/180.0
      xfac = 111.1*cos(slat*dtr)
      yfac = 111.1
c
      n = ifix(rkm/dx)
c
      ntotal = 0
      nland  = 0
      do j=-n,n
      do i=-n,n
         x = float(i)*dx
         y = float(j)*dy
         r = sqrt(x*x + y*y)
         if (r .gt. rkm) go to 1000
c
         tlat = slat + x/xfac
         tlon = slon + y/yfac
c
         call aland(tlon,tlat,tdtl)
         ntotal = ntotal+1
         if (tdtl .le. 0.0) nland = nland+1
 1000    continue
c
      enddo
      enddo
c
      fraction = float(nland)/float(ntotal)
c
      return
      end

      subroutine xint(x,f,n,iflag,lflag,xi,fi,ierr)
c     This routine applies a quadratic interpolation procedure
c     to f(x) between x(1) and x(n). f(x) is assumed to be
c     represented by quadratic polynomials between the points
c     x(i). The polynomials are chosen so that they equal f(i)
c     at the points x(i), the first derviatives on either
c     side of the interior x(i) match at x(i), and the second
c     derivative of the approximated function integrated
c     over the domain is minimized.
c
c     This version is for interpolating longitude
c
c     Input:  x(1),x(2) ... x(n)      The x values (must be sequential)
c             f(1),f(2) ... f(n)      The function values
c             n                       The number of x,f pairs
c             iflag                   Flag for initialization
c                                      =1 for coefficient calculation
c                                      =0 to use previous coefficients
c             lflag                   Flag for linear interpolation
c                                      =0 to perform linear interpolation
c                                      =1 to perform quadratic interpolation
c             xi                      The x value at which to interpolate f
c
c     Output: fi                      The interpolated function value
c             ierr                    Error flag
c                                      =0  Normal return
c                                      =1  Parameter nmax is too small
c                                      =2  The x values are not sequential
c                                      =3  Coefficient iteration did not
c                                          converge
c                                      =4  Mix-up finding coefficients
c                                      =5  if xi .gt. x(n) or .lt. x(1),
c                                          xi is set to nearest endpoint
c                                          before the interpolation
c
c                                     Note: fi is set to -99.9 if
c                                           ierr=1,2,3 or 4
c
      parameter (nmax=1000)
c
      dimension x(n),f(n)
c
c     Save variables
      dimension ax(nmax),bx(nmax),cx(nmax)
c
c     Temporary local variables
      dimension df(nmax),dx(nmax),gm(nmax),ct(nmax)
c
      common /xsave/ ax,bx,cx
c
c     Specify unit number for debug write statements
c     and debug flag
      idbug  = 0
      lutest = 6
c
c     Initialize error flag
      ierr   = 0
c
c     Specify minimum reduction in cost function for convergence
      thresh = 1.0e-10
c
c     Check to make sure nmax is large enough, and n is .gt. 1
      if (n .gt. nmax .or. n .lt. 2) then
         ierr=1
         fi = -99.9
         return
      endif
c
      if (iflag .eq. 1) then
c        Perform the initialization for later interpolation
c
c        Check to make sure x is sequential
         do 10 i=1,n-1
            if (x(i) .ge. x(i+1)) then
               ierr=2
               fi = -99.9
               return
            endif
   10    continue
c
c        Check for special case where n=2. Only linear interpolation
c        is possible.
         if (n .eq. 2) then
            cx(1) = 0.0
            bx(1) = (f(2)-f(1))/(x(2)-x(1))
            ax(1) = f(1) - bx(1)*x(1)
            go to 1500
         endif
c
c        Calculate x and f differences
         do 15 i=1,n-1
            df(i) = f(i+1)-f(i)
            dx(i) = x(i+1)-x(i)
   15    continue
c
c        Calculate domain size
         d = x(n) - x(1)
c
c        Check for linearity of input points
         eps = 1.0e-10
         bb = (f(2)-f(1))/(x(2)-x(1))
         aa = f(1) - bb*x(1)
         dev = 0.0
         do 12 i=3,n
            dev = dev + abs(aa + bb*x(i) - f(i))
   12    continue
c
         if (dev .lt. eps .or. lflag .eq. 0) then
            do 13 i=1,n-1
               cx(i) = 0.0
   13       continue
            go to 1000
         endif
c
c        Iterate to find the c-coefficients
         cx(1) = 0.0
         nit  = 100
         slt  = 0.01
         cfsave = 1.0e+10
c
         do 20 k=1,nit
c           Calculate c values
            do 25 i=2,n-1
               cx(i) = -cx(i-1)*dx(i-1)/dx(i)
     +                -df(i-1)/(dx(i)*dx(i-1))
     +                +df(i  )/(dx(i)*dx(i  ))
   25       continue
c
c           Calculate current value of cost function
            cf0 = 0.0
            do 26 i=1,n-1
               cf0 = cf0 + cx(i)*cx(i)*dx(i)
   26       continue
            cf0 = 0.5*cf0/d
c
            if (idbug .ne. 0) then
               write(lutest,101) cf0
  101          format(/,' cf0=',e13.6)
            endif
c
c           Check for convergence
            rel = abs(cf0 - cfsave)/abs(cfsave)
            if (rel .lt. thresh) go to 1000
            cfsave = cf0
c
c           Calculate values of Lagrange multipliers
            gm(n-1) = cx(n-1)*dx(n-1)/d
c
            if (n .gt. 3) then
               do 30 i=n-2,2,-1
                  gm(i) = cx(i)*dx(i)/d - gm(i+1)*dx(i)/dx(i+1)
   30          continue
            endif
c
c           Calculate gradient of cost function with respect to c1
            dsdc1 =  dx(1)*(cx(1)/d - gm(2)/dx(2))
c
c           Adjust cx(1) using trial step
            ct(1) = cx(1) - slt*dsdc1
c
c           Calculate remaining c values at trial step
            do 33 i=2,n-1
               ct(i) = -ct(i-1)*dx(i-1)/dx(i)
     +                 -df(i-1)/(dx(i)*dx(i-1))
     +                 +df(i  )/(dx(i)*dx(i  ))
   33       continue
c
c           Calculate cost function at trial step
            cft = 0.0
            do 31 i=1,n-1
               cft = cft + ct(i)*ct(i)*dx(i)
   31       continue
            cft = 0.5*cft/d
c
c            write(6,*) 'dsdc1,cft,cf0',dsdc1,cft,cf0
c           Calculate optimal step length and re-adjust cx(1)
            den = 2.0*((cft-cf0) + slt*dsdc1*dsdc1)
            if (den .ne. 0.0) then
               slo = dsdc1*dsdc1*slt*slt/den
            else
               slo =0.0
            endif
c
c           Adjust slo if desired
            slo = 1.0*slo
c
            cx(1) = cx(1) - slo*dsdc1
c
            if (idbug .ne. 0) then
               write(lutest,100) k,cft,slt,slo
  100          format(' Iteration=',i4,'  cf1=',e11.4,' slt=',e11.4,
     +                                                ' slo=',e11.4)
c
               do 99 j=1,n-1
                  write(lutest,102) j,cx(j)
  102             format('    i=',i2,' c=',f8.4)
   99          continue
            endif
c
c           Calculate trial step for next time step
            slt = 0.5*slo
   20    continue
c
c        Iteration did not converge
         ierr=3
         fi=-99.9
         return
c
c        Iteration converged
 1000    continue
c
         if (idbug .ne. 0) then
            write(lutest,104)
  104       format(/,' Iteration converged')
         endif
c
c        Calculate b and a coefficients
         do 40 i=1,n-1
            bx(i) = df(i)/dx(i) - cx(i)*(x(i+1) + x(i))
            ax(i) = f(i) - bx(i)*x(i) - cx(i)*x(i)*x(i)
   40    continue
      endif
c
 1500 continue
c     Interpolate the function
c
c     Check for xi out of bounds
      if (xi .lt. x(1)) then
         xi = x(1)
         ierr = 5
      endif
c
      if (xi .gt. x(n)) then
         xi = x(n)
         ierr = 5
      endif
c
c     Find the interval for the interpolation
      ii = 1
      do 50 i=2,n
         if (xi .le. x(i)) then
            ii = i-1
            go to 2000
         endif
   50 continue
c
      fi = -99.9
      ierr=4
      return
c
 2000 continue
      fi = ax(ii) + bx(ii)*xi + cx(ii)*xi*xi
c
      return
      end

c
      subroutine uvcomp (dir,spd)
c
c     this subroutine changes dir to u, and spd to v, where dir is
c     given in meteorological degrees.  The original values of dir
c     and spd are destroyed.
c
      degrad = atan(1.0) / 45.
      dirdg = 270.0 - dir
      if (dirdg .lt. 0.0) dirdg = dirdg + 360.
      dirrd = dirdg * degrad
      dir = spd * cos(dirrd)
      spd = spd * sin(dirrd)
      return
      end
c
c
c
      subroutine uvcomp2(dir,spd,u,v)
      degrad=atan(1.)/45.
      dirdg=270.-dir
      if(dirdg.lt.0.)then
        dirdg=dirdg+360.
      endif
      dirrd=dirdg*degrad
      u=spd*cos(dirrd)
      v=spd*sin(dirrd)
      return
      end
c

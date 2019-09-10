       SUBROUTINE CALCAPE(ivirt,itype,T,Q,P,p1d,t1d,q1d,PINT,LMH,
     *    IMM,JMM,LMM,CAPE,CINS,
     *    PLCL,PEQL,slindx)
C
C PROGRAM: CALCAPE
C PROGRAMMER: PERRY C. SHAFRAN
C DATE: 20 AUGUST 1998
C
C PURPOSE: USING THE SUBROUTINE CALCAPE.F WRITTEN FOR THE ETAPOST BY
C RUSS TREADON AS A GUIDE, AND FOLLOWING THE LOGIC BY MIKE BALDWIN
C IN A STAND-ALONE VERSION WRITTEN FOR SAMEX (98-03-25) THE CAPE
C AND CONVECTIVE INHIBITION IS CALCULATED.  SEE SUBROUTINE IN
C ETAPOST FOR MORE DETAILS.
C
C NOTE: UNLIKE THE POST, WHICH CALCULATES TWO DIFFERENT KINDS OF CAPES
C (ITYPE=1, WHERE A PARCEL IS LIFTED FROM THE GROUND, AND ITYPE=2, WHERE
C THE "BEST CAPE" IN A NUMBER OF PARCELS), HERE WE ARE FINDING ONLY
C SURFACE-BASED CAPE, SO ITYPE IS ASSUMED TO EQUAL TO 1.
C
C ALSO UNLIKE POST, THIS VERSION OF CALCAPE IS USED TO CALCULATE THE
C CAPE/CIN FROM OBSERVED RAOB DATA.  THEREFORE, SINCE THE DATA IS NOT
C GRIDDED, THERE IS ONLY ONE HORIZONTAL VARIABLE, WHICH IS THE STATION
C NUMBER, STANUM.
C
C INPUT VARIABLES:
C  T(STANUM,LM) - TEMPERATURE (K)
C  Q(STANUM,LM) - SPECIFIC HUMIDITY (kg/kg)
C  P(STANUM,LM) - PRESSURE (Pa)
C
C OUTPUT VARIABLES:
C  CAPE(STANUM) - CAPE (J/KG)
C  CIN(STANUM) - CIN (J/KG)
C  PLCL(STANUM) - PRESSURE OF LCL (PA)
C  PEQL(STANUM) - PRESSURE OF EQULIBRIUM LEVEL (PA)
C
C PARAMETERS (TAKEN FROM ETAPOST VERSION, CONSTANTS FROM BOLTON (MWR, 1980))
C
C----------------------------------------------------------------------
      PARAMETER (IM=1,JM=1,LM=200,LP1=LM+1)
                             P A R A M E T E R
     & (H10E5=100000.E0
     &, EPSQ=2.E-12
     &, G=9.8E0,CP=1004.6E0,CAPA=0.28589641E0,ROG=287.04/G)
C----------------------------------------------------------------------
                             P A R A M E T E R
     & (ITB=076,JTB=134,ITBQ=152,JTBQ=440)
c     PARAMETER (DPBND=0.E2,
c    & ELIVW=2.72E6,ELOCP=ELIVW/CP)
      parameter (ELIVW=2.72E6,ELOCP=ELIVW/CP)
C     
C     DECLARE VARIABLES.
C     
      INTEGER IEQL(IM,JM)
      INTEGER LCL(IM,JM)
C     
      REAL  P1D(IM,JM),T1D(IM,JM),Q1D(IM,JM)
      REAL  CAPE(IM,JM),CINS(IM,JM)
      REAL  PLCL(IM,JM),PEQL(IM,JM)
      REAL  TPAR(IM,JM,LM)

                             D I M E N S I O N
     & P  (IM,JM,LM),PINT  (IM,JM,LM+1)
     &,T(IM,JM,LM),Q(IM,JM,LM),tv(im,jm,lm)
     &,LMH  (IM,JM)
                             D I M E N S I O N
     & QS0 (JTB),SQS (JTB),THE0  (ITB),STHE  (ITB)
     &,                    THE0Q(ITBQ),STHEQ(ITBQ)
     &,PTBL  (ITB,JTB),TTBL  (JTB,ITB),TTBLQ(JTBQ,ITBQ)
C
                             D I M E N S I O N
     & IPTB  (IM,JM),ITHTB (IM,JM)
     &,PP    (IM,JM)
     &,QQ    (IM,JM)
     &,PSP   (IM,JM),THESP (IM,JM)
     &,ILRES (IM*JM),JLRES (IM*JM),IHRES (IM*JM),JHRES (IM*JM)
     &,APE   (IM,JM,LM)
C
C
      DATA ITABL/0/,THL/210./,PLQ/70000./,PTT/2500./
C
      SAVE PTBL,TTBL,RDQ,RDTH,RDP,RDTHE,PL,THL,QS0,SQS,STHE,THE0,
     &TTBLQ,RDPQ,RDTHEQ,PLQ,STHEQ,THE0Q
C-----------------------------------------------------------------------
C
C   SET UP LOOKUP TABLE
C
      IF (ITABL.EQ.0) THEN
       CALL TABLE1(PTBL,TTBL,PTT,
     &     RDQ,RDTH,RDP,RDTHE,PL,THL,QS0,SQS,STHE,THE0)
       CALL TABLEQ(TTBLQ,RDPQ,RDTHEQ,PLQ,THL,STHEQ,THE0Q)
       ITABL=1
      ENDIF
      IMJM=IM*JM
C-----------------------------------------------------------------------
C     
C
C**************************************************************
C     START CALCAPE HERE.
C     
C
C     COMPUTE CAPE/CINS
C
C        WHICH IS: THE SUM FROM THE LCL TO THE EQ LEVEL OF
C             G * (LN(THETAP) - LN(THETAA)) * DZ
C
C             (POSITIVE AREA FOR CAPE, NEGATIVE FOR CINS)
C
C        WHERE:
C             THETAP IS THE PARCEL THETA
C             THETAA IS THE AMBIENT THETA
C             DZ IS THE THICKNESS OF THE LAYER
C
C         USING LCL AS LEVEL DIRECTLY BELOW SATURATION POINT
C         AND EQ LEVEL IS THE HIGHEST POSITIVELY BUOYANT LEVEL.
C  
C         IEQL = EQ LEVEL
C
C     INITIALIZE CAPE AND CINS ARRAYS
C 
c     ITYPE=1
      if(itype.eq.1) then
        dpbnd=10.E2
      elseif(itype.eq.2) then
        dpbnd=0.
      endif
c     print*,'itype=',itype
      DO J = 1,JM
       DO I = 1,IM
         CAPE(I,J) = 0.0
         CINS(I,J) = 0.0
         THESP(I,J)= 0.0
         IEQL(I,J) = LM+1
C      print*,' DEBUG HS calcape IEQL(I,J): by LM+1: ',IEQL(I,J)
C      print*,' DEBUG HS calcape LCL(I,J) to 0 '
         
         LCL(I,J)=0
       ENDDO
      ENDDO
      DO L=1,LM
       DO J = 1,JM
        DO I = 1,IM
          TPAR(I,J,L)= 0.0
          IF (L.LE.LMH(I,J)) THEN
            APESTS=P(I,J,L)
            APE(I,J,L)=(H10E5/APESTS)**CAPA
            IF(Q(I,J,L).LT.EPSQ)Q(I,J,L)=EPSQ
          ENDIF
        ENDDO
       ENDDO
      ENDDO
c     print*,'after APE array'
C     
C     NOTE THAT FOR TYPE 1 CAPE/CINS ARRAYS P1D, T1D, Q1D 
C     ARE DUMMY ARRAYS.

      if(itype.eq.2) then
       do j=1,jm
       do i=1,im
        q1d(i,j)=amin1(amax1(1.e-12,q1d(i,j)),99999.0)
       enddo
       enddo
      endif
C     
C-------FOR ITYPE=1-----------------------------------------------------
C---------FIND MAXIMUM THETA E LAYER IN LOWEST DPBND ABOVE GROUND-------
C-------FOR ITYPE=2-----------------------------------------------------
C---------FIND THETA E LAYER OF GIVEN T1D, Q1D, P1D---------------------
C--------------TRIAL MAXIMUM BUOYANCY LEVEL VARIABLES-------------------
      DO KB=1,LMM
c      IF (ITYPE.EQ.1.OR.KB.EQ.1) THEN
       IF (ITYPE.EQ.1.OR.(ITYPE.EQ.2.AND.KB.EQ.1)) THEN
        DO J=1,JM
        DO I=1,IM
         LMHK   =LMH(I,J)
         PSFCK  =P(I,J,LMHK)
         PKL = P(I,J,KB)
c        IF (ITYPE.EQ.2.OR.(PKL.GE.PSFCK-DPBND.AND.PKL.LE.PSFCK)) THEN
         IF (ITYPE.EQ.2.OR.
     *      (ITYPE.EQ.1.AND.(PKL.GE.PSFCK-DPBND.AND.PKL.LE.PSFCK)))THEN
          IF (ITYPE.EQ.1) THEN
            TBTK   =T(I,J,KB)
            QBTK   =Q(I,J,KB)
            APEBTK =APE(I,J,KB)
          ELSE
            PKL    =P1D(I,J)
            TBTK   =T1D(I,J)
            QBTK   =Q1D(I,J)
            APEBTK =(H10E5/PKL)**CAPA
          ENDIF
C--------------SCALING POTENTIAL TEMPERATURE & TABLE INDEX--------------
          TTHBTK =TBTK*APEBTK
          TTHK   =(TTHBTK-THL)*RDTH
          QQ  (I,J)=TTHK-AINT(TTHK)
          ITTBK  =INT(TTHK)+1
C--------------KEEPING INDICES WITHIN THE TABLE-------------------------
          IF(ITTBK.LT.1)    THEN
            ITTBK  =1
            QQ  (I,J)=0.0
          ENDIF
          IF(ITTBK.GE.JTB)  THEN
            ITTBK  =JTB-1
            QQ  (I,J)=0.0
          ENDIF
C--------------BASE AND SCALING FACTOR FOR SPEC. HUMIDITY---------------
          BQS00K=QS0(ITTBK)
          SQS00K=SQS(ITTBK)
          BQS10K=QS0(ITTBK+1)
          SQS10K=SQS(ITTBK+1)
C--------------SCALING SPEC. HUMIDITY & TABLE INDEX---------------------
          BQK    =(BQS10K-BQS00K)*QQ(I,J)+BQS00K
          SQK    =(SQS10K-SQS00K)*QQ(I,J)+SQS00K
          TQK    =(QBTK-BQK)/SQK*RDQ
          PP  (I,J)=TQK-AINT(TQK)
          IQ     =INT(TQK)+1
C--------------KEEPING INDICES WITHIN THE TABLE-------------------------
          IF(IQ.LT.1)    THEN
            IQ     =1
            PP  (I,J)=0.0
          ENDIF
          IF(IQ.GE.ITB)  THEN
            IQ     =ITB-1
            PP  (I,J)=0.0
          ENDIF
C--------------SATURATION PRESSURE AT FOUR SURROUNDING TABLE PTS.-------
          IT=ITTBK
          P00K=PTBL(IQ  ,IT  )
          P10K=PTBL(IQ+1,IT  )
          P01K=PTBL(IQ  ,IT+1)
          P11K=PTBL(IQ+1,IT+1)
C--------------SATURATION POINT VARIABLES AT THE BOTTOM-----------------
          TPSPK=P00K+(P10K-P00K)*PP(I,J)+(P01K-P00K)*QQ(I,J)
     2      +(P00K-P10K-P01K+P11K)*PP(I,J)*QQ(I,J)
          APESPK=(H10E5/TPSPK)**CAPA
          TTHESK=TTHBTK*EXP(ELOCP*QBTK*APESPK/TTHBTK)
C--------------CHECK FOR MAXIMUM THETA E--------------------------------
C      print*,' DEBUG HS calcape PTBL(IQ  ,IT  )',PTBL(IQ  ,IT  ),
C     2 PTBL(IQ+1  ,IT  ),PTBL(IQ  ,IT+1  ),PTBL(IQ+1  ,IT+1  )
C      print*,' DEBUG HS calcape TTHESK.GT.THESP(I,J:',TTHESK,THESP(I,J)
          IF(TTHESK.GT.THESP(I,J))    THEN
C      print*,' DEBUG HS calcape Update PSP:',TPSPK,' LMM:',LMM
            PSP  (I,J)=TPSPK
            THESP(I,J)=TTHESK
          ENDIF
         ENDIF
        ENDDO
        ENDDO
       ENDIF
      ENDDO
C-----CHOOSE LAYER DIRECTLY BELOW PSP AS LCL AND------------------------
C-----ENSURE THAT THE LCL IS ABOVE GROUND.------------------------------
C-------(IN SOME RARE CASES FOR ITYPE=2, IT IS NOT)---------------------
        DO L=1,LM
         DO J=1,JM
          DO I=1,IM
            IF (L.LT.LMH(I,J)) THEN
C      print*,' DEBUG HS calcape L.LT.LMH(I,J):',L,LMH(I,J)
             PKL = P(I,J,L)
C      print*,' DEBUG HS calcape L,PKL.LT.PSP(I,J)',L,PKL,PSP(I,J)
             IF (PKL.LT.PSP(I,J)) THEN
              LCL(I,J)=L+1
              PLCL(I,J)=P(I,J,L+1)
             ENDIF
            ENDIF
          ENDDO
         ENDDO
        ENDDO
c      print*,'after LCL array'
C-----------------------------------------------------------------------
C---------FIND TEMP OF PARCEL LIFTED ALONG MOIST ADIABAT (TPAR)---------
C-----------------------------------------------------------------------
      DO 30 IVI=1,LM
        L=LP1-IVI
C--------------SCALING PRESSURE & TT TABLE INDEX------------------------
        KNUML=0
        KNUMH=0
        DO J=1,JM
         DO I=1,IM
          PKL = P(I,J,L)
C      print*,' DEBUG HS calcape L.LE.LCL(I,J):',L,LCL(I,J)
          IF(L.LE.LCL(I,J)) THEN
            IF(PKL.LT.PLQ)THEN
              KNUML=KNUML+1
              ILRES(KNUML)=I
              JLRES(KNUML)=J
            ELSE
              KNUMH=KNUMH+1
              IHRES(KNUMH)=I
              JHRES(KNUMH)=j
            ENDIF
          ENDIF
         ENDDO
        ENDDO
C***
C***  COMPUTE PARCEL TEMPERATURE ALONG MOIST ADIABAT FOR PRESSURE<PLQ
C**
        IF(KNUML.GT.0)THEN
         CALL TTBLEX(TPAR(1,1,L),TTBL,IM,JM,IMJM,ITB,JTB 
     &,             KNUML,ILRES,JLRES
     1,             P(1,1,L),PL,QQ,PP,RDP,THE0,STHE
     2,             RDTHE,THESP,IPTB,ITHTB)
        ENDIF

C***
C***  COMPUTE PARCEL TEMPERATURE ALONG MOIST ADIABAT FOR PRESSURE>PLQ
C**
        IF(KNUMH.GT.0)THEN
         CALL TTBLEX(TPAR(1,1,L),TTBLQ,IM,JM,IMJM,ITBQ,JTBQ
     &,             KNUMH,IHRES,JHRES
     1,             P(1,1,L),PLQ,QQ,PP,RDPQ,THE0Q,STHEQ
     2,             RDTHEQ,THESP,IPTB,ITHTB)
        ENDIF

C------------SEARCH FOR EQ LEVEL----------------------------------------
C       print*,' DEBUG HS calcape SEARCH FOR EQ LEVEL, KNUMH:',KNUMH
       DO N=1,KNUMH
        I=IHRES(N)
        J=JHRES(N)
        IF(TPAR(I,J,L).GT.T(I,J,L)) THEN
              IEQL(I,J)=L
C       print*,' DEBUG HS calcape 111 IEQL(I,J):',IEQL(I,J)
              PEQL(I,J)=P(I,J,L)
        ENDIF
       ENDDO
C       print*,' DEBUG HS calcape SEARCH FOR EQ LEVEL, KNUML:',KNUML
       DO N=1,KNUML
        I=ILRES(N)
        J=JLRES(N)
        IF(TPAR(I,J,L).GT.T(I,J,L)) THEN
              IEQL(I,J)=L
C           print*,' DEBUG HS calcape 222 IEQL(I,J):',IEQL(I,J)
              PEQL(I,J)=P(I,J,L)
        ENDIF
       ENDDO
C-----------------------------------------------------------------------
 30   CONTINUE
C------------COMPUTE CAPE AND CINS--------------------------------------
c       print*,'Get to computing CAPE and CINS'
       DO J=1,JM
        DO I=1,IM
         LCLK=LCL(I,J)
         IEQK=IEQL(I,J)
C       print*,' DEBUG HS calcape  IEQK,LCLK:',IEQK,LCLK
         DO L=IEQK,LCLK
c          print*,'l=',l
c          print*,'p(i,j,l)=',p(i,j,l)
c          print*,'p(i,j,l+1)=',p(i,j,l+1)
c          print*,'t(i,j,l)=',t(i,j,l)
c          print*,'pint(i,j,l)=',pint(i,j,l)
c          print*,'pint(i,j,l+1)=',pint(i,j,l+1)
c          print*,'q(i,j,l)=',q(i,j,l)
           PRESK=P(I,J,L)
           DP=P(I,J,L+1)-P(I,J,L)
c          DP=PINT(I,J,L+1)-PINT(I,J,L)
           DZKL=T(I,J,L)*(Q(I,J,L)*0.608+1.0)*ROG*DP/PRESK
           if(ivirt.eq.0) then
           THETAP=TPAR(I,J,L)*(H10E5/PRESK)**CAPA
           THETAA=T(I,J,L)*(H10E5/PRESK)**CAPA
c          print*,'regular thetap=',thetap
c          print*,'regular thetaa=',thetaa
           elseif(ivirt.eq.1) then
           esatp=fpvsnew(tpar(i,j,l))
           eps=18.015/28.964
           oneps=1.-eps
           qsatp=eps*esatp/(presk-esatp*oneps)
           tvp=tpar(i,j,l)*(1+0.608*qsatp)
           thetap=tvp*(h10e5/presk)**capa
           tv(i,j,l)=t(i,j,l)*(1+0.608*q(i,j,l))
           thetaa=tv(i,j,l)*(h10e5/presk)**capa 
C          print*,'virtual thetap=',thetap
C          print*,'virtual thetaa=',thetaa
           endif
C       print*,' DEBUG HS calcape before CAPE(I,J)=',CAPE(I,J)
           IF (THETAP.LT.THETAA)
     &      CINS(I,J)=CINS(I,J)+G*(ALOG(THETAP)-ALOG(THETAA))*DZKL
           IF (THETAP.GT.THETAA)
     &      CAPE(I,J)=CAPE(I,J)+G*(ALOG(THETAP)-ALOG(THETAA))*DZKL
C           print*,' DEBUG HS calcape  after CAPE(I,J)=',CAPE(I,J)
         ENDDO
        ENDDO
       ENDDO

c      print*,'after computing CAPE'
      
C    
C     ENFORCE LOWER LIMIT OF 0.0 ON CAPE AND UPPER
C     LIMIT OF 0.0 ON CINS.
C
      DO J = 1,JM
      DO I = 1,IM
         CAPE(I,J) = AMAX1(0.0,CAPE(I,J))
         CINS(I,J) = AMIN1(CINS(I,J),0.0)
C      print*,' DEBUG HS calcape final CAPE(I,J)=',CAPE(I,J)
      ENDDO
      ENDDO
c     if(itype.eq.2) print*,'CAPE,CINS=',CAPE,CINS
c
c  Now calculate the lifted index, which is simply the difference 
c  between the ambient temperature and the parcel temperature at 500 mb.
c
      do l=1,lmm
        do j=1,jm
         do i=1,im
          presk=p(i,j,l)
          if(nint(presk).eq.50000) then
             slindx=t(i,j,l)-tpar(i,j,l)
           endif
          enddo
         enddo
      enddo
C     
C     END OF ROUTINE.
C     
      RETURN
      END
      SUBROUTINE TABLE1(PTBL,TTBL,PT
     &,                RDQ,RDTH,RDP,RDTHE,PL,THL,QS0,SQS,STHE,THE0)
C     ******************************************************************
C     *                                                                *
C     *    GENERATE VALUES FOR LOOK-UP TABLES USED IN CONVECTION       *
C     *                                                                *
C     ******************************************************************
                             P A R A M E T E R
     & (ITB=076,JTB=134)
                             P A R A M E T E R
     & (THH=365.,PH=105000.
     &, PQ0=379.90516,A1=610.78,A2=17.2693882,A3=273.16,A4=35.86
     &, R=287.04,CP=1004.6,ELIWV=2.683E6,EPS=1.E-9)
                             D I M E N S I O N
     &  PTBL  (ITB,JTB),TTBL  (JTB,ITB),QSOLD (JTB),POLD  (JTB)
     &, QS0   (JTB),SQS   (JTB),QSNEW (JTB)
     &, Y2P   (JTB),APP   (JTB),AQP   (JTB),PNEW  (JTB)
     &, TOLD  (JTB),THEOLD(JTB),THE0  (ITB),STHE  (ITB)
     &, Y2T   (JTB),THENEW(JTB),APT   (JTB),AQT   (JTB),TNEW  (JTB)
C--------------COARSE LOOK-UP TABLE FOR SATURATION POINT----------------
      KTHM=JTB
      KPM=ITB
      KTHM1=KTHM-1
      KPM1=KPM-1
C
      PL=PT
C
      DTH=(THH-THL)/REAL(KTHM-1)
      DP =(PH -PL )/REAL(KPM -1)
C
      RDTH=1./DTH
      RDP=1./DP
      RDQ=KPM-1
C
      TH=THL-DTH
C-----------------------------------------------------------------------
      DO 500 KTH=1,KTHM
       TH=TH+DTH
       P=PL-DP
       DO 510 KP=1,KPM
        P=P+DP
        APE=(100000./P)**(R/CP)
        QSOLD(KP)=PQ0/P*EXP(A2*(TH-A3*APE)/(TH-A4*APE))
 510    POLD(KP)=P
C      
       QS0K=QSOLD(1)
       SQSK=QSOLD(KPM)-QSOLD(1)
       QSOLD(1  )=0.
       QSOLD(KPM)=1.
C      
       DO 520 KP=2,KPM1
        QSOLD(KP)=(QSOLD(KP)-QS0K)/SQSK
C      
        IF((QSOLD(KP)-QSOLD(KP-1)).LT.EPS) QSOLD(KP)=QSOLD(KP-1)+EPS
C      
 520   CONTINUE
C      
       QS0(KTH)=QS0K
       SQS(KTH)=SQSK
C-----------------------------------------------------------------------
       QSNEW(1  )=0.
       QSNEW(KPM)=1.
       DQS=1./REAL(KPM-1)
C      
       DO 530 KP=2,KPM1
 530    QSNEW(KP)=QSNEW(KP-1)+DQS
C      
       Y2P(1   )=0.
       Y2P(KPM )=0.
C      
       CALL SPLINE(JTB,KPM,QSOLD,POLD,Y2P,KPM,QSNEW,PNEW,APP,AQP)
C      
       DO 540 KP=1,KPM
 540    PTBL(KP,KTH)=PNEW(KP)
C-----------------------------------------------------------------------
 500  CONTINUE
C--------------COARSE LOOK-UP TABLE FOR T(P) FROM CONSTANT THE----------
      P=PL-DP
      DO 550 KP=1,KPM
       P=P+DP
       TH=THL-DTH
       DO 560 KTH=1,KTHM
        TH=TH+DTH
        APE=(100000./P)**(R/CP)
        QS=PQ0/P*EXP(A2*(TH-A3*APE)/(TH-A4*APE))
        TOLD(KTH)=TH/APE
 560    THEOLD(KTH)=TH*EXP(ELIWV*QS/(CP*TOLD(KTH)))
C
       THE0K=THEOLD(1)
       STHEK=THEOLD(KTHM)-THEOLD(1)
       THEOLD(1   )=0.
       THEOLD(KTHM)=1.
C
       DO 570 KTH=2,KTHM1
        THEOLD(KTH)=(THEOLD(KTH)-THE0K)/STHEK
C
        IF((THEOLD(KTH)-THEOLD(KTH-1)).LT.EPS)
     1      THEOLD(KTH)=THEOLD(KTH-1)  +  EPS
C
 570   CONTINUE
C
       THE0(KP)=THE0K
       STHE(KP)=STHEK
C-----------------------------------------------------------------------
       THENEW(1  )=0.
       THENEW(KTHM)=1.
       DTHE=1./REAL(KTHM-1)
       RDTHE=1./DTHE
C      
       DO 580 KTH=2,KTHM1
 580    THENEW(KTH)=THENEW(KTH-1)+DTHE
C      
       Y2T(1   )=0.
       Y2T(KTHM)=0.
C      
       CALL SPLINE(JTB,KTHM,THEOLD,TOLD,Y2T,KTHM,THENEW,TNEW,APT,AQT)
C      
       DO 590 KTH=1,KTHM
 590    TTBL(KTH,KP)=TNEW(KTH)
C-----------------------------------------------------------------------
 550  CONTINUE
C
      RETURN
      END
C&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
      SUBROUTINE TABLEQ(TTBLQ
     &,                 RDP,RDTHE,PL,THL,STHE,THE0)
C     ******************************************************************
C    *                                                                *
C     *        GENERATE VALUES FOR FINER LOOK-UP TABLES USED           *
C     *                       IN CONVECTION                            *
C     *                                                                *
C     ******************************************************************
                             P A R A M E T E R
     & (ITB=152,JTB=440)
                             P A R A M E T E R
     & (THH=325.,PH=105000.
     &, PQ0=379.90516,A1=610.78,A2=17.2693882,A3=273.16,A4=35.86
     &, R=287.04,CP=1004.6,ELIWV=2.683E6,EPS=1.E-9)
                             D I M E N S I O N
     &  TTBLQ (JTB,ITB)
     &, TOLD  (JTB),THEOLD(JTB),THE0  (ITB),STHE  (ITB)
     &, Y2T   (JTB),THENEW(JTB),APT   (JTB),AQT   (JTB),TNEW  (JTB)
C--------------COARSE LOOK-UP TABLE FOR SATURATION POINT----------------
      KTHM=JTB
      KPM=ITB
      KTHM1=KTHM-1
      KPM1=KPM-1
C
      DTH=(THH-THL)/REAL(KTHM-1)
      DP =(PH -PL )/REAL(KPM -1)
C
      RDP=1./DP
      TH=THL-DTH
C--------------COARSE LOOK-UP TABLE FOR T(P) FROM CONSTANT THE----------
      P=PL-DP
              DO 550 KP=1,KPM
          P=P+DP
          TH=THL-DTH
          DO 560 KTH=1,KTHM
      TH=TH+DTH
      APE=(100000./P)**(R/CP)
      QS=PQ0/P*EXP(A2*(TH-A3*APE)/(TH-A4*APE))
      TOLD(KTH)=TH/APE
      THEOLD(KTH)=TH*EXP(ELIWV*QS/(CP*TOLD(KTH)))
 560  CONTINUE
C
      THE0K=THEOLD(1)
      STHEK=THEOLD(KTHM)-THEOLD(1)
      THEOLD(1   )=0.
      THEOLD(KTHM)=1.
C
          DO 570 KTH=2,KTHM1
      THEOLD(KTH)=(THEOLD(KTH)-THE0K)/STHEK
C
      IF((THEOLD(KTH)-THEOLD(KTH-1)).LT.EPS)
     1    THEOLD(KTH)=THEOLD(KTH-1)  +  EPS
C
 570  CONTINUE
C
      THE0(KP)=THE0K
      STHE(KP)=STHEK
C-----------------------------------------------------------------------
      THENEW(1  )=0.
      THENEW(KTHM)=1.
      DTHE=1./REAL(KTHM-1)
      RDTHE=1./DTHE
C
      DO 580 KTH=2,KTHM1
       THENEW(KTH)=THENEW(KTH-1)+DTHE
 580  CONTINUE
C
      Y2T(1   )=0.
      Y2T(KTHM)=0.
C
      CALL SPLINE(JTB,KTHM,THEOLD,TOLD,Y2T,KTHM,THENEW,TNEW,APT,AQT)
C
      DO 590 KTH=1,KTHM
       TTBLQ(KTH,KP)=TNEW(KTH)
 590  CONTINUE
C-----------------------------------------------------------------------
 550  CONTINUE
C
      RETURN
      END
      SUBROUTINE TTBLEX(TREF,TTBL,IMM,JMM,IMJM,ITB,JTB
     &,                 KNUM,IARR,JARR
     1,                 PIJL,PL,QQ,PP,RDP,THE0
     2,                 STHE,RDTHE,THESP,IPTB,ITHTB)
C     ******************************************************************
C     *                                                                *
C     *         EXTRACT TEMPERATURE OF THE MOIST ADIABAT FROM          *
C     *                    THE APPROPRIATE TTBL                        *
C     *                                                                *
C     ******************************************************************
C----------------------------------------------------------------------
      PARAMETER(IM=1,JM=1,LM=50,IMXJM=IM*JM)
                             D I M E N S I O N
     1 TREF(IM,JM),TTBL(JTB,ITB),IARR(IMXJM),JARR(IMXJM)
     2,PIJL(IM,JM),QQ(IM,JM),PP(IM,JM),THE0(ITB)
     3,STHE(ITB),THESP(IM,JM),IPTB(IM,JM),ITHTB(IM,JM)
C-----------------------------------------------------------------------
      DO 500 KK=1,KNUM
C--------------SCALING PRESSURE & TT TABLE INDEX------------------------
      I=IARR(KK)
      J=JARR(KK)
      PK=PIJL(I,J)
      TPK=(PK-PL)*RDP
      QQ(I,J)=TPK-AINT(TPK)
      IPTB(I,J)=INT(TPK)+1
C--------------KEEPING INDICES WITHIN THE TABLE-------------------------
      IF(IPTB(I,J).LT.1)THEN
        IPTB(I,J)=1
        QQ(I,J)=0.
      ENDIF
      IF(IPTB(I,J).GE.ITB)THEN
        IPTB(I,J)=ITB-1
        QQ(I,J)=0.
      ENDIF
C--------------BASE AND SCALING FACTOR FOR THE--------------------------
      IPTBK=IPTB(I,J)
      BTHE00K=THE0(IPTBK)
      STHE00K=STHE(IPTBK)
      BTHE10K=THE0(IPTBK+1)
      STHE10K=STHE(IPTBK+1)
C--------------SCALING THE & TT TABLE INDEX-----------------------------
      BTHK=(BTHE10K-BTHE00K)*QQ(I,J)+BTHE00K
      STHK=(STHE10K-STHE00K)*QQ(I,J)+STHE00K
      TTHK=(THESP(I,J)-BTHK)/STHK*RDTHE
      PP(I,J)=TTHK-AINT(TTHK)
      ITHTB(I,J)=INT(TTHK)+1
C--------------KEEPING INDICES WITHIN THE TABLE-------------------------
      IF(ITHTB(I,J).LT.1)THEN
        ITHTB(I,J)=1
        PP(I,J)=0.
      ENDIF
      IF(ITHTB(I,J).GE.JTB)THEN
        ITHTB(I,J)=JTB-1
        PP(I,J)=0.
      ENDIF
C--------------TEMPERATURE AT FOUR SURROUNDING TT TABLE PTS.------------
      ITH=ITHTB(I,J)
      IP=IPTB(I,J)
      T00K=TTBL(ITH  ,IP  )
      T10K=TTBL(ITH+1,IP  )
      T01K=TTBL(ITH  ,IP+1)
      T11K=TTBL(ITH+1,IP+1)
C--------------PARCEL TEMPERATURE-------------------------------------
      TREF(I,J)=(T00K+(T10K-T00K)*PP(I,J)+(T01K-T00K)*QQ(I,J)
     2         +(T00K-T10K-T01K+T11K)*PP(I,J)*QQ(I,J))
  500 CONTINUE
C
      RETURN
      END
C&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
C&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
      SUBROUTINE SPLINE(JTB,NOLD,XOLD,YOLD,Y2,NNEW,XNEW,YNEW,P,Q)
C     ******************************************************************
C     *                                                                *
C     *  THIS IS A ONE-DIMENSIONAL CUBIC SPLINE FITTING ROUTINE        *
C     *  PROGRAMED FOR A SMALL SCALAR MACHINE.                         *
C     *                                                                *
C     *  PROGRAMER Z. JANJIC, YUGOSLAV FED. HYDROMET. INST., BEOGRAD  *
C     *                                                                *
C     *                                                                *
C     *                                                                *
C     *  NOLD - NUMBER OF GIVEN VALUES OF THE FUNCTION.  MUST BE GE 3. *
C     *  XOLD - LOCATIONS OF THE POINTS AT WHICH THE VALUES OF THE     *
C     *         FUNCTION ARE GIVEN.  MUST BE IN ASCENDING ORDER.       *
C     *  YOLD - THE GIVEN VALUES OF THE FUNCTION AT THE POINTS XOLD.   *
C     *  Y2   - THE SECOND DERIVATIVES AT THE POINTS XOLD.  IF NATURAL *
C     *         SPLINE IS FITTED Y2(1)=0. AND Y2(NOLD)=0. MUST BE      *
C     *         SPECIFIED.                                             *
C     *  NNEW - NUMBER OF VALUES OF THE FUNCTION TO BE CALCULATED.     *
C     *  XNEW - LOCATIONS OF THE POINTS AT WHICH THE VALUES OF THE     *
C     *         FUNCTION ARE CALCULATED.  XNEW(K) MUST BE GE XOLD(1)   *
C     *         AND LE XOLD(NOLD).                                     *
C     *  YNEW - THE VALUES OF THE FUNCTION TO BE CALCULATED.           *
C     *  P, Q - AUXILIARY VECTORS OF THE LENGTH NOLD-2.                *
C     *                                                                *
C     ******************************************************************
                             D I M E N S I O N
     & XOLD(JTB),YOLD(JTB),Y2(JTB),P(JTB),Q(JTB)
     &,XNEW(JTB),YNEW(JTB)
C-----------------------------------------------------------------------
      NOLDM1=NOLD-1
C
      DXL=XOLD(2)-XOLD(1)
      DXR=XOLD(3)-XOLD(2)
      DYDXL=(YOLD(2)-YOLD(1))/DXL
      DYDXR=(YOLD(3)-YOLD(2))/DXR
      RTDXC=.5/(DXL+DXR)
C
      P(1)= RTDXC*(6.*(DYDXR-DYDXL)-DXL*Y2(1))
      Q(1)=-RTDXC*DXR
C
      IF(NOLD.EQ.3) GO TO 700
C-----------------------------------------------------------------------
      K=3
C
 100  DXL=DXR
      DYDXL=DYDXR
      DXR=XOLD(K+1)-XOLD(K)
      DYDXR=(YOLD(K+1)-YOLD(K))/DXR
      DXC=DXL+DXR
      DEN=1./(DXL*Q(K-2)+DXC+DXC)
C
      P(K-1)= DEN*(6.*(DYDXR-DYDXL)-DXL*P(K-2))
      Q(K-1)=-DEN*DXR
C
      K=K+1
      IF(K.LT.NOLD) GO TO 100
C-----------------------------------------------------------------------
 700  K=NOLDM1
C
 200  Y2(K)=P(K-1)+Q(K-1)*Y2(K+1)
C
      K=K-1
      IF(K.GT.1) GO TO 200
C-----------------------------------------------------------------------
      K1=1
C
 300  XK=XNEW(K1)
C
      DO 400 K2=2,NOLD
      IF(XOLD(K2).LE.XK) GO TO 400
      KOLD=K2-1
      GO TO 450
 400  CONTINUE
      YNEW(K1)=YOLD(NOLD)
      GO TO 600
C
 450  IF(K1.EQ.1)   GO TO 500
      IF(K.EQ.KOLD) GO TO 550
C
 500  K=KOLD
C
      Y2K=Y2(K)
      Y2KP1=Y2(K+1)
      DX=XOLD(K+1)-XOLD(K)
      RDX=1./DX
C
CVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
C     WRITE(6,5000) K,Y2K,Y2KP1,DX,RDX,YOLD(K),YOLD(K+1)
C5000 FORMAT(' K=',I4,' Y2K=',E12.4,' Y2KP1=',E12.4,' DX=',E12.4,' RDX='
C    2,E12.4,' YOK=',E12.4,' YOP1=',E12.4)
CAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
      AK=.1666667*RDX*(Y2KP1-Y2K)
      BK=.5*Y2K
      CK=RDX*(YOLD(K+1)-YOLD(K))-.1666667*DX*(Y2KP1+Y2K+Y2K)
C
 550  X=XK-XOLD(K)
      XSQ=X*X
C
      YNEW(K1)=AK*XSQ*X+BK*XSQ+CK*X+YOLD(K)
C
 600  K1=K1+1
      IF(K1.LE.NNEW) GO TO 300
C-----------------------------------------------------------------------
C      print*,' DEBUG HS calcape NOLD,NNEW: ',NOLD,NNEW
CC      print*,' DEBUG HS calcape XOLD: ',XOLD
CC      print*,' DEBUG HS calcape YOLD: ',YOLD
C      print*,' DEBUG HS calcape XOLD(1)->: XNEW(1)',XOLD(1),XNEW(1)
C      print*,' DEBUG HS calcape XOLD(2)->: XNEW(2)',XOLD(2),XNEW(2)
CC      print*,' DEBUG HS calcape XNEW: ',XNEW
CC      print*,' DEBUG HS calcape YNEW: ',YNEW
C      print*,' DEBUG HS calcape YOLD(1)->: YNEW(1)',YOLD(1),YNEW(1)
C      print*,' DEBUG HS calcape YOLD(2)->: YNEW(2)',YOLD(2),YNEW(2)
C      print*,' DEBUG HS calcape YOLD(3)->: YNEW(3)',YOLD(3),YNEW(3)
C      print*,' DEBUG HS calcape YOLD(-2)->: YNEW(-2)',
C     2 YOLD(NOLD-1),YNEW(NNEW-1)
C      print*,' DEBUG HS calcape YOLD(-1)->: YNEW(-1)',
C     2 YOLD(NOLD),YNEW(NNEW)
      RETURN
      END

      function fpvsnew(t)

      integer nxpvs
      real con_ttp,con_psat,con_cvap,con_cliq,con_hvap,con_rv,con_csol,con_hfus,
     *     tliq,tice,dldtl,heatl,xponal,xponbl,dldti,heati,xponai,xponbi

      real tr,w,pvl,pvi
      real fpvsnew
      real t
      integer jx
      real xj,x,tbpvs(7501),xp1
      real xmin,xmax,xinc,c2xpvs,c1xpvs

      nxpvs=7501
      con_ttp=2.7316e+2
      con_psat=6.1078e+2
      con_cvap=1.8460e+3
      con_cliq=4.1855e+3
      con_hvap=2.5000e+6
      con_rv=4.6150e+2
      con_csol=2.1060e+3
      con_hfus=3.3358e+5

      tliq=con_ttp
      tice=con_ttp-20.0
      dldtl=con_cvap-con_cliq
      heatl=con_hvap
      xponal=-dldtl/con_rv
      xponbl=-dldtl/con_rv+heatl/(con_rv*con_ttp)
      dldti=con_cvap-con_csol
      heati=con_hvap+con_hfus
      xponai=-dldti/con_rv
      xponbi=-dldti/con_rv+heati/(con_rv*con_ttp)

      xmin=180.0
      xmax=330.0
      xinc=(xmax-xmin)/(nxpvs-1)
c   c1xpvs=1.-xmin/xinc
      c2xpvs=1./xinc
      c1xpvs=1.-xmin*c2xpvs
c    xj=min(max(c1xpvs+c2xpvs*t,1.0),real(nxpvs,krealfp))
      xj=min(max(c1xpvs+c2xpvs*t,1.0),float(nxpvs))
      jx=min(xj,float(nxpvs)-1.0)
      x=xmin+(jx-1)*xinc

      tr=con_ttp/x
      if(x.ge.tliq) then
        tbpvs(jx)=con_psat*(tr**xponal)*exp(xponbl*(1.-tr))
      elseif(x.lt.tice) then
        tbpvs(jx)=con_psat*(tr**xponai)*exp(xponbi*(1.-tr))
      else
        w=(t-tice)/(tliq-tice)
        pvl=con_psat*(tr**xponal)*exp(xponbl*(1.-tr))
        pvi=con_psat*(tr**xponai)*exp(xponbi*(1.-tr))
        tbpvs(jx)=w*pvl+(1.-w)*pvi
      endif

      xp1=xmin+(jx-1+1)*xinc


      tr=con_ttp/xp1
      if(xp1.ge.tliq) then
        tbpvs(jx+1)=con_psat*(tr**xponal)*exp(xponbl*(1.-tr))
      elseif(xp1.lt.tice) then
        tbpvs(jx+1)=con_psat*(tr**xponai)*exp(xponbi*(1.-tr))
      else
        w=(t-tice)/(tliq-tice)
        pvl=con_psat*(tr**xponal)*exp(xponbl*(1.-tr))
        pvi=con_psat*(tr**xponai)*exp(xponbi*(1.-tr))
        tbpvs(jx+1)=w*pvl+(1.-w)*pvi
      endif

      fpvsnew=tbpvs(jx)+(xj-jx)*(tbpvs(jx+1)-tbpvs(jx))
      end 

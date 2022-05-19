      subroutine acerr(btlat,btlon,flat,flon,npts,
     +                         crosse,alonge,ierr)
c
c     This routine calculates the along and cross track errors
c     of a track forecast.
c
c     Input: btlat(npts)  - best track latitude points (deg N)
c            btlon(npts)  - best track longitude points (deg E positive,
c                                                        deg W negative)
c            flat(npts)   - forecast latitude points (deg N)
c            flon(npts)   - forecast longitude points (deg E)
c            npts         - The number of lat/lon points (times)
c
c     Output: crosse(npts)- The cross track component of the error (km)
c             alonge(npts)- The along track component of the error (km)
c             ierr        - Error flag
c                           =0 for normal return
c                           >0 for error
c
c     Notes: The btlat, btlon, flat, and flon arrays can have missing
c            values (use -9999. for missing). The missing values
c            are replaced by linearly interpolated values. Valid
c            lat/lons must be available for at least two time periods.
c
c            It is assumed that btlat,btlon and flat,flon are in
c            arrays with equally spaced times. For example, btlat(1)
c            is at t=0, btlat(2) is at t=12 hr, btlat(3) is at t=24 hr
c            etc. The time interval does not need to be 12 hr.
c
c            The along and cross track errors will be calculated for
c            up to the last time that a valid btlat,btlon,flat,flon
c            are all available.
c
c            The along track error is positive if it is in the same
c            direction as the best track motion.
c
c            The cross track error is positive if it is to the right
c            of the best track motion.
c
c            The motion vector for error at time=n is calculated using
c            the positions btlat(n-1),bton(n-1) and
c            btlat(n),btlon(n), except when n=1.

c            If the motion vector has zero magnitude at a particular time,
c            the motion vector from the previously available time
c            is used instead.
c
c     Passed variables
      dimension btlat(npts),btlon(npts)
      dimension flat(npts),flon(npts)
      dimension alonge(npts),crosse(npts)
c
c     Local variables
      parameter (mxp=1000)
      dimension blat(mxp),blon(mxp),plat(mxp),plon(mxp)
      dimension head(mxp)
c
c     Set debug flag (idbug=0 for no debug writes)
      idbug=0
      lulog=6
c
c     Specify value for missing lat/lons
      rmiss=-9999.0
c
c     Check working array size
      if (npts .gt. mxp) then
         ierr=1
         return
      endif
c
c     Copy best track and forecast lat/lon to temporary arrays
c     and initialize errors to zero
      do k=1,npts
         blat(k) = btlat(k)
         blon(k) = btlon(k)
         plat(k) = flat(k)
         plon(k) = flon(k)
c
      crosse(k) = 0.0
      alonge(k) = 0.0
      enddo
c
c     Find the first and last point with a valid lat/lon
      nfirst=0
      nlast = 0
      nsum  = 0
      do k=1,npts
         if (blat(k) .gt. rmiss .and.
     +       blon(k) .gt. rmiss .and.
     +       plat(k) .gt. rmiss .and.
     +       plon(k) .gt. rmiss       ) then
c
             nlast=k
        nsum=nsum+1
c
        if (nfirst .eq. 0) nfirst=k
         endif
      enddo
c
c     Check to make sure at least 2 valid points are available
      if (nsum .lt. 2) then
      ierr=2
      return
      endif
c
      if (nlast .gt. nfirst+1) then
c        Check for missing lat/lons and fill them in
c        using linear interpolation
c
         do 10 k=nfirst+1,nlast-1
            if (blat(k) .le. rmiss .or. blon(k) .le. rmiss) then
c              A best track point is missing. Fill in with interpolated value.
c
c              Find first good blat before current point
               km = 0
          do kk=k-1,nfirst,-1
        if (blat(kk) .gt. rmiss) then
                     km = kk
           go to 1000
                  endif
               enddo
 1000          continue
c
c              Find first good blat after current point
               kp = 0
          do kk=k+1,nlast
        if (blat(kk) .gt. rmiss) then
                     kp = kk
           go to 1001
                  endif
               enddo
 1001          continue
c
          if (km .eq. 0 .or. kp .eq. 0) go to 910
c
          rk = float(k)
          rm = float(km)
          rp = float(kp)
c
c              Perform interpolation
          wm = (rp-rk)/(rp-rm)
          wp = (rk-rm)/(rp-rm)
          blat(k) = wm*blat(km) + wp*blat(kp)
c
c              Find first good blon before current point
               km = 0
          do kk=k-1,nfirst,-1
        if (blon(kk) .gt. rmiss) then
                     km = kk
           go to 1002
                  endif
               enddo
 1002          continue
c
c              Find first good blon after current point
               kp = 0
          do kk=k+1,nlast
        if (blon(kk) .gt. rmiss) then
                     kp = kk
           go to 1003
                  endif
               enddo
 1003          continue
c
          if (km .eq. 0 .or. kp .eq. 0) go to 910
c
          rk = float(k)
          rm = float(km)
          rp = float(kp)
c
c              Perform interpolation
          wm = (rp-rk)/(rp-rm)
          wp = (rk-rm)/(rp-rm)
          blon(k) = wm*blon(km) + wp*blon(kp)
c
       endif
c
            if (plat(k) .le. rmiss .or. plon(k) .le. rmiss) then
c              A forecast track point is missing. Fill in with interpolated value.
c
c              Find first good plat before current point
               km = 0
          do kk=k-1,nfirst,-1
        if (plat(kk) .gt. rmiss) then
                     km = kk
           go to 2000
                  endif
               enddo
 2000          continue
c
c              Find first good plat after current point
               kp = 0
          do kk=k+1,nlast
        if (plat(kk) .gt. rmiss) then
                     kp = kk
           go to 2001
                  endif
               enddo
 2001          continue
c
          if (km .eq. 0 .or. kp .eq. 0) go to 910
c
          rk = float(k)
          rm = float(km)
          rp = float(kp)
c
c              Perform interpolation
          wm = (rp-rk)/(rp-rm)
          wp = (rk-rm)/(rp-rm)
          plat(k) = wm*plat(km) + wp*plat(kp)
c
c              Find first good plon before current point
               km = 0
          do kk=k-1,nfirst,-1
        if (plon(kk) .gt. rmiss) then
                     km = kk
           go to 2002
                  endif
               enddo
 2002          continue
c
c              Find first good plon after current point
               kp = 0
          do kk=k+1,nlast
        if (plon(kk) .gt. rmiss) then
                     kp = kk
           go to 2003
                  endif
               enddo
 2003          continue
c
          if (km .eq. 0 .or. kp .eq. 0) go to 910
c
          rk = float(k)
          rm = float(km)
          rp = float(kp)
c
c              Perform interpolation
          wm = (rp-rk)/(rp-rm)
          wp = (rk-rm)/(rp-rm)
          plon(k) = wm*plon(km) + wp*plon(kp)
       endif
c
   10    continue
      endif
c
c     Calculate the storm heading using the best track
      call sthcal(blat,blon,mxp,nfirst,nlast,head)
c
c     Calculate the cross and along track errors
      dtr = 3.14159265/180.0
      do n=nfirst,nlast
      rlat2 = plat(n)
      rlon2 = plon(n)
      rlat1 = blat(n)
      rlon1 = blon(n)
c
c        Calculate eastward,northward components of the error
         call distk(rlon1,rlat1,rlon2,rlat2,dx,dy,rad)
c
c        Rotate error into cross and along track components
         crosse(n) = dx*sin(dtr*head(n)) - dy*cos(dtr*head(n))
         alonge(n) = dy*sin(dtr*head(n)) + dx*cos(dtr*head(n))
      enddo
c
      if (idbug .ne. 0) then
      write(lulog,300) nfirst,nlast,nsum
  300    format(/,'nfirst,nlast,nsum: ',i3,1x,i3,1x,i3,/)
c
         write(lulog,310)
  310    format(' i time   btlat    btlon     flat     flon',
     +                 '    blat     blon     plat     plon',
     +                 '  head crosse alonge')
c
         do i=1,npts
       itime = 12*(i-1)
            write(lulog,320) i,itime,btlat(i),btlon(i),flat(i),flon(i),
     +                               blat(i),blon(i),plat(i),plon(i),
     +                               head(i),crosse(i),alonge(i)
  320       format( i2,1x,i4,2(1x,f7.2,1x,f8.2,2x,f7.2,1x,f8.2),
     +             f6.1,1x,f6.1,1x,f6.1)
         enddo
      endif
c
      ierr=0
      return
c
  910 continue
      ierr=3
      return
c
      end      
      subroutine sthcal(blat,blon,mxp,nfirst,nlast,head)
c     This routine calculates the storm heading measured CCW
c     in degrees CCW relative to the +x axis (east). It is assumed
c     that blat and blon are available for all points from n=nfirst
c     to n=nlast.
c
      dimension blat(mxp),blon(mxp),head(mxp)
c
      do n=nfirst+1,nlast
      rlat2 = blat(n  )
      rlat1 = blat(n-1)
      rlon2 = blon(n  )
      rlon1 = blon(n-1)
c
         call distk(rlon1,rlat1,rlon2,rlat2,dx,dy,rad)
c
      if (rad .le. 0.0) then
       head(n) = -9999.
         else
       call ctor(dx,dy,rad,head(n))
         endif
      enddo
      head(nfirst) = head(nfirst+1)
c
c     Check to see how many headings are missing
      nmiss = 0
      ntot  = 1 + nlast-nfirst
      do n=nfirst,nlast
      if (head(n) .le. -9999.) nmiss=nmiss+1
      enddo
c
      if (nmiss .eq. 0) then
c        None are missing
      return
      elseif (nmiss .eq. ntot) then
c        The storm is stationary, set all headings to north
      do n=nfirst,nlast
       head(n) = 90.0
         enddo
      else
c        Some heading missing, find nearest values
c
         do n=nfirst+1,nlast
       if (head(n) .le. -9999.) then
c              Search backwards for first available value
               do k=n-1,nfirst,-1
                  if (head(k) .gt. -9999.) then
           head(n) = head(k)
           go to 1000
                  endif
               enddo
 1000          continue
            endif
         enddo
c
         do n=nfirst,nlast-1
       if (head(n) .le. -9999.) then
c              Search forward for first available value
               do k=n+1,nlast
                  if (head(k) .gt. -9999.) then
           head(n) = head(k)
           go to 1001
                  endif
               enddo
 1001          continue
            endif
         enddo
c
      endif
c
      return
      end
      subroutine ctor(x,y,r,theta)
c     This routine converts from Cartesion coordinates
c     to radial coordinates, where theta is in
c     degrees measured counter-clockwise from
c     the +x-axis.
c
      r = sqrt(x*x + y*y)
c
      if (r .le. 0.0) then
         theta = 0.0
         return
      endif
c
      rtd = 57.296
      theta = rtd*acos(x/r)
      if (y .lt. 0.0) theta = 360.0 - theta
c
      return
      end
      subroutine distk(rlon1,rlat1,rlon2,rlat2,dx,dy,rad)
c     This routine calculates the distance in km (rad) between the
c     points (rlon1,rlat1) and (rlon2,rlat2) using an approximate
c     formula. The lon and lat are in deg E and N. The east and
c     north components of the distance (dx,dy) are also calculated.
c
      dtk = 111.1
      dtr = 0.0174533
c
      cfac = cos(0.5*dtr*(rlat1+rlat2))
c
      dx  = dtk*(rlon2-rlon1)*cfac
      dy  = dtk*(rlat2-rlat1)
      rad = sqrt(dx*dx + dy*dy)
c
      return
      end

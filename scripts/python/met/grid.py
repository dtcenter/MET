import numpy as np

from met.logger import met_base

###########################################

class met_grid_tools(met_base):

   class_name = "met_grid_tools"

   DEFAULT_EARTH_RADIUS_KM = 6371.2

   #
   #  Derive a MET grid definition directly from latitude/longitude
   #  coordinate arrays, for Python embedding scripts that don't
   #  already know their input's projection.
   #
   #  1D lat/lon -> requires regularly spaced longitude; the latitude
   #  coordinate then picks among the grid types MET supports with
   #  1D coordinates:
   #     - regularly spaced latitude       -> LatLon
   #     - Gaussian quadrature latitudes   -> Gaussian
   #     - regularly spaced in Mercator-projected space -> Mercator
   #
   #  2D lat/lon -> fits the projected grid types MET supports for
   #  curvilinear coordinates -- Lambert Conformal Conic, Polar
   #  Stereographic, Lambert Azimuthal Equal Area, and Rotated
   #  Lat/Lon -- by searching for the projection under which the data
   #  forms a regular Cartesian (or rotated-angular) grid (needs
   #  SciPy), and keeping whichever fits best.
   #
   #  Returns (grid_dict, flip_row, flip_col). Both flags tell the
   #  caller how its data array must be reordered to satisfy MET's
   #  numpy-embedding contract: load_numpy() in
   #  dataplane_from_numpy_array.hpp maps numpy row r to DataPlane row
   #  (Ny - 1 - r) -- unconditionally, for every grid type -- so row 0
   #  of the array handed to MET must always be the row farthest from
   #  the grid's pin point (flip_row). Columns are never reordered by
   #  load_numpy(), so flip_col is only ever needed for Gaussian grids,
   #  whose longitude direction is fixed by MET (always decreasing
   #  with increasing column index) rather than read from the data;
   #  every other grid type takes its longitude directly from the
   #  array's own column order and flip_col is always False.
   #
   #  Also logs a ready-to-use set_attr_grid config line so a future
   #  run against the same data can skip this detection entirely.
   #
   @staticmethod
   def derive_grid(lat, lon, grid_name=None):
      lat = np.asarray(lat)
      lon = np.asarray(lon)

      if lat.ndim != lon.ndim:
         met_base.quit(
            "met_grid_tools.derive_grid() -> latitude and longitude "
            f"arrays must have the same number of dimensions; got "
            f"{lat.ndim}D and {lon.ndim}D")

      if lat.ndim == 1:
         grid, flip_row, flip_col = met_grid_tools._grid_from_1d_coords(
            lat, lon, grid_name)
      elif lat.ndim == 2:
         if lat.shape != lon.shape:
            met_base.quit(
               "met_grid_tools.derive_grid() -> 2D latitude and "
               f"longitude arrays must have matching shapes; got "
               f"{lat.shape} and {lon.shape}")
         grid, flip_row, flip_col = met_grid_tools._fit_projected_grid(
            lat, lon, grid_name)
      else:
         met_base.quit(
            "met_grid_tools.derive_grid() -> latitude/longitude arrays "
            f"must be 1D or 2D; got {lat.ndim}D")

      met_base.log_message(
         "To skip automatic grid detection on future runs against this "
         "data, add the following to the field's MET config:\n    " +
         met_grid_tools._suggest_set_attr_grid(grid))

      return grid, flip_row, flip_col

   #
   #  Cheaply determine whether an array needs its rows reversed to
   #  satisfy MET's numpy-embedding contract (see derive_grid() above),
   #  by comparing the first and last row of a latitude coordinate.
   #  Works for both 1D and 2D latitude arrays, and needs no grid
   #  definition at all -- useful when set_attr_grid is configured and
   #  the grid itself will be discarded, but the data still needs to
   #  be oriented correctly. Valid for every grid type MET supports:
   #  index y = 0 is always the southernmost row.
   #
   @staticmethod
   def detect_flip_row(lat):
      lat = np.asarray(lat)
      first, last = (float(lat[0]), float(lat[-1])) if lat.ndim == 1 \
         else (float(lat[0, 0]), float(lat[-1, 0]))
      return first < last

   #
   #  Return (flip_row, flip_col) for a grid whose type is already
   #  known, without running any projection fitting. Useful when
   #  set_attr_grid is configured and only the data orientation needs
   #  to be resolved.
   #
   #  flip_row follows the same rule as detect_flip_row() above and
   #  applies to every grid type.
   #
   #  flip_col is True only for Gaussian grids, whose longitude
   #  direction is fixed by MET (always decreasing with increasing
   #  column index) rather than read from the data; the caller must
   #  supply the longitude coordinate so the actual direction can be
   #  compared against MET's expectation. For all other grid types
   #  flip_col is always False.
   #
   #  grid_type should be the 'type' string from the grid dictionary
   #  (e.g. 'Gaussian', 'LatLon', 'Lambert Conformal', ...).
   #
   @staticmethod
   def detect_flip(lat, lon, grid_type):
      flip_row = met_grid_tools.detect_flip_row(lat)

      if grid_type == 'Gaussian':
         lon  = np.asarray(lon)
         dlon = np.diff(lon) if lon.ndim == 1 else np.diff(lon[0, :])
         descending_lon = float(dlon[0]) < 0
         flip_col = not descending_lon
      else:
         flip_col = False

      return flip_row, flip_col

   ##########################################################
   #
   #  1D coordinates: LatLon, Gaussian, or Mercator
   #
   ##########################################################

   @staticmethod
   def _grid_from_1d_coords(lat, lon, grid_name):
      dlon = np.diff(lon)

      if lon.size < 2 or not np.allclose(dlon, dlon[0], rtol=1e-3):
         met_base.quit(
            "met_grid_tools.derive_grid() -> longitude coordinates are "
            "not regularly spaced; LatLon, Gaussian, and Mercator grids "
            "all require regularly spaced longitude")

      dlat = np.diff(lat)

      if lat.size >= 2 and np.allclose(dlat, dlat[0], rtol=1e-3):
         return met_grid_tools._latlon_grid(lat, lon, dlat, dlon, grid_name)

      gaussian = met_grid_tools._try_gaussian_grid(lat, lon, dlon, grid_name)
      if gaussian is not None:
         return gaussian

      mercator = met_grid_tools._try_mercator_grid(lat, lon, grid_name)
      if mercator is not None:
         return mercator

      met_base.quit(
         "met_grid_tools.derive_grid() -> latitude coordinates are not "
         "regularly spaced, do not match a Gaussian grid's quadrature "
         "latitudes, and are not regularly spaced in Mercator-projected "
         "space either; no supported 1D grid type matches this data")

   @staticmethod
   def _latlon_grid(lat, lon, dlat, dlon, grid_name):
      # lat_ll is whichever end of the (monotonic) coordinate is
      # southernmost; flip when ascending (row 0 = south) -- see
      # derive_grid() above
      descending = dlat[0] < 0
      lat_ll = float(lat[-1] if descending else lat[0])
      flip_row = not descending

      grid = {
         'type':      'LatLon',
         'name':      grid_name or 'PyEmbedLatLon',
         'lat_ll':    lat_ll,
         'lon_ll':    float(lon[0]),
         'delta_lat': float(abs(dlat[0])),
         'delta_lon': float(abs(dlon[0])),
         'Nlat':      int(lat.size),
         'Nlon':      int(lon.size),
      }

      return grid, flip_row, False

   #
   #  A Gaussian grid's latitudes are the fixed roots of the Ny-degree
   #  Legendre polynomial (numpy.polynomial.legendre.leggauss computes
   #  these directly); if the sorted input latitudes match that set,
   #  this is a Gaussian grid. Ny must be even, per GaussianGrid's own
   #  requirement.
   #
   #  Unlike every other grid type, MET's Gaussian grid has no
   #  configurable longitude direction: GaussianGrid always assigns
   #  Delta_Lon = -360 / (Nx - 1), i.e. longitude strictly decreasing
   #  with increasing column index. Since load_numpy() never reorders
   #  columns, an ascending input longitude coordinate must have its
   #  column order reversed (flip_col) so column 0 lines up with
   #  whichever longitude MET will treat as lon_zero.
   #
   @staticmethod
   def _try_gaussian_grid(lat, lon, dlon, grid_name):
      ny = lat.size
      nx = lon.size

      if ny < 4 or ny % 2 != 0:
         return None

      roots, _ = np.polynomial.legendre.leggauss(ny)
      expected_lat = np.sort(np.degrees(np.arcsin(np.clip(roots, -1.0, 1.0))))
      observed_lat = np.sort(lat)

      if not np.allclose(observed_lat, expected_lat, atol=0.02):
         return None

      descending_lon = dlon[0] < 0
      lon_zero = float(lon[0] if descending_lon else lon[-1])
      flip_col = not descending_lon

      dlat = np.diff(lat)
      descending_lat = dlat[0] < 0
      flip_row = not descending_lat

      met_base.log_message(
         f"Latitude coordinates match {ny}-point Gaussian quadrature; "
         "deriving a Gaussian grid definition")

      grid = {
         'type':     'Gaussian',
         'name':     grid_name or 'PyEmbedGaussian',
         'lon_zero': lon_zero,
         'nx':       int(nx),
         'ny':       int(ny),
      }

      return grid, flip_row, flip_col

   #
   #  A Mercator grid is regularly spaced in longitude (already
   #  required above) and, in latitude, regularly spaced in the
   #  standard Mercator-projected coordinate
   #  v = ln(tan(pi/4 + lat/2)) rather than in latitude itself.
   #
   @staticmethod
   def _try_mercator_grid(lat, lon, grid_name):
      if lat.size < 2:
         return None

      v = np.log(np.tan(np.pi / 4.0 + np.radians(lat) / 2.0))
      dv = np.diff(v)

      if not np.allclose(dv, dv[0], rtol=1e-3):
         return None

      # MercatorData's lat_ll/lat_ur (and lon_ll/lon_ur) are taken
      # directly from the data with no fixed direction requirement,
      # but the universal row-orientation contract still applies:
      # lat_ll (index y = 0) must be the southernmost row
      descending = dv[0] < 0
      lat_ll = float(lat[-1] if descending else lat[0])
      lat_ur = float(lat[0] if descending else lat[-1])
      flip_row = not descending

      met_base.log_message(
         "Latitude coordinates are regularly spaced in Mercator-"
         "projected space; deriving a Mercator grid definition")

      grid = {
         'type':   'Mercator',
         'name':   grid_name or 'PyEmbedMercator',
         'lat_ll': lat_ll,
         'lon_ll': float(lon[0]),
         'lat_ur': lat_ur,
         'lon_ur': float(lon[-1]),
         'nx':     int(lon.size),
         'ny':     int(lat.size),
      }

      return grid, flip_row, False

   ##########################################################
   #
   #  2D coordinates: fit Lambert Conformal Conic or Polar
   #  Stereographic, whichever reproduces the data more precisely
   #
   ##########################################################

   @staticmethod
   def _fit_projected_grid(lat2d, lon2d, grid_name):
      try:
         from scipy.optimize import minimize
      except ImportError:
         met_base.quit(
            "met_grid_tools.derive_grid() -> fitting a projection to "
            "2D latitude/longitude coordinates requires SciPy, which "
            "is not installed in this Python environment")

      ny, nx = lat2d.shape

      if ny < 3 or nx < 3:
         met_base.quit(
            "met_grid_tools.derive_grid() -> grid is too small "
            f"({ny} x {nx}) to reliably fit a projection")

      candidates = []

      lcc = met_grid_tools._fit_lcc_candidate(lat2d, lon2d, grid_name, minimize)
      if lcc is not None:
         candidates.append(lcc)

      pss = met_grid_tools._fit_stereographic_candidate(
         lat2d, lon2d, grid_name, minimize)
      if pss is not None:
         candidates.append(pss)

      laea = met_grid_tools._fit_laea_candidate(lat2d, lon2d, grid_name, minimize)
      if laea is not None:
         candidates.append(laea)

      rll = met_grid_tools._fit_rotlatlon_candidate(lat2d, lon2d, grid_name, minimize)
      if rll is not None:
         candidates.append(rll)

      if not candidates:
         met_base.quit(
            "met_grid_tools.derive_grid() -> could not fit any "
            "supported projected grid type (Lambert Conformal Conic, "
            "Polar Stereographic, Lambert Azimuthal Equal Area, "
            "Rotated Lat/Lon) to this data; it may use an "
            "unsupported projection, or need a different assumed "
            f"earth radius than the default "
            f"{met_grid_tools.DEFAULT_EARTH_RADIUS_KM} km")

      # keep whichever candidate reproduces the data most precisely,
      # relative to its own fitted grid spacing
      grid, flip_row, flip_col, max_err_km, cell_km = min(
         candidates, key=lambda c: c[3] / c[4])

      met_base.log_message(
         f"Selected {grid['type']} as the best-fitting supported "
         f"projection (max residual {max_err_km:.4f} km against a "
         f"{cell_km:.4f} km grid cell)")

      return grid, flip_row, flip_col

   #
   #  Shared final step for both projection fits below: given the
   #  candidate projection's (x, y) map-plane coordinates (km) at
   #  every grid point, fit x and y as affine functions of column and
   #  row index and report the fitted spacing (dx, dy), the worst-case
   #  residual against that affine fit, and the average cell size.
   #
   @staticmethod
   def _affine_fit_quality(x, y, ny, nx):
      ii, jj = np.mgrid[0:ny, 0:nx]
      A_x = np.column_stack([np.ones(jj.size), jj.ravel().astype(float)])
      A_y = np.column_stack([np.ones(ii.size), ii.ravel().astype(float)])

      coef_x, *_ = np.linalg.lstsq(A_x, x.ravel(), rcond=None)
      coef_y, *_ = np.linalg.lstsq(A_y, y.ravel(), rcond=None)

      dx, dy = float(coef_x[1]), float(coef_y[1])

      pred_x = (A_x @ coef_x).reshape(ny, nx)
      pred_y = (A_y @ coef_y).reshape(ny, nx)
      max_err_km = float(np.max(np.hypot(x - pred_x, y - pred_y)))
      cell_km = float((abs(dx) + abs(dy)) / 2.0)

      return dx, dy, max_err_km, cell_km

   @staticmethod
   def _fit_is_good(result, dx, dy, max_err_km, cell_km):
      return result.success and np.isfinite(max_err_km) and \
         max_err_km <= 0.1 * cell_km and \
         abs(dx) > 1e-9 and abs(dy) > 1e-9 and \
         abs(abs(dx) - abs(dy)) <= 0.05 * cell_km

   #
   #  Spherical, tangent-case Lambert Conformal Conic forward
   #  projection (Snyder 1987); n is the cone constant, tangent
   #  standard parallel is asin(n)
   #
   @staticmethod
   def _lcc_forward(lat_deg, lon_deg, n, lon0_deg, r_km):
      lat  = np.radians(lat_deg)
      lon  = np.radians(lon_deg)
      lon0 = np.radians(lon0_deg)
      lat0 = np.arcsin(np.clip(n, -0.999999, 0.999999))

      F    = np.cos(lat0) * np.power(np.tan(np.pi / 4.0 + lat0 / 2.0), n) / n
      rho  = r_km * F / np.power(np.tan(np.pi / 4.0 + lat  / 2.0), n)
      rho0 = r_km * F / np.power(np.tan(np.pi / 4.0 + lat0 / 2.0), n)

      theta = n * (lon - lon0)

      x = rho * np.sin(theta)
      y = rho0 - rho * np.cos(theta)

      return x, y

   #
   #  objective for the fit below: project with trial (n, lon0) and
   #  measure how far the result is from a regular Cartesian grid;
   #  zero at the true projection parameters
   #
   @staticmethod
   def _lcc_grid_regularity_sse(params, lat2d, lon2d):
      n, lon0_deg = params

      if not (-0.999 < n < 0.999) or abs(n) < 1e-6:
         return 1.0e12

      x, y = met_grid_tools._lcc_forward(
         lat2d, lon2d, n, lon0_deg, met_grid_tools.DEFAULT_EARTH_RADIUS_KM)

      ny, nx = lat2d.shape
      ii, jj = np.mgrid[0:ny, 0:nx]

      A_x = np.column_stack([np.ones(jj.size), jj.ravel().astype(float)])
      A_y = np.column_stack([np.ones(ii.size), ii.ravel().astype(float)])

      coef_x, *_ = np.linalg.lstsq(A_x, x.ravel(), rcond=None)
      coef_y, *_ = np.linalg.lstsq(A_y, y.ravel(), rcond=None)

      return float(np.sum((x.ravel() - A_x @ coef_x) ** 2) +
                  np.sum((y.ravel() - A_y @ coef_y) ** 2))

   #
   #  Recover an LCC grid definition: fit the cone constant and
   #  central meridian by searching for the projection under which
   #  the data forms a regular Cartesian grid, then read the pin
   #  point off the real coordinate data and the spacing off the fit.
   #  Returns None (rather than quitting) when the fit isn't good
   #  enough, so the caller can fall back to another projection.
   #
   @staticmethod
   def _fit_lcc_candidate(lat2d, lon2d, grid_name, minimize):
      ny, nx = lat2d.shape

      n0     = float(np.sin(np.radians(np.nanmedian(lat2d))))
      lon0_0 = float(np.nanmedian(lon2d))

      result = minimize(
         met_grid_tools._lcc_grid_regularity_sse, x0=[n0, lon0_0],
         args=(lat2d, lon2d), method='Nelder-Mead',
         options={'xatol': 1e-10, 'fatol': 1e-8, 'maxiter': 20000})

      n_fit, lon0_fit = result.x

      x, y = met_grid_tools._lcc_forward(
         lat2d, lon2d, n_fit, lon0_fit, met_grid_tools.DEFAULT_EARTH_RADIUS_KM)
      dx, dy, max_err_km, cell_km = met_grid_tools._affine_fit_quality(
         x, y, ny, nx)

      met_base.log_message(
         "Lambert Conformal fit: standard parallel "
         f"{np.degrees(np.arcsin(np.clip(n_fit, -1.0, 1.0))):.4f} deg, "
         f"central meridian {lon0_fit % 360:.4f} deg, dx={dx:.4f} km, "
         f"dy={dy:.4f} km, max residual={max_err_km:.4f} km")

      if not met_grid_tools._fit_is_good(result, dx, dy, max_err_km, cell_km):
         return None

      scale_lat = float(np.degrees(np.arcsin(np.clip(n_fit, -1.0, 1.0))))

      # pin point (x_pin, y_pin) = (0, 0) is the min-y row; flip when
      # y is ascending (row 0 = pin row) -- see derive_grid() above
      descending_y = dy < 0
      y_idx = -1 if descending_y else 0
      flip_row = not descending_y

      grid = {
         'type':        'Lambert Conformal',
         'name':        grid_name or 'PyEmbedLambertConformalFit',
         'hemisphere':  'N' if scale_lat >= 0 else 'S',
         'scale_lat_1': scale_lat,
         'scale_lat_2': scale_lat,
         'lat_pin':     float(lat2d[y_idx, 0]),
         'lon_pin':     float(lon2d[y_idx, 0]),
         'x_pin':       0.0,
         'y_pin':       0.0,
         'lon_orient':  float(lon0_fit % 360.0),
         'd_km':        cell_km,
         'r_km':        met_grid_tools.DEFAULT_EARTH_RADIUS_KM,
         'nx':          int(nx),
         'ny':          int(ny),
      }

      return grid, flip_row, False, max_err_km, cell_km

   #
   #  Spherical polar stereographic forward projection (Snyder 1987),
   #  tangent at the pole -- see st_grid.cc's StereographicGrid
   #  constructor and latlon_to_xy(). Unlike Lambert Conformal, polar
   #  stereographic has no free angular "cone constant": for a given
   #  hemisphere, its shape is fixed, and only the central meridian
   #  (lon0_deg) needs fitting. hemisphere_sign is +1 for the tangent-
   #  at-the-north-pole case (equivalent to MET's scale_lat = 90) or
   #  -1 for the south pole (scale_lat = -90).
   #
   @staticmethod
   def _pss_forward(lat_deg, lon_deg, lon0_deg, hemisphere_sign, r_km):
      H = hemisphere_sign

      if H > 0:
         r = np.tan(np.radians(45.0 - 0.5 * lat_deg))
      else:
         r = np.tan(np.radians(45.0 + 0.5 * lat_deg))

      theta = np.radians(H * (lon0_deg - lon_deg))
      alpha_km = 2.0 * r_km   # tangent-at-pole case: scale_lat = H*90

      x = alpha_km * r * H * np.sin(theta)
      y = -alpha_km * r * H * np.cos(theta)

      return x, y

   @staticmethod
   def _pss_grid_regularity_sse(params, lat2d, lon2d, hemisphere_sign):
      lon0_deg = params[0]

      x, y = met_grid_tools._pss_forward(
         lat2d, lon2d, lon0_deg, hemisphere_sign,
         met_grid_tools.DEFAULT_EARTH_RADIUS_KM)

      ny, nx = lat2d.shape
      ii, jj = np.mgrid[0:ny, 0:nx]

      A_x = np.column_stack([np.ones(jj.size), jj.ravel().astype(float)])
      A_y = np.column_stack([np.ones(ii.size), ii.ravel().astype(float)])

      coef_x, *_ = np.linalg.lstsq(A_x, x.ravel(), rcond=None)
      coef_y, *_ = np.linalg.lstsq(A_y, y.ravel(), rcond=None)

      return float(np.sum((x.ravel() - A_x @ coef_x) ** 2) +
                  np.sum((y.ravel() - A_y @ coef_y) ** 2))

   #
   #  Recover a Polar Stereographic grid definition. The hemisphere is
   #  decided up front from the median latitude (not fit -- polar
   #  stereographic's shape is hemisphere-specific, not continuously
   #  variable), then only the central meridian is fit. Returns None
   #  (rather than quitting) when the fit isn't good enough, so the
   #  caller can fall back to another projection.
   #
   @staticmethod
   def _fit_stereographic_candidate(lat2d, lon2d, grid_name, minimize):
      ny, nx = lat2d.shape

      hemisphere_sign = 1.0 if np.nanmedian(lat2d) >= 0 else -1.0
      lon0_0 = float(np.nanmedian(lon2d))

      result = minimize(
         met_grid_tools._pss_grid_regularity_sse, x0=[lon0_0],
         args=(lat2d, lon2d, hemisphere_sign), method='Nelder-Mead',
         options={'xatol': 1e-10, 'fatol': 1e-8, 'maxiter': 20000})

      lon0_fit = float(result.x[0])

      x, y = met_grid_tools._pss_forward(
         lat2d, lon2d, lon0_fit, hemisphere_sign,
         met_grid_tools.DEFAULT_EARTH_RADIUS_KM)
      dx, dy, max_err_km, cell_km = met_grid_tools._affine_fit_quality(
         x, y, ny, nx)

      met_base.log_message(
         "Polar Stereographic fit "
         f"({'N' if hemisphere_sign > 0 else 'S'} hemisphere): central "
         f"meridian {lon0_fit % 360:.4f} deg, dx={dx:.4f} km, "
         f"dy={dy:.4f} km, max residual={max_err_km:.4f} km")

      if not met_grid_tools._fit_is_good(result, dx, dy, max_err_km, cell_km):
         return None

      # pin point (x_pin, y_pin) = (0, 0) is the min-y row; flip when
      # y is ascending (row 0 = pin row) -- see derive_grid() above
      descending_y = dy < 0
      y_idx = -1 if descending_y else 0
      flip_row = not descending_y

      grid = {
         'type':       'Polar Stereographic',
         'name':       grid_name or 'PyEmbedPolarStereographicFit',
         'hemisphere': 'N' if hemisphere_sign > 0 else 'S',
         'scale_lat':  90.0 * hemisphere_sign,
         'lat_pin':    float(lat2d[y_idx, 0]),
         'lon_pin':    float(lon2d[y_idx, 0]),
         'x_pin':      0.0,
         'y_pin':      0.0,
         'lon_orient': float(lon0_fit % 360.0),
         'd_km':       cell_km,
         'r_km':       met_grid_tools.DEFAULT_EARTH_RADIUS_KM,
         'nx':         int(nx),
         'ny':         int(ny),
      }

      return grid, flip_row, False, max_err_km, cell_km

   ##########################################################
   #
   #  2D coordinates: Lambert Azimuthal Equal Area
   #
   ##########################################################

   #
   #  Spherical LAEA forward projection (Snyder 1987, eq. 24-3/24-4):
   #  x = R k' cos(lat) sin(lon - lon0)
   #  y = R k' [cos(lat0) sin(lat) - sin(lat0) cos(lat) cos(lon - lon0)]
   #  where k' = sqrt(2 / (1 + sin(lat0) sin(lat) + cos(lat0) cos(lat) cos(lon - lon0)))
   #
   @staticmethod
   def _laea_forward(lat_deg, lon_deg, lat0_deg, lon0_deg, r_km):
      lat  = np.radians(lat_deg)
      lon  = np.radians(lon_deg)
      lat0 = np.radians(lat0_deg)
      lon0 = np.radians(lon0_deg)
      dlon = lon - lon0
      kp   = np.sqrt(2.0 / (1.0 + np.sin(lat0)*np.sin(lat)
                              + np.cos(lat0)*np.cos(lat)*np.cos(dlon)))
      x = r_km * kp * np.cos(lat) * np.sin(dlon)
      y = r_km * kp * (np.cos(lat0)*np.sin(lat) - np.sin(lat0)*np.cos(lat)*np.cos(dlon))
      return x, y

   @staticmethod
   def _laea_grid_regularity_sse(params, lat2d, lon2d):
      lat0, lon0 = params
      x, y = met_grid_tools._laea_forward(
         lat2d, lon2d, lat0, lon0, met_grid_tools.DEFAULT_EARTH_RADIUS_KM)
      ny, nx = lat2d.shape
      ii, jj = np.mgrid[0:ny, 0:nx]
      A_x = np.column_stack([np.ones(jj.size), jj.ravel().astype(float)])
      A_y = np.column_stack([np.ones(ii.size), ii.ravel().astype(float)])
      coef_x, *_ = np.linalg.lstsq(A_x, x.ravel(), rcond=None)
      coef_y, *_ = np.linalg.lstsq(A_y, y.ravel(), rcond=None)
      return float(np.sum((x.ravel() - A_x @ coef_x)**2) +
                   np.sum((y.ravel() - A_y @ coef_y)**2))

   #
   #  Recover a Lambert Azimuthal Equal Area grid definition by fitting
   #  the projection center (standard_lat, central_lon) under which the
   #  data forms a regular Cartesian grid. Returns None when the fit is
   #  not good enough, so the caller can fall back to another projection.
   #
   @staticmethod
   def _fit_laea_candidate(lat2d, lon2d, grid_name, minimize):
      ny, nx = lat2d.shape

      lat0_0 = float(np.nanmedian(lat2d))
      lon0_0 = float(np.nanmedian(lon2d))

      result = minimize(
         met_grid_tools._laea_grid_regularity_sse, x0=[lat0_0, lon0_0],
         args=(lat2d, lon2d), method='Nelder-Mead',
         options={'xatol': 1e-10, 'fatol': 1e-8, 'maxiter': 20000})

      lat0_fit, lon0_fit = result.x

      x, y = met_grid_tools._laea_forward(
         lat2d, lon2d, lat0_fit, lon0_fit,
         met_grid_tools.DEFAULT_EARTH_RADIUS_KM)
      dx, dy, max_err_km, cell_km = met_grid_tools._affine_fit_quality(
         x, y, ny, nx)

      met_base.log_message(
         "Lambert Azimuthal Equal Area fit: center "
         f"lat={lat0_fit:.4f} deg, lon={lon0_fit % 360:.4f} deg, "
         f"dx={dx:.4f} km, dy={dy:.4f} km, max residual={max_err_km:.4f} km")

      if not met_grid_tools._fit_is_good(result, dx, dy, max_err_km, cell_km):
         return None

      descending_y = dy < 0
      y_idx = -1 if descending_y else 0
      flip_row = not descending_y

      grid = {
         'type':        'Lambert Azimuthal Equal Area',
         'name':        grid_name or 'PyEmbedLambertAzimuthalFit',
         'lat_first':   float(lat2d[y_idx, 0]),
         'lon_first':   float(lon2d[y_idx, 0]),
         'standard_lat': float(lat0_fit),
         'central_lon': float(lon0_fit % 360.0),
         'dx_km':       float(abs(dx)),
         'dy_km':       float(abs(dy)),
         'nx':          int(nx),
         'ny':          int(ny),
         'radius_km':   met_grid_tools.DEFAULT_EARTH_RADIUS_KM,
      }

      return grid, flip_row, False, max_err_km, cell_km

   ##########################################################
   #
   #  2D coordinates: Rotated Lat/Lon
   #
   ##########################################################

   #
   #  Convert true (east-positive) lat/lon to rotated lat/lon using
   #  MET's RotatedLatLonGrid convention: the rotated south pole is at
   #  true (lat_sp_deg, lon_sp_deg), and aux_rotation is zero.
   #
   #  Derived from MET's earth_rotation.cc and latlon_xyz.cc, which
   #  use a non-standard Cartesian convention (x = cos*sin_lon,
   #  y = cos*cos_lon) and compose R_x(-(90+lat_sp)) * R_z(lon_sp).
   #
   @staticmethod
   def _rotlatlon_forward(lat_deg, lon_deg, lat_sp_deg, lon_sp_deg):
      lat    = np.radians(lat_deg)
      lon    = np.radians(lon_deg)
      lat_sp = np.radians(lat_sp_deg)
      lon_sp = np.radians(lon_sp_deg)
      dlon   = lon - lon_sp
      sin_rl = (-np.sin(lat_sp) * np.sin(lat)
                - np.cos(lat_sp) * np.cos(lat) * np.cos(dlon))
      rot_lat = np.degrees(np.arcsin(np.clip(sin_rl, -1.0, 1.0)))
      rot_lon = np.degrees(np.arctan2(
         np.cos(lat) * np.sin(dlon),
         np.cos(lat_sp)*np.sin(lat) - np.sin(lat_sp)*np.cos(lat)*np.cos(dlon)))
      return rot_lat, rot_lon

   #
   #  For a regular rotated lat/lon grid, rot_lat must be constant
   #  within every row and rot_lon constant within every column.
   #  The SSE measures how far the data deviates from those constraints.
   #
   @staticmethod
   def _rotlatlon_grid_regularity_sse(params, lat2d, lon2d):
      lat_sp, lon_sp = params
      rot_lat, rot_lon = met_grid_tools._rotlatlon_forward(
         lat2d, lon2d, lat_sp, lon_sp)

      row_means = rot_lat.mean(axis=1, keepdims=True)
      lat_sse   = float(np.sum((rot_lat - row_means)**2))

      col_means = rot_lon.mean(axis=0, keepdims=True)
      raw_err   = rot_lon - col_means
      lon_err   = ((raw_err + 180.0) % 360.0) - 180.0
      lon_sse   = float(np.sum(lon_err**2))

      return lat_sse + lon_sse

   #
   #  Recover a Rotated Lat/Lon grid definition. Fits the south-pole
   #  location by minimising in-row variation of rot_lat and in-column
   #  variation of rot_lon. Returns None when the fit isn't good enough
   #  so the caller can fall back to another projection. aux_rotation
   #  is fixed at zero (the common case); non-zero aux_rotation is not
   #  currently detected.
   #
   @staticmethod
   def _fit_rotlatlon_candidate(lat2d, lon2d, grid_name, minimize):
      ny, nx = lat2d.shape

      lat_sp0 = max(float(np.nanmedian(lat2d)) - 90.0, -90.0)
      lon_sp0 = float(np.nanmedian(lon2d))

      result = minimize(
         met_grid_tools._rotlatlon_grid_regularity_sse, x0=[lat_sp0, lon_sp0],
         args=(lat2d, lon2d), method='Nelder-Mead',
         options={'xatol': 1e-10, 'fatol': 1e-8, 'maxiter': 20000})

      lat_sp_fit, lon_sp_fit = result.x

      rot_lat, rot_lon = met_grid_tools._rotlatlon_forward(
         lat2d, lon2d, lat_sp_fit, lon_sp_fit)

      row_means  = rot_lat.mean(axis=1)
      drlat      = np.diff(row_means)
      delta_rlat = float(drlat.mean())

      col_means_raw = rot_lon.mean(axis=0)
      col_means     = np.unwrap(col_means_raw, period=360)
      drlon         = np.diff(col_means)
      delta_rlon    = float(drlon.mean())

      max_lat_err = float(np.max(np.abs(rot_lat - row_means[:, np.newaxis])))
      raw_lon_err = rot_lon - col_means_raw[np.newaxis, :]
      max_lon_err = float(np.max(np.abs(((raw_lon_err + 180.0) % 360.0) - 180.0)))
      max_err_deg = max(max_lat_err, max_lon_err)
      cell_deg    = (abs(delta_rlat) + abs(delta_rlon)) / 2.0

      met_base.log_message(
         "Rotated Lat/Lon fit: south pole at "
         f"lat={lat_sp_fit:.4f} deg, lon={lon_sp_fit % 360:.4f} deg, "
         f"delta_rot_lat={delta_rlat:.6f} deg, "
         f"delta_rot_lon={delta_rlon:.6f} deg, "
         f"max residual={max_err_deg:.6f} deg")

      if (not result.success or
              not np.isfinite(max_err_deg) or
              cell_deg < 1e-9 or
              max_err_deg > 0.01 * cell_deg or
              abs(delta_rlat) < 1e-9 or abs(delta_rlon) < 1e-9):
         return None

      # Scale to km for comparison with the other projected candidates
      scale      = np.radians(1.0) * met_grid_tools.DEFAULT_EARTH_RADIUS_KM
      max_err_km = max_err_deg * scale
      cell_km    = cell_deg    * scale

      # MET's y=0 is the minimum rot_lat row (pin point).
      # numpy row 0 must be farthest from it (max rot_lat), so flip when
      # the data is ascending (row 0 = min rot_lat).
      descending_rlat = delta_rlat < 0
      y_idx    = -1 if descending_rlat else 0
      flip_row = not descending_rlat

      rot_lat_ll = float(row_means[-1] if descending_rlat else row_means[0])
      rot_lon_ll = float(col_means[0])

      grid = {
         'type':                'Rotated LatLon',
         'name':                grid_name or 'PyEmbedRotatedLatLonFit',
         'rot_lat_ll':          rot_lat_ll,
         'rot_lon_ll':          rot_lon_ll,
         'delta_rot_lat':       float(abs(delta_rlat)),
         'delta_rot_lon':       float(abs(delta_rlon)),
         'Nlat':                int(ny),
         'Nlon':                int(nx),
         'true_lat_south_pole': float(lat_sp_fit),
         'true_lon_south_pole': float(lon_sp_fit % 360.0),
         'aux_rotation':        0.0,
      }

      return grid, flip_row, False, max_err_km, cell_km

   ##########################################################
   #
   #  set_attr_grid suggestion
   #
   ##########################################################

   #
   #  Format a grid dictionary as a ready-to-use set_attr_grid config
   #  line (grid-spec string format, see docs/Users_Guide/appendixB.rst
   #  and find_grid_by_name.cc's parse_*_grid() functions, one per
   #  grid type below). Those functions use the same lat_ll/lon_ll,
   #  lat_pin/lon_pin (with (x_pin, y_pin) = (0, 0)), and lon_zero
   #  convention as the grid dictionaries built above, so the numeric
   #  values can be dropped into the string as-is with no conversion.
   #
   @staticmethod
   def _suggest_set_attr_grid(grid):
      gtype = grid['type']

      if gtype == 'LatLon':
         parts = ['latlon', str(grid['Nlon']), str(grid['Nlat']),
                  f"{grid['lat_ll']:.6f}", f"{grid['lon_ll']:.6f}",
                  f"{grid['delta_lat']:.6f}", f"{grid['delta_lon']:.6f}"]

      elif gtype == 'Gaussian':
         parts = ['gaussian', f"{grid['lon_zero']:.6f}",
                  str(grid['nx']), str(grid['ny'])]

      elif gtype == 'Mercator':
         parts = ['mercator', str(grid['nx']), str(grid['ny']),
                  f"{grid['lat_ll']:.6f}", f"{grid['lon_ll']:.6f}",
                  f"{grid['lat_ur']:.6f}", f"{grid['lon_ur']:.6f}"]

      elif gtype == 'Polar Stereographic':
         parts = ['stereo', str(grid['nx']), str(grid['ny']),
                  f"{grid['lat_pin']:.6f}", f"{grid['lon_pin']:.6f}",
                  f"{grid['lon_orient']:.6f}", f"{grid['d_km']:.6f}",
                  f"{grid['r_km']:.6f}", f"{grid['scale_lat']:.6f}",
                  grid['hemisphere']]

      elif gtype == 'Lambert Azimuthal Equal Area':
         parts = ['laea', str(grid['nx']), str(grid['ny']),
                  f"{grid['lat_first']:.6f}", f"{grid['lon_first']:.6f}",
                  f"{grid['central_lon']:.6f}",
                  f"{grid['dx_km']:.6f}", f"{grid['dy_km']:.6f}",
                  f"{grid['standard_lat']:.6f}", f"{grid['radius_km']:.6f}"]

      elif gtype == 'Rotated LatLon':
         parts = ['rotlatlon', str(grid['Nlon']), str(grid['Nlat']),
                  f"{grid['rot_lat_ll']:.6f}", f"{grid['rot_lon_ll']:.6f}",
                  f"{grid['delta_rot_lat']:.6f}", f"{grid['delta_rot_lon']:.6f}",
                  f"{grid['true_lat_south_pole']:.6f}",
                  f"{grid['true_lon_south_pole']:.6f}",
                  f"{grid['aux_rotation']:.6f}"]

      else:   # Lambert Conformal
         parts = ['lambert', str(grid['nx']), str(grid['ny']),
                  f"{grid['lat_pin']:.6f}", f"{grid['lon_pin']:.6f}",
                  f"{grid['lon_orient']:.6f}", f"{grid['d_km']:.6f}",
                  f"{grid['r_km']:.6f}", f"{grid['scale_lat_1']:.6f}"]
         if abs(grid['scale_lat_2'] - grid['scale_lat_1']) > 1.0e-9:
            parts.append(f"{grid['scale_lat_2']:.6f}")
         parts.append(grid['hemisphere'])

      return 'set_attr_grid = "' + ' '.join(parts) + '";'

   ##########################################################
   #
   #  CF grid_mapping → MET grid dictionary
   #
   #  grid_from_cf_mapping(mapping_name, attrs, ...)  -- public entry
   #
   #  Supported CF grid_mapping_name values:
   #    latitude_longitude           -> LatLon
   #    mercator                     -> Mercator
   #    lambert_conformal_conic      -> Lambert Conformal
   #    lambert_azimuthal_equal_area -> Lambert Azimuthal Equal Area
   #    polar_stereographic          -> Polar Stereographic
   #    rotated_latitude_longitude   -> Rotated LatLon
   #
   ##########################################################

   # Attribute keys to probe for embedded CRS metadata, ordered
   # most-to-least completely specified (rioxarray's de facto convention
   # first, then the GeoZarr proposal's attributes).
   PROJ_CRS_ATTR_KEYS = ['crs_wkt', 'spatial_ref', 'proj:wkt2',
                          'proj:projjson', 'proj:code']

   @staticmethod
   def _earth_radius_km_from_cf(attrs, default_km=None):
      """Extract earth radius in km from CF grid_mapping attributes."""
      if default_km is None:
         default_km = met_grid_tools.DEFAULT_EARTH_RADIUS_KM
      r = attrs.get('earth_radius')
      if r is not None:
         return float(r) / 1000.0
      semi = attrs.get('semi_major_axis')
      if semi is not None:
         return float(semi) / 1000.0
      return default_km

   @staticmethod
   def _validate_projected_xy(x, y):
      """Validate 1D regularly-spaced x, y coordinate arrays (meters).
      Returns (dx_km, dy_km, descending_y)."""
      x = np.asarray(x, dtype=float)
      y = np.asarray(y, dtype=float)
      if x.ndim != 1 or y.ndim != 1 or x.size < 2 or y.size < 2:
         met_base.quit(
            "met_grid_tools: projected x/y coordinate arrays must be "
            "1-dimensional and have at least 2 elements")
      dx = np.diff(x)
      dy = np.diff(y)
      if not np.allclose(dx, dx[0], rtol=1e-3):
         met_base.quit(
            "met_grid_tools: projected x coordinates are not regularly spaced")
      if not np.allclose(dy, dy[0], rtol=1e-3):
         met_base.quit(
            "met_grid_tools: projected y coordinates are not regularly spaced")
      return (float(abs(dx[0])) / 1000.0,
              float(abs(dy[0])) / 1000.0,
              float(dy[0]) < 0)

   @staticmethod
   def _pin_from_latlon2d(lat2d, lon2d, descending_y):
      """Return (lat_pin, lon_pin, flip_row) from 2D lat/lon arrays.
      The pin point is the grid corner nearest the south-west origin.
      flip_row is True when row 0 is the northernmost row (y ascending),
      because load_numpy() maps numpy row 0 to DataPlane row Ny-1."""
      lat2d = np.asarray(lat2d)
      lon2d = np.asarray(lon2d)
      if lat2d.ndim != 2 or lon2d.ndim != 2:
         met_base.quit(
            "met_grid_tools: 2D latitude/longitude auxiliary coordinate "
            "arrays are required to determine the pin point for this "
            "projected grid type")
      y_idx    = -1 if descending_y else 0
      flip_row = not descending_y
      return float(lat2d[y_idx, 0]), float(lon2d[y_idx, 0]), flip_row

   @staticmethod
   def _cf_latlon_grid(attrs, lat, lon, grid_name):
      """LatLon grid from a CF 'latitude_longitude' grid_mapping."""
      lat = np.asarray(lat, dtype=float)
      lon = np.asarray(lon, dtype=float)
      if lat.ndim != 1 or lon.ndim != 1 or lat.size < 2 or lon.size < 2:
         met_base.quit(
            "met_grid_tools._cf_latlon_grid(): latitude and longitude "
            "must be 1D with at least 2 elements")
      dlat = np.diff(lat)
      dlon = np.diff(lon)
      if not np.allclose(dlat, dlat[0], rtol=1e-3) or \
         not np.allclose(dlon, dlon[0], rtol=1e-3):
         met_base.quit(
            "met_grid_tools._cf_latlon_grid(): latitude/longitude "
            "coordinates must be regularly spaced")
      grid, flip_row, _ = met_grid_tools._latlon_grid(
         lat, lon, dlat, dlon, grid_name or 'CfLatLon')
      return grid, flip_row

   @staticmethod
   def _cf_mercator_grid(attrs, x, y, lat2d, lon2d, grid_name):
      """Mercator grid from CF 'mercator' grid_mapping attributes.
      MET defines a Mercator grid by its lat/lon corners; 2D auxiliary
      lat/lon coordinates are used if available, otherwise the corners
      are computed analytically from the Mercator inverse projection."""
      x = np.asarray(x, dtype=float)
      y = np.asarray(y, dtype=float)
      if x.ndim != 1 or y.ndim != 1 or x.size < 2 or y.size < 2:
         met_base.quit(
            "met_grid_tools._cf_mercator_grid(): projected x/y must be "
            "1D with at least 2 elements")
      dy          = np.diff(y)
      descending_y = float(dy[0]) < 0
      flip_row    = not descending_y
      y_ll = -1 if descending_y else 0    # row index with minimum latitude
      y_ur =  0 if descending_y else -1   # row index with maximum latitude

      have_2d = (lat2d is not None and lon2d is not None)
      if have_2d:
         lat2d = np.asarray(lat2d)
         lon2d = np.asarray(lon2d)
         have_2d = (lat2d.ndim == 2 and lon2d.ndim == 2)
      if have_2d:
         lat_ll = float(lat2d[y_ll,  0]);  lon_ll = float(lon2d[y_ll,  0])
         lat_ur = float(lat2d[y_ur, -1]);  lon_ur = float(lon2d[y_ur, -1])
      else:
         # spherical Mercator inverse: lat = 2*atan(exp(y_eff/R)) - pi/2
         r_m  = met_grid_tools._earth_radius_km_from_cf(attrs) * 1000.0
         lat_s = np.radians(float(attrs.get('standard_parallel', 0.0)))
         lon_0 = float(attrs.get('longitude_of_projection_origin', 0.0))
         fe    = float(attrs.get('false_easting',  0.0))
         fn    = float(attrs.get('false_northing', 0.0))
         def _merc_inv(xi, yi):
            xr = xi - fe;  yr = yi - fn
            lat_i = np.degrees(2.0 * np.arctan(
               np.exp(yr * np.cos(lat_s) / r_m)) - np.pi / 2.0)
            lon_i = lon_0 + np.degrees(xr * np.cos(lat_s) / r_m)
            return float(lat_i), float(lon_i)
         lat_ll, lon_ll = _merc_inv(x[0],  y[y_ll])
         lat_ur, lon_ur = _merc_inv(x[-1], y[y_ur])

      grid = {
         'type':   'Mercator',
         'name':   grid_name or 'CfMercator',
         'lat_ll': lat_ll,  'lon_ll': lon_ll,
         'lat_ur': lat_ur,  'lon_ur': lon_ur,
         'nx':     int(x.size),
         'ny':     int(y.size),
      }
      return grid, flip_row

   @staticmethod
   def _cf_lcc_grid(attrs, x, y, lat2d, lon2d, grid_name):
      """Lambert Conformal Conic grid from CF 'lambert_conformal_conic' attrs."""
      if 'standard_parallel' not in attrs or \
         'longitude_of_central_meridian' not in attrs:
         met_base.quit(
            "met_grid_tools._cf_lcc_grid(): CF lambert_conformal_conic "
            "attrs must include 'standard_parallel' and "
            "'longitude_of_central_meridian'")
      std        = np.atleast_1d(np.asarray(attrs['standard_parallel'],
                                            dtype=float))
      lat_origin = float(attrs.get('latitude_of_projection_origin', std[0]))
      lon_orient = float(attrs['longitude_of_central_meridian'])
      r_km = met_grid_tools._earth_radius_km_from_cf(attrs,
                                                      default_km=6371.229)
      dx_km, dy_km, descending_y = met_grid_tools._validate_projected_xy(x, y)
      lat_pin, lon_pin, flip_row = met_grid_tools._pin_from_latlon2d(
         lat2d, lon2d, descending_y)
      grid = {
         'type':        'Lambert Conformal',
         'name':        grid_name or 'CfLambertConformal',
         'hemisphere':  'N' if lat_origin >= 0 else 'S',
         'scale_lat_1': float(std[0]),
         'scale_lat_2': float(std[-1]),
         'lat_pin':     lat_pin,
         'lon_pin':     lon_pin,
         'x_pin':       0.0,
         'y_pin':       0.0,
         'lon_orient':  lon_orient,
         'd_km':        dx_km,
         'r_km':        r_km,
         'nx':          int(np.asarray(x).size),
         'ny':          int(np.asarray(y).size),
      }
      return grid, flip_row

   @staticmethod
   def _cf_laea_grid(attrs, x, y, lat2d, lon2d, grid_name):
      """Lambert Azimuthal Equal Area grid from CF attrs."""
      standard_lat = float(attrs.get('latitude_of_projection_origin',  0.0))
      central_lon  = float(attrs.get('longitude_of_projection_origin', 0.0))
      r_km = met_grid_tools._earth_radius_km_from_cf(attrs)
      dx_km, dy_km, descending_y = met_grid_tools._validate_projected_xy(x, y)
      lat_first, lon_first, flip_row = met_grid_tools._pin_from_latlon2d(
         lat2d, lon2d, descending_y)
      grid = {
         'type':         'Lambert Azimuthal Equal Area',
         'name':         grid_name or 'CfLambertAzimuthal',
         'lat_first':    lat_first,
         'lon_first':    lon_first,
         'standard_lat': standard_lat,
         'central_lon':  central_lon,
         'dx_km':        dx_km,
         'dy_km':        dy_km,
         'nx':           int(np.asarray(x).size),
         'ny':           int(np.asarray(y).size),
         'radius_km':    r_km,
      }
      return grid, flip_row

   @staticmethod
   def _cf_pss_grid(attrs, x, y, lat2d, lon2d, grid_name):
      """Polar Stereographic grid from CF 'polar_stereographic' attrs."""
      lat_origin = float(attrs.get('latitude_of_projection_origin', 90.0))
      lon_orient = float(attrs.get('straight_vertical_longitude_from_pole', 0.0))
      scale_lat  = float(attrs['standard_parallel']) \
         if 'standard_parallel' in attrs else float(lat_origin)
      r_km = met_grid_tools._earth_radius_km_from_cf(attrs)
      dx_km, dy_km, descending_y = met_grid_tools._validate_projected_xy(x, y)
      lat_pin, lon_pin, flip_row = met_grid_tools._pin_from_latlon2d(
         lat2d, lon2d, descending_y)
      grid = {
         'type':       'Polar Stereographic',
         'name':       grid_name or 'CfPolarStereographic',
         'hemisphere': 'N' if lat_origin >= 0 else 'S',
         'scale_lat':  scale_lat,
         'lat_pin':    lat_pin,
         'lon_pin':    lon_pin,
         'x_pin':      0.0,
         'y_pin':      0.0,
         'lon_orient': lon_orient,
         'd_km':       dx_km,
         'r_km':       r_km,
         'nx':         int(np.asarray(x).size),
         'ny':         int(np.asarray(y).size),
      }
      return grid, flip_row

   @staticmethod
   def _cf_rotlatlon_grid(attrs, lat, lon, grid_name):
      """Rotated Lat/Lon grid from CF 'rotated_latitude_longitude' attrs.
      CF stores the north pole position; MET uses the south pole.
      lat/lon must be the rotated-system 1D coordinate arrays."""
      lat_np  = float(attrs.get('grid_north_pole_latitude',   90.0))
      lon_np  = float(attrs.get('grid_north_pole_longitude',   0.0))
      aux_rot = float(attrs.get('north_pole_grid_longitude',   0.0))
      # antipode of the north pole gives the south pole
      true_lat_sp = -lat_np
      true_lon_sp = (lon_np + 180.0) % 360.0
      lat = np.asarray(lat, dtype=float)
      lon = np.asarray(lon, dtype=float)
      if lat.ndim != 1 or lon.ndim != 1 or lat.size < 2 or lon.size < 2:
         met_base.quit(
            "met_grid_tools._cf_rotlatlon_grid(): rotated lat/lon "
            "must be 1D with at least 2 elements")
      dlat = np.diff(lat)
      dlon = np.diff(lon)
      if not np.allclose(dlat, dlat[0], rtol=1e-3) or \
         not np.allclose(dlon, dlon[0], rtol=1e-3):
         met_base.quit(
            "met_grid_tools._cf_rotlatlon_grid(): rotated lat/lon "
            "coordinates must be regularly spaced")
      descending_lat = float(dlat[0]) < 0
      rot_lat_ll     = float(lat[-1] if descending_lat else lat[0])
      flip_row       = not descending_lat
      grid = {
         'type':                'Rotated LatLon',
         'name':                grid_name or 'CfRotatedLatLon',
         'rot_lat_ll':          rot_lat_ll,
         'rot_lon_ll':          float(lon[0]),
         'delta_rot_lat':       float(abs(dlat[0])),
         'delta_rot_lon':       float(abs(dlon[0])),
         'Nlat':                int(lat.size),
         'Nlon':                int(lon.size),
         'true_lat_south_pole': true_lat_sp,
         'true_lon_south_pole': true_lon_sp,
         'aux_rotation':        aux_rot,
      }
      return grid, flip_row

   @staticmethod
   def grid_from_cf_mapping(mapping_name, attrs, x=None, y=None,
                             lat=None, lon=None, lat2d=None, lon2d=None,
                             grid_name=None):
      """Build a MET grid dict from a CF grid_mapping variable.

      Returns (grid_dict, flip_row).

      mapping_name  the grid_mapping_name attribute value
      attrs         the grid_mapping variable's attribute dict
      x, y          1D projected coordinate arrays (meters), for
                    projected types (LCC, PSS, LAEA, Mercator)
      lat, lon      1D coordinate arrays (degrees), for geographic types
                    (LatLon, RotatedLatLon); for RotatedLatLon these must
                    be the rotated-system coordinates, not true lat/lon
      lat2d, lon2d  2D auxiliary lat/lon arrays (degrees), used as the
                    pin-point / corner source for projected grid types
      grid_name     optional name string for the returned grid dict
      """
      projected = {
         'mercator':                     met_grid_tools._cf_mercator_grid,
         'lambert_conformal_conic':      met_grid_tools._cf_lcc_grid,
         'lambert_azimuthal_equal_area': met_grid_tools._cf_laea_grid,
         'polar_stereographic':          met_grid_tools._cf_pss_grid,
      }
      geographic = {
         'latitude_longitude':          met_grid_tools._cf_latlon_grid,
         'rotated_latitude_longitude':  met_grid_tools._cf_rotlatlon_grid,
      }
      if mapping_name in projected:
         return projected[mapping_name](attrs, x, y, lat2d, lon2d, grid_name)
      elif mapping_name in geographic:
         return geographic[mapping_name](attrs, lat, lon, grid_name)
      else:
         supported = sorted(projected) + sorted(geographic)
         met_base.quit(
            f"met_grid_tools.grid_from_cf_mapping(): unsupported "
            f"grid_mapping_name '{mapping_name}'; supported types: "
            + ', '.join(supported))

   ##########################################################
   #
   #  PROJ CRS → MET grid dictionary
   #
   #  find_proj_crs(candidates)       -- locate embedded CRS metadata
   #  grid_from_crs(crs, ...)         -- public entry point
   #
   #  Supported projection families (detected from the CRS):
   #    Geographic (lat/lon)           -> LatLon
   #    Mercator                       -> Mercator
   #    Lambert Conformal Conic        -> Lambert Conformal
   #    Lambert Azimuthal Equal Area   -> Lambert Azimuthal Equal Area
   #    Polar Stereographic            -> Polar Stereographic
   #
   ##########################################################

   @staticmethod
   def _crs_from_attrs(pyproj_mod, attrs, source_desc):
      """Try each key in PROJ_CRS_ATTR_KEYS; return (crs, key) for the
      first that parses successfully, or (None, None)."""
      for key in met_grid_tools.PROJ_CRS_ATTR_KEYS:
         if key not in attrs:
            continue
         value = attrs[key]
         try:
            if key == 'proj:projjson':
               crs = pyproj_mod.CRS.from_json_dict(value) \
                  if isinstance(value, dict) \
                  else pyproj_mod.CRS.from_json(value)
            else:
               crs = pyproj_mod.CRS.from_user_input(value)
            return crs, key
         except Exception as ex:
            met_base.log_message(
               f"Found a '{key}' attribute in {source_desc} but could "
               f"not parse it as a CRS: {ex}")
      return None, None

   @staticmethod
   def find_proj_crs(candidates):
      """Search (attrs_dict, source_description) pairs for CRS metadata.

      Returns (pyproj.CRS, source_description) for the first match, or
      (None, None). pyproj is imported lazily so it remains optional.

      'candidates' should be ordered most-preferred to least-preferred,
      typically: CF grid_mapping variable, spatial_ref coordinate,
      data-variable attributes, dataset global attributes.
      """
      if not any(key in attrs
                 for attrs, _ in candidates
                 for key in met_grid_tools.PROJ_CRS_ATTR_KEYS):
         return None, None
      try:
         import pyproj
      except ImportError:
         met_base.log_message(
            "Found crs_wkt/spatial_ref/proj:* attributes, but pyproj is "
            "not installed in this Python environment, so they can't be "
            "used; continuing on to the next grid-detection method")
         return None, None
      for attrs, source_desc in candidates:
         crs, key = met_grid_tools._crs_from_attrs(pyproj, attrs, source_desc)
         if crs is not None:
            return crs, f"{source_desc} ('{key}')"
      return None, None

   @staticmethod
   def _proj_crs_type(crs):
      """Identify the projection family of a pyproj CRS.
      Returns 'latlon', 'mercator', 'lcc', 'laea', 'pss', or None."""
      try:
         p = crs.to_dict().get('proj', '').lower()
         if p in ('latlong', 'longlat', 'lonlat', 'latlon'):
            return 'latlon'
         if p == 'merc':
            return 'mercator'
         if p == 'lcc':
            return 'lcc'
         if p == 'laea':
            return 'laea'
         if p == 'stere':
            return 'pss'
      except Exception:
         pass
      try:
         method = (crs.coordinate_operation.method_name or '').lower()
         if 'lambert conformal conic' in method:
            return 'lcc'
         if 'lambert azimuthal' in method:
            return 'laea'
         if 'stereographic' in method:
            return 'pss'
         if 'mercator' in method:
            return 'mercator'
      except Exception:
         pass
      try:
         if crs.is_geographic:
            return 'latlon'
      except Exception:
         pass
      return None

   @staticmethod
   def _earth_radius_m_from_crs(crs, attrs=None, default_m=None):
      """Extract earth radius (meters) from a pyproj CRS, with fallbacks."""
      if default_m is None:
         default_m = met_grid_tools.DEFAULT_EARTH_RADIUS_KM * 1000.0
      try:
         d = crs.to_dict()
         if 'R' in d:
            return float(d['R'])
      except Exception:
         pass
      try:
         r = crs.ellipsoid.semi_major_metre
         if r is not None and np.isfinite(r):
            return float(r)
      except Exception:
         pass
      if attrs:
         r = attrs.get('earth_radius') or attrs.get('semi_major_axis')
         if r is not None:
            return float(r)
      return default_m

   @staticmethod
   def _crs_latlon_grid(crs, lat, lon, grid_name):
      """LatLon grid from a geographic (lat/lon) PROJ CRS."""
      return met_grid_tools._cf_latlon_grid({}, lat, lon,
                                             grid_name or 'CrsLatLon')

   @staticmethod
   def _crs_mercator_grid(crs, x, y, lat2d, lon2d, grid_name):
      """Mercator grid from a pyproj CRS."""
      try:
         d = crs.to_dict()
      except Exception:
         d = {}
      r_m = met_grid_tools._earth_radius_m_from_crs(crs)
      attrs = {
         'earth_radius':                   r_m,
         'standard_parallel':              d.get('lat_ts', 0.0),
         'longitude_of_projection_origin': d.get('lon_0',  0.0),
         'false_easting':                  d.get('x_0',    0.0),
         'false_northing':                 d.get('y_0',    0.0),
      }
      return met_grid_tools._cf_mercator_grid(
         attrs, x, y, lat2d, lon2d, grid_name or 'CrsMercator')

   @staticmethod
   def _crs_lcc_grid(crs, x, y, lat2d, lon2d, grid_name):
      """Lambert Conformal Conic grid from a pyproj CRS."""
      try:
         d = crs.to_dict()
      except Exception:
         d = {}
      lat_1 = d.get('lat_1', d.get('lat_0'))
      lat_2 = d.get('lat_2', lat_1)
      lat_0 = d.get('lat_0', lat_1)
      lon_0 = d.get('lon_0')
      if lat_1 is None or lon_0 is None:
         met_base.quit(
            "met_grid_tools.grid_from_crs(): Lambert Conformal Conic CRS "
            "is missing standard parallel or central meridian parameters "
            f"(parsed: {d})")
      r_m = met_grid_tools._earth_radius_m_from_crs(crs, default_m=6371229.0)
      met_base.log_message(
         f"Building a Lambert Conformal grid from CRS: standard "
         f"parallel(s) {lat_1}/{lat_2}, central meridian {lon_0}, "
         f"earth radius {r_m / 1000.0:.3f} km")
      attrs = {
         'standard_parallel':              [float(lat_1), float(lat_2)],
         'latitude_of_projection_origin':  float(lat_0),
         'longitude_of_central_meridian':  float(lon_0),
         'earth_radius':                   r_m,
      }
      return met_grid_tools._cf_lcc_grid(
         attrs, x, y, lat2d, lon2d, grid_name or 'CrsLambertConformal')

   @staticmethod
   def _crs_laea_grid(crs, x, y, lat2d, lon2d, grid_name):
      """Lambert Azimuthal Equal Area grid from a pyproj CRS."""
      try:
         d = crs.to_dict()
      except Exception:
         d = {}
      r_m = met_grid_tools._earth_radius_m_from_crs(crs)
      met_base.log_message(
         f"Building a Lambert Azimuthal Equal Area grid from CRS: "
         f"center lat={d.get('lat_0', 0.0)}, lon={d.get('lon_0', 0.0)}, "
         f"earth radius {r_m / 1000.0:.3f} km")
      attrs = {
         'latitude_of_projection_origin':  d.get('lat_0', 0.0),
         'longitude_of_projection_origin': d.get('lon_0', 0.0),
         'earth_radius':                   r_m,
      }
      return met_grid_tools._cf_laea_grid(
         attrs, x, y, lat2d, lon2d, grid_name or 'CrsLambertAzimuthal')

   @staticmethod
   def _crs_pss_grid(crs, x, y, lat2d, lon2d, grid_name):
      """Polar Stereographic grid from a pyproj CRS."""
      try:
         d = crs.to_dict()
      except Exception:
         d = {}
      lat_0  = d.get('lat_0',  90.0)
      lon_0  = d.get('lon_0',   0.0)
      lat_ts = d.get('lat_ts', lat_0)
      r_m    = met_grid_tools._earth_radius_m_from_crs(crs)
      met_base.log_message(
         f"Building a Polar Stereographic grid from CRS: "
         f"hemisphere={'N' if lat_0 >= 0 else 'S'}, "
         f"central meridian {lon_0}, standard parallel {lat_ts}, "
         f"earth radius {r_m / 1000.0:.3f} km")
      attrs = {
         'latitude_of_projection_origin':           float(lat_0),
         'straight_vertical_longitude_from_pole':   float(lon_0),
         'standard_parallel':                       float(lat_ts),
         'earth_radius':                            r_m,
      }
      return met_grid_tools._cf_pss_grid(
         attrs, x, y, lat2d, lon2d, grid_name or 'CrsPolarStereographic')

   @staticmethod
   def grid_from_crs(crs, source_desc=None, x=None, y=None,
                     lat=None, lon=None, lat2d=None, lon2d=None,
                     grid_name=None):
      """Build a MET grid dict from a pyproj CRS object.

      Returns (grid_dict, flip_row).

      crs          a pyproj.CRS instance
      source_desc  optional human-readable description of where the CRS
                   came from (used in log messages)
      x, y         1D projected coordinate arrays (meters), for projected
                   grid types
      lat, lon     1D coordinate arrays (degrees), for geographic CRS types
      lat2d, lon2d 2D auxiliary lat/lon arrays (degrees), for pin-point
                   or corner coordinates of projected grid types
      grid_name    optional name string for the returned grid dict
      """
      proj_type = met_grid_tools._proj_crs_type(crs)
      label = f"'{crs.name}'" + (f" from {source_desc}" if source_desc else '')
      if proj_type is None:
         met_base.quit(
            f"met_grid_tools.grid_from_crs(): unsupported or unrecognised "
            f"CRS {label}; supported projections: geographic (lat/lon), "
            "Mercator, Lambert Conformal Conic, Lambert Azimuthal Equal "
            "Area, Polar Stereographic")
      dispatch = {
         'latlon':   met_grid_tools._crs_latlon_grid,
         'mercator': met_grid_tools._crs_mercator_grid,
         'lcc':      met_grid_tools._crs_lcc_grid,
         'laea':     met_grid_tools._crs_laea_grid,
         'pss':      met_grid_tools._crs_pss_grid,
      }
      fn = dispatch[proj_type]
      if proj_type == 'latlon':
         return fn(crs, lat, lon, grid_name)
      else:
         return fn(crs, x, y, lat2d, lon2d, grid_name)

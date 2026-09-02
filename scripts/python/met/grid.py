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
   #  curvilinear coordinates -- Lambert Conformal Conic and Polar
   #  Stereographic -- by searching for the projection under which
   #  the data forms a regular Cartesian grid (needs SciPy), and
   #  keeping whichever fits best.
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

      if not candidates:
         met_base.quit(
            "met_grid_tools.derive_grid() -> could not fit either "
            "supported projected grid type (Lambert Conformal Conic, "
            "Polar Stereographic) to this data; it may use an "
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

      else:   # Lambert Conformal
         parts = ['lambert', str(grid['nx']), str(grid['ny']),
                  f"{grid['lat_pin']:.6f}", f"{grid['lon_pin']:.6f}",
                  f"{grid['lon_orient']:.6f}", f"{grid['d_km']:.6f}",
                  f"{grid['r_km']:.6f}", f"{grid['scale_lat_1']:.6f}"]
         if abs(grid['scale_lat_2'] - grid['scale_lat_1']) > 1.0e-9:
            parts.append(f"{grid['scale_lat_2']:.6f}")
         parts.append(grid['hemisphere'])

      return 'set_attr_grid = "' + ' '.join(parts) + '";'

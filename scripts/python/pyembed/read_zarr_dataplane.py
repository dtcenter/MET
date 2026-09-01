########################################################################
#
#  read_zarr_dataplane.py
#
#  Reads a single, deterministic 2D gridded field out of a Zarr store
#  and hands it to MET as an xarray DataArray, following the standard
#  MET Python embedding contract (see met.dataplane and
#  scripts/python/examples/read_ascii_xarray.py).
#
#  Called (via MetZarrDataFile::build_zarr_python_command(), see
#  src/libcode/vx_data2d_zarr/data2d_zarr.cc) with exactly 6 arguments:
#
#     <zarr_store_path> <init_time> <lead_time> <variable> <level> \
#         <set_attr_grid_flag>
#
#     zarr_store_path     path to the Zarr store (a directory)
#     init_time           YYYYMMDD_HHMMSS
#     lead_time           decimal hours, e.g. "6" or "7.5" (NOT HHMMSS)
#     variable            the Zarr data variable name, e.g. "TMP"
#     level               a MET level string; only pressure levels are
#                         currently supported for fields with a
#                         vertical dimension, e.g. "P850"
#     set_attr_grid_flag  "1" if the MET config's set_attr_grid option
#                         is set for this field, "0" otherwise (see
#                         VarInfo::grid_attr())
#
#  Grid support (per MET issue #3340, initial phase). If set_attr_grid
#  is configured, grid detection below is skipped and a placeholder
#  grid matching the data's own shape is returned instead; MET
#  substitutes the real grid and checks the dimensions itself.
#  Otherwise, detection tries the following, in order, using the first
#  one that applies:
#
#     1) a plain latitude/longitude grid, from 1D lat/lon coordinates
#     2) a Lambert Conformal Conic grid, from a CF "grid_mapping"
#        variable (see build_lambert_conformal_grid())
#     3) a Lambert Conformal Conic grid, from a general-purpose CRS
#        attribute -- rioxarray's crs_wkt/spatial_ref, or the GeoZarr
#        proposal's proj:wkt2/proj:projjson/proj:code (see
#        find_proj_crs()). Needs pyproj, imported lazily.
#     4) otherwise, if 2D lat/lon coordinates are present, the
#        projection is recovered numerically (see
#        fit_lambert_conformal_grid()). Needs SciPy, and assumes a
#        fixed spherical earth radius (DEFAULT_EARTH_RADIUS_KM below).
#        Prints a ready-to-use set_attr_grid config line so future
#        runs can skip the fit (see suggest_set_attr_grid()).
#
#  Ensemble data, distributed/cloud-optimized reads, and other
#  projections are out of scope for this initial phase.
#
########################################################################

import sys
import numpy as np
import xarray as xr
import datetime as dt

from met.dataplane import dataplane

########################################################################

def log(msg):
   dataplane.log_message(msg)

########################################################################
#
#  aliases for the dimension/coordinate names this script recognizes
#
########################################################################

LAT_NAMES  = ['latitude', 'lat']
LON_NAMES  = ['longitude', 'lon']
X_NAMES    = ['x', 'projection_x_coordinate']
Y_NAMES    = ['y', 'projection_y_coordinate']
INIT_NAMES = ['time', 'init_time', 'reftime', 'forecast_reference_time']
LEAD_NAMES = ['lead_time', 'step', 'forecast_period']
LEV_NAMES  = ['level', 'isobaricInhPa', 'plev', 'pressure']

# standard MET/GRIB spherical earth radius, in km; used as a fixed
# assumption when numerically fitting a projection (see
# fit_lambert_conformal_grid())
DEFAULT_EARTH_RADIUS_KM = 6371.2

def find_name(da_or_ds, candidates):
   # first name from candidates that is a coordinate or dimension of
   # da_or_ds, or None
   for c in candidates:
      if c in da_or_ds.coords or c in da_or_ds.dims:
         return c
   return None

def format_time_list(values, max_show=10):
   # format datetime64-like coordinate values as YYYYMMDD_HHMMSS
   # strings for error messages, truncated to max_show
   try:
      vals = np.unique(np.asarray(values))
   except Exception:
      vals = np.atleast_1d(np.asarray(values))

   def one(v):
      try:
         s = np.datetime_as_string(np.datetime64(v), unit='s')
         return s.replace('-', '').replace('T', '_').replace(':', '')
      except Exception:
         return str(v)

   shown = [one(v) for v in vals[:max_show]]
   if vals.size > max_show:
      shown.append(f"... and {vals.size - max_show} more")

   return shown

def hours_to_hhmmss(hours):
   # format decimal hours as a signed HHMMSS string, for the 'lead'
   # and 'accum' attrs
   total_sec = int(round(hours * 3600))
   sign = '-' if total_sec < 0 else ''
   total_sec = abs(total_sec)
   hh, rem = divmod(total_sec, 3600)
   mm, ss  = divmod(rem, 60)
   return f"{sign}{hh:02d}{mm:02d}{ss:02d}"

########################################################################
#
#  parse and validate the command line arguments
#
########################################################################

if len(sys.argv) != 7:
   dataplane.quit(
      "read_zarr_dataplane.py -> Must supply exactly 6 arguments: "
      "<zarr_store> <init_time> <lead_time> <variable> <level> "
      "<set_attr_grid_flag> "
      f"(got {len(sys.argv) - 1})")

input_file         = sys.argv[1]
init_time_in       = sys.argv[2]
lead_time_in       = sys.argv[3]
var_name           = sys.argv[4]
level_in           = sys.argv[5]
set_attr_grid_flag = sys.argv[6]

set_attr_grid_specified = (set_attr_grid_flag == '1')

log("Input File:\t" + repr(input_file))
log("Init Time:\t" + repr(init_time_in))
log("Lead Time (hr):" + repr(lead_time_in))
log("Variable:\t" + repr(var_name))
log("Level:\t\t" + repr(level_in))
log("set_attr_grid:\t" + repr(set_attr_grid_specified))

try:
   init_time  = dt.datetime.strptime(init_time_in, '%Y%m%d_%H%M%S')
   lead_hours = float(lead_time_in)
   lead_delta = dt.timedelta(hours=lead_hours)
   valid_time = init_time + lead_delta
except Exception as ex:
   dataplane.quit(
      "read_zarr_dataplane.py -> Unable to parse init time "
      f"'{init_time_in}' (expected YYYYMMDD_HHMMSS) or lead time "
      f"'{lead_time_in}' (expected decimal hours): {ex}")

########################################################################
#
#  open the Zarr store and select the requested variable
#
########################################################################

try:
   ds = xr.open_zarr(input_file, decode_timedelta=True)
except Exception as ex:
   dataplane.quit(
      f"read_zarr_dataplane.py -> Unable to open '{input_file}' as a "
      f"Zarr store: {ex}")

if var_name not in ds.data_vars:
   dataplane.quit(
      f"read_zarr_dataplane.py -> Variable '{var_name}' not found in "
      f"'{input_file}'. Available variables: {sorted(ds.data_vars)}")

da = ds[var_name]

########################################################################
#
#  select init time and lead time
#
########################################################################

init_dim = find_name(da, INIT_NAMES)
lead_dim = find_name(da, LEAD_NAMES)

try:
   if init_dim is not None:
      da = da.sel({init_dim: np.datetime64(init_time)})
except Exception as ex:
   available = format_time_list(da[init_dim].values) if init_dim is not None else []
   dataplane.quit(
      f"read_zarr_dataplane.py -> Init time '{init_time_in}' not present "
      f"for variable '{var_name}' along its '{init_dim}' dimension: {ex}. "
      f"Init times present in the store: {available}")

if lead_dim is not None:
   try:
      # most Zarr archives store lead/forecast-period as timedelta64
      da = da.sel({lead_dim: np.timedelta64(int(round(lead_hours * 3600)), 's')},
                  method='nearest')
   except (TypeError, KeyError):
      try:
         # fall back to a plain numeric (hours) lead coordinate
         da = da.sel({lead_dim: lead_hours}, method='nearest')
      except Exception as ex:
         dataplane.quit(
            f"read_zarr_dataplane.py -> Lead time '{lead_time_in}' not "
            f"present for variable '{var_name}' along its '{lead_dim}' "
            f"dimension: {ex}")

########################################################################
#
#  select the requested level, if the variable has a vertical
#  dimension (only pressure levels are supported in this release)
#
########################################################################

lev_dim = find_name(da, LEV_NAMES)

if lev_dim is not None:
   if not level_in or level_in[0] != 'P':
      dataplane.quit(
         f"read_zarr_dataplane.py -> Variable '{var_name}' has a "
         f"'{lev_dim}' dimension, so the level argument must be a "
         f"pressure level string like 'P850'; got '{level_in}'")

   try:
      lev_val = float(level_in[1:])
   except ValueError:
      dataplane.quit(
         f"read_zarr_dataplane.py -> Unable to parse a numeric pressure "
         f"value out of level '{level_in}'")

   if not bool((da[lev_dim] == lev_val).any()):
      dataplane.quit(
         f"read_zarr_dataplane.py -> Level '{level_in}' ({lev_val}) not "
         f"found in the '{lev_dim}' coordinate for variable '{var_name}'. "
         f"Available levels: {np.atleast_1d(da[lev_dim].values).tolist()}")

   da = da.sel({lev_dim: lev_val})

########################################################################
#
#  drop remaining length-1 dimensions and confirm a 2D field is left
#
########################################################################

da = da.squeeze()

if da.ndim != 2:
   dataplane.quit(
      f"read_zarr_dataplane.py -> After selecting time, lead, and "
      f"level, variable '{var_name}' still has {da.ndim} dimensions "
      f"{da.dims}; this initial release only supports single "
      f"deterministic 2D fields (see MET issue #3340)")

########################################################################
#
#  build the MET grid dictionary from the dataset's CF metadata
#
########################################################################

def build_latlon_grid(da, lat_name, lon_name):
   lat = np.asarray(da[lat_name].values)
   lon = np.asarray(da[lon_name].values)

   if lat.ndim != 1 or lon.ndim != 1:
      dataplane.quit(
         "read_zarr_dataplane.py -> latitude/longitude coordinates must "
         "be 1-dimensional to build a LatLon grid")

   dlat = np.diff(lat)
   dlon = np.diff(lon)

   if lat.size < 2 or lon.size < 2 or \
      not np.allclose(dlat, dlat[0], rtol=1e-3) or \
      not np.allclose(dlon, dlon[0], rtol=1e-3):
      dataplane.quit(
         "read_zarr_dataplane.py -> latitude/longitude coordinates are "
         "not regularly spaced; only regular LatLon grids are supported "
         "in this initial release")

   # lat_ll is whichever end of the (monotonic) coordinate is
   # southernmost
   descending = dlat[0] < 0
   lat_ll = float(lat[-1] if descending else lat[0])

   # load_numpy() in dataplane_from_numpy_array.hpp maps numpy row r
   # to DataPlane row (Ny - 1 - r), so row 0 must be the row farthest
   # from the pin (south); flip when the coordinate is ascending
   flip_row = not descending   # i.e. flip_row == (dlat[0] > 0)

   grid = {
      'type':      'LatLon',
      'name':      'ZarrLatLon',
      'lat_ll':    lat_ll,
      'lon_ll':    float(lon[0]),
      'delta_lat': float(abs(dlat[0])),
      'delta_lon': float(abs(dlon[0])),
      'Nlat':      int(lat.size),
      'Nlon':      int(lon.size),
   }

   return grid, flip_row


def _lambert_conformal_grid_from_params(ds, da, x_name, y_name, grid_name,
                                        scale_lat_1, scale_lat_2, lat_origin,
                                        lon_orient, earth_radius_m):
   # shared grid-dictionary builder for both Lambert Conformal
   # detection paths (CF grid_mapping and general-purpose CRS); they
   # differ only in where the projection parameters came from
   x = np.asarray(da[x_name].values)
   y = np.asarray(da[y_name].values)

   if x.ndim != 1 or y.ndim != 1:
      dataplane.quit(
         "read_zarr_dataplane.py -> projected x/y coordinates must be "
         "1-dimensional to build a Lambert Conformal grid")

   dx = np.diff(x)
   dy = np.diff(y)

   if x.size < 2 or y.size < 2 or \
      not np.allclose(dx, dx[0], rtol=1e-3) or \
      not np.allclose(dy, dy[0], rtol=1e-3):
      dataplane.quit(
         "read_zarr_dataplane.py -> projected x/y coordinates are not "
         "regularly spaced")

   # read the pin point off the 2D lat/lon auxiliary coordinates
   # directly, rather than re-deriving it through a reprojection
   lat2d_name = find_name(ds, LAT_NAMES)
   lon2d_name = find_name(ds, LON_NAMES)

   if lat2d_name is None or lon2d_name is None:
      dataplane.quit(
         "read_zarr_dataplane.py -> a Lambert Conformal grid requires 2D "
         "latitude/longitude auxiliary coordinate variables; none were "
         "found in the store")

   # pin point (x_pin, y_pin) = (0, 0) is the grid's south-west corner;
   # whichever row has the smallest y is the pin row
   descending_y = dy[0] < 0
   y_idx = -1 if descending_y else 0

   lat2d = np.asarray(ds[lat2d_name].values)
   lon2d = np.asarray(ds[lon2d_name].values)

   if lat2d.ndim != 2 or lon2d.ndim != 2:
      dataplane.quit(
         "read_zarr_dataplane.py -> expected 2D latitude/longitude "
         "auxiliary coordinate arrays for a Lambert Conformal grid")

   # see the row-orientation note in build_latlon_grid() above
   flip_row = not descending_y   # i.e. flip_row == (dy[0] > 0)

   grid = {
      'type':        'Lambert Conformal',
      'name':        grid_name,
      'hemisphere':  'N' if lat_origin >= 0 else 'S',
      'scale_lat_1': float(scale_lat_1),
      'scale_lat_2': float(scale_lat_2),
      'lat_pin':     float(lat2d[y_idx, 0]),
      'lon_pin':     float(lon2d[y_idx, 0]),
      'x_pin':       0.0,
      'y_pin':       0.0,
      'lon_orient':  float(lon_orient),
      'd_km':        float(abs(dx[0])) / 1000.0,
      'r_km':        float(earth_radius_m) / 1000.0,
      'nx':          int(x.size),
      'ny':          int(y.size),
   }

   return grid, flip_row


def build_lambert_conformal_grid(ds, da, x_name, y_name, cf_var):
   if 'standard_parallel' not in cf_var.attrs or \
      'longitude_of_central_meridian' not in cf_var.attrs:
      dataplane.quit(
         f"read_zarr_dataplane.py -> the '{cf_var.name}' grid_mapping "
         "variable is missing required lambert_conformal_conic "
         "attributes (standard_parallel, longitude_of_central_meridian)")

   std_parallel = np.atleast_1d(cf_var.attrs['standard_parallel']).astype(float)
   lat_origin = float(cf_var.attrs.get('latitude_of_projection_origin',
                                       std_parallel[0]))
   earth_radius_m = float(cf_var.attrs.get(
      'earth_radius', cf_var.attrs.get('semi_major_axis', 6371229.0)))

   return _lambert_conformal_grid_from_params(
      ds, da, x_name, y_name, 'ZarrLambertConformal',
      scale_lat_1=std_parallel[0], scale_lat_2=std_parallel[-1],
      lat_origin=lat_origin,
      lon_orient=cf_var.attrs['longitude_of_central_meridian'],
      earth_radius_m=earth_radius_m)


########################################################################
#
#  general-purpose CRS metadata support (step 3: rioxarray / GeoZarr)
#
########################################################################

# rioxarray's de facto convention first, then the GeoZarr proposal's
# attributes, most to least completely specified
PROJ_CRS_ATTR_KEYS = ['crs_wkt', 'spatial_ref', 'proj:wkt2', 'proj:projjson',
                      'proj:code']


def _crs_from_attrs(pyproj_mod, attrs, source_desc):
   # try each key in PROJ_CRS_ATTR_KEYS and return the first that
   # parses as a pyproj CRS, plus which key it came from; a malformed
   # attribute logs a warning rather than blocking a later fallback
   for key in PROJ_CRS_ATTR_KEYS:
      if key not in attrs:
         continue
      value = attrs[key]
      try:
         if key == 'proj:projjson':
            crs = pyproj_mod.CRS.from_json_dict(value) \
               if isinstance(value, dict) else pyproj_mod.CRS.from_json(value)
         else:
            crs = pyproj_mod.CRS.from_user_input(value)
         return crs, key
      except Exception as ex:
         log(f"Found a '{key}' attribute in {source_desc} but could not "
             f"parse it as a CRS: {ex}")

   return None, None


def find_proj_crs(ds, da):
   # look for CRS metadata under the rioxarray or GeoZarr conventions:
   # a grid_mapping variable, a bare 'spatial_ref' coordinate, or
   # proj:* attributes on the data variable or dataset. Returns
   # (pyproj.CRS, source description) for the first match, or
   # (None, None). pyproj is imported lazily so it stays optional.

   candidates = []

   grid_mapping_name = da.attrs.get('grid_mapping')
   if grid_mapping_name and grid_mapping_name in ds.variables:
      candidates.append((ds[grid_mapping_name].attrs,
                          f"the '{grid_mapping_name}' grid_mapping variable"))

   if 'spatial_ref' in ds.variables and grid_mapping_name != 'spatial_ref':
      candidates.append((ds['spatial_ref'].attrs,
                          "the 'spatial_ref' coordinate variable"))

   candidates.append((da.attrs, f"the '{da.name}' variable's attributes"))
   candidates.append((ds.attrs, "the dataset's global attributes"))

   if not any(key in attrs for attrs, _ in candidates
              for key in PROJ_CRS_ATTR_KEYS):
      return None, None

   try:
      import pyproj
   except ImportError:
      log("Found crs_wkt/spatial_ref/proj:* attributes, but pyproj is "
          "not installed in this Python environment, so they can't be "
          "used; continuing on to the next grid-detection method")
      return None, None

   for attrs, source_desc in candidates:
      crs, key = _crs_from_attrs(pyproj, attrs, source_desc)
      if crs is not None:
         return crs, f"{source_desc} ('{key}')"

   return None, None


def _is_lambert_conformal(crs):
   try:
      if crs.to_dict().get('proj') == 'lcc':
         return True
   except Exception:
      pass
   try:
      if crs.coordinate_operation is not None:
         return 'lambert conformal conic' in \
            (crs.coordinate_operation.method_name or '').lower()
   except Exception:
      pass
   return False


def _lcc_params_from_crs(crs):
   # extract LCC parameters from a pyproj CRS in proj4-style terms;
   # lat_2 defaults to lat_1 (tangent, single-standard-parallel case)
   d = crs.to_dict()

   lat_1 = d.get('lat_1', d.get('lat_0'))
   lat_2 = d.get('lat_2', lat_1)
   lat_0 = d.get('lat_0', lat_1)
   lon_0 = d.get('lon_0')

   if lat_1 is None or lon_0 is None:
      dataplane.quit(
         "read_zarr_dataplane.py -> found a Lambert Conformal Conic CRS "
         "but could not read its standard parallel / central meridian "
         f"parameters from it (parsed parameters: {d})")

   # prefer an explicit spherical radius ('R'), else the ellipsoid's
   # semi-major axis, else the standard MET/GRIB sphere
   earth_radius_m = d.get('R')
   if earth_radius_m is None:
      try:
         earth_radius_m = crs.ellipsoid.semi_major_metre
      except Exception:
         earth_radius_m = None
   if earth_radius_m is None:
      earth_radius_m = 6371229.0

   return float(lat_1), float(lat_2), float(lat_0), float(lon_0), \
      float(earth_radius_m)


def build_lambert_conformal_grid_from_crs(ds, da, x_name, y_name, crs,
                                          source_desc):
   lat_1, lat_2, lat_0, lon_0, earth_radius_m = _lcc_params_from_crs(crs)

   log(f"Building a Lambert Conformal grid from CRS metadata found in "
       f"{source_desc}: standard parallel(s) {lat_1}/{lat_2}, central "
       f"meridian {lon_0}, earth radius {earth_radius_m / 1000.0:.3f} km")

   return _lambert_conformal_grid_from_params(
      ds, da, x_name, y_name, 'ZarrLambertConformalCRS',
      scale_lat_1=lat_1, scale_lat_2=lat_2, lat_origin=lat_0,
      lon_orient=lon_0, earth_radius_m=earth_radius_m)


def _lcc_forward(lat_deg, lon_deg, n, lon0_deg, r_km):
   # spherical, tangent-case Lambert Conformal Conic forward
   # projection (Snyder 1987); n is the cone constant, tangent
   # standard parallel is asin(n)
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


def _lcc_grid_regularity_sse(params, lat2d, lon2d):
   # objective for the fit below: project with trial (n, lon0) and
   # measure how far the result is from a regular Cartesian grid;
   # zero at the true projection parameters
   n, lon0_deg = params

   if not (-0.999 < n < 0.999) or abs(n) < 1e-6:
      return 1.0e12

   x, y = _lcc_forward(lat2d, lon2d, n, lon0_deg, DEFAULT_EARTH_RADIUS_KM)

   ny, nx = lat2d.shape
   ii, jj = np.mgrid[0:ny, 0:nx]

   A_x = np.column_stack([np.ones(jj.size), jj.ravel().astype(float)])
   A_y = np.column_stack([np.ones(ii.size), ii.ravel().astype(float)])

   coef_x, *_ = np.linalg.lstsq(A_x, x.ravel(), rcond=None)
   coef_y, *_ = np.linalg.lstsq(A_y, y.ravel(), rcond=None)

   return float(np.sum((x.ravel() - A_x @ coef_x) ** 2) +
               np.sum((y.ravel() - A_y @ coef_y) ** 2))


def fit_lambert_conformal_grid(da, lat_name, lon_name):
   # recover an LCC grid definition from a 2D lat/lon coordinate field
   # with no CF grid_mapping (or other CRS) metadata: fit the cone
   # constant and central meridian by searching for the projection
   # under which the data forms a regular Cartesian grid, then read
   # the pin point off the real coordinate data and the spacing off
   # the fit

   try:
      from scipy.optimize import minimize
   except ImportError:
      dataplane.quit(
         "read_zarr_dataplane.py -> this variable has 2D latitude/"
         "longitude coordinates with no CF grid_mapping metadata; "
         "recovering the projection numerically requires SciPy, which "
         "is not installed in this Python environment")

   lat2d = np.asarray(da[lat_name].values, dtype=float)
   lon2d = np.asarray(da[lon_name].values, dtype=float)

   if lat2d.ndim != 2 or lon2d.ndim != 2 or lat2d.shape != lon2d.shape:
      dataplane.quit(
         "read_zarr_dataplane.py -> expected matching 2D latitude/"
         "longitude coordinate arrays to fit a projection")

   ny, nx = lat2d.shape

   if ny < 3 or nx < 3:
      dataplane.quit(
         "read_zarr_dataplane.py -> grid is too small "
         f"({ny} x {nx}) to reliably fit a projection")

   n0     = float(np.sin(np.radians(np.nanmedian(lat2d))))
   lon0_0 = float(np.nanmedian(lon2d))

   log("No CF grid_mapping metadata found; fitting a Lambert "
       f"Conformal projection to the {ny}x{nx} 2D lat/lon field "
       f"instead (assuming a {DEFAULT_EARTH_RADIUS_KM} km spherical "
       "earth radius, the standard MET/GRIB convention)")

   result = minimize(
      _lcc_grid_regularity_sse, x0=[n0, lon0_0], args=(lat2d, lon2d),
      method='Nelder-Mead',
      options={'xatol': 1e-10, 'fatol': 1e-8, 'maxiter': 20000})

   n_fit, lon0_fit = result.x

   x, y = _lcc_forward(lat2d, lon2d, n_fit, lon0_fit, DEFAULT_EARTH_RADIUS_KM)
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

   scale_lat = float(np.degrees(np.arcsin(np.clip(n_fit, -1.0, 1.0))))

   log(f"Lambert Conformal fit: standard parallel "
       f"{scale_lat:.4f} deg, central meridian {lon0_fit % 360:.4f} "
       f"deg, dx={dx:.4f} km, dy={dy:.4f} km, "
       f"max residual={max_err_km:.4f} km")

   # a large residual means this grid likely isn't a Lambert
   # Conformal projection (or needs a different earth radius)
   if not np.isfinite(max_err_km) or not result.success or \
      max_err_km > 0.1 * cell_km:
      dataplane.quit(
         "read_zarr_dataplane.py -> could not fit a Lambert Conformal "
         f"projection to this grid (max residual {max_err_km:.4f} km "
         f"against a {cell_km:.4f} km grid cell); this grid may not "
         "be a Lambert Conformal projection, or may need a different "
         f"earth radius than the default {DEFAULT_EARTH_RADIUS_KM} km")

   if abs(dx) < 1e-9 or abs(dy) < 1e-9 or \
      abs(abs(dx) - abs(dy)) > 0.05 * cell_km:
      dataplane.quit(
         "read_zarr_dataplane.py -> fitted grid spacing is not square "
         f"(dx={dx:.4f} km, dy={dy:.4f} km); non-square Lambert "
         "Conformal grids are not supported")

   # same pin-point and row-orientation convention as
   # _lambert_conformal_grid_from_params() above
   descending_y = dy < 0
   y_idx = -1 if descending_y else 0
   flip_row = not descending_y   # i.e. flip_row == (dy > 0)

   grid = {
      'type':        'Lambert Conformal',
      'name':        'ZarrLambertConformalFit',
      'hemisphere':  'N' if scale_lat >= 0 else 'S',
      'scale_lat_1': scale_lat,
      'scale_lat_2': scale_lat,
      'lat_pin':     float(lat2d[y_idx, 0]),
      'lon_pin':     float(lon2d[y_idx, 0]),
      'x_pin':       0.0,
      'y_pin':       0.0,
      'lon_orient':  float(lon0_fit % 360.0),
      'd_km':        cell_km,
      'r_km':        DEFAULT_EARTH_RADIUS_KM,
      'nx':          int(nx),
      'ny':          int(ny),
   }

   return grid, flip_row


def detect_flip_row(da, lat_name):
   # cheaply determine whether the array needs its rows reversed
   # (see the row-orientation note in build_latlon_grid()), by
   # comparing the first and last row of the latitude coordinate;
   # works for both 1D and 2D latitude coordinates and needs no grid
   # definition. Used even when set_attr_grid overrides the grid,
   # since it never reorders the array data itself.
   lat = np.asarray(da[lat_name].values)
   first, last = (float(lat[0]), float(lat[-1])) if lat.ndim == 1 \
      else (float(lat[0, 0]), float(lat[-1, 0]))
   return first < last


def build_placeholder_grid(ny, nx):
   # minimal, throwaway LatLon grid used only to satisfy
   # dataplane_from_numpy_array.cc's dimension check; geolocation is
   # meaningless since MET replaces this with the real set_attr_grid
   # grid in Met2dDataFile::process_data_plane()
   return {
      'type':      'LatLon',
      'name':      'ZarrPlaceholder',
      'lat_ll':    0.0,
      'lon_ll':    0.0,
      'delta_lat': 1.0,
      'delta_lon': 1.0,
      'Nlat':      int(ny),
      'Nlon':      int(nx),
   }


def suggest_set_attr_grid(grid):
   # format a fitted LCC grid dictionary as a ready-to-use
   # set_attr_grid config line (grid-spec string format, see
   # docs/Users_Guide/appendixB.rst); no unit conversion needed since
   # find_grid_by_name.cc's parse_lambert_grid() uses the same
   # lat_pin/lon_pin/(x_pin,y_pin)=(0,0) convention as this script
   parts = ['lambert', str(grid['nx']), str(grid['ny']),
            f"{grid['lat_pin']:.6f}", f"{grid['lon_pin']:.6f}",
            f"{grid['lon_orient']:.6f}", f"{grid['d_km']:.6f}",
            f"{grid['r_km']:.6f}", f"{grid['scale_lat_1']:.6f}"]
   if abs(grid['scale_lat_2'] - grid['scale_lat_1']) > 1.0e-9:
      parts.append(f"{grid['scale_lat_2']:.6f}")
   parts.append(grid['hemisphere'])
   return 'set_attr_grid = "' + ' '.join(parts) + '";'


lat_name = find_name(da, LAT_NAMES)
lon_name = find_name(da, LON_NAMES)

grid = None
flip_row = False

ny, nx = da.shape

if set_attr_grid_specified:

   # MET will discard whatever grid this script returns and
   # substitute the configured one, checking only that the dimensions
   # match, so skip automatic grid detection; row orientation still
   # needs to be right, since set_attr_grid doesn't reorder the array
   log("set_attr_grid is configured for this field; skipping automatic "
       "grid detection and returning a placeholder grid. MET will "
       "apply the configured set_attr_grid override and confirm its "
       f"dimensions match this variable's data dimensions ({nx} x {ny}, "
       "Nx x Ny).")

   if lat_name is not None:
      flip_row = detect_flip_row(da, lat_name)
   else:
      flip_row = False
      log("Warning: no latitude coordinate was found to confirm row "
          "orientation; reading the data in its native row order. If "
          "the field looks flipped north/south after applying "
          "set_attr_grid, this Zarr store's data may need to be read "
          "differently.")

   grid = build_placeholder_grid(ny, nx)

elif lat_name is not None and lon_name is not None and \
     getattr(da[lat_name], 'ndim', 2) == 1:

   grid, flip_row = build_latlon_grid(da, lat_name, lon_name)

else:
   # steps 2-4 (CF grid_mapping -> CRS attributes -> numeric fit),
   # each attempted only if the ones before it found nothing usable
   x_name = find_name(da, X_NAMES)
   y_name = find_name(da, Y_NAMES)

   grid_mapping_name = da.attrs.get('grid_mapping')
   cf_var = ds[grid_mapping_name] \
      if grid_mapping_name and grid_mapping_name in ds.variables else None
   cf_mapping_type = cf_var.attrs.get('grid_mapping_name') if cf_var is not None else None

   has_cf_lcc_attrs = (
      cf_mapping_type == 'lambert_conformal_conic' and
      'standard_parallel' in cf_var.attrs and
      'longitude_of_central_meridian' in cf_var.attrs)

   #
   #  step 2: CF grid_mapping convention
   #
   if has_cf_lcc_attrs and x_name is not None and y_name is not None:

      grid, flip_row = build_lambert_conformal_grid(
         ds, da, x_name, y_name, cf_var)

   else:
      #
      #  step 3: general-purpose CRS attributes (rioxarray / GeoZarr)
      #
      crs, crs_source = find_proj_crs(ds, da)

      if crs is not None and x_name is not None and y_name is not None:

         if not _is_lambert_conformal(crs):
            dataplane.quit(
               f"read_zarr_dataplane.py -> found CRS metadata in "
               f"{crs_source} ('{crs.name}'), but its projection is not "
               "Lambert Conformal Conic; only Lambert Conformal and "
               "plain latitude/longitude grids are supported in this "
               "initial release")

         grid, flip_row = build_lambert_conformal_grid_from_crs(
            ds, da, x_name, y_name, crs, crs_source)

      elif crs is not None:
         dataplane.quit(
            f"read_zarr_dataplane.py -> found CRS metadata in "
            f"{crs_source}, but no projected x/y coordinates were found "
            f"alongside it (checked for: {X_NAMES} / {Y_NAMES})")

      #
      #  step 4: numeric fit, the last resort
      #
      elif lat_name is not None and lon_name is not None and \
           getattr(da[lat_name], 'ndim', None) == 2:

         grid, flip_row = fit_lambert_conformal_grid(da, lat_name, lon_name)

         log("This grid was derived automatically from the 2D latitude/"
             "longitude coordinates because set_attr_grid is not "
             "configured for this field, and no CF grid_mapping or "
             "recognized CRS metadata was found.")
         log("Add the following configuration option to skip this fit "
             "in future runs:")
         log("\t" + suggest_set_attr_grid(grid))

      else:
         reason = ""
         if cf_mapping_type is not None and cf_mapping_type != 'lambert_conformal_conic':
            reason = (f" A CF grid_mapping variable was found, but its "
                      f"grid_mapping_name ('{cf_mapping_type}') is not "
                      f"yet supported -- only 'lambert_conformal_conic' "
                      f"is.")
         dataplane.quit(
            f"read_zarr_dataplane.py -> could not determine the grid for "
            f"variable '{var_name}'.{reason} Expected either 1D "
            f"latitude/longitude coordinates, CF grid_mapping / "
            f"rioxarray / GeoZarr CRS metadata referencing a supported "
            f"projection, or 2D latitude/longitude coordinates a "
            f"projection can be fit to")

########################################################################
#
#  pull out the 2D array, orient it to match the grid dictionary, and
#  build the attrs + met_data DataArray MET expects
#
########################################################################

met_data_2d = np.asarray(da.values)

if flip_row:
   met_data_2d = met_data_2d[::-1, :]

units     = da.attrs.get('units', 'NA')
long_name = da.attrs.get('long_name', var_name)

attrs = dataplane.set_dataplane_attrs(
   var_name,
   valid_time.strftime('%Y%m%d_%H%M%S'),
   init_time_in,
   hours_to_hhmmss(lead_hours),
   '000000',
   level_in,
   units,
   grid,
   long_name=long_name)

log("Attributes:\t" + repr(attrs))
log("Data Shape:\t" + repr(met_data_2d.shape))

met_data = xr.DataArray(met_data_2d, attrs=attrs)

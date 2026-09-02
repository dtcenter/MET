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
#  Grid support (per MET issue #3340). If set_attr_grid is configured,
#  grid detection below is skipped and a placeholder grid matching the
#  data's own shape is returned instead; MET substitutes the real grid
#  and checks the dimensions itself. Otherwise, detection tries the
#  following, in order, using the first one that applies:
#
#     1) from a CF "grid_mapping" variable: LatLon, Mercator, Lambert
#        Conformal Conic, Lambert Azimuthal Equal Area, Polar
#        Stereographic, or Rotated LatLon (via
#        met_grid_tools.grid_from_cf_mapping())
#     2) from a general-purpose CRS attribute -- rioxarray's
#        crs_wkt/spatial_ref, or the GeoZarr proposal's
#        proj:wkt2/proj:projjson/proj:code -- supporting the same
#        projection families as step 1 (via
#        met_grid_tools.find_proj_crs() + grid_from_crs()). Needs
#        pyproj, imported lazily.
#     3) from 1D lat/lon coordinates: a LatLon, Gaussian, or Mercator
#        grid, whichever the latitude coordinate's spacing matches
#        (see met.grid's met_grid_tools.derive_grid())
#     4) otherwise, if 2D lat/lon coordinates are present: Lambert
#        Conformal Conic, Polar Stereographic, Lambert Azimuthal Equal
#        Area, or Rotated LatLon, whichever fits best (again via
#        met.grid's derive_grid()). Needs SciPy. Either of met.grid's
#        grid detection methods (3 or 4) also logs a ready-to-use
#        set_attr_grid config line so future runs can skip it.
#
########################################################################

import sys
import numpy as np
import xarray as xr
import datetime as dt

from met.dataplane import dataplane
from met.grid import met_grid_tools

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
RLAT_NAMES = ['rlat', 'grid_latitude',  'rlatitude']
RLON_NAMES = ['rlon', 'grid_longitude', 'rlongitude']
X_NAMES    = ['x', 'projection_x_coordinate']
Y_NAMES    = ['y', 'projection_y_coordinate']
INIT_NAMES = ['time', 'init_time', 'reftime', 'forecast_reference_time']
LEAD_NAMES = ['lead_time', 'step', 'forecast_period']
LEV_NAMES  = ['level', 'isobaricInhPa', 'plev', 'pressure']

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
#  build the MET grid dictionary from the dataset's CF/CRS metadata
#  (all projection logic lives in met.grid.met_grid_tools)
#
########################################################################


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


lat_name = find_name(da, LAT_NAMES)
lon_name = find_name(da, LON_NAMES)

grid = None
flip_row = False
flip_col = False

ny, nx = da.shape

if set_attr_grid_specified:

   # MET will discard whatever grid this script returns and
   # substitute the configured one, checking only that the dimensions
   # match, so skip automatic grid detection; row orientation still
   # needs to be right, since set_attr_grid doesn't reorder the array.
   # Column order is only ever a concern for a Gaussian target grid
   # (see met.grid); this shortcut can't tell what type was
   # configured, so it doesn't attempt a flip_col here.
   log("set_attr_grid is configured for this field; skipping automatic "
       "grid detection and returning a placeholder grid. MET will "
       "apply the configured set_attr_grid override and confirm its "
       f"dimensions match this variable's data dimensions ({nx} x {ny}, "
       "Nx x Ny).")

   if lat_name is not None:
      flip_row = met_grid_tools.detect_flip_row(da[lat_name].values)
   else:
      flip_row = False
      log("Warning: no latitude coordinate was found to confirm row "
          "orientation; reading the data in its native row order. If "
          "the field looks flipped north/south after applying "
          "set_attr_grid, this Zarr store's data may need to be read "
          "differently.")

   grid = build_placeholder_grid(ny, nx)

else:
   # steps 1-4: CF grid_mapping -> CRS attributes -> 1D lat/lon -> 2D
   # numeric fit; each attempted only if the ones before it found nothing
   x_name = find_name(da, X_NAMES)
   y_name = find_name(da, Y_NAMES)

   grid_mapping_name = da.attrs.get('grid_mapping')
   cf_var = ds[grid_mapping_name] \
      if grid_mapping_name and grid_mapping_name in ds.variables else None
   cf_mapping_type = cf_var.attrs.get('grid_mapping_name') \
      if cf_var is not None else None

   # projection types that grid_from_cf_mapping() can handle
   SUPPORTED_CF_TYPES = {
      'latitude_longitude', 'mercator', 'lambert_conformal_conic',
      'lambert_azimuthal_equal_area', 'polar_stereographic',
      'rotated_latitude_longitude',
   }

   #
   #  step 1: CF grid_mapping convention (most authoritative when present)
   #
   if cf_mapping_type in SUPPORTED_CF_TYPES:

      if cf_mapping_type == 'rotated_latitude_longitude':
         # rotated lat/lon coordinates may use non-standard names
         rlat_name = find_name(da, RLAT_NAMES)
         rlon_name = find_name(da, RLON_NAMES)
         if rlat_name is None or rlon_name is None:
            dataplane.quit(
               f"read_zarr_dataplane.py -> CF grid_mapping type is "
               f"'rotated_latitude_longitude' but no rotated lat/lon "
               f"coordinate variables were found (checked for: "
               f"{RLAT_NAMES} / {RLON_NAMES})")
         grid, flip_row = met_grid_tools.grid_from_cf_mapping(
            cf_mapping_type, cf_var.attrs,
            lat=np.asarray(da[rlat_name].values),
            lon=np.asarray(da[rlon_name].values),
            grid_name='ZarrRotatedLatLon')

      elif cf_mapping_type == 'latitude_longitude':
         if lat_name is None or lon_name is None:
            dataplane.quit(
               "read_zarr_dataplane.py -> CF grid_mapping type is "
               "'latitude_longitude' but no lat/lon coordinate variables "
               f"were found (checked for: {LAT_NAMES} / {LON_NAMES})")
         grid, flip_row = met_grid_tools.grid_from_cf_mapping(
            cf_mapping_type, cf_var.attrs,
            lat=np.asarray(da[lat_name].values),
            lon=np.asarray(da[lon_name].values),
            grid_name='ZarrLatLon')

      else:
         # projected types need x/y coordinates and 2D lat/lon for
         # pin-point / corner coordinates
         if x_name is None or y_name is None:
            dataplane.quit(
               f"read_zarr_dataplane.py -> CF grid_mapping type is "
               f"'{cf_mapping_type}' but no projected x/y coordinate "
               f"variables were found (checked for: "
               f"{X_NAMES} / {Y_NAMES})")
         lat2d_name = find_name(ds, LAT_NAMES)
         lon2d_name = find_name(ds, LON_NAMES)
         lat2d = np.asarray(ds[lat2d_name].values) if lat2d_name else None
         lon2d = np.asarray(ds[lon2d_name].values) if lon2d_name else None
         grid, flip_row = met_grid_tools.grid_from_cf_mapping(
            cf_mapping_type, cf_var.attrs,
            x=np.asarray(da[x_name].values),
            y=np.asarray(da[y_name].values),
            lat2d=lat2d, lon2d=lon2d,
            grid_name='Zarr' + cf_mapping_type.replace('_', ' ')
                                               .title().replace(' ', ''))

   else:
      #
      #  step 2: general-purpose CRS attributes (rioxarray / GeoZarr)
      #
      crs_candidates = []
      if grid_mapping_name and grid_mapping_name in ds.variables:
         crs_candidates.append(
            (ds[grid_mapping_name].attrs,
             f"the '{grid_mapping_name}' grid_mapping variable"))
      if 'spatial_ref' in ds.variables and \
         grid_mapping_name != 'spatial_ref':
         crs_candidates.append(
            (ds['spatial_ref'].attrs,
             "the 'spatial_ref' coordinate variable"))
      crs_candidates.append(
         (da.attrs, f"the '{da.name}' variable's attributes"))
      crs_candidates.append(
         (ds.attrs, "the dataset's global attributes"))

      crs, crs_source = met_grid_tools.find_proj_crs(crs_candidates)

      if crs is not None:
         if x_name is None or y_name is None:
            dataplane.quit(
               f"read_zarr_dataplane.py -> found CRS metadata in "
               f"{crs_source}, but no projected x/y coordinates were "
               f"found alongside it (checked for: "
               f"{X_NAMES} / {Y_NAMES})")
         lat2d_name = find_name(ds, LAT_NAMES)
         lon2d_name = find_name(ds, LON_NAMES)
         lat2d = np.asarray(ds[lat2d_name].values) if lat2d_name else None
         lon2d = np.asarray(ds[lon2d_name].values) if lon2d_name else None
         grid, flip_row = met_grid_tools.grid_from_crs(
            crs, crs_source,
            x=np.asarray(da[x_name].values),
            y=np.asarray(da[y_name].values),
            lat2d=lat2d, lon2d=lon2d)

      #
      #  step 3: 1D lat/lon coordinates
      #
      elif lat_name is not None and lon_name is not None and \
           getattr(da[lat_name], 'ndim', 2) == 1:

         grid, flip_row, flip_col = met_grid_tools.derive_grid(
            da[lat_name].values, da[lon_name].values)

      #
      #  step 4: 2D lat/lon numeric fit, the last resort
      #
      elif lat_name is not None and lon_name is not None and \
           getattr(da[lat_name], 'ndim', None) == 2:

         log("No CF grid_mapping or recognized CRS metadata was found; "
             "falling back to fitting a projection numerically from the "
             "2D latitude/longitude coordinates")

         grid, flip_row, flip_col = met_grid_tools.derive_grid(
            da[lat_name].values, da[lon_name].values)

      else:
         reason = ""
         if cf_mapping_type is not None and \
            cf_mapping_type not in SUPPORTED_CF_TYPES:
            reason = (f" A CF grid_mapping variable was found, but its "
                      f"grid_mapping_name ('{cf_mapping_type}') is not "
                      f"supported.")
         dataplane.quit(
            f"read_zarr_dataplane.py -> could not determine the grid for "
            f"variable '{var_name}'.{reason} Expected either a CF "
            f"grid_mapping variable, rioxarray / GeoZarr CRS metadata, "
            f"1D latitude/longitude coordinates, or 2D latitude/longitude "
            f"coordinates a projection can be fit to")

########################################################################
#
#  pull out the 2D array, orient it to match the grid dictionary, and
#  build the attrs + met_data DataArray MET expects
#
########################################################################

met_data_2d = np.asarray(da.values)

if flip_row:
   met_data_2d = met_data_2d[::-1, :]

if flip_col:
   met_data_2d = met_data_2d[:, ::-1]

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

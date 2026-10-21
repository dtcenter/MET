# UGRID interpolation verification

Independent Python checks of the unstructured grid (UGRID) interpolation
in Point-Stat and smoothing in Grid-Stat (MET #3334). Each script
recomputes the MET results from the raw MPAS test data with numpy/scipy
and compares them.

| Script | Unit test (`unit_ugrid.xml`) | Config (`internal/test_unit/config`) | Checks |
|---|---|---|---|
| `check_ugrid_point_stat.py` | `point_stat_ugrid_mpas_npts` | `PointStatConfig_ugrid_mpas_npts` | MPR forecast values for NEAREST, UW_MEAN, DW_MEAN, AW_MEAN, MIN, MAX, MEDIAN, BEST, BILIN |
| `check_ugrid_grid_stat_smooth.py` | `grid_stat_ugrid_mpas_smooth` | `GridStatConfig_ugrid_mpas_smooth` | SL1L2 TOTAL, FBAR, OBAR, FFBAR for NEAREST, UW_MEAN, AW_MEAN, MIN, MAX, MEDIAN |
| `verify_ugrid_interp.sh` | both | | runs both scripts on the unit test output |

The negative tests (`point_stat_ugrid_mpas_BAD_INTERP`,
`point_stat_ugrid_mpas_BAD_TOPO_INTERP`, `grid_stat_ugrid_mpas_BAD_SMOOTH`)
are verified by `unit_ugrid.xml` itself (return code 1 and no output).

## Usage

Run `unit_ugrid.xml`, then:

```bash
MET_TEST_INPUT=/path/to/unit_test MET_TEST_OUTPUT=/path/to/unit_test_output \
  internal/test_unit/python/ugrid/verify_ugrid_interp.sh
```

`MET_PYTHON_EXE` selects the Python interpreter (default `python3`); it
needs numpy, netCDF4 and scipy. Each script exits with 1 if a value
differs from the reference by more than `--tol` (default 1e-3).

The command line arguments are validated by `ugrid_check_util.py` before
any file is opened (exit status 2 if invalid):

- Each input file must be an existing regular file under `MET_TEST_INPUT`
  or `MET_TEST_OUTPUT` (the current directory if neither is set). Paths
  are resolved with `os.path.realpath()`, so `..` and symbolic links can
  not point outside of them.
- The STAT file must end with `.stat` and the NetCDF files with `.nc` or
  `.nc4`.
- Variable names may contain only letters, digits, `_`, `.` and `-`.

## Notes

- `PointStatConfig_ugrid_mpas_npts` sets `ugrid_max_distance_km = 20`.
  The reduced MPAS mesh covers only part of the observation domain, and
  without the limit most observations are matched to faces far outside
  the mesh. With `vld_thresh = 1.0`, an observation is kept only if all N
  faces are within the limit, so the reference does not need to apply it.
- DW_MEAN differs by up to about 1e-4 K because MET uses the KD-tree
  (chord) distances and single precision data.
- For the 3-D observation field (`theta`) in the Grid-Stat check, the
  vertical index is chosen by matching the unsmoothed (NEAREST) OBAR, or
  set with `--obs-level`.

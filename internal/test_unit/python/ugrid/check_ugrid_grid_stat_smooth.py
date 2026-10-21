#!/usr/bin/env python3
"""
Verify the Grid-Stat smoothing on the unstructured grid (UGRID) by
recomputing the smoothed forecast and observation fields from the raw MPAS
data and comparing the SL1L2 statistics (TOTAL, FBAR, OBAR, FFBAR).

Each face is smoothed using its N closest faces (scipy cKDTree on the unit
sphere). With vld_thresh = 1.0, a face is bad data if any of its N closest
faces is bad data.

  NEAREST  no smoothing
  UW_MEAN  mean of the N closest faces
  AW_MEAN  mean weighted by the face area (areaCell)
  MIN, MAX, MEDIAN

N is INTERP_PNTS from the SL1L2 line.

Usage:
  check_ugrid_grid_stat_smooth.py stat_file fcst_file fcst_var obs_file obs_var \\
                                  coord_file [--obs-level K] [--tol T]

Example (unit_ugrid.xml grid_stat_ugrid_mpas_smooth):
  check_ugrid_grid_stat_smooth.py \\
    ${MET_TEST_OUTPUT}/grid_stat_ugrid/grid_stat_UGRID_MPAS_SMOOTH_120000L_20120409_120000V.stat \\
    ${MET_TEST_INPUT}/ugrid_data/mpas/diag.2012-04-09_12.00.00_reduced.nc temperature_925hPa \\
    ${MET_TEST_INPUT}/ugrid_data/mpas/mpasout.2012-04-09_12.00.00_reduced.nc theta \\
    ${MET_TEST_INPUT}/ugrid_data/mpas/static.40962_reduced.nc

For a 3-D observation variable (Time, nCells, nVertLevels), the vertical
index is selected with --obs-level. By default, the index whose mean
matches the unsmoothed (NEAREST) OBAR is used and reported.

Exits with 1 if any statistic differs from the reference by more than --tol.
"""

import argparse
import sys

import numpy as np
import netCDF4
from scipy.spatial import cKDTree

STAT_LINE_TYPE_COL = 23          # LINE_TYPE column index in the STAT file
BAD_DATA_LIMIT = 1.0e20          # treat larger magnitudes as fill values


def as_nan(values):
    a = np.ma.filled(np.ma.asarray(values, dtype=float), np.nan)
    a[np.abs(a) > BAD_DATA_LIMIT] = np.nan
    return a


def aw_mean(nv, nbr, area):
    a = area[nbr] if area is not None else np.ones(nbr.shape)
    return (a * nv).sum(axis=1) / a.sum(axis=1)


# Smoothing of the [n_face, N] values of the closest faces
# (NaN propagates, which is vld_thresh = 1.0)
METHODS = {
    'UW_MEAN': lambda nv, nbr, area: np.mean(nv, axis=1),
    'MIN':     lambda nv, nbr, area: np.min(nv, axis=1),
    'MAX':     lambda nv, nbr, area: np.max(nv, axis=1),
    'MEDIAN':  lambda nv, nbr, area: np.median(nv, axis=1),
    'AW_MEAN': aw_mean,
}


def smooth(v, method, nbr, area):
    if method == 'NEAREST':
        return v
    if method not in METHODS:
        raise ValueError(f"unsupported smoothing method: {method}")
    return METHODS[method](v[nbr], nbr, area)


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('stat_file')
    parser.add_argument('fcst_file')
    parser.add_argument('fcst_var')
    parser.add_argument('obs_file')
    parser.add_argument('obs_var')
    parser.add_argument('coord_file', help='MPAS file with latCell, lonCell (and areaCell)')
    parser.add_argument('--obs-level', type=int, default=None,
                        help='vertical index of a 3-D observation variable')
    parser.add_argument('--tol', type=float, default=1.0e-3,
                        help='maximum allowed difference (default 1e-3)')
    return parser.parse_args()


def read_coords(coord_file):
    """Return the unit vectors and the area (None if missing) of the faces."""
    coord = netCDF4.Dataset(coord_file)
    lat = np.asarray(coord.variables['latCell'][:], dtype=float)
    lon = np.asarray(coord.variables['lonCell'][:], dtype=float)
    area = None
    if 'areaCell' in coord.variables:
        area = np.asarray(coord.variables['areaCell'][:], dtype=float)
    xyz = np.stack([np.cos(lat) * np.cos(lon),
                    np.cos(lat) * np.sin(lon),
                    np.sin(lat)], axis=1)
    return xyz, area


def read_sl1l2(stat_file):
    """Return {method: {n_points, total, fbar, obar, ffbar}} of the SL1L2 lines."""
    with open(stat_file) as stat:
        rows = [l.split() for l in stat]
    # SL1L2: TOTAL FBAR OBAR FOBAR FFBAR OOBAR MAE
    met = {}
    for r in rows:
        if len(r) <= STAT_LINE_TYPE_COL or r[STAT_LINE_TYPE_COL] != 'SL1L2':
            continue
        method = r[17].replace('_SQUARE', '').replace('_CIRCLE', '')
        met[method] = dict(n_points=int(r[18]), total=int(r[24]),
                           fbar=float(r[25]), obar=float(r[26]), ffbar=float(r[28]))
    return met


def select_obs(obs_all, level, met):
    """Return the observation field (the vertical level of a 3-D variable)
    or None if the level can not be determined."""
    if obs_all.ndim == 1:
        return obs_all
    if level is None:
        if 'NEAREST' not in met:
            print("ERROR: --obs-level is required without a NEAREST line")
            return None
        nearest_obar = met['NEAREST']['obar']
        level = min(range(obs_all.shape[1]),
                    key=lambda k: abs(np.nanmean(obs_all[:, k]) - nearest_obar))
    print(f"observation vertical index: {level}")
    return obs_all[:, level]


def check_method(method, m, f, o, tol):
    """Compare the MET SL1L2 statistics with the smoothed fields and print."""
    ok_pts = ~np.isnan(f) & ~np.isnan(o)
    d_fbar = m['fbar'] - f[ok_pts].mean()
    d_obar = m['obar'] - o[ok_pts].mean()
    # FFBAR is about F^2, so an error of tol in F is about 2*|F|*tol in FFBAR
    d_ffbar = m['ffbar'] - (f[ok_pts] ** 2).mean()
    ffbar_tol = 2.0 * max(1.0, abs(m['fbar'])) * tol
    ok = (m['total'] == ok_pts.sum() and abs(d_fbar) <= tol and
          abs(d_obar) <= tol and abs(d_ffbar) <= ffbar_tol)
    print(f"{method:8s} {m['n_points']:3d} {m['total']:6d}/{ok_pts.sum():<6d} "
          f"{d_fbar:10.5f} {d_obar:10.5f} {d_ffbar:10.4f}  {'ok' if ok else 'FAIL'}")
    return ok


def main():
    args = parse_args()
    xyz, area = read_coords(args.coord_file)
    tree = cKDTree(xyz)

    fcst = as_nan(netCDF4.Dataset(args.fcst_file).variables[args.fcst_var][0, ...])
    obs_all = as_nan(netCDF4.Dataset(args.obs_file).variables[args.obs_var][0, ...])

    met = read_sl1l2(args.stat_file)
    if not met:
        print(f"ERROR: no SL1L2 lines found in {args.stat_file}")
        return 1

    obs = select_obs(obs_all, args.obs_level, met)
    if obs is None:
        return 1

    status = 0
    print(f"{'method':8s} {'N':>3s} {'TOTAL':>13s} {'dFBAR':>10s} {'dOBAR':>10s} {'dFFBAR':>10s}")
    nbr_cache = {}
    for method, m in sorted(met.items()):
        n_points = m['n_points']
        if n_points not in nbr_cache:
            nbr_cache[n_points] = tree.query(xyz, k=n_points)[1].reshape(len(xyz), -1)
        nbr = nbr_cache[n_points]
        f, o = smooth(fcst, method, nbr, area), smooth(obs, method, nbr, area)
        status |= not check_method(method, m, f, o, args.tol)
    print("PASS" if status == 0 else "FAIL: difference larger than the tolerance")
    return status


if __name__ == '__main__':
    sys.exit(main())

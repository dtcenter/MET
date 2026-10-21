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


def smooth(v, method, nbr, area):
    if method == 'NEAREST':
        return v
    nv = v[nbr]                  # [n_face, N], NaN propagates (vld_thresh = 1.0)
    if method == 'UW_MEAN':
        return np.mean(nv, axis=1)
    if method == 'MIN':
        return np.min(nv, axis=1)
    if method == 'MAX':
        return np.max(nv, axis=1)
    if method == 'MEDIAN':
        return np.median(nv, axis=1)
    if method == 'AW_MEAN':
        a = area[nbr] if area is not None else np.ones(nbr.shape)
        return (a * nv).sum(axis=1) / a.sum(axis=1)
    raise ValueError(f"unsupported smoothing method: {method}")


def main():
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
    args = parser.parse_args()

    coord = netCDF4.Dataset(args.coord_file)
    lat = np.asarray(coord.variables['latCell'][:], dtype=float)
    lon = np.asarray(coord.variables['lonCell'][:], dtype=float)
    area = None
    if 'areaCell' in coord.variables:
        area = np.asarray(coord.variables['areaCell'][:], dtype=float)
    xyz = np.stack([np.cos(lat) * np.cos(lon),
                    np.cos(lat) * np.sin(lon),
                    np.sin(lat)], axis=1)
    tree = cKDTree(xyz)

    fcst = as_nan(netCDF4.Dataset(args.fcst_file).variables[args.fcst_var][0, ...])
    obs_all = as_nan(netCDF4.Dataset(args.obs_file).variables[args.obs_var][0, ...])

    rows = [l.split() for l in open(args.stat_file)]
    rows = [r for r in rows
            if len(r) > STAT_LINE_TYPE_COL and r[STAT_LINE_TYPE_COL] == 'SL1L2']
    # SL1L2: TOTAL FBAR OBAR FOBAR FFBAR OOBAR MAE
    met = {}
    for r in rows:
        method = r[17].replace('_SQUARE', '').replace('_CIRCLE', '')
        met[method] = dict(n_points=int(r[18]), total=int(r[24]),
                           fbar=float(r[25]), obar=float(r[26]), ffbar=float(r[28]))
    if not met:
        print(f"ERROR: no SL1L2 lines found in {args.stat_file}")
        return 1

    if obs_all.ndim == 1:
        obs = obs_all
    else:
        level = args.obs_level
        if level is None:
            if 'NEAREST' not in met:
                print("ERROR: --obs-level is required without a NEAREST line")
                return 1
            level = min(range(obs_all.shape[1]),
                        key=lambda k: abs(np.nanmean(obs_all[:, k]) - met['NEAREST']['obar']))
        obs = obs_all[:, level]
        print(f"observation vertical index: {level}")

    status = 0
    print(f"{'method':8s} {'N':>3s} {'TOTAL':>13s} {'dFBAR':>10s} {'dOBAR':>10s} {'dFFBAR':>10s}")
    nbr_cache = {}
    for method, m in sorted(met.items()):
        n_points = m['n_points']
        if n_points not in nbr_cache:
            nbr_cache[n_points] = tree.query(xyz, k=n_points)[1].reshape(len(xyz), -1)
        nbr = nbr_cache[n_points]
        f, o = smooth(fcst, method, nbr, area), smooth(obs, method, nbr, area)
        ok_pts = ~np.isnan(f) & ~np.isnan(o)
        d_fbar = m['fbar'] - f[ok_pts].mean()
        d_obar = m['obar'] - o[ok_pts].mean()
        # FFBAR is about F^2, so an error of tol in F is about 2*|F|*tol in FFBAR
        d_ffbar = m['ffbar'] - (f[ok_pts] ** 2).mean()
        ffbar_tol = 2.0 * max(1.0, abs(m['fbar'])) * args.tol
        ok = (m['total'] == ok_pts.sum() and abs(d_fbar) <= args.tol and
              abs(d_obar) <= args.tol and abs(d_ffbar) <= ffbar_tol)
        status |= (not ok)
        print(f"{method:8s} {n_points:3d} {m['total']:6d}/{ok_pts.sum():<6d} "
              f"{d_fbar:10.5f} {d_obar:10.5f} {d_ffbar:10.4f}  {'ok' if ok else 'FAIL'}")
    print("PASS" if status == 0 else "FAIL: difference larger than the tolerance")
    return status


if __name__ == '__main__':
    sys.exit(main())

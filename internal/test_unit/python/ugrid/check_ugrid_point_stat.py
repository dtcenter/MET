#!/usr/bin/env python3
"""
Verify the Point-Stat N closest faces interpolation for the unstructured
grid (UGRID) by recomputing each MPR forecast value from the raw MPAS data.

The reference is independent of MET: it finds the N closest faces by the
chord distance on the unit sphere and applies each interpolation method
with numpy.

  NEAREST  value of the closest face
  UW_MEAN  mean of the N closest faces
  AW_MEAN  mean weighted by the face area (areaCell), equal weights if missing
  DW_MEAN  mean weighted by 1/distance^2
  MIN, MAX, MEDIAN, BEST (closest to the observation)
  BILIN    least-squares fit of a + b*x + c*y + d*x*y in the local tangent
           plane, plane fit if singular, limited to the range of the faces

N is INTERP_PNTS from the MPR line. HiRA (NBRHD) lines are skipped.

Usage:
  check_ugrid_point_stat.py stat_file data_file coord_file var_name [--tol K]

Example (unit_ugrid.xml point_stat_ugrid_mpas_npts):
  check_ugrid_point_stat.py \\
    ${MET_TEST_OUTPUT}/point_stat_ugrid/point_stat_UGRID_MPAS_NPTS_120000L_20250518_120000V.stat \\
    ${MET_TEST_INPUT}/ugrid_data/mpas/diag.2025-05-18_12.00.00.9506.OK.nc \\
    ${MET_TEST_INPUT}/ugrid_data/mpas/static.2025-05-17.9506.OK_reduced.nc \\
    temperature_925hPa

Exits with 1 if any method differs from the reference by more than --tol.
"""

import argparse
import sys

import numpy as np
import netCDF4

EARTH_RADIUS_KM = 6378.14        # earth_radius_km in MET
SAME_POINT_KM = 0.001            # MET uses the face value within 1 meter
STAT_LINE_TYPE_COL = 23          # LINE_TYPE column index in the STAT file


def read_coords(coord_file):
    coord = netCDF4.Dataset(coord_file)
    lat = np.asarray(coord.variables['latCell'][:], dtype=float)
    lon = np.asarray(coord.variables['lonCell'][:], dtype=float)
    xyz = np.stack([np.cos(lat) * np.cos(lon),
                    np.cos(lat) * np.sin(lon),
                    np.sin(lat)], axis=1)
    area = None
    if 'areaCell' in coord.variables:
        area = np.asarray(coord.variables['areaCell'][:], dtype=float)
    return xyz, area


def unit_vector(lat_deg, lon_deg):
    la, lo = np.radians(lat_deg), np.radians(lon_deg)
    return la, lo, np.array([np.cos(la) * np.cos(lo),
                             np.cos(la) * np.sin(lo),
                             np.sin(la)])


def bilin(v, xyz_faces, p, la, lo):
    """Bilinear (or plane) least-squares fit evaluated at the target point."""
    east = np.array([-np.sin(lo), np.cos(lo), 0.0])
    north = np.array([-np.sin(la) * np.cos(lo), -np.sin(la) * np.sin(lo), np.cos(la)])
    dxyz = xyz_faces - p
    x = EARTH_RADIUS_KM * dxyz @ east
    y = EARTH_RADIUS_KM * dxyz @ north
    for cols in ([np.ones_like(x), x, y, x * y], [np.ones_like(x), x, y]):
        a_mat = np.stack(cols, axis=1)
        n_coef = a_mat.shape[1]
        if len(v) >= n_coef and \
           np.linalg.matrix_rank(a_mat, tol=1e-6 * np.abs(a_mat).max()) == n_coef:
            coef = np.linalg.lstsq(a_mat, v, rcond=None)[0]
            return min(max(coef[0], v.min()), v.max())
    return np.nan


class Faces:
    """The N closest faces of an observation."""

    def __init__(self, obs_lat, obs_lon, obs_v, n_points, xyz, area, vals):
        self.la, self.lo, self.p = unit_vector(obs_lat, obs_lon)
        dist = np.linalg.norm(xyz - self.p, axis=1)
        self.idx = np.argsort(dist)[:n_points]
        self.v = vals[self.idx]
        self.d_km = dist[self.idx] * EARTH_RADIUS_KM
        self.xyz = xyz[self.idx]
        self.area = area[self.idx] if area is not None else np.ones(len(self.idx))
        self.obs_v = obs_v

    def at_face(self):
        """True if the observation is at the closest face (within 1 meter)."""
        return self.d_km[0] <= SAME_POINT_KM


def weighted_mean(v, w):
    return (w * v).sum() / w.sum()


def dw_mean(f):
    return f.v[0] if f.at_face() else weighted_mean(f.v, f.d_km ** -2)


def bilin_faces(f):
    return f.v[0] if f.at_face() else bilin(f.v, f.xyz, f.p, f.la, f.lo)


METHODS = {
    'NEAREST': lambda f: f.v[0],
    'UW_MEAN': lambda f: f.v.mean(),
    'AW_MEAN': lambda f: weighted_mean(f.v, f.area),
    'DW_MEAN': dw_mean,
    'MIN':     lambda f: f.v.min(),
    'MAX':     lambda f: f.v.max(),
    'MEDIAN':  lambda f: np.median(f.v),
    'BEST':    lambda f: f.v[np.argmin(np.abs(f.v - f.obs_v))],
    'BILIN':   bilin_faces,
}


def reference(method, faces):
    if method not in METHODS:
        raise ValueError(f"unsupported interpolation method: {method}")
    return METHODS[method](faces)


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('stat_file')
    parser.add_argument('data_file', help='MPAS file with the forecast variable')
    parser.add_argument('coord_file', help='MPAS file with latCell, lonCell (and areaCell)')
    parser.add_argument('var_name', help='forecast variable name, e.g. temperature_925hPa')
    parser.add_argument('--tol', type=float, default=1.0e-3,
                        help='maximum allowed |MET - reference| (default 1e-3)')
    return parser.parse_args()


def read_mpr(stat_file):
    """Yield (method, n_points, obs_lat, obs_lon, fcst, obs) of the MPR lines."""
    with open(stat_file) as stat:
        lines = [l.split() for l in stat]
    col = {name: i for i, name in enumerate(lines[0])}
    # MPR: TOTAL INDEX OBS_SID OBS_LAT OBS_LON OBS_LVL OBS_ELV FCST OBS ...
    b = col['LINE_TYPE'] + 1
    for r in lines[1:]:
        if len(r) <= STAT_LINE_TYPE_COL or r[STAT_LINE_TYPE_COL] != 'MPR':
            continue
        method = r[col['INTERP_MTHD']].replace('_SQUARE', '').replace('_CIRCLE', '')
        yield (method, int(r[col['INTERP_PNTS']]),
               float(r[b + 3]), float(r[b + 4]), float(r[b + 7]), float(r[b + 8]))


def compare(mpr_lines, xyz, area, vals):
    """Return {(method, n_points): [count, max |MET - reference|]}."""
    stats = {}
    for method, n_points, obs_lat, obs_lon, fcst, obs in mpr_lines:
        if method == 'NBRHD':        # HiRA
            continue
        faces = Faces(obs_lat, obs_lon, obs, n_points, xyz, area, vals)
        ref = reference(method, faces)
        s = stats.setdefault((method, n_points), [0, 0.0])
        s[0] += 1
        s[1] = max(s[1], abs(fcst - ref)) if np.isfinite(ref) else np.inf
    return stats


def report(stats, tol):
    status = 0
    print(f"{'method':8s} {'N':>3s} {'pairs':>6s} {'max|MET-ref|':>13s}")
    for (method, n_points), (count, err) in sorted(stats.items()):
        ok = err <= tol
        status |= (not ok)
        print(f"{method:8s} {n_points:3d} {count:6d} {err:13.5f}  {'ok' if ok else 'FAIL'}")
    print("PASS" if status == 0 else f"FAIL: difference larger than {tol}")
    return status


def main():
    args = parse_args()
    xyz, area = read_coords(args.coord_file)
    vals = np.asarray(netCDF4.Dataset(args.data_file).variables[args.var_name][0, :],
                      dtype=float)

    stats = compare(read_mpr(args.stat_file), xyz, area, vals)
    if not stats:
        print(f"ERROR: no MPR lines found in {args.stat_file}")
        return 1
    return report(stats, args.tol)


if __name__ == '__main__':
    sys.exit(main())

import abc
import sys
from typing import Tuple

import numpy as np
from scipy import interpolate

KM_PER_DEGREE = 111.1


def gen_grid(
    n_radii: int, n_theta: int, radii_step_km: float
) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    rad_1d_km = np.arange(0.0, n_radii * radii_step_km, radii_step_km)
    circle_radians = 2 * np.pi
    theta_1d_radians = np.arange(0.0, circle_radians, circle_radians / n_theta)

    rad_2d_km, theta_2d_radians = np.meshgrid(rad_1d_km, theta_1d_radians)

    # The x,y coordinates of each cylindrical grid point in km relative to the
    # center of the grid.
    cyl_x_km = rad_2d_km * np.cos(theta_2d_radians)
    cyl_y_km = rad_2d_km * np.sin(theta_2d_radians)

    return rad_2d_km, theta_2d_radians, cyl_x_km, cyl_y_km


def convert_grid_to_tc_centric_km(
        lons_2d: np.ndarray, lats_2d: np.ndarray, tc_center_lon: float,
        tc_center_lat: float) -> Tuple[np.ndarray, np.ndarray]:
    """Converts 2d lon lat grid to km relative to TC center."""
    diff_lon = lons_2d[0, 0] - lons_2d[0, 1]
    diff_lat = lats_2d[0, 0] - lats_2d[1, 0]

    grid_diff_x_km = KM_PER_DEGREE * diff_lon * np.cos(
        np.deg2rad(tc_center_lat))
    grid_diff_y_km = KM_PER_DEGREE * diff_lat

    x_km = ((lons_2d - tc_center_lon) / diff_lon) * grid_diff_x_km
    y_km = ((lats_2d - tc_center_lat) / diff_lat) * grid_diff_y_km

    return x_km, y_km


def interpolate_to_cyl_grid(cyl_x_km: np.ndarray, cyl_y_km: np.ndarray,
                            x_km: np.ndarray, y_km: np.ndarray,
                            data: np.ndarray) -> np.ndarray:
    flat_x = x_km.flatten()
    flat_y = y_km.flatten()
    flat_cyl_x = cyl_x_km.flatten()
    flat_cyl_y = cyl_y_km.flatten()
    flat_data = data.flatten()
    flat_x.shape = (flat_x.shape[0], 1)
    flat_y.shape = (flat_y.shape[0], 1)
    flat_cyl_x.shape = (flat_cyl_x.shape[0], 1)
    flat_cyl_y.shape = (flat_cyl_y.shape[0], 1)
    flat_data.shape = (flat_data.shape[0], 1)

    lerp = interpolate.LinearNDInterpolator(np.hstack((flat_x, flat_y)),
                                            flat_data)
    pts = np.hstack((flat_cyl_x, flat_cyl_y))
    new_data = lerp(pts)
    new_data.shape = cyl_x_km.shape
    return new_data


def azimuthal_average(cyl_data: np.ndarray, axis=1) -> np.ndarray:
    return np.mean(cyl_data, axis=axis)


class CylindricalGridInterpolator(metaclass=abc.ABCMeta):
    def __init__(self, n_radii: int, n_theta: int, radii_step_km: float,
                 tc_center_lon: float, tc_center_lat: float,
                 src_lons: np.ndarray, src_lats: np.ndarray) -> None:
        self.rad_2d_km, self.theta_2d_radians, self.cyl_x_km, self.cyl_y_km = gen_grid(
            n_radii, n_theta, radii_step_km)
        self.x_km, self.y_km = convert_grid_to_tc_centric_km(
            src_lons, src_lats, tc_center_lon, tc_center_lat)

    def __call__(self, data: np.ndarray):
        pass


class ScipyLinearNDInterpolator(CylindricalGridInterpolator):
    def __init__(self, n_radii: int, n_theta: int, radii_step_km: float,
                 tc_center_lon: float, tc_center_lat: float,
                 src_lons: np.ndarray, src_lats: np.ndarray) -> None:
        super().__init__(n_radii, n_theta, radii_step_km, tc_center_lon,
                         tc_center_lat, src_lons, src_lats)

    def __call__(self, data: np.ndarray) -> np.ndarray:
        return interpolate_to_cyl_grid(self.cyl_x_km, self.cyl_y_km, self.x_km,
                                       self.y_km, data)


class BilinearInterpolator(CylindricalGridInterpolator):
    """High performance Bilinear Interpolation with cached weights."""
    def __init__(self, n_radii: int, n_theta: int, radii_step_km: float,
                 tc_center_lon: float, tc_center_lat: float,
                 src_lons: np.ndarray, src_lats: np.ndarray) -> None:
        super().__init__(n_radii, n_theta, radii_step_km, tc_center_lon,
                         tc_center_lat, src_lons, src_lats)
        self.target_shape = self.cyl_x_km.shape
        self.dx = self.x_km[0, 1] - self.x_km[0, 0]
        self.dy = self.y_km[1, 0] - self.y_km[0, 0]

        i = ((self.cyl_x_km - self.x_km[0, 0]) / self.dx).astype(int).flatten()
        j = ((self.cyl_y_km - self.y_km[0, 0]) / self.dy).astype(int).flatten()

        i[i >= self.x_km.shape[1]] = self.x_km.shape[1] - 1
        j[j >= self.x_km.shape[0]] = self.x_km.shape[0] - 1
        i[i < 0] = 0
        j[j < 0] = 0

        self.i = i
        self.j = j
        self.i_upper = self.i + 1
        self.i_upper[
            self.i_upper >= self.x_km.shape[1]] = self.x_km.shape[1] - 1
        self.j_upper = self.j + 1
        self.j_upper[
            self.j_upper >= self.x_km.shape[0]] = self.x_km.shape[0] - 1

        x11 = self.x_km[self.j, self.i]
        y11 = self.y_km[self.j, self.i]
        x21 = self.x_km[self.j, self.i_upper]
        y21 = self.y_km[self.j, self.i_upper]
        x12 = self.x_km[self.j_upper, self.i]
        y12 = self.y_km[self.j_upper, self.i]
        x22 = self.x_km[self.j_upper, self.i_upper]
        y22 = self.y_km[self.j_upper, self.i_upper]

        self.w11 = self._cyl_dist(x11, y11)
        self.w21 = self._cyl_dist(x21, y21)
        self.w12 = self._cyl_dist(x12, y12)
        self.w22 = self._cyl_dist(x22, y22)

        self._convert_distances_to_weights(
            [self.w11, self.w21, self.w12, self.w22])

    def _cyl_dist(self, px: np.ndarray, py: np.ndarray):
        dx = self._1d_distance(self.cyl_x_km, px)
        dy = self._1d_distance(self.cyl_y_km, py)
        return 1 / np.sqrt(dx**2 + dy**2)

    def _1d_distance(self, cyl_km: np.ndarray,
                     points: np.ndarray) -> np.ndarray:
        diff = cyl_km.flat - points

        # Fudge the 0 values so that interpolated points right on source points
        # don't create a divide by zero error.
        epsilon = sys.float_info.epsilon
        mask = (diff <= epsilon) & (diff >= -epsilon)
        diff[mask] = epsilon

        return diff

    def _convert_distances_to_weights(self, weights):
        total = np.sum(weights, axis=0)
        for weight in weights:
            weight /= total

    def __call__(self, data: np.ndarray) -> np.ndarray:
        z11 = data[self.j, self.i]
        z21 = data[self.j, self.i_upper]
        z12 = data[self.j_upper, self.i]
        z22 = data[self.j_upper, self.i_upper]

        on_grid = z11 * self.w11 + z12 * self.w12 + z21 * self.w21 + z22 * self.w22
        on_grid.shape = self.target_shape

        return on_grid

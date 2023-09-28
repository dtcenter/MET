import numpy as np


def mean_in_radius_range(data: np.ndarray, distances_from_tc_km: np.ndarray,
                         min_radius_km: float, max_radius_km: float) -> float:
    """Computes the mean value inside a min/max radius.

    Args:
        data (np.ndarray): The data to compute the average of.
        distances_from_tc_km (np.ndarray): Array of distances in km from a TC 
            center for each point in the data array.
        min_radius_km (float): Minimum distance in km to be included in the mean
            calculation.
        max_radius_km (float): Maximum distance in km to be included in the mean
            calculation.

    Returns:
        float: The mean data value whose distance is within(inclusive) the range
            min_radius_km - max_radius_km.
    """
    indices = (distances_from_tc_km >= min_radius_km) & (distances_from_tc_km
                                                         <= max_radius_km)
    avg = np.mean(data[indices])
    return avg

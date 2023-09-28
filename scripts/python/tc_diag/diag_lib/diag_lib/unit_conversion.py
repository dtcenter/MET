"""This module contains unit conversion routines needed by the TC diag code.
"""
from typing import Union
import numpy as np

KELVIN_TO_CELSIUS = -273.15
PA_TO_HPA = 0.01

Numeric = Union[int, float, np.ndarray]


def kelvin_to_celsius(data: Numeric, inplace=False) -> Numeric:
    """Converts data from Kelvin to Celsius.

    Can convert most numeric types including arrays.  If an array is passed,
    the array can be converted in-place to avoid making a duplicate.

    Args:
        data (Numeric): Data to convert.
        inplace (bool, optional): If set to True, the passed data is assumed to 
            be an array that will be converted in-place. If set to True, it will
            raise a ValueError if an int or float is passed. Defaults to False.

    Returns:
        Numeric: The data converted to Celsius.

    Raises:
        ValueError:  Raised if an int or float is passed instead of an array 
            when inplace is set to True.
    """
    if inplace:
        _validate_inplace_operation(data)
        data += KELVIN_TO_CELSIUS
        return data

    return data + KELVIN_TO_CELSIUS


def pa_to_hpa(data: Numeric, inplace=False) -> Numeric:
    """Converts data from Pa to hPa.

    Can convert most numeric types including arrays.  If an array is passed,
    the array can be converted in-place to avoid making a duplicate.

    Args:
        data (Numeric): Data to convert.
        inplace (bool, optional): If set to True, the passed data is assumed to 
            be an array that will be converted in-place. If set to True, it will
            raise a ValueError if an int or float is passed. Defaults to False.

    Returns:
        Numeric: The data converted to hPa.

    Raises:
        ValueError:  Raised if an int or float is passed instead of an array 
            when inplace is set to True.
    """
    if inplace:
        _validate_inplace_operation(data)
        data *= PA_TO_HPA
        return data

    return data * PA_TO_HPA


def _validate_inplace_operation(data: Numeric) -> None:
    if type(data) == int or type(data) == float:
        raise ValueError(
            f"Attempted to perform in-place conversion on non-array data: {data}."
        )

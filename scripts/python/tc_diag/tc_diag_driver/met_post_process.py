MPS_TO_KT = 1.944
KELVIN_TO_CELCIUS = -273.15
PA_TO_MB = 0.01
M_TO_DM = 0.1


def mps_to_kt(value: float) -> float:
    return value * MPS_TO_KT


def mps_to_10kt(value: float) -> float:
    return scale10(mps_to_kt(value))


def k_to_10c(value: float) -> float:
    return (value + KELVIN_TO_CELCIUS) * 10


def scale10(value: float) -> float:
    return value * 10


def pa_to_mb(value: float) -> float:
    return value * PA_TO_MB


def m_to_dm(value: float) -> float:
    return value * M_TO_DM

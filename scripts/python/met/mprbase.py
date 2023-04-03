import pandas as pd

########################################################################

def read_mpr(input_file, usecols=range(1,37), header=None,
             delim_whitespace=True, keep_default_na=False,
             skiprows=1, dtype=str):
    mpr_data = pd.read_csv(input_file, header=header,
                           delim_whitespace=delim_whitespace,
                           keep_default_na=keep_default_na,
                           skiprows=skiprows, usecols=usecols,
                           dtype=dtype).values.tolist()
    return mpr_data


########################################################################

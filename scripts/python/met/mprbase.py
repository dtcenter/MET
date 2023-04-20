import pandas as pd

########################################################################

class mpr_data():

   # Read a text file with N columns and returns the list of N column data
   # Skip first "col_start" columns if col_start is not 0.
   def read_mpr(input_file, col_last, col_start = 0, header=None,
                delim_whitespace=True, keep_default_na=False,
                skiprows=1, dtype=str):
      mpr_data = pd.read_csv(input_file, header=header,
                             delim_whitespace=delim_whitespace,
                             keep_default_na=keep_default_na,
                             skiprows=skiprows,
                             usecols=range(col_start,col_last+1),
                             dtype=dtype).values.tolist()
      return mpr_data


########################################################################

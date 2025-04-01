import pandas as pd

########################################################################

class mpr_data():

   # Read a text file with N columns and returns the list of N column data
   # Skip first "col_start" columns if col_start is not 0.
   @staticmethod
   def read_mpr(input_file, col_start=0, col_last=0, header=None,
                sep=r'\s+', keep_default_na=False,
                skiprows=1, dtype='string'):

      if col_last == 0:
          maj_vrs = pd.read_csv(input_file, header=header, sep=sep,
                                skiprows=skiprows, usecols=range(0, 1),
                                dtype=dtype, nrows=1).values[0][0]

          # The number of MPR columns vary by MET version
          if int(maj_vrs.strip("V").split(".")[0]) < 12:
              col_last = 36
          else:
              col_last = 38

      mpr_data = pd.read_csv(input_file, header=header, sep=sep,
                             keep_default_na=keep_default_na,
                             skiprows=skiprows,
                             usecols=range(col_start,col_last+1),
                             dtype=dtype).values.tolist()

      return mpr_data


########################################################################

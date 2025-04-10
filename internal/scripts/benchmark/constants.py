# Contains constant values used in the processing of benchmark metrics produced by the
# CTRACK tool

TIME_COLUMNS = ['total_time', 'time_tracked', 'time_accumulated',
                 'all_time_active_exclusive', 'std_deviation', 'coefficient_of_variation']

# Conversion factor for sec, millisec (ms), and microseconds (mcs) to nanoseconds
# Useful for calculating the mean values when units are not uniform
CONVERT_TO_NANOSEC = {'sec':float(1.0e9), 'ms':float(1.0e6),'mcs':float(1.0e3), 'ns':float(1.0) }

# Conversion FROM nanoseconds to one of the following: s (seconds), ms (milliseconds), or mcs (microseconds)
CONVERT_FROM_NANOSECOND = {'sec':float(1.0e-9), 'ms':float(1.0e-6),'mcs':float(1.0e-3), 'ns':float(1.0) }

# Columns of interest for analysis
# Add more columns for the other metrics provided by CTRACK if necessary
# Refer to the ALL_AVAILABLE_COLS list (below) for a list of all the available metrics that can be added
COLS_OF_INTEREST = ['filename', 'function', 'line', 'new_index', 'number_of_calls', 'number_of_calling_threads']


# All available metrics
# the values of these columns are a value + time (s, ms, mcs, ns) or %
# and these values will need to be separated into a values column and
# a corresponding units column
ALL_AVAILABLE_COLS = ['total_time',
               'time_tracked',
               'ae',
               'center_interval_%',
               'ae[0-100]%',
               'time_ae[0-100]%',
               'time_a[0-100]',
               'all_time_active_exclusive',
               'time_accumulated',
               'std_deviation',
               'coefficient_of_variation',
               'fastest_min',
               'fastest_mean',
               'center_min',
               'center_mean',
               'center_med',
               'center_time_active',
               'center_time_active_exclusive',
               'center_max',
               'slowest_mean',
               'slowest_max']

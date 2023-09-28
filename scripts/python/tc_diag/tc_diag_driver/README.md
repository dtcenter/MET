# tc_diag_driver

Driver application to generate TC diagnostic files from ATCF files, model
forecast files, and a lookup table of global distances to land.

## Installation
1. Clone the `tc_diag_driver` repository.
2. `cd` to the repository directory (`tc_diag_driver`)
3. Use the following command to generate an environment that contains all of the 
    driver's prerequisite packages:
    ```bash
    conda env create -f environment_deploy.yml
    ```
4. Activate the environment with the command:
    ```bash
    conda activate tc_diag_driver_deploy
    ```
5. `cd ..` to the directory above the repository directory.
6. Install the `tc_diag_driver` package with the command:
    ```bash
    pip install tc_diag_driver/
    ```

## Basic Usage
Once the `tc_diag_driver` package has been installed, you can run the driver
with the following command:
```bash
python -m tc_diag_driver.driver model_entry_file land_lut_file
```
* model_entry_file: YAML file containing a list of model times to process.
* land_lut_file: A file containing a global lookup table of distances to land.

You can also optionally use the following command to receive a usage message:
```bash
python -m tc_diag_driver.driver -h
```

## Algorithm Overview

This application iterates over a provided list of times and other information
needed to generate diagnostic variables for that time.  Each entry in this list
is referred to as a `Model Entry`.  Each `Model Entry` provides a `model time`,
an `ATCF ID`, and a `Model Specification`.  The `Model Specification` provides a
wide variety of information needed to compute diagnostic variables for the
desired model.  Information such as the levels, forecast hours, directory grib
files are found in, and the specific variables that should be computed are
contained in the `Model Specification`.

For each provided `Model Entry`, all of the grib files corresponding to the
`model time` and `forecast time` are found.  Additionally, the ATCF file
corresponding to the `Model Entry` is also discovered.  Prerequiste Data needed
to generate the diagnostic variables is then computed. Finally, once this is
done, the variables are computed.

To summarize the algorithm, it performs the following steps:
1. Read lookup table of global distances to land (`Land LUT`)
2. Read list of `Model Entries`
3. For each `Model Entry`:
    1. Read the `Model Specification` given by the `Model Entry`
    2. Iterate over each forecast time:
        1. Read appropriate grib file
        2. Generate data needed to perform diagnostic calculations  
            (EX: Calculate cylindrical grid resampling weights, read ATCF file.)
        3. Perform diagnostic variable calculations
    3. Output diagnostic variables

## Model Entry
A `YAML` file containing a list of `Model Entries` is provided as input to the
driver program.  Each `Model Entry` provides the driver with a `Model Time` and
additional information needed to process that `Model Time`.  The following
describes the `Model Entry` file:
```yaml
---
# Dictionary containing a list of Model Entries
model_entries:
    - model_spec: # Path to the Model Specification file
      atcf_id: # The ATCF ID of the storm EX: al092021
      model_time: # The model time, takes YYYY-MM-DDTHH:MM:SS EX: 2021-08-27T12:00:00
      atcf_file: # Path to the ATCF file
      output_dir: # Directory to write output diagnostic files 
```

See `tests/special_test/entry_spec.yml` for an example `Model Entry` file.

## Model Specification
The `Model Specification` provides information to the driver that tells it:
* What grib records should be read
* What variables should be computed, what functions compute them, and what 
    parameters are used to comput them.
* How the computed variables should be written to the diagnostic output file

This gives users of the driver immense flexibility to experiment with diagnostic
calculations without the need to alter the driver code itself.  This also allows
us to maintain a "gold" `Model Specification` that can always be returned to.

However, as a result the `Model Specification` file format is a little 
complicated.  It is given below:

```yaml
---
model_name: # The name of the model. EX: gfs
path_format: # A Python format string used to generate the path of the model
# file.  The following variables are available for use in the format string:
# model_time: Datetime
# resolution: str
# model_name: str
# forecast_time_hours: int
# nested_grid_id: str
# EX: "data/gfs/{model_time:%Y%m%d%H}/{resolution}/{model_name}.{model_time:%Y%m%d%H}.pgrb2.{resolution}.f{forecast_time_hours:03d}
resolution: # A string indicating the resolution of the file EX: 0p50
level_hPa: # List of int pressure levels EX: [1000, 850, 200]
forecast_hours: # List of int forecast hours EX: [0, 6, 12]
nav_var_name: # The lats/lons need to be looked up from a grib record. This 
# provides the name of record to use. EX: t
nav_var_level: # Pressure level of the navigation variable as an int. EX: 1000
nav_var_level_type: # Type of pressure level of the navigation variable. EX: isobaricInhPa
nav_var_is_surface: # Bool indicating whether the navigation variable is a surface variable. EX: false
n_radii: # An int indicating the number of radii to use for the cylindrical grid. EX: 150
n_theta: # An int indicating the number of theta divisions in the cylindrical grid. EX: 8
radii_step_km: # An int indicating the step size in kilometers between radii in the cylindrical grid. EX: 10
atcf_tech_id: # The ATCF tech id used to generate the model track. EX: AVNO
output_file_format: # A Python format string used to generate the filename of 
# each output file. The following variables are available for use in the format 
# string: 
# atcf_id: str
# atcf_tech_id: str
# model_time: Datetime 
# EX: s{atcf_id}_{atcf_tech_id}_doper_{model_time:%Y%m%d%H_diag.dat}

# List of input var specifications.  This tells the driver what grib 
# records to read and how to read them.
input_var_specs: 
    - var_name: # The name of the variable. If no grib_name is provided, it is 
      # assumed that the name of the variable and the grib record name are the 
      # same.  Diagnostic variable calculations will use the var_name to access
      # its values.
      grib_name: # Optional name of the grib record.  This is provided so that
      # the desired name of the variable and the name in grib file do not have
      # to be the same.  As an example, the tpw variable corresponds to the pwat
      # grib record.
      level_type: # The type of level for the grib record. EX: isobaricInhPa
      is_surface: # Optional bool that determines if the grib record is a surface
      # record.
      level: # If is_surface is true, then level should be provided as an int to
      # provide the correct level to read from the grib file.
      select_name_only: # Optional bool that determines whether just the name 
      # should be used to select the variable from the grib file. If set to false 
      # (the default) the level and level_type are used as well.  This argument 
      # is only used for surface variables since the level_type and
      # level are essential to select profile variables.  If set to true and
      # is_surface is false, this parameter will be ignored.

# Dictionary of pressure independent computation specifications.  This section is for
# diagnostic variables that will be computed once per forecast time, independent 
# of pressure levels.  Each specification tells the driver what variables will 
# be computed and what function is used to compute them.  Keyword arguments can
# optionally be passed to the function when the variable is generated.
pressure_independent_computation_specs:
    # The diagnostic variable name, such as min_slp, should be provided instead 
    # of "var_name". If multiple diagnostic variables are returned, then this
    # name will be ignored and instead the "output_vars" entry will be used.
    var_name:
        callable: # The function to perform the diagnostic variable computations.
        # This should be a valid absolute import path to the function. EX:
        # "tc_diag_driver.diag_vars.mean_in_radius_range". The function provided 
        # here will be imported at run-time and then called with a standard set
        # of inputs as well as any kwargs provided by the specification.  The 
        # function should use **kwargs as an argument in order to prevent unused
        # inputs from raising a TypeError.  The function should return one or 
        # more floats.  If more than one float is returned then the "output_vars"
        # entry should be used in the spec to tell the driver what variables are
        # produced by the function.  If only one float is returned then the 
        # driver will use the spec's name as the variable name.
        kwargs: # Optional dictionary of keyword-arguments to be passed to the
        # function in order to generate the diagnostic variable(s).
        output_vars: # Optional list of the names of diagnostic variables 
        # produced by the function given by "callable".  If more than one float
        # is returned by the function, then "output_vars" should list the 
        # names of the diagnostic variables that map to the function output. For
        # example, the "storm_r_theta" function returns the speed and heading of
        # the storm based on the ATCF track.  So "output_vars: [stm_spd, stm_hdg]"
        # will map the output of the "storm_r_theta" function to the "stm_spd" and
        # "stm_hdg" diagnostic variables.  However the "mean_in_radius_range"
        # function returns a single float representing an area average.  In this
        # case, "output_vars" can be omitted and the name of the spec will be 
        # used as the diagnostic variable name.
        batch_order: # Optional int that specifies the order that the variable
        # should be processed.  By default a var spec will have a "batch_order"
        # of 0.  Higher values are procesed after lower values.  This allows for
        # diagnostic variables produced by one function to be used as input to
        # another function. Functions with the same "batch_order" are processed
        # in an arbitrary order.
    
# Dictionary of sounding computation specifications.  This section provides
# computation specs for diagnostic variables that will be computed at each
# pressure level.  The computation specifications contain the same entries
# in this section as they do in the "pressure_independent_computation_specs"
# section.
sounding_computation_specs:
    # Replace the "var_name" with the name of the diagnostic variable.
    var_name:
        callable: # The function to call
        kwargs: # Optional dictionary of keyword arguments
        output_vars: # Optional list mapping function output to diagnostic 
        #variable names
        batch_order: # Optional int specifying the order that the function
        # should be called in.  Default is 0.  Functions are processed from
        # low to high "batch_order" values.

# List of output specification entries.  Each entry tells the driver what
# diagnostic variable should be output, which diagnostic file section it 
# belongs to, what units to print, and if any optional conversions should
# be performed before it is output.
output_specs:
    - var_name: # The diagnostic variable name.  This will be converted to 
      # uppercase in the output.  The variable names used in the computation
      # specifications should be used here.
      units: # The units to print in the output file. This will be converted to
      # uppercase in the output.
      output_type: # Determines Which section should this variable will appear in 
      # within the output file.  Must be one of: storm, surface, or sounding. 
      # "surface" variables will have _SURF appended to their name and 
      # "sounding" variables will appear once for each pressure level with the 
      # pressure level appended to the name.
      scale_factor: # Optional float to scale the variable by before being 
      # written to the file.  This allows for variables to be kept in more
      # convenient units for computation and then converted to a unit more 
      # convenient for the diagnostic output.
      output_float: # Optional bool that determines whether the variable should
      # be printed as an integer or a float.  By default, variables will be
      # converted to integers prior to being written to the diagnostic file.
```

See `tests/special_test/gfs_spec.yml` for an example `Model Specification` file.

## Land LUT
This is a simple text file that provides a global lookup table of distances to 
land in km.  Due to its relatively large size, this is stored outside of the 
repository and will need to be obtained separately.  The path to the file 
needs to be provided as an input argument.

## Computation Specification Functions
A function listed in the `pressure_independent_computation_specs` or 
`sounding_computation_specs` sections of a `Model Specification` must meet the 
following requirements:
* It must return at least one `float` value. No other return types are supported.
* It must use the `**kwargs` argument to ignore any unused input.
* Returned missing values should be `nan`.

The following arguments are always available to a function, but may be ignored.
Additional input can be provided by the `kwargs` entry in a computation specification.
* `grib_dataset`: Xarray `Dataset` of input read from the grib file. All sounding
    `DataArray`s in the `Dataset` use `level`, `lat`, `lon` as dimensions. All
    surface `DataArray`s just use `lat` and `lon`.
* `cylindrical_grid_interpolator`: A `diag_lib.cylindrical_grid.BilinearInterpolator`
    instance that generates the cylindrical grid and provides bilinear interpolation
    on to that grid.  It caches weights to make the interpolation very fast. 
* `land_lut`: A `tc_diag_driver.diag_vars.LandLUT` instance that provides distance 
    to land estimates for a given lon, lat position.  These distances are pulled
    from a look-up-table and then interpolated to the desired position.
* `model_track`: A pandas `DataFrame` that provides ATCF values for the desired
    model at the desired `Model Time`.
* track_row: A pandas `DataFrame` representing the row selected from the 
    `model_track`for the appropriate `forecast_hour`.
* `grib_distances_from_tc_km`: A 2D numpy array of the grib lons, lats converted
    to distances in km from the `tc_lon`, `tc_lat` at the approprate forecast 
    hour.
* `tc_lon`: The longitude (0-360) of the TC at the forecast hour looked up from 
    the `model_track`.
* `tc_lat`: The latitude of the TC at the forecast hour looked up from the 
    `model_track`.
* `model_spec`: The `Model Specification` as an instance of 
    `tc_diag_driver.model_files.ModelSpec`.
* `hour`: The forecast hour as an integer.
* `grib_lons`: A 2D numpy array of longitudes (0-360) from the grib data.
* `grib_lats`: A 2D numpy array of latitudes from the grib data.
* `results`: A `tc_diag_driver.results.ForecastHourResults` instance storing 
    previously computed results from other functions.  If this input is used
    make sure that the `batch_group` in the computation spec has an appropriate
    value to ensure that the prerequisite results are available.
* `model_time`: A `Datetime` representing the `Model Time`.


## A Note on Design
This code was designed to make it easy to experiment with a variety of different
parameters and functions to generate diagnostic variables without the need to
alter code in this package.  The "computation engine" was created to support 
this goal.  However, it does make assumptions about how data are obtained from 
grib files and how cylindrical coordinates are generated.  Additionally, it may
be desirable to simply provide a fixed set of code that performs the diagnostic
computations.  If this is the case, the `tc_diag_driver.diag_vars` module may
simply be imported and the functions contained within can be used directly. 
Alternatively, if use of the computation engine is still desirable but the 
assumptions made by the driver do not hold, then the 
`tc_diag_driver.driver.process_model_entry` can be used as an example of how to
use the computation engine.

## Automated Tests
An automated test suite is provied in the `tc_diag_driver` repository.  From the
repository directory, tests can be performed with the following commands:
```bash
python -m unittest discover -s tests -p "*test_*"
```
## Dumping Cylindrical Grid Output
During debugging, it may be desirable to dump the result of the interpolation to
the cylindrical grid for different variables at different levels.  The
`debug_cyl_grid_dump` function in `tc_diag_driver.diag_vars` is provided to
accomplish this.  The function can be added to the
`pressure_independent_computation_specs`or `sounding_computation_specs` to dump
a variable after conversion to the cylindrical grid to a netCDF file.  The
function allows you to specify a grib variable to dump, optionally convert its
units, and specify a file to dump the resampled variable to.  The
`output_filename` parameter allows you to pass a format string that will be
converted to an output filename.  You may use `grib_var_name`, `level_hPa`, and
`hour` in the format string.  If you're dumping a variable in the
`sounding_computation_specs` section, each level will be dumped to a separate
file.  An example GFS `Model Specification` config that dumps resampled
variables is provided here: `tests/special_test/debug_dump_gfs_spec.yml`.

The following command will run the example config:
```bash
python -m tc_diag_driver.driver tests/special_test/debug_dump_entry_spec.yml tests/land_lut/current_operational_gdland.dat
``` 
The above command will dump a few variables to the
`tests/special_test/cyl_dumps/` directory.

## Alternate "Post Resample" Driver
An alternate driver is provided for use with MetPlus.  This driver assumes that
the input grib file has already been read, its data has been resampled to a
cylindrical grid, and the resampled data for each forecast time has been written
to a temporary netCDF file.  

This driver provides a function that can be used to perform diagnostic
computations from a config and a netCDF file.  It will return a
`tc_diag_driver.results.ForecastHourResults` instance. This class provides
pressure independent and sounding xarray `Dataset` objects containing the
results of the diagnostic computations.  

You may choose to suppress exceptions while performing diagnostic computations.
This will prevent exceptions encountered while generating diagnostic variables
from propagating upwards.  The failure will be logged if logging is used by the
client program.  If a variable fails to compute, its `DataArray` in the results
will simply be filled with `NaN`. Errors encountered outside of the diagnostic
calculations, such as failing to read the pressure levels from the provided 
netCDF file, will still raise an exception.

If no netCDF file is available, an alternative function is available which
expects a config, the forecast hour, and a list of levels and will return a
`ForecastHourResults` object filled with missing values.

Nested grids are supported by simply providing an alternative config that uses 
smaller radius values for the diagnostic computations.

Example config files are provided here:
- `tests/post_resample_test/post_resample.yml`
- `tests/post_resample_test/post_resample_nest.yml`
The example configs can likely be used as is in MetPlus with the exception that
you will need to update the `land_lut_file` entries to point to the correct
location of the text distance to land LUT file.

Example Usage:
```python
import pathlib
from tc_diag_driver import post_resample_driver

config_filename = pathlib.Path("tests/post_resample_test/post_resample.yml")

# Read your config file and return a DriverConfig object:
config = post_resample_driver.config_from_file(
    config_filename, post_resample_driver.DriverConfig)

# You may override any of the values read in from the file if you need to
config.in_radii_name = "overridden_nc_radii_var_name"

# DriverConfig objects are just a Dataclass, so you can construct them entirely
# in memory if you want to.
some_dictionary = get_a_dictionary_of_parameters_from_somewhere()
alternate_config = post_resample_driver.DriverConfig(**some_dictionary)

# Perform the diagnostic calculations:
data_filename = pathlib.Path("tests/post_resample_test/input_data/tmp_tc_diag_AL092022_GFSO_2022092400_f6_parent_29849_0_.nc")
results = post_resample_driver.diag_calcs(
    config, data_filename, suppress_exceptions=True)

# The returned tc_diag_driver.results.ForecastHourResults object provides the
# results of the diagnostic computations in two Xarray Datasets:
print(results.pressure_independent)
print(results.soundings)

# In order to pre-fill the results with missing data, diag_calcs reads the
# forecast hour and pressure levels from the provided netCDF file.  If no
# netCDF file is available then an alternative function should be used to 
# retrieve results pre-filled with NaNs.  The forecast hour and pressure levels
# will need to be provided:
forecast_hour = 6
levels_hPa = [1000, 850, 200, 100]
empty_results = post_resample_driver.populate_missing_results(
    config, forecast_hour, levels_hPa)

print(empty_results.soundings["u"].sel(forecast_hour=forecast_hour, level_hPa=200))
```

### Post Resample Driver Manual Test
A manual test can be run from the top dir of the repo using the following
command:
```bash
tests/post_resample_test/run.sh
```

This will use wget to download the `v2023-04-07_gdland_table.dat` distance to
land file and place it in `tests/post_resample_test/input_data` if the file
doesn't exist.  It will then run the driver for the parent and nest netCDF files
in the `input_data` dir.  The output will be dumped to netCDF files in the
`tests/post_resample_test/output` dir.
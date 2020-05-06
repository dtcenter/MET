.. _reformat_point:

Chapter 4 Re-Formatting of Point Observations
=============================================

There are several formats of point observations that may preprocessed using the suite of reformatting tools in MET. These include PrepBUFR data from NCEP, SURFRAD data from NOAA, AERONET data from NASA, MADIS data from NOAA, little_r from WRF simulations, and user-defined data in a generic ASCII format. These steps are represented by the first columns in the MET flowchart depicted in Figure [Fig_Overview_MET_Overview_Flowchart]. The software tools used to reformat point data are described in this chapter.

4.1 PB2NC tool
______________

This section describes how to configure and run the PB2NC tool. The PB2NC tool is used to stratify the contents of an input PrepBUFR point observation file and reformat it into NetCDF format for use by other MET tools. The PB2NC tool must be run on the input PrepBUFR point observation file prior to performing verification with the MET statistics tools.

4.1.1 pb2nc usage
~~~~~~~~~~~~~~~~~

The usage statement for the PB2NC tool is shown below:

Usage: pb2nc

{\hskip 0.5in}prepbufr_file

{\hskip 0.5in}netcdf_file

{\hskip 0.5in}config_file

{\hskip 0.5in}[-pbfile PrepBUFR_file]

{\hskip 0.5in}[-valid_beg time]

{\hskip 0.5in}[-valid_end time]

{\hskip 0.5in}[-nmsg n]

{\hskip 0.5in}[-dump path]

{\hskip 0.5in}[-index]

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

{\hskip 0.5in}[-compress level]

pb2nc has both required and optional arguments.

Required arguments for pb2nc

1.
The **prepbufr_file** argument is the input PrepBUFR file to be processed.

2.
The **netcdf_file** argument is the output NetCDF file to be written.

3.
The **config_file** argument is the configuration file to be used. The contents of the configuration file are discussed below.

	:underline:**`Optional arguments for pb2nc`**

1.
The **-pbfile prepbufr_file** option is used to pass additional input PrepBUFR files.

2.
The **-valid_beg** time option in YYYYMMDD[_HH[MMSS]] format sets the beginning of the retention time window.

3.
The **-valid_end** time option in YYYYMMDD[_HH[MMSS]] format sets the end of the retention time window.
4.
The **-nmsg num_messages** option may be used for testing purposes. This argument indicates that only the first “num_messages” PrepBUFR messages should be processed rather than the whole file. This option is provided to speed up testing because running the PB2NC tool can take a few minutes for each file. Most users will not need this option.

5.
The **-dump path** option may be used to dump the entire contents of the PrepBUFR file to several ASCII files written to the directory specified by “path”. The user may use this option to view a human-readable version of the input PrepBUFR file, although writing the contents to ASCII files can be slow.

6.
The **-index** option shows the available variables with valid data from the BUFR input. It collects the available variable list from BUFR input and checks the existence of valid data and directs the variable names with valid data to the screen. The NetCDF output won't be generated.

7.
The **-log** file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

8.
The **-v level** option indicates the desired level of verbosity. The value of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

9.
The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

    An example of the pb2nc calling sequence is shown below:

|				    pb2nc sample_pb.blk \

|				    sample_pb.nc \

|				    PB2NCConfig

In this example, the PB2NC tool will process the input **sample_pb.blk** file applying the configuration specified in the **PB2NCConfig** file and write the output to a file named **sample_pb.nc**.

4.1.2 pb2nc configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the PB2NC tool named **PB2NCConfig_default** can be found in the installed share/met/config directory. The version used for the example run in Section [sec:Install_Sample-test-cases] is available in **scripts/config**. It is recommended that users make a copy of configuration files prior to modifying their contents.

When editing configuration files, environment variables may be used for setting the configurable parameters if convenient. The configuration file parser expands any environment variables to their full value before proceeding. Within the configuration file, environment variables must be specified in the form: **${VAR_NAME}**.

For example, using an environment variable to set the **message_type** (see below) parameter to use ADPUPA and ADPSFC message types might consist of the following:

* In a C-Shell: **setenv MSG_TYP ' “ADPUPA”, “ADPSFC” '**

* In the configuration file: **message_type[] = [ ${MSG_TYP} ];**

 The contents of the default pb2nc configuration file are described below.

					obs_window = { beg  = -5400; end  = 5400; }

					mask       = { grid = "";    poly = "";   }

					tmp_dir    = "/tmp";

					version    = "VN.N";

The configuration options listed above are common to many MET tools and are described in Section [subsec:IO_General-MET-Config-Options].

					message_type = [];

Each PrepBUFR message is tagged with one of eighteen message types as listed in the share/met/config/README file. The 'message_type' refers to the type of observation from which the observation value (or 'report') was derived. The user may specify a comma-separated list of message types to be retained. Providing an empty list indicates that all message types should be retained.


					message_type_map = [ { key = “AIRCAR”; val = “AIRCAR_PROFILES”; } ];

The message_type_map entry is an array of dictionaries, each containing a key string and val string. This defines a mapping of input PrepBUFR message types to output message types. This provides a method for renaming input PrepBUFR message types.




message_type_group_map = [


{ key = "SURFACE"; val = "ADPSFC,SFCSHP,MSONET";               },


{ key = "ANYAIR";  val = "AIRCAR,AIRCFT";                      },


{ key = "ANYSFC";  val = "ADPSFC,SFCSHP,ADPUPA,PROFLR,MSONET"; },


{ key = "ONLYSF";  val = "ADPSFC,SFCSHP";                      }

						    ];

The message_type_group_map entry is an array of dictionaries, each containing a key string and val string. This defines a mapping of message type group names to a comma-separated list of values. This map is defined in the config files for PB2NC, Point-Stat, or Ensemble-Stat. Modify this map to define sets of message types that should be processed together as a group. The SURFACE entry must be present to define message types for which surface verification logic should be applied.


			  station_id = [];

Each PrepBUFR message has a station identification string associated with it. The user may specify a comma-separated list of station IDs to be retained. Providing an empty list indicates that messages from all station IDs will be retained. It can be a file name containing a list of stations.




elevation_range = { beg = -1000; end = 100000; }


The beg and end variables are used to stratify the elevation (in meters) of the observations to be retained. The range shown above is set to -1000 to 100000 meters, which essentially retains every observation.


pb_report_type  = [];


in_report_type  = [];


instrument_type = [];

						  
The pb_report_type, in_report_type, and instrument_type variables are used to specify comma-separated lists of PrepBUFR report types, input report types, and instrument types to be retained, respectively. If left empty, all PrepBUFR report types, input report types, and instrument types will be retained. See the following for more details:


http://www.emc.ncep.noaa.gov/mmb/data_processing/PrepBUFR.doc/table_4.htm


http://www.emc.ncep.noaa.gov/mmb/data_processing/PrepBUFR.doc/table_6.htm




level_range    = { beg = 1; end = 255; }


level_category = [];


The beg and end variables are used to stratify the model level of observations to be retained. The range shown above is 1 to 255.


The level_category variable is used to specify a comma-separated list of PrepBUFR data level categories to retain. An empty string indicates that all level categories should be retained. Accepted values and their meanings are described in Table [table_reform-point_pb2nc_level_category] :ref:`table_reform-point_pb2nc_level_category`. See the following for more details:


http://www.emc.ncep.noaa.gov/mmb/data_processing/PrepBUFR.doc/table_1.htm


Values for the level_category option.

.. list-table:: table_reform-point_pb2nc_level_category
   :widths: auto
   :header-rows: 1

   * - Level category value
     - Description
   * - 0
     - Surface level
   * - 1
     - Mandatory level
   * - 2
     - Significant temperature level
   * - 3
     - Winds-by-pressure level
   * - 4
     - Winds-by-height level 
   * - 5
     - Tropopause level 
   * - 6
     - Reports on a single level     
   * - 7
     - Auxiliary levels generated via interpolation from spanning levels
       

obs_bufr_var = [ 'QOB', 'TOB', 'ZOB', 'UOB', 'VOB' ];


Each PrepBUFR message will likely contain multiple observation variables. The obs_bufr_var variable is used to specify which observation variables should be retained or derived. The variable name comes from BUFR file which includes BUFR table. The following BUFR names may be retained: QOB, TOB, ZOB, UOB, and VOB for specific humidity, temperature, height, and the u and v components of winds. The following BUFR names may be derived: D_DPT, D_WIND, D_RH, D_MIXR, D_PRMSL, D_PBL, and D_CAPE for dew point, wind speed, relative humidity, mixing ratio, pressure reduced to MSL, planetary boundary layer height, and convective available potential energy. This configuration replaces obs_grib_code. If the list is empty, all BUFR variables are retained.




obs_bufr_map = [


{ key = 'POB';      val = 'PRES';  },


{ key = 'QOB';      val = 'SPFH';  },


{ key = 'TOB';      val = 'TMP';   },


{ key = 'ZOB';      val = 'HGT';   },


{ key = 'UOB';      val = 'UGRD';  },


{ key = 'VOB';      val = 'VGRD';  },


{ key = 'D_DPT';    val = 'DPT';   },


{ key = 'D_WDIR';   val = 'WDIR';  },


{ key = 'D_WIND';   val = 'WIND';  },


{ key = 'D_RH';     val = 'RH';    },


{ key = 'D_MIXR';   val = 'MIXR';  },


{ key = 'D_PRMSL';  val = 'PRMSL'; },


{ key = 'D_PBL';    val = 'PBL';   },


{ key = 'D_CAPE';   val = 'CAPE';  }


];


The BUFR variable names are not shared with other forecast data. This map is used to convert the BUFR name to the common name, like GRIB2. It allows to share the configuration for forecast data with PB2NC observation data. If there is no mapping, the BUFR variable name will be saved to output NetCDF file.




quality_mark_thresh = 2;


Each observation has a quality mark value associated with it. The quality_mark_thresh is used to stratify out which quality marks will be retained. The value shown above indicates that only observations with quality marks less than or equal to 2 will be retained.




event_stack_flag = TOP;


A PrepBUFR message may contain duplicate observations with different quality mark values. Theevent_stack_flag indicates whether to use the observations at the top of the event stack (observation values have had more quality control processing applied) or the bottom of the event stack (observation values have had no quality control processing applied). The flag value of TOP listed above indicates the observations with the most amount of quality control processing should be used, the BOTTOM option uses the data closest to raw values.




time_summary = {


flag       = FALSE;


raw_data   = FALSE;


beg        = "000000";


end        = "235959";


step       = 300;


width      = 600;


// width   = { beg = -300; end = 300; }


grib_code  = [];


obs_var    = [ "TMP", "WDIR", "RH" ];


type       = [ "min", "max", "range", "mean", "stdev", "median", "p80" ];


vld_freq   = 0;


vld_thresh = 0.0;


}


The time_summary dictionary enables additional processing for observations with high temporal resolution. The flag entry toggles the time_summary on (TRUE) and off (FALSE). If the raw_data flag is set to TRUE, then both the individual observation values and the derived time summary value will be written to the output. If FALSE, only the summary values are written. Observations may be summarized across the user specified time period defined by the beg and end entries in HHMMSS format. The step entry defines the time between intervals in seconds. The width entry specifies the summary interval in seconds. It may either be set as an integer number of seconds for a centered time interval or a dictionary with beginning and ending time offsets in seconds.


This example listed above does a 10-minute time summary (width = 600;) every 5 minutes (step = 300;) throughout the day (beg = “000000”; end = 235959”;). The first interval will be from 23:55:00 the previous day through 00:04:59 of the current day. The second interval will be from 0:00:00 through 00:09:59. And so on.


The two width settings listed above are equivalent. Both define a centered 10-minute time interval. Use the beg and end entries to define uncentered time intervals. The following example requests observations for one hour prior:


width = { beg = -3600; end = 0; }


The summaries will only be calculated for the observations specified in the grib_code or obs_var entries. The grib_code entry is an array of integers while the obs_var entries is an array of strings. The supported summaries are min (minimum), max (maximum), range, mean, stdev (standard deviation), median and p## (percentile, with the desired percentile value specified in place of ##). If multiple summaries are selected in a single run, a string indicating the summary method applied will be appended to the output message type.


The vld_freq and vld_thresh entries specify the required ratio of valid data for an output time summary value to be computed. This option is only applied when these entries are set to non-zero values. The vld_freq entry specifies the expected frequency of observations in seconds. The width of the time window is divided by this frequency to compute the expected number of observations for the time window. The actual number of valid observations is divided by the expected number to compute the ratio of valid data. An output time summary value will only be written if that ratio is greater than or equal to the vld_thresh entry. Detailed information about which observations are excluded is provided at debug level 4.

4.1.3 pb2nc output
~~~~~~~~~~~~~~~~~~

Each NetCDF file generated by the PB2NC tool contains the dimensions and variables shown in Tables [table_reform-point_pb2nc_output_dim] and [table_reform-point_pb2nc_output_vars].

.. list-table:: NetCDF file dimensions for pb2n output pb2n NetCDF DIMENSIONS
   :widths: auto
   :header-rows: 1

   * - pb2nc NetCDF DIMENSIONS
     - 
   * - **NetCDF Dimension**
     - **Description**
   * - mxstr, mxstr2, mxstr3
     - Maximum string lengths (16, 40, and 80)
   * - nobs
     - Number of PrepBUFR observations in the file (UNLIMITED)
   * - nhdr, npbhdr
     - Number of PrepBUFR messages in the file (variable)
   * - nhdr_typ, nhdr_sid, nhdr_vld
     - Number of unique header message type, station ID, and valid time strings (variable)
   * - nobs_qty
     - Number of unique quality control strings (variable)
   * - obs_var_num
     - Number of unique observation variable types (variable)							 


Table 4.3 NetCDF file dimensions for pb2nc output.NetCDF variables in pb2nc output.

.. list-table:: table_reform-point_pb2nc_level_category
   :widths: auto
   :header-rows: 1
		 
   * - pb2nc NetCDF VARIABLES
     -
     -
   * - **NetCDF Variable**
     - **Dimension**
     - **Description**
   * - obs_qty
     - nobs
     - Integer value of the n_obs_qty dimension for the observation quality control string.
   * - obs_hid
     - nobs
     - Integer value of the nhdr dimension for the header arrays with which this observation is associated.
   * - obs_vid
     - nobs
     - Integer value of the obs_var_num dimension for the observation variable name, units, and description.
   * - obs_lvl
     - nobs
     - Floating point pressure level in hPa or accumulation interval.
   * - obs_hgt
     - nobs
     - Floating point height in meters above sea level.
   * - obs_val
     - nobs
     - Floating point observation value.
   * - hdr_typ
     - nhdr
     - Integer value of the nhdr_typ dimension for the message type string.
   * - hdr_sid
     - nhdr
     - Integer value of the nhdr_sid dimension for the station ID string.
   * - hdr_vld
     - nhdr
     - Integer value of the nhdr_vld dimension for the valid time string.
   * - hdr_lat, hdr_lon
     - nhdr
     - Floating point latitude in degrees north and longitude in degrees east.
   * - hdr_elv
     - nhdr
     - Floating point elevation of observing station in meters above sea level.
   * - hdr_prpt_typ
     - npbhdr
     - Integer PrepBUFR report type value.
   * - hdr_irpt_typ
     - npbhdr
     - Integer input report type value.
   * - hdr_inst_typ
     - npbhdr
     - Integer instrument type value.
   * - hdr_typ_table
     - nhdr_typ,
     - mxstr2 Lookup table containing unique message type strings.
   * - hdr_sid_table
     - nhdr_sid,
     - mxstr2 Lookup table containing unique station ID strings.
   * - hdr_vld_table
     - nhdr_vld, mxstr
     - Lookup table containing unique valid time strings in YYYYMMDD_HHMMSS UTC format.
   * - obs_qty_table
     - nobs_qty, mxstr
     - Lookup table containing unique quality control strings.
   * - obs_var
     - obs_var_num, mxstr
     - Lookup table containing unique observation variable names.
   * - obs_unit
     - obs_var_num, mxstr2
     - Lookup table containing a units string for the unique observation variable names in obs_var.
   * - obs_desc
     - obs_var_num, mxstr3
     - Lookup table containing a description string for the unique observation variable names in obs_var.


4.2 ASCII2NC tool
_________________

This section describes how to run the ASCII2NC tool. The ASCII2NC tool is used to reformat ASCII point observations into the NetCDF format expected by the Point-Stat tool. For those users wishing to verify against point observations that are not available in PrepBUFR format, the ASCII2NC tool provides a way of incorporating those observations into MET. If the ASCII2NC tool is used to perform a reformatting step, no configuration file is needed. However, for more complex processing, such as summarizing time series observations, a configuration file may be specified. For details on the configuration file options, see the share/met/config/README file and example configuration files distributed with the MET code.

Initial versions of the ASCII2NC tool supported only a simple 11 column ASCII point observation format. It currently supports point observation data in the following formats: the default 11 column format, little_r format, SURFace RADiation (SURFRAD) and Integrated Surface Irradiance Study (ISIS) formats (found at http://www.esrl.noaa.gov/gmd/grad/surfrad/), the Western Wind and Solar Integration Study (WWSIS) format, and the AErosol RObotic NEtwork (AERONET) versions 2 and 3 format (found at http://aeronet.gsfc.nasa.gov/). WWSIS data are available by request from National Renewable Energy Laboratory (NREL) in Boulder, CO.

MET version 9.0 adds support for the passing observations to ascii2nc using a Python script with the “-format python” option. An example of running ASCII2NC with Python embedding is included below.

The default ASCII point observation format consists of one row of data per observation value. Each row of data consists of 11 columns as shown in Table [table_reform-point_ascii2nc_format].

Input MET ascii2nc point observation format

4.2.1 ascii2nc usage
~~~~~~~~~~~~~~~~~~~~

Once the ASCII point observations have been formatted as expected, the ASCII file is ready to be processed by the ASCII2NC tool. The usage statement for ASCII2NC tool is shown below:

Usage: ascii2nc

{\hskip 0.5in}ascii_file1 [ascii_file2 ... ascii_filen]

{\hskip 0.5in}netcdf_file

{\hskip 0.5in}[-format ASCII_format]

{\hskip 0.5in}[-config file]

{\hskip 0.5in}[-mask_grid string]

{\hskip 0.5in}[-mask_poly file]

{\hskip 0.5in}[-mask_sid file|list]

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

{\hskip 0.5in}[-compress level]

ascii2nc has two required arguments and can take several optional ones.

Required arguments for ascii2nc

1. The ascii_file argument is the ASCII point observation file(s) to be processed. If using Python embedding with “-format python” provide a quoted string containing the Python script to be run followed by any command line arguments that script takes.

2. The netcdf_file argument is the NetCDF output file to be written.

Optional arguments for ascii2nc

3. The -format ASCII_format option may be set to “met_point”, “little_r”, “surfrad”, “wwsis”, “aeronet”, “aeronetv2”, “aeronetv3”, or “python”. If passing in ISIS data, use the “surfrad” format flag.

4. The -config file option is the configuration file for generating time summaries.

												   5. The -mask_grid string option is a named grid or a gridded data file to filter the point observations spatially.
												   6. The -mask_poly file option is a polyline masking file to filter the point observations spatially.
												   7. The -mask_sid file|list option is a station ID masking file or a comma-separated list of station ID's to filter the point observations spatially. See the description of the “sid” entry in [subsec:IO_General-MET-Config-Options].

8. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

9. The -v level option indicates the desired level of verbosity. The value of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

10. The -compress level option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

An example of the ascii2nc calling sequence is shown below:
		       ascii2nc sample_ascii_obs.txt \

		       sample_ascii_obs.nc

In this example, the ASCII2NC tool will reformat the input sample_ascii_obs.txt file into NetCDF format and write the output to a file named sample_ascii_obs.nc.

4.2.1.1 Python Embedding for Point Observations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here is an example of processing the same set of observations but using Python embedding instead:

ascii2nc -format python \

“MET_BASE/python/read_ascii_point.py sample_ascii_obs.txt" \

sample_ascii_obs_python.nc

Please refer to Appendix [chap:App_F_Python_Embedding] for more details about Python embedding in MET.

4.2.2 ascii2nc configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the ASCII2NC tool named Ascii2NcConfig_default can be found in the installed share/met/config directory. It is recommended that users make a copy of this file prior to modifying its contents.

The ASCII2NC configuration file is optional and only necessary when defining time summaries or message type mapping for little_r data. The contents of the default ASCII2NC configuration file are described below.


version = "VN.N";

The configuration options listed above are common to many MET tools and are described in Section [subsec:IO_General-MET-Config-Options].


time_summary = { ... }


The time_summary feature was implemented to allow additional processing of observations with high temporal resolution, such as SURFRAD data every 5 minutes. This option is described in Section [subsec:pb2nc-configuration-file].




message_type_map = [


{ key = "FM-12 SYNOP";  val = "ADPSFC"; },


{ key = "FM-13 SHIP";   val = "SFCSHP"; },


{ key = "FM-15 METAR";  val = "ADPSFC"; },


{ key = "FM-18 BUOY";   val = "SFCSHP"; },


{ key = "FM-281 QSCAT"; val = "ASCATW"; },


{ key = "FM-32 PILOT";  val = "ADPUPA"; },


{ key = "FM-35 TEMP";   val = "ADPUPA"; },


{ key = "FM-88 SATOB";  val = "SATWND"; },


{ key = "FM-97 ACARS";  val = "AIRCFT"; }


];


This entry is an array of dictionaries, each containing a key string and val string which define a mapping of input strings to output message types. This mapping is currently only applied when converting input little_r report types to output message types.


4.2.3 ascii2nc output
~~~~~~~~~~~~~~~~~~~~~

The NetCDF output of the ASCII2NC tool is structured in the same way as the output of the PB2NC tool described in Section [subsec:pb2nc-output].


4.3 MADIS2NC tool
_________________


This section describes how to run the MADIS2NC tool. The MADIS2NC tool is used to reformat Meteorological Assimilation Data Ingest System (MADIS) point observations into the NetCDF format expected by the MET statistics tools. More information about MADIS data and formatting is available at http://madis.noaa.gov. Since the MADIS2NC tool simply performs a reformatting step, no configuration file is needed. The MADIS2NC tool supports many of the MADIS data types, as listed in the usage statement below. Support for additional MADIS data types may be added in the future based on user feedback.


4.3.1 madis2nc usage
~~~~~~~~~~~~~~~~~~~~


The usage statement for MADIS2NC tool is shown below:


Usage: madis2nc


{\hskip 0.5in}madis_file [madis_file2 ... madis_filen]


{\hskip 0.5in}out_file


{\hskip 0.5in}-type str


{\hskip 0.5in}[-config file]


{\hskip 0.5in}[-qc_dd list]


{\hskip 0.5in}[-lvl_dim list]


{\hskip 0.5in}[-rec_beg n]


{\hskip 0.5in}[-rec_end n]


{\hskip 0.5in}[-mask_grid string]


{\hskip 0.5in}[-mask_poly file]


{\hskip 0.5in}[-mask_sid file|list]


{\hskip 0.5in}[-log file]


{\hskip 0.5in}[-v level]


{\hskip 0.5in}[-compress level]


madis2nc has required arguments and can also take optional ones.


Required arguments for madis2nc


1. The madis_file argument is one or more input MADIS point observation files to be processed.


2. The netcdf_file argument is the NetCDF output file to be written.


3. The argument -type str is type of MADIS observations (metar, raob, profiler, maritime, mesonet or acarsProfiles).


   Optional arguments for madis2nc


4. The -config file option specifies the configuration file to generate summaries of the fields in the ASCII files.


5. The -qc_dd list option specifies a comma-separated list of QC flag values to be accepted(Z,C,S,V,X,Q,K,G,B).


6. The -lvl_dim list option specifies a comma-separated list of vertical level dimensions to be processed.


7. To specify the exact records to be processed, the -rec_beg n specifies the index of the first MADIS record to process and -rec_end n specifies the index of the last MADIS record to process. Both are zero-based.


8. The -mask_grid string option specifies a named grid or a gridded data file for filtering the point observations spatially.


9. The -mask_poly file option defines a polyline masking file for filtering the point observations spatially.


10. The -mask_sid file|list option is a station ID masking file or a comma-separated list of station ID's for filtering the point observations spatially. See the description of the “sid” entry in [subsec:IO_General-MET-Config-Options].


11. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.


12. The -v level option indicates the desired level of verbosity. The value of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.


13. The -compress level option specifies the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.


An example of the madis2nc calling sequence is shown below:


    madis2nc sample_madis_obs.nc \


    sample_madis_obs_met.nc -log madis.log -v 3


In this example, the MADIS2NC tool will reformat the input sample_madis_obs.nc file into NetCDF format and write the output to a file named sample_madis_obs_met.nc. Warnings and error messages will be written to the madis.log file, and the verbosity level of logging is three.


4.3.2 madis2nc configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


The default configuration file for the MADIS2NC tool named Madis2NcConfig_default can be found in the installed share/met/config directory. It is recommended that users make a copy of this file prior to modifying its contents.


The MADIS2NC configuration file is optional and only necessary when defining time summaries. The contents of the default MADIS2NC configuration file are described below.




version = "VN.N";


The configuration options listed above are common to many MET tools and are described in Section [subsec:IO_General-MET-Config-Options].




time_summary = { ... }


The time_summary dictionary is described in Section [subsec:pb2nc-configuration-file].


4.3.3 madis2nc output
~~~~~~~~~~~~~~~~~~~~~

The NetCDF output of the MADIS2NC tool is structured in the same way as the output of the PB2NC tool described in Section [subsec:pb2nc-output].


4.4 LIDAR2NC tool
_________________


The LIDAR2NC tool creates a NetCDF point observation file from a CALIPSO HDF data file. Not all of the data present in the CALIPSO file is reproduced in the output, however. Instead, the output focuses mostly on information about clouds (as opposed to aerosols) as seen by the satellite along its ground track.


4.4.1 lidar2nc usage
~~~~~~~~~~~~~~~~~~~~

The usage statement for LIDAR2NC tool is shown below:


Usage: lidar2nc


{\hskip 0.5in}lidar_file


{\hskip 0.5in}-out out_file


{\hskip 0.5in}[-log file]


{\hskip 0.5in}[-v level]


{\hskip 0.5in}[-compress level]


Unlike most of the MET tools, lidar2nc does not use a config file. Currently, the options needed to run lidar2nc are not complex enough to require one.


Required arguments for lidar2nc


1. The lidar_file argument is the input HDF lidar data file to be processed. Currently, CALIPSO files are supported but support for additional file types will be added in future releases.


2. The out_file argument is the NetCDF output file to be written.


   Optional arguments for lidar2nc


3. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

4. The -v level option indicates the desired level of verbosity. The value of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

5. The -compress level option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

4.4.2 lidar2nc output
~~~~~~~~~~~~~~~~~~~~~

Each observation type in the lidar2nc output is assigned a GRIB code. These are outlined in Table [lidar2nc_grib_code_table]. GRIB codes were assigned to these fields arbitrarily, with GRIB codes in the 600s denoting individual bit fields taken from the feature classification flag field in the CALIPSO file.


We will not give a detailed description of each CALIPSO data product that lidar2nc reads. Users should refer to existing CALIPSO documentation for this information. We will, however, give some explanation of how the cloud layer base and top information is encoded in the lidar2nc NetCDF output file.


Layer_Base gives the elevation in meters above ground level of the cloud base for each cloud level at each observation location. Similarly, Layer_Top gives the elevation of the top of each cloud layer. Note that if there are multiple cloud layers at a particular location, then there will be more than one base (or top) given for that location. For convenience, Min_Base and Max_Top give, respectively, the base elevation for the bottom cloud layer, and the top elevation for the top cloud layer. For these data types, there will be only one value per observation location regardless of how many cloud layers there are at that location.


lidar2nc GRIB codes and their meaning, units, and abbreviations


4.5 Point2Grid tool
___________________

The Point2Grid tool takes point observations from a NetCDF output file from one of the four previously mentioned MET tools (ascii2nc, madis2nc, pb2nc, lidar2nc) and creates a gridded NetCDF file. The other point observations are GOES-16/17 input files in NetCDF format (especially, Aerosol Optical Depth. Future development will include support for reading input files not produced from MET tools.


4.5.1 point2grid usage
~~~~~~~~~~~~~~~~~~~~~~

The usage statement for the Point2Grid tool is shown below:


Usage: point2grid


{\hskip 0.5in}input_filename


{\hskip 0.5in}to_grid


{\hskip 0.5in}output_filename


{\hskip 0.5in}-field string


{\hskip 0.5in}[-config file]


{\hskip 0.5in}[-qc flags]


{\hskip 0.5in}[-adp adp_file_name]


{\hskip 0.5in}[-method type]


{\hskip 0.5in}[-gaussian_dx n]


{\hskip 0.5in}[-gaussian_radius n]


{\hskip 0.5in}[-prob_cat_thresh string]


{\hskip 0.5in}[-vld_thresh n]


{\hskip 0.5in}[-name list]


{\hskip 0.5in}[-log file]


{\hskip 0.5in}[-v level]


{\hskip 0.5in}[-compress level]


Required arguments for point2grid


1. The input_filename argument indicates the name of the input NetCDF file to be processed. Currently, only NetCDF files produced from the ascii2nc, madis2nc, pb2nc, and lidar2nc are supported. And AOD dataset from GOES16/17 are supported, too. Support for additional file types will be added in future releases.

2. The to_grid argument defines the output grid as: (1) a named grid, (2) the path to a gridded data file, or (3) an explicit grid specification string.


3. The output_filename argument is the name of the output NetCDF file to be written.


4. The -field string argument is a string that defines the data to be regridded. It may be used multiple times. If -adp option is given (for AOD data from GOES16/17), the name consists with the variable name from the input data file and the variable name from ADP data file (for example, “AOD_Smoke” or “AOD_Dust”: getting AOD variable from the input data and applying smoke or dust variable from ADP data file).


   Optional arguments for point2grid


5. The -config file option is the configuration file to be used.

6. The -qc flags option specifies a comma-separated list of quality control (QC) flags, for example “0,1”. This should only be applied if grid_mapping is set to “goes_imager_projection” and the QC variable exists.

7. The -adp adp_file_name option provides an additional Aerosol Detection Product (ADP) information on aerosols, dust, and smoke. This option is ignored if the requested variable is not AOD (“AOD_Dust” or “AOD_Smoke”) from GOES16/17. The gridded data is filtered by the presence of dust/smoke. If -qc options is given, it's applied to QC of dust/smoke, too (First filtering with AOD QC values and the second filtering with dust/smoke QC values).

8. The -method type option specifies the regridding method. The default method is UW_MEAN.

9. The -gaussian_dx n option defines the distance interval for Gaussian smoothing. The default is 81.271 km. Ignored if the method is not GAUSSIAN.


10. The -gaussian_radius n option defines the radius of influence for Gaussian interpolation. The default is 120. Ignored if the method is not GAUSSIAN.


11.The -prob_cat_thresh string option sets the threshold to compute the probability of occurrence. The default is set to disabled. This option is relevant when calculating practically perfect forecasts.


12. The -vld_thresh n option sets the required ratio of valid data for regridding. The default is 0.5.


13. The -name list option specifies a comma-separated list of output variable names for each field specified.


14. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.


15. The -v level option indicates the desired level of verbosity. The value of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.


16. The -compress level option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.


For the GOES-16 and GOES-17 data, the computing lat/long is time consuming. So the computed coordinate (lat/long) is saved into the NetCDF file to the environment variable MET_TMP_DIR or /tmp if MET_TMP_DIR is not defined. The computing lat/long step can be skipped if the coordinate file is given through the environment variable MET_GEOSTATIONARY_DATA. An example of call point2grid to process GOES-16 AOD data is shown below:


point2grid \


OR_ABI-L2-AODC-M3_G16_s20181341702215_e20181341704588_c20181341711418.nc \


G212 \


regrid_data_plane_GOES-16_AOD_TO_G212.nc \


-field 'name="AOD"; level="(*,*)";' \


-qc 0,1,2


-method MAX -v 1


When processing GOES-16 data, the -qc option may also be used to specify the acceptable quality control flag values. The example above regrids the GOES-16 AOD values to NCEP Grid number 212 (which QC flags are high, medium, and low), writing to the output the maximum AOD value falling inside each grid box.


4.5.2 point2grid output
~~~~~~~~~~~~~~~~~~~~~~~

The point2grid tool will output a gridded NetCDF file containing the following:


1. Latitude


2. Longitude


3. The variable specified in the -field string regridded to the grid defined in the to_grid argument.


4. The count field which represents the number of point observations that were included calculating the value of the variable at that grid cell.


5. The mask field which is a binary field representing the presence or lack thereof of point observations at that grid cell. A value of “1” indicates that there was at least one point observation within the bounds of that grid cell and a value of “0” indicates the lack of point observations at that grid cell.


6. The probability field which is the probability of the event defined by the -prob_cat_thresh command line option. The output variable name includes the threshold used to define the probability. Ranges from 0 to 1.


7. The probability mask field which is a binary field that represents whether or not there is probability data at that grid point. Can be either “0” or “1” with “0” meaning the probability value does not exist and a value of “1” meaning that the probability value does exist. 

.. _tc-pairs:

Chapter 20 TC-Pairs Tool
========================

20.1 Introduction
_________________

The TC-Pairs tool provides verification for tropical cyclone forecasts in ATCF file format. It matches an ATCF format tropical cyclone (TC) forecast with a second ATCF format reference TC dataset (most commonly the Best Track analysis). The TC-Pairs tool processes both track and intensity adeck data and probabilistic edeck data. The adeck matched pairs contain position errors, as well as wind, sea level pressure, and distance to land values for each TC dataset. The edeck matched pairs contain probabilistic forecast values and the verifying observation values. The pair generation can be subset based on user-defined filtering criteria. Practical aspects of the TC-Pairs tool are described in Section [sec:TC-Pairs_Practical-information]. 

20.2 Practical information

This section describes how to configure and run the TC-Pairs tool. The TC-Pairs tool is used to match a tropical cyclone model forecast to a corresponding reference dataset. Both tropical cyclone forecast/reference data must be in ATCF format. Output from the TC-dland tool (NetCDF gridded distance file) is also a required input for the TC-Pairs tool. It is recommended to run tc_pairs on a storm-by-storm basis, rather than over multiple storms or seasons to avoid memory issues.

20.2.1 tc_pairs usage

The usage statement for tc_pairs is shown below:

Usage: tc_pairs

{\hskip 0.5in}-adeck source and/or -edeck source

{\hskip 0.5in}-bdeck source

{\hskip 0.5in}-config file

{\hskip 0.5in}[-out base]

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

tc_pairs has required arguments and can accept several optional arguments.

Required arguments for tc_pairs

1. The -adeck source argument indicates the adeck ATCF format data source containing tropical cyclone model forecast (output from tracker) data to be verified. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. The -adeck or -edeck option must be used at least once.

2. The -edeck source argument indicates the edeck ATCF format data source containing probabilistic track data to be verified. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. The -adeck or -edeck option must be used at least once.

3. The -bdeck source argument indicates the ATCF format data source containing the tropical cyclone reference dataset to be used for verifying the adeck source. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. This source is expected to be the NHC Best Track Analysis, but could also be any ATCF format reference.

4. The -config file argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for tc_pairs

5. The -out base argument indicates the path of the output file base. This argument overrides the default output file base (./out_tcmpr)

6. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

7. The -v level option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

MET version 6.0 supports only the rapid intensification (RI) edeck probability type but support for additional edeck probability types will be added in future releases. At least one -adeck or -edeck option must be specified. The -adeck, -edeck, and -bdeck options may optionally be followed with suffix=string to append that string to all model names found within that data source. This option may be useful when processing track data from two different sources which reuse the same model names.

An example of the tc_pairs calling sequence is shown below:

tc_pairs -adeck aal092010.dat -bdeck bal092010.dat -config TCPairsConfig

In this example, the TC-Pairs tool matches the model track (aal092010.dat) and the best track analysis (bal092010.dat) for the 9th Atlantic Basin storm in 2010. The track matching and subsequent error information is generated with configuration options specified in the TCPairsConfig file.

The TC-Pairs tool implements the following logic:

• Parse the adeck, edeck, and bdeck data files and store them as track objects.

• Apply configuration file settings to filter the adeck, edeck, and bdeck track data down to a subset of interest.

• Apply configuration file settings to derive additional adeck track data, such as interpolated tracks, consensus tracks, time-lagged tracks, and statistical track and intensity models.

• For each adeck track that was parsed or derived, search for a matching bdeck track with the same basin and cyclone number and overlapping valid times. If not matching against the BEST track, also ensure that the model initialization times match.

• For each adeck/bdeck track pair, match up their track points in time, lookup distances to land, compute track location errors, and write an output TCMPR line for each track point.

• For each set of edeck probabilities that were parsed, search for a matching bdeck track.

• For each edeck/bdeck pair, write paired edeck probabilities and matching bdeck values to output PROBRIRW lines.

20.2.2 tc_pairs configuration file

The default configuration file for the TC-Pairs tool named 'TCPairsConfig_default' can be found in the installed share/met/config/ directory. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

The contents of the tc_pairs configuration file are described below.



storm_id     = [];

basin        = [];

cyclone      = [];

storm_name   = [];

init_beg     = "";

init_end     = "";

init_inc     = [];

init_exc     = [];

valid_beg    = "";

valid_end    = "";

init_hour    = [];

init_mask    = [];

lead_req     = [];

valid_mask   = [];

match_points = TRUE;

version      = "VN.N";

The configuration options listed above are common to multiple MET tools and are described in Section [subsec:IO_MET-TC-Config-Options].



model = [ "DSHP", "LGEM", "HWRF" ];

The model variable contains a list of comma-separated models to be used. The models are identified with an ATCF ID (normally four unique characters). This model identifier should match the model column in the ATCF format input file. An empty list indicates that all models in the input file(s) will be processed.



check_dup = FALSE;

The check_dup flag expects either TRUE and FALSE, indicating whether the code should check for duplicate ATCF lines when building tracks. Setting check_dup to TRUE will check for duplicated lines, and produce output information regarding the duplicate. The duplicated ATCF line will not be processed in the tc_pairs output. Setting check_dup to FALSE, will still exclude tracks that decrease with time, and will overwrite repeated lines, but specific duplicate log information will not be output. Setting check_dup to FALSE will make parsing the track quicker.



interp12 = NONE;

The interp12 flag expects the entry NONE, FILL, or REPLACE, indicating whether special processing should be performed for interpolated forecasts. The NONE option indicates no changes are made to the interpolated forecasts. The FILL and REPLACE (default) options determine when the 12-hour interpolated forecast (normally indicated with a "2" or "3" at the end of the ATCF ID) will be renamed with the 6-hour interpolated ATCF ID (normally indicated with the letter "I" at the end of the ATCF ID). The FILL option renames the 12-hour interpolated forecasts with the 6-hour interpolated forecast ATCF ID only when the 6-hour interpolated forecasts is missing (in the case of a 6-hour interpolated forecast which only occurs every 12-hours (e.g. EMXI, EGRI), the 6-hour interpolated forecasts will be "filled in" with the 12-hour interpolated forecasts in order to provide a record every 6-hours). The REPLACE option renames all 12-hour interpolated forecasts with the 6-hour interpolated forecasts ATCF ID regardless of whether the 6-hour interpolated forecast exists. The original 12-hour ATCF ID will also be retained in the output file (all modified ATCF entries will appear at the end of the TC-Pairs output file). This functionality expects both the 12-hour and 6-hour early (interpolated) ATCF IDs are listed in the model field.



consensus = [

   {

      name     = "CON1";

      members  = [ "MOD1", "MOD2", "MOD3" ];

      required = [   true,  false, false  ];

      min_req  = 2;

   }

];

The consensus field allows the user to generate a user-defined consensus forecasts from any number of models. All models used in the consensus forecast need to be included in the model field (1st entry in TCPairsConfig_default). The name field is the desired consensus model name. The members field is a comma-separated list of model IDs that make up the members of the consensus. The required field is a comma-separated list of true/false values associated with each consensus member. If a member is designated as true, the member is required to be present in order for the consensus to be generated. If a member is false, the consensus will be generated regardless of whether the member is present. The length of the required array must be the same length as the members array. The min_req field is the number of members required in order for the consensus to be computed. The required and min_req field options are applied at each forecast lead time. If any member of the consensus has a non-valid position or intensity value, the consensus for that valid time will not be generated.



lag_time = [ “06”, “12” ];

The lag_time field is a comma-separated list of forecast lag times to be used in HH[MMSS] format. For each adeck track identified, a lagged track will be derived for each entry. In the tc_pairs output, the original adeck record will be retained, with the lagged entry listed as the adeck name with "_LAG_HH" appended.



best_technique = [ "BEST" ];

best_baseline  = [ "BCLP", "BCD5", "BCLA" ];

The best_technique field specifies a comma-separated list of technique name(s) to be interpreted as BEST track data. The default value (BEST) should suffice for most users. The best_baseline field specifies a comma-separated list of CLIPER/SHIFOR baseline forecasts to be derived from the best tracks. Specifying multiple best_technique values and at least one best_baseline value results in a warning since the derived baseline forecast technique names may be used multiple times.

The following are valid baselines for the best_baseline field:

BTCLIP: Neumann original 3-day CLIPER in best track mode. Used for the Atlantic basin only. Specify model as BCLP.

BTCLIP5: 5-day CLIPER (Aberson, 1998)/SHIFOR (DeMaria and Knaff, 2001) in best track mode for either Atlantic or eastern North Pacific basins. Specify model as BCS5.

BTCLIPA: Sim Aberson's recreation of Neumann original 3-day CLIPER in best-track mode. Used for Atlantic basin only. Specify model as BCLA.



oper_technique = [ "CARQ" ];

oper_baseline  = [ "OCLP", "OCS5", "OCD5" ];

The oper_technique field specifies a comma-separated list of technique name(s) to be interpreted as operational track data. The default value (CARQ) should suffice for most users. The oper_baseline field specifies a comma-separated list of CLIPER/SHIFOR baseline forecasts to be derived from the operational tracks. Specifying multiple oper_technique values and at least one oper_baseline value results in a warning since the derived baseline forecast technique names may be used multiple times.

The following are valid baselines for the oper_baseline field:

OCLIP: Merrill modified (operational) 3-day CLIPER run in operational mode. Used for Atlantic basin only. Specify model as OCLP.

OCLIP5: 5-day CLIPER (Aberson, 1998)/ SHIFOR (DeMaria and Knaff, 2001) in operational mode, rerun using CARQ data. Specify model as OCS5.

OCLIPD5: 5-day CLIPER (Aberson, 1998)/ DECAY-SHIFOR (DeMaria and Knaff, 2001). Specify model as OCD5.



anly_track = BDECK;

Analysis tracks consist of multiple track points with a lead time of zero for the same storm. An analysis track may be generated by running model analysis fields through a tracking algorithm. The anly_track field specifies which datasets should be searched for analysis track data and may be set to NONE, ADECK, BDECK, or BOTH. Use BOTH to create pairs using two different analysis tracks.



match_points = TRUE;

The match_points field specifies whether only those track points common to both the adeck and bdeck tracks should be written out. If match_points is selected as FALSE, the union of the adeck and bdeck tracks will be written out, with "NA" listed for unmatched data.



dland_file = "MET_BASE/tc_data/dland_global_tenth_degree.nc";

The dland_file string specifies the path of the NetCDF format file (default file: dland_global_tenth_degree.nc) to be used for the distance to land check in the tc_pairs code. This file is generated using tc_dland (default file provided in installed share/met/tc_data directory).



watch_warn = {

   file_name   = "MET_BASE/tc_data/wwpts_us.txt";

   time_offset = -14400;

}

The watch_warn field specifies the file name and time applied offset to the watch_warn flag. The file_name string specifies the path of the watch/warning file to be used to determine when a watch or warning is in affect during the forecast initialization and verification times. The default file is named wwpts_us.txt, which is found in the installed share/met/tc_data/ directory within the MET build. The time_offset string is the time window (in seconds) assigned to the watch/warning. Due to the non-uniform time watches and warnings are issued, a time window is assigned for which watch/warnings are included in the verification for each valid time. The default watch/warn file is static, and therefore may not include warned storms beyond the current MET code release date; therefore users may wish to contact met_help@ucar.edu to obtain the most recent watch/warning file if the static file does not contain storms of interest.

20.2.3 tc_pairs output

TC-Pairs produces output in TCST format. The default output file name can be overwritten using the -out file argument in the usage statement. The TCST file output from TC-Pairs may be used as input into the TC-Stat tool. The header column in the TC-Pairs output is described in Table [TCST Header].

Header information for TC-Pairs TCST output.

Format information for TCMPR (Tropical Cyclone Matched Pairs) output line type.

Format information for PROBRIRW (Probability of Rapid Intensification) output line type.

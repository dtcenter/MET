.. _gsi_tools:

Chapter 11 GSI Tools
====================

Gridpoint Statistical Interpolation (GSI) diagnostic files are binary files written out from the data assimilation code before the first and after each outer loop. The files contain useful information about how a single observation was used in the analysis by providing details such as the innovation (O-B), observation values, observation error, adjusted observation error, and quality control information.

For more detail on generating GSI diagnostic files and their contents, see the GSI User's Guide: http://www.dtcenter.org/com-GSI/users/docs/index.php

When MET reads GSI diagnostic files, the innovation (O-B; generated prior to the first outer loop) or analysis increment (O-A; generated after the final outer loop) is split into separate values for the observation (OBS) and the forecast (FCST), where the forecast value corresponds to the background (O-B) or analysis (O-A).

MET includes two tools for processing GSI diagnostic files. The gsid2mpr tool reformats individual GSI diagnostic files into the MET matched pair (MPR) format, similar to the output of the Point-Stat tool. The gsidens2orank tool processes an ensemble of GSI diagnostic files and reformats them into the MET observation rank (ORANK) line type, similar to the output of the Ensemble-Stat tool. The output of both tools may be passed to the Stat-Analysis tool to compute a wide variety of continuous, categorical, and ensemble statistics.

11.1 GSID2MPR tool

This section describes how to run the tool gsid2mpr tool. The gsid2mpr tool reformats one or more GSI diagnostic files into an ASCII matched pair (MPR) format, similar to the MPR output of the Point-Stat tool. The output MPR data may be passed to the Stat-Analysis tool to compute a wide variety of continuous or categorical statistics.

11.1.1 gsid2mpr usage

The usage statement for the gsid2mpr tool is shown below:

Usage: gsid2mpr

{\hskip 0.5in}gsi_file_1 [gsi_file_2 ... gsi_file_n]

{\hskip 0.5in}[-swap]

{\hskip 0.5in}[-no_check_dup]

{\hskip 0.5in}[-channel n]

{\hskip 0.5in}[-set_hdr col_name value]

{\hskip 0.5in}[-suffix string]

{\hskip 0.5in}[-outdir path]

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

gsid2mpr has one required argument and and accepts several optional ones.

Required arguments for gsid2mpr

1. The gsi_file_1 [gsi_file2 ... gsi_file_n] argument indicates the GSI diagnostic files (conventional or radiance) to be reformatted.

   Optional arguments for gsid2mpr

   2. The -swap option switches the endianness when reading the input binary files.

      3. The -no_check_dup option disables the checking for duplicate matched pairs which slows down the tool considerably for large files.

	 4. The -channel n option overrides the default processing of all radiance channels with the values of a comma-separated list.

	    5. The -set_hdr col_name value option specifies what should be written to the output header columns.

	       6. The -suffix string option overrides the default output filename suffix (.stat).

		  7. The -outdir path option overrides the default output directory (./).

		     8. The -log file option outputs log messages to the specified file.

			9. The -v level option overrides the default level of logging (2).

			   An example of the gsid2mpr calling sequence is shown below:

			   gsid2mpr diag_conv_ges.mem001 \

			   -set_hdr MODEL GSI_MEM001 \

			   -outdir out

			   In this example, the gsid2mpr tool will process a single input file named diag_conv_ges.mem001 file, set the output MODEL header column to GSI_MEM001, and write output to the out directory. The output file is named the same as the input file but a .stat suffix is added to indicate its format.

			   11.1.2 gsid2mpr output

			   The gsid2mpr tool performs a simple reformatting step and thus requires no configuration file. It can read both conventional and radiance binary GSI diagnostic files. Support for additional GSI diagnostic file type may be added in future releases. Conventional files are determined by the presence of the string conv in the filename. Files that are not conventional are assumed to contain radiance data. Multiple files of either type may be passed in a single call to the gsid2mpr tool. For each input file, an output file will be generated containing the corresponding matched pair data.

			   The gsid2mpr tool writes the same set of MPR output columns for the conventional and radiance data types. However, it also writes additional columns at the end of the MPR line which depend on the input file type. Those additional columns are described in the following tables.

			   Format information for GSI Diagnostic Conventional MPR (Matched Pair) output line type.

			   Format information for GSI Diagnostic Radiance MPR (Matched Pair) output line type.

			   The gsid2mpr output may be passed to the Stat-Analysis tool to derive additional statistics. In particular, users should consider running the aggregate_stat job type to read MPR lines and compute partial sums (SL1L2), continuous statistics (CNT), contingency table counts (CTC), or contingency table statistics (CTS). Stat-Analysis has been enhanced to parse any extra columns found at the end of the input lines. Users can filter the values in those extra columns using the -column_thresh and -column_str job command options.

			   An example of the Stat-Analysis calling sequence is shown below:

			   stat_analysis -lookin diag_conv_ges.mem001.stat \

			   -job aggregate_stat -line_type MPR -out_line_type CNT \

			   -fcst_var t -column_thresh ANLY_USE eq1

			   In this example, the Stat-Analysis tool will read MPR lines from the input file nameddiag_conv_ges.mem001.stat, retain only those lines where the FCST_VAR column indicates temperature (t) and where the ANLY_USE column has a value of 1.0, and derive continuous statistics.

			   11.2 GSIDENS2ORANK tool

			   This section describes how to run the tool gsidens2orank tool. The gsidens2orank tool processes an ensemble of GSI diagnostic files and reformats them into the MET observation rank (ORANK) line type, similar to the output of the Ensemble-Stat tool. The ORANK line type contains ensemble matched pair information and is analogous to the MPR line type for a deterministic model. The output ORANK data may be passed to the Stat-Analysis tool to compute ensemble statistics.

			   11.2.1 gsidens2orank usage

			   The usage statement for the gsidens2orank tool is shown below:

			   Usage: gsidens2orank

			   {\hskip 0.5in}ens_file_1 ... ens_file_n | ens_file_list

			   {\hskip 0.5in}-out path

			   {\hskip 0.5in}[-ens_mean path]

			   {\hskip 0.5in}[-swap]

			   {\hskip 0.5in}[-rng_name str]

			   {\hskip 0.5in}[-rng_seed str]

			   {\hskip 0.5in}[-set_hdr col_name value]

			   {\hskip 0.5in}[-log file]

			   {\hskip 0.5in}[-v level]

			   gsidens2orank has three required arguments and accept several optional ones.

			   Required arguments for gsidens2orank

			   1. The ens_file_1 ... ens_file_n argument is a list of ensemble binary GSI diagnostic files to be reformatted.

			      2. The ens_file_list argument is an ASCII file containing a list of ensemble GSI diagnostic files.

				 3. The -out path argument specifies the name of the output .stat file.

				    Optional arguments for gsidens2orank

				    4. The -ens_mean path option is the ensemble mean binary GSI diagnostic file.

				       5. The -swap option switches the endianness when reading the input binary files.

					  6. The -channel n option overrides the default processing of all radiance channels with a comma-separated list.

					     7. The -rng_name str option overrides the default random number generator name (mt19937).

						8. The -rng_seed str option overrides the default random number generator seed.

						   9. The -set_hdr col_name value option specifies what should be written to the output header columns.

						      10. The -log file option outputs log messages to the specified file.

							  11. The -v level option overrides the default level of logging (2).

							      An example of the gsidens2orank calling sequence is shown below:

							      gsidens2orank diag_conv_ges.mem* \

							      -ens_mean diag_conv_ges.ensmean \

							      -out diag_conv_ges_ens_mean_orank.txt

							      In this example, the gsidens2orank tool will process all of the ensemble members whose file name matches diag_conv_ges.mem*, write output to the file named diag_conv_ges_ens_mean_orank.txt, and populate the output ENS_MEAN column with the values found in the diag_conv_ges.ensmean file rather than computing the ensemble mean values from the ensemble members on the fly.

							      11.2.2 gsidens2orank output

							      The gsidens2orank tool performs a simple reformatting step and thus requires no configuration file. The multiple files passed to it are interpreted as members of the same ensemble. Therefore, each call to the tool processes exactly one ensemble. All input ensemble GSI diagnostic files must be of the same type. Mixing conventional and radiance files together will result in a runtime error. The gsidens2orank tool processes each ensemble member and keeps track of the observations it encounters. It constructs a list of the ensemble values corresponding to each observation and writes an output ORANK line listing the observation value, its rank, and all the ensemble values. The random number generator is used by the gsidens2orank tool to randomly assign a rank value in the case of ties.

							      The gsid2mpr tool writes the same set of ORANK output columns for the conventional and radiance data types. However, it also writes additional columns at the end of the ORANK line which depend on the input file type. The extra columns are limited to quantities which remain constant over all the ensemble members and are therefore largely a subset of the extra columns written by the gsid2mpr tool. Those additional columns are described in the following tables.

							      Format information for GSI Diagnostic Conventional ORANK (Observation Rank) output line type.

							      Format information for GSI Diagnostic Radiance ORANK (Observation Rank) output line type.

							      The gsidens2orank output may be passed to the Stat-Analysis tool to derive additional statistics. In particular, users should consider running the aggregate_stat job type to read ORANK lines and ranked histograms (RHIST), probability integral transform histograms (PHIST), and spread-skill variance output (SSVAR). Stat-Analysis has been enhanced to parse any extra columns found at the end of the input lines. Users can filter the values in those extra columns using the -column_thresh and -column_str job command options.

							      An example of the Stat-Analysis calling sequence is shown below:

							      stat_analysis -lookin diag_conv_ges_ens_mean_orank.txt \

							      -job aggregate_stat -line_type ORANK -out_line_type RHIST \

							      -by fcst_var -column_thresh N_USE eq20

							      In this example, the Stat-Analysis tool will read ORANK lines fromdiag_conv_ges_ens_mean_orank.txt, retain only those lines where the N_USE column indicates that all 20 ensemble members were used, and write ranked histogram (RHIST) output lines for each unique value of encountered in the FCST_VAR column.

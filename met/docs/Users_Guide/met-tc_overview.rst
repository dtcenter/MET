.. _met-tc_overview:

MET-TC Overview
===============

Introduction
____________

The purpose of this User's Guide is to provide basic information to the users of the Model Evaluation Tools - Tropical Cyclone (MET-TC) to enable users to apply MET-TC to their tropical cyclone datasets and evaluation studies. MET-TC is intended for use with model forecasts run through a vortex tracking software or with operational model forecasts in Automated Tropical Cyclone Forecast (ATCF) file format.

The following sections provide an overview of MET-TC and its components, as well as basic information on the software build. The required input, including file format and the MET-TC are discussed followed by a description of the TC-Dland tool, TC-Pairs, and TC-Stat tools. Each section covers the input, output and practical usage including a description of the configuration files. This is followed by a short overview of graphical utilities available within the MET-TC release.

MET-TC components
_________________

The MET tools used in the verification of Tropical Cyclones are referred to as MET-TC. These tools are shown across the bottom of the flowchart in :numref:`overview-figure`. The MET-TC tools are described in more detail in later sections.

The TC-Dland tool is used to generate a gridded file that determines the location of coastlines and islands, and is used as input to the TC-Pairs tool to determine the distance from land of a particular track point. The TC-Pairs tool matches pairs of input model data and BEST track (or any reference forecast) and calculates position errors. The TC-Stat tool uses the TC-Pairs output to perform filter and summary jobs over the matched pair dataset. The TC-Gen tool performs a categorical analysis for tropical cyclone genesis forecasts. The TC-RMW tool performs a coordinate transformation of gridded model data, centered on the storm's location. The RMW-Analysis tool aggregates TC-RMW output across multiple cases.

Input data format
_________________

This section discusses the input and output file formats expected and produced by MET-TC. When discussing the input data, it is expected that users have run model output through vortex tracking software in order to obtain position and intensity information in Automated Tropical Cyclone Forecasting System (ATCF) file format. Best track and aids files in Automated Tropical Cyclone Forecasting System (ATCF) format (hereafter referred to as ATCF format) are necessary for model data input into the TC-Pairs tool. The ATCF format was first developed at the Naval Oceanographic and Atmospheric Research Laboratory (NRL), and is currently used for the National Hurricane Center (NHC) operations. ATCF format must be adhered to in order for the MET-TC tools to properly parse the input data.

The ATCF file format includes a section with common fields:

BASIN, CY, YYYYMMDDHH, TECHNUM/MIN, TECH, TAU, LatN/S, LonE/W, VMAX, MSLP, TY, RAD, WINDCODE, RAD1, RAD2, RAD3, RAD4, POUTER, ROUTER, RMW, GUSTS, EYE, SUBREGION, MAXSEAS, INITIALS, DIR, SPEED, STORMNAME, DEPTH, SEAS, SEASCODE, SEAS1, SEAS2, SEAS3, SEAS4

**BASIN**: basin

**CY**: annual cyclone number: 1 - 99

**YYYYMMDDHH**: Warning Date-Time-Group.

**TECHNUM/MIN**: objective technique sorting number, minutes for best track: 00 - 99

**TECH**: acronym for each objective technique or CARQ or WRNG, BEST for best track

**TAU**: forecast period: -24 through 240 hours, 0 for best-track

**LatN/S**: Latitude for the date time group (DTG)

**LonE/W**: Longitude for the DTG

**VMAX**: Maximum sustained wind speed in knots

**MSLP**: Minimum sea level pressure, 850 - 1050 mb.

**TY**: Highest level of tropical cyclone development

**RAD**: Wind intensity for the radii defined in this record: 34, 50 or 64 kt.

**WINDCODE**: Radius code

**RAD1**: If full circle, radius of specified wind intensity, or radius of first quadrant wind intensity

**RAD2**: If full circle this field not used, or radius of 2nd quadrant wind intensity

**RAD3**: If full circle this field not used, or radius of 3rd quadrant wind intensity

**RAD4**: If full circle this field not used, or radius of 4th quadrant wind intensity

**POUTER**: pressure in millibars of the last closed isobar

**ROUTER**: radius of the last closed isobar

**RMW**: radius of max winds

**GUSTS**: gusts

**EYE**: eye diameter

**SUBREGION**: subregion

**MAXSEAS**: max seas

**INITIALS**: Forecaster's initials

**DIR**: storm direction

**SPEED**: storm speed

**STORMNAME**: literal storm name, number, NONAME or INVEST, or TCcyx

**DEPTH**: system depth

**SEAS**: Wave height for radii defined in SEAS1 - SEAS4

**SEASCODE** - Radius code

**SEAS1**: first quadrant seas radius as defined by SEASCODE

**SEAS2**: second quadrant seas radius as defined by SEASCODE

**SEAS3**: third quadrant seas radius as defined by SEASCODE

**SEAS4**: fourth quadrant seas radius as defined by SEASCODE

**Of the above common fields in the ATCF file format, MET-TC requires the input file to have the first 8 comma-separated columns present.** Although all 8 columns must exist, valid data in each field is not required. In order to ensure proper matching, unique data in the BASIN, CY, YYYYMMDDHH, and TAU fields should be present.

The TC-Pairs tool expects two input data sources in order to generate matched pairs and subsequent error statistics. The expected input for MET-TC is an ATCF format file from model output, or the operational aids files with the operational model output for the 'adeck' and the NHC best track analysis (BEST) for the 'bdeck'. The BEST is a subjectively smoothed representation of the storm's location and intensity over its lifetime. The track and intensity values are based on a retrospective assessment of all available observations of the storm.

The BEST is in ATCF file format and contains all the above listed common fields. Given the reference dataset is expected in ATCF file format, any second ATCF format file from model output or operational model output from the NHC aids files can be supplied as well. The expected use of the TC-Pairs tool is to generate matched pairs between model output and the BEST. Note that some of the columns in the TC-Pairs output are populated based on the BEST information (e.g. storm category), therefore use of a different baseline may reduce the available filtering options.

All operational model aids and the BEST can be obtained from the `NHC ftp server. <ftp://ftp.nhc.noaa.gov/atcf/archive/>`_

`Click here for detailed information on the ATCF format description and specifications. <http://www.nrlmry.navy.mil/atcf_web/docs/database/new/abdeck.txt>`_

If a user has gridded model output, the model data must be run through a vortex tracking algorithm in order to obtain the ATCF-formatted input that MET-TC requires. Many vortex tracking algorithms have been developed in order to obtain basic position, maximum wind, and minimum sea level pressure information from model forecasts. One vortex tracking algorithm that is supported and freely available is the `GFDL vortex tracker package. <https://dtcenter.org/community-code/gfdl-vortex-tracker>`_

Output data format
__________________

The MET package produces output in four basic file formats: STAT files, ASCII files, NetCDF files, and Postscript plots. The MET-TC tool produces output in TCSTAT, which stands for Tropical Cyclone - STAT. This output format consists of tabular ASCII data that can be easily read by many analysis tools and software packages, making the output from MET-TC very versatile. Like STAT, TCSTAT is a specialized ASCII format containing one record on each line. Currently, the only line type available in MET-TC is TCMPR (Tropical Cyclone Matched Pairs). As more line types are included in future releases, all line types will be included in a single TCSTAT file. MET-TC also outputs a NetCDF format file in the TC-Dland tool, as input to the TC-Pairs tool.

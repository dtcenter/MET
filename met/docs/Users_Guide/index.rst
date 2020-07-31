============
User's Guide
============

**Foreword: A note to MET users**

This User's guide is provided as an aid to users of the Model Evaluation Tools (MET).  MET is a set of verification tools developed by the Developmental Testbed Center (DTC) for use by the numerical weather prediction community to help them assess and evaluate the performance of numerical weather predictions. It is also the core component of the unified METplus verification framework.  METplus details can be found at: `http://dtcenter.org/community-code/metplus <http://dtcenter.org/community-code/metplus>`_.

It is important to note here that MET is an evolving software package.  Previous releases of MET have occurred each year since 2008. This documentation describes the 9.1 release in July 2020.  Intermediate releases may include bug fixes.  MET is also able to accept new modules contributed by the community. If you have code you would like to contribute, we will gladly consider your contribution.  Please send an email to:  `met_help@ucar.edu <mailto:>`__.  We will then determine the maturity of new verification method and coordinate the inclusion of the new module in a future version.

This User's Guide was prepared by the developers of the MET, including Tressa Fowler, John Halley Gotway, Randy Bullock, Kathryn Newman, Julie Prestopnik, Lisa Goodrich, Tara Jensen, Barbara Brown, Howard Soh, Tatiana Burek, Minna Win-Gildenmeister, George McCabe, David Fillmore, Paul Prestopnik, Eric Gilleland, Nancy Rehak, Paul Oldenburg, Anne Holmes, Lacey Holland, David Ahijevych and Bonny Strong.

**New for METv9.1**

The release notes for METv9.1 can be found on the `Release Notes page of the MET wepage <https://dtcenter.org/community-code/model-evaluation-tools-met/met-version-9-1#notes>`_.

**Model Evaluation Tools (MET)  TERMS OF USE - IMPORTANT!**

Copyright 2020, UCAR/NCAR, NOAA, and CSU/CIRA
Licensed under the Apache License, Version 2.0 (the "License");
You may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
ANY KIND, either express or implied.  See the License for the specific language
governing permissions and limitations under the License.

**Citations**

The citation for this User's Guide should be:
Brown, B., Bullock, R., Fowler, T., Halley Gotway, J., Newman, K., Jensen, T., 2020: The MET Version 9.1 User's Guide.
Developmental Testbed Center.  Available at: https://github.com/NCAR/MET/releases.

**Acknowledgments**

We thank the the National Science Foundation (NSF) along with three organizations within the National Oceanic and Atmospheric Administration (NOAA): 1) Office of Atmospheric Research (OAR); 2) Next Generation Global Prediction System project (NGGPS); and 3) United State Weather Research Program (USWRP), the United States Air Force (USAF), and the United States Department of Energy (DOE) for their support of this work.  Funding for the development of MET-TC is from the NOAA's Hurricane Forecast Improvement Project (HFIP) through the Developmental Testbed Center (DTC).  Funding for the expansion of capability to address many methods pertinent to global and climate simulations was provided by NOAA's Next Generation Global Prediction System (NGGPS) and NSF Earth System Model 2 (EaSM2) projects.   We would like to thank James Franklin at the National Hurricane Center (NHC) for his insight into the original development of the existing NHC verification software.  Thanks also go to the staff at the Developmental Testbed Center for their help, advice, and many types of support.  We released METv1.0 in January 2008 and would not have made a decade of cutting-edge verification support without those who participated in the original MET planning workshops and the now dis-banded verification advisory group (Mike Baldwin, Matthew Sittel, Elizabeth Ebert, Geoff DiMego, Chris Davis, and Jason Knievel).

The National Center for Atmospheric Research (NCAR) is sponsored by NSF.  The DTC is sponsored by the National Oceanic and Atmospheric Administration (NOAA), the United States Air Force, and the National Science Foundation (NSF).  NCAR is sponsored by the National Science Foundation (NSF).
		  
.. toctree::
   :titlesonly:
   :numbered: 4

   overview
   installation
   data_io
   reformat_point
   reformat_grid
   masking
   point-stat
   grid-stat
   ensemble-stat
   wavelet-stat
   gsi-tools
   stat-analysis
   series-analysis
   grid-diag
   mode
   mode-analysis
   mode-td
   met-tc_overview
   tc-dland
   tc-pairs
   tc-stat
   tc-gen
   tc-rmw
   rmw-analysis
   plotting
   refs
   appendixA
   appendixB
   appendixC
   appendixD
   appendixE
   appendixF
   appendixG
   

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

      

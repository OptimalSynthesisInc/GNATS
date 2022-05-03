/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
*/

/**
 * Global Constants
 */

#ifndef OSI_GLOBAL_CONSTANTS_H
#define OSI_GLOBAL_CONSTANTS_H

#include <string>
#include <cmath>
#include <vector>
#include <limits>

using std::numeric_limits;
using std::vector;
using std::string;

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279
#endif /* M_PI */

/*
 * Default unset values
 */
static const double         UNSET_DOUBLE = -999999.9999;
static const float          UNSET_FLOAT  = -999999.9;
static const int            UNSET_INT    = -999999;
static const unsigned int   UNSET_UINT   = 999999;
static const long           UNSET_LONG   = -999999L;
static const unsigned long  UNSET_ULONG  = 999999L;
static const char           UNSET_CHAR   = ' ';
static const unsigned       UNSET_UCHAR  = 0;
static const string         UNSET_STRING = "unset";
static const vector<string> EMPTY_STRING_VECTOR;
static const vector<double> EMPTY_DOUBLE_VECTOR;

/*
 * Max/Min values
 */
static const double         MAX_DOUBLE = numeric_limits<double>::max();
static const double         MIN_DOUBLE = numeric_limits<double>::min();

/*
 * Conversion factors
 */
static const double         DEG_TO_RAD = M_PI / 180.;
static const double         RAD_TO_DEG = 180. / M_PI;

/*
 * Constants
 */
static const double         RADIUS_EARTH_FT = 20925524.9;
static const double         RADIUS_EARTH_M = 6371008.8;
static const double 		SEMI_MAJOR_EARTH_FT = 6378137.0*3.2808399;
static const double 		ECCENTRICITY_SQ_EARTH = 0.00669437999014;

#endif  /* OSI_GLOBAL_CONSTANTS_H */

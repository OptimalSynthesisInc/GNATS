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

#include "pub_logger.h"

#include <cstdio>
#include <fstream>

#include "util_string.h"

#include <stdarg.h>

#define stringify(name) #name

ENUM_Log_Level console_log_level;

ENUM_Log_Level getLog_Level(const char* log_level_string) {
	ENUM_Log_Level retLog_level;

	if (log_level_string != NULL) {
		for (unsigned int i = 0; i < ENUM_Log_Level_Count; i++) {
			if (strcmp(ENUM_Log_Level_String[i], log_level_string) == 0) {
				retLog_level = ENUM_Log_Level(i);

				break;
			}
		}
	}

	return retLog_level;
}

void logger_set_console_log_level(const char* inputStr) {
	string tmp_log_level = string("LOG_LEVEL_");
	tmp_log_level.append(inputStr);

	console_log_level = getLog_Level(tmp_log_level.c_str());
}

/**
 * Print stdout log with log level
 *
 * This function is evolved from C printf() function.
 * The [fmt] and [...] arguments are original arguments used in printf() function.
 *
 * Parameter
 * logLevel: ENUM_Log_Level type.
 * arg_count: Number of parameters in the fmt string
 * fmt: String format which is used in printf() function
 * ...: Parameters used in fmt string content.
 */
void logger_printLog(ENUM_Log_Level logLevel, int arg_count, const char* fmt, ...) {
	if (console_log_level == logLevel) {
		va_list ap;

		va_start(ap, arg_count);

	    vprintf(fmt, ap);

	    va_end(ap);
	}
}

void write_file(const string pathfilename,
		const stringstream& oss_doc) {
    ofstream out;
    out.open(pathfilename.c_str());

    out << oss_doc.str();
    out.close();
}

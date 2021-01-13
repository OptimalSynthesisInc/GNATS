/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
*/

#include "TrxInputStream_GroundVehicle.h"

#include "TrxHandler_GroundVehicle.h"
#include "TrxRecord_GroundVehicle.h"
#include "TrxInputStream.h"

#include "util_string.h"
#include "geometry_utils.h"

#include <fstream>
#include <deque>

using std::deque;
using std::ifstream;

namespace osi {

static inline double convertLatLonStringToDeg(const char* entry) {
    int i, entryLen;
    double deg, result;
    char degStr[5], minStr[3], secStr[3];

    entryLen = strlen(entry);

    for (i = 0; i < 2; i++) {
        minStr[i] = entry[entryLen - 4 + i];
        secStr[i] = entry[entryLen - 2 + i];
    }
    minStr[2] = '\0';
    secStr[2] = '\0';

    for (i = 0; i < entryLen - 4; i++) {
        degStr[i] = entry[i];
    }
    degStr[entryLen - 4] = '\0';

    deg = atof(degStr);

    if (deg >= 0) {
        result = deg + atof(minStr)/60 + atof(secStr)/3600;
    }else{
        result = deg - atof(minStr)/60 - atof(secStr)/3600;
    }

    return result;
}

TrxInputStream_GroundVehicle::TrxInputStream_GroundVehicle() :
	filename(""),
	listeners(set<TrxHandler_GroundVehicle*>()) {
}

TrxInputStream_GroundVehicle::TrxInputStream_GroundVehicle(const string& filename) :
  filename(filename) {
}

TrxInputStream_GroundVehicle::~TrxInputStream_GroundVehicle() {

}

/**
 * Generate TrxRecord data
 */
TrxRecord_GroundVehicle TrxInputStream_GroundVehicle::generateTrxRecord(const long timestamp,
		const deque<string>& trackTokens,
		const string& trx_line) {
	string vehicle_id = trackTokens.at(0);
	string aircraft_id = trackTokens.at(1);
	double latitude = convertLatLonString_to_deg(trackTokens.at(2).c_str());
	double longitude = -convertLatLonString_to_deg(trackTokens.at(3).c_str());
	double airspeed = atof(trackTokens.at(4).c_str());
	double altitude = atof(trackTokens.at(5).c_str());
	double heading = atof(trackTokens.at(6).c_str());
	string route_str = trackTokens.at(7);

	if (longitude < -180.) longitude += 360.;
	if (longitude > 180.) longitude -= 360.;

	TrxRecord_GroundVehicle record(timestamp,
			 vehicle_id,
			 aircraft_id,
			 route_str.substr(0, route_str.find(".")),
			 latitude,
			 longitude,
			 airspeed,
			 altitude,
			 heading,
			 route_str,
			 trx_line);

	return record;
}

void TrxInputStream_GroundVehicle::addTrxHandler(TrxHandler_GroundVehicle* const handler) {
	listeners.insert(handler);
}

void TrxInputStream_GroundVehicle::removeTrxHandler(TrxHandler_GroundVehicle* const handler) {
	listeners.erase(handler);
}

/**
 * Read ground vehicle TRX file.  Parse the file and generate TRX records variables.
 */
int TrxInputStream_GroundVehicle::parse(vector<TrxRecord_GroundVehicle>& groundVehicleRecords) {
	if (!endsWith(filename.c_str(), ".trx")) {
		fprintf(stderr, "ERROR: %s is not a valid TRX file\n",
		                filename.c_str());
		exit(-1);
	}

    // Open the file
    ifstream in;
    in.open(filename.c_str());
    if (!in.is_open()) {
        fprintf(stderr, "ERROR: could not open TRX file %s\n",
                filename.c_str());
        exit(-1);
    }

    long timestamp = -1;

    while (in.good()) {
        string line;
        getline(in, line);

        // Remove leading or trailing whitespace
        line = trim(line);

        // Skip blank lines
        if (line.length() < 1) continue;

        // Skip "#" comment line
        if (line.find("#") == 0)
        	continue;

        // Tokenize the line on space
        deque<string> tokens = tokenize(line, " ");

        // Determine if the record is a TRACK_TIME or TRACK record.
        if (tokens.at(0) == "TRACK_TIME") {
            tokens.pop_front();
            timestamp = atol(tokens.front().c_str());
        } else if (tokens.at(0) == "TRACK") {
            // Remove the "TRACK" token from the tokens deque
            tokens.pop_front();

            if (tokens.size() > 9) {
                tokens.pop_back();
            }

            // Try to parse the FP_ROUTE line
            string nextLine("#"); // Reset
            while (nextLine.find("#") == 0) {
            	getline(in, nextLine);
            	nextLine = trim(nextLine);
            }

			if (nextLine.length() > 0) {
				// String starts with "FP_ROUTE"
				if (nextLine.find("FP_ROUTE") == 0) {
					tokens.insert(tokens.end(), nextLine.substr(strlen("FP_ROUTE ")));

					TrxRecord_GroundVehicle record;
					record = generateTrxRecord(timestamp,
							tokens,
							line);

					groundVehicleRecords.push_back(record);
				}
			}
        }
    } // end - while loop

    in.close();

    return 0;
}

}

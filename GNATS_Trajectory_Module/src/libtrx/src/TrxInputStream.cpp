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

/*
 * TrxInputStream.cpp
 *
 *  Created on: May 4, 2012
 *      Author: jason
 */

#include "TrxInputStream.h"
#include "TrxRecord.h"

#include "geometry_utils.h"

#include "util_string.h"

#include <fstream>
#include <deque>

using std::deque;
using std::ifstream;

namespace osi {


static inline double convertLatLonStringToDeg(const char* entry) {
    int i, entryLen;
    double deg, result;
    int pos_decimal_dot = -1;
    int num_decimal_digit = 0;
    char degStr[5], minStr[3];
    char* secStr;

    string str_entry(entry);
    string str_entry_without_decimal_point;
    string str_sec;

    if (str_entry.find(".") != string::npos) {
    	pos_decimal_dot = str_entry.find(".");

    	str_entry_without_decimal_point = str_entry.substr(0, pos_decimal_dot);

    	num_decimal_digit = str_entry.length() - str_entry.find(".") - 1;
    } else {
    	str_entry_without_decimal_point = str_entry;
    }

    entryLen = str_entry_without_decimal_point.length();

    for (i = 0; i < 2; i++) {
        minStr[i] = entry[entryLen - 4 + i];
    }
    minStr[2] = '\0';

	if ((pos_decimal_dot != -1) && (0 < num_decimal_digit)) {
		str_sec = str_entry.substr(pos_decimal_dot-2, (str_entry.length()-pos_decimal_dot+2));
	} else {
		str_sec = str_entry_without_decimal_point.substr(str_entry_without_decimal_point.length()-2, 2);
	}

	secStr = (char*)calloc(str_sec.length()+1, sizeof(char));
	strcpy(secStr, str_sec.c_str());
	secStr[str_sec.length()] = '\0';

    for (i = 0; i < entryLen - 4; i++) {
        degStr[i] = entry[i];
    }
    degStr[entryLen - 4] = '\0';

    deg = atof(degStr);

    if (deg >= 0) {
        result = deg + atof(minStr)/60 + atof(secStr)/3600;
    } else {
        result = deg - atof(minStr)/60 - atof(secStr)/3600;
    }

    if (secStr != NULL)
    	free(secStr);

    return result;
}

TrxInputStream::TrxInputStream() :
	filename(""),
	mflname(""),
	listeners(set<TrxInputStreamListener*>()) {
}

TrxInputStream::TrxInputStream(const string& filename, const string& mflname) :
  filename(filename),
  mflname(mflname) {
}

TrxInputStream::~TrxInputStream() {

}

/**
 * Generate TrxRecord data
 */
TrxRecord TrxInputStream::generateTrxRecord(const long timestamp,
		const deque<string>& trackTokens,
		const string& trx_line,
		const double mfl,
		const bool flag_geoStyle) {
	string acid = trackTokens.at(0);
	string actype = trackTokens.at(1);
	double latitude = convertLatLonString_to_deg(trackTokens.at(2).c_str());
	double longitude = convertLatLonString_to_deg(trackTokens.at(3).c_str());

	// If this is not Geo-style TRX data
	if (!flag_geoStyle) {
		longitude = -1 * longitude;

		if (longitude > 0)
				longitude = (-1) * longitude;
	}

	double airspeed = atof(trackTokens.at(4).c_str());
	double altitude = atof(trackTokens.at(5).c_str())*100.0;
	double heading = atof(trackTokens.at(6).c_str());

	string center("");
	string sector("");
	string route("");

	if (!flag_geoStyle) {
		center = trackTokens.at(7);
		sector = trackTokens.at(8);
		route = trackTokens.at(9);
	} else {
		route = trackTokens.at(7);
	}

	if (longitude < -180.) longitude += 360.;
	if (longitude > 180.) longitude -= 360.;

	TrxRecord record(timestamp,
			 acid,
			 actype,
			 latitude,
			 longitude,
			 airspeed,
			 altitude,
			 heading,
			 center,
			 sector,
			 route,
			 trx_line,
			 mfl);

	record.flag_geoStyle = flag_geoStyle;

	return record;
}

void TrxInputStream::addTrxInputStreamListener(TrxInputStreamListener* const listener) {
  listeners.insert(listener);
}

void TrxInputStream::removeTrxInputStreamListener(TrxInputStreamListener* const listener) {
  listeners.erase(listener);
}

/**
 * Read TRX file and max-flight-level TRX file.  Parse the file and generate TRX records variables.
 */
int TrxInputStream::parse() {
	if (!endsWith(filename.c_str(), ".trx")) {
		fprintf(stderr, "ERROR: %s is not a valid TRX file\n",
		                filename.c_str());
		exit(-1);
	} else if (!endsWith(mflname.c_str(), ".trx")) {
		fprintf(stderr, "ERROR: %s is not a valid TRX file\n",
				mflname.c_str());
		exit(-1);
	}

    // open the file
    ifstream in;
    in.open(filename.c_str());
    if (!in.is_open()) {
        fprintf(stderr, "ERROR: could not open TRX file %s\n",
                filename.c_str());
        exit(-1);
    }

    long timestamp = -1;

    int mflLineNo = 0;
    ifstream mflin;
    if (mflname != "") {
        mflin.open(mflname.c_str());
    }

    while (in.good()) {
        string line;
        getline(in, line);

        // remove leading or trailing whitespace
        line = trim(line);

        if (line.find("#") == 0)
        	continue;

        // skip blank lines
        if (line.length() < 1) continue;

        // tokenize the line on space
        deque<string> tokens = tokenize(line, " ");

        // determine if the record is a TRACK_TIME or TRACK record.
        // if its a TRACK_TIME record, remove the TRACK_TIME token from
        //   the vector and then signal the listeners.
        // if its a TRACK record, read and tokenize the following
        //   FP_ROUTE line.  remove the TRACK token from the first vector.
        //   remove the FP_ROUTE from the second vector.  append the second
        //   vector to the first vector.  then signal the listeners.
        if (tokens.at(0) == "TRACK_TIME") {
            tokens.pop_front();
            timestamp = atol(tokens.front().c_str());
            notifyListenersTrackTime(tokens);
        } else if (tokens.at(0) == "TRACK") {
            double mfl = 0;

            // remove the "TRACK" token from the tokens deque
            tokens.pop_front();

            // osi - jason: update 7 nov 2017
            // i'm so sick of the stupid separate mfl file.
            // i believe newer versions of Nats did away with it long ago
            // and combined the cruise altitude into the main trx file.
            // i'm going to do the same. not sure what the new Nats trx file
            // looks like since i haven't seen one since about 2006 when
            // they still used the mfl file.
            // i'm just going to append the mfl to the track line.
            // still want to be able to handle old mfl files though.
            // so if the track line has 9 tokens (not counting 'TRACK')
            // then we use the old separate mfl file. else if the track
            // line has 10 tokens (not counting 'TRACK') then assume that
            // the last token is the mfl.
            if (tokens.size() > 9) {

                // use mfl from last token
                // we'll parse the mfl and then remove it from the tokens
                // vector so that we have the same set of tokens
                // as the old case with the mfl file. this way we can
                // use the same tokens parser and listener notifier.
                mfl = 100. * atof(tokens.at(9).c_str());
                tokens.pop_back();
            } else {

                // if an mfl file was supplied then read the MFL line
                // to get the cruise altitude.
                while (mflin.is_open() && mflin.good()) {
                    mflLineNo++;

                    // read the first token, callsign
                    string mflAcid;
                    getline(mflin, mflAcid, ' ');

                    // read the second token, max flight level
                    string mflStr;
                    getline(mflin, mflStr);

					// Skip the line which starts with '#' sign
                    if (mflAcid.find("#") == 0) {
                    	continue;
                    }

                    // make sure th mfl acid matches the track acid
                    string trackAcid = tokens.at(0);
                    if (trackAcid != mflAcid) {
                        cerr << "ERROR: TRX callsign (" << trackAcid << ") does not match MFL callsign (" << mflAcid << ") at MFL line " << mflLineNo << endl;
                        exit(-1);
                    }

                    mfl = 100. * atof(mflStr.c_str());

                    break;
                }
            }

            // try to parse the FP_ROUTE line
            string nextLine;
            while (in.good()) {
				getline(in, nextLine);
				nextLine = trim(nextLine);

				if (nextLine.length() == 0) {
					break;
				} else {
					if (nextLine.find("#") == 0) {
						continue;
					}

					// String starts with "FP_ROUTE"
					if (nextLine.find("FP_ROUTE") == 0) {
						string str_fp_route(nextLine.substr(strlen("FP_ROUTE ")));

						bool tmpFlag_geoStyle = false;
						if (str_fp_route.find("ap_code") != string::npos) {
							tmpFlag_geoStyle = true;
						}

						tokens.insert(tokens.end(), str_fp_route);

						notifyListenersTrack(timestamp, tokens, line, mfl, tmpFlag_geoStyle);
					}
				}

            	break;
            }
        }
    }

    in.close();
    if (mflin.is_open()) mflin.close();

    return 0;
}

void TrxInputStream::notifyListenersTrackTime(deque<string>& trackTimeTokens) {

	long trackTime = atol(trackTimeTokens.at(0).c_str());

	set<TrxInputStreamListener*>::iterator listenerIter;
	for(listenerIter=listeners.begin(); listenerIter!=listeners.end(); ++listenerIter) {
		TrxInputStreamListener* listener = (*listenerIter);
		listener->onTrackTime(trackTime);
	}
}

void TrxInputStream::notifyListenersTrack(long timestamp,
		deque<string>& trackTokens,
		string& trx_line,
		double mfl,
		const bool flag_geoStyle) {
	TrxRecord record;
	record = generateTrxRecord(timestamp,
			trackTokens,
			trx_line,
			mfl,
			flag_geoStyle);

	set<TrxInputStreamListener*>::iterator listenerIter;
	for (listenerIter=listeners.begin(); listenerIter!=listeners.end(); ++listenerIter) {
		TrxInputStreamListener* listener = (*listenerIter);
		listener->onTrack(record);
	}
}

}

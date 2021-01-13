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

/*
 * AdbOPFParser.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbOPFParser.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <deque>
#include <map>
#include <algorithm>
#include "AdbFileParser.h"
#include "AdbOPFModel.h"
#include "AdbParserListener.h"

using std::string;
using std::deque;
using std::replace;
using std::remove;

#ifndef NDEBUG
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace osi {


static inline deque<string> tokenize(const string& str,
		                             const string& delimiter) {
	deque<string> tokens;
	char* saveptr = NULL;
	char* cstr = (char*)calloc(str.length()+1, sizeof(char));
	strcpy(cstr, str.c_str());
	char* token = strtok_r(cstr, delimiter.c_str(), &saveptr);
	if(token) {
		string s(token);
		tokens.push_back(s);
	}
	while(token) {
		token = strtok_r(NULL, delimiter.c_str(), &saveptr);
		if(token) {
			string s(token);
			tokens.push_back(s);
		}
	}
	free(cstr);

	return tokens;
}

static inline string& replaceChar(string& s, const string& oldCharStr, const string& newCharStr) {
	if(newCharStr.length() > 0) {
		// replace char
		char oldChar = oldCharStr.at(0);
		char newChar = newCharStr.at(0);
		replace(s.begin(), s.end(), oldChar, newChar);
	} else {
		// remove char
		char* buffer = (char*)calloc(s.length()+1, sizeof(char));
		memset(buffer, 0, (s.length()+1)*sizeof(char));
		int index = 0;
		char oldChar = oldCharStr.at(0);
		for(unsigned int i=0; i<s.length(); ++i) {
			char c = s.at(i);
			if(oldChar != c) {
				buffer[index++] = s.at(i);
			}
		}
		s = string(buffer);
		free(buffer);
	}
	return s;
}


AdbOPFParser::AdbOPFParser(AdbOPFModel& ac, const string& fname):
	AdbFileParser(fname),
	AdbParserListener(),
	ac(&ac) {

	addAdbParserListener(this);
}

AdbOPFParser::~AdbOPFParser() {
	removeAdbParserListener(this);
}

void AdbOPFParser::parsedCommentLine(const string& line, int commentNum, int lineNum) {
	(void)line;
	(void)commentNum;
	(void)lineNum;
}

void AdbOPFParser::parsedDataLine(const string& line, int dataNum, int lineNum) {
	(void)lineNum;

    if(dataNum == 1) {
        parseAcTypeLine(line);
    } else if(dataNum == 2) {
        parseMassLine(line);
    } else if(dataNum == 3) {
        parseFlightEnvelopeLine(line);
    } else if(dataNum == 4) {
        parseWingAreaLine(line);
    } else if(dataNum == 5) {
        parseCruiseConfigLine(line);
    } else if(dataNum == 6) {
        parseClimbConfigLine(line);
    } else if(dataNum == 7) {
        parseTakeoffConfigLine(line);
    } else if(dataNum == 8) {
        parseApproachConfigLine(line);
    } else if(dataNum == 9) {
        parseLandingConfigLine(line);
    } else if(dataNum == 10) {
        parseSpoilerRetLine(line);
    } else if(dataNum == 11) {
        parseSpoilerExtLine(line);
    } else if(dataNum == 12) {
        parseGearUpLine(line);
    } else if(dataNum == 13) {
        parseGearDownLine(line);
    } else if(dataNum == 14) {
        parseBrakesOffLine(line);
    } else if(dataNum == 15) {
        parseBrakesOnLine(line);
    } else if(dataNum == 16) {
        parseClimbThrustCoefsLine(line);
    } else if(dataNum == 17) {
        parseDescentThrustCoefsLine(line);
    } else if(dataNum == 18) {
        parseDescentSpeedsLine(line);
    } else if(dataNum == 19) {
        parseFuelConsumptionCoefsLine(line);
    } else if(dataNum == 20) {
        parseDescentFuelFlowLine(line);
    } else if(dataNum == 21) {
        parseCruiseFuelFlowLine(line);
    } else if(dataNum == 22) {
        parseGroundMovementsLine(line);
    }
}

void AdbOPFParser::parsedEofLine(const string& line, int eofNum, int lineNum) {
	(void)line;
	(void)eofNum;
	(void)lineNum;
}


adb_engine_type_e AdbOPFParser::getAdbEngineType(const string& s) {
    if(s == "Jet") {
        return JET;
    } else if(s == "Turboprop") {
        return TURBOPROP;
    } else {
        return PISTON;
    }
}

adb_wake_category_e AdbOPFParser::getAdbWakeCategory(const string& s) {
    if(s == "J") {
        return JUMBO;
    } else if(s == "H") {
        return HEAVY;
    } else if(s == "M") {
        return MEDIUM;
    } else {
        return LIGHT;
    }
}

void AdbOPFParser::parseAcTypeLine(const string& line) {
    deque<string> tokens = tokenize(line, " ");
    ac->type = replaceChar(tokens[0], "_", "");
    ac->numEngines = atoi(tokens[1].c_str());
    ac->engineType = getAdbEngineType(tokens[3]);
    ac->wakeCategory = getAdbWakeCategory(tokens[4]);
}

void AdbOPFParser::parseMassLine(const string& line) {
    double tonsToPounds = 2000;

    deque<string> tokens = tokenize(line, " ");
    ac->mref = atof(tokens[0].c_str()) * tonsToPounds;
    ac->mmin = atof(tokens[1].c_str()) * tonsToPounds;
    ac->mmax = atof(tokens[2].c_str()) * tonsToPounds;
    ac->mpyld = atof(tokens[3].c_str()) * tonsToPounds;
    ac->gw = atof(tokens[4].c_str());
}

void AdbOPFParser::parseFlightEnvelopeLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->vmo = atof(tokens[0].c_str());
    ac->mmo = atof(tokens[1].c_str());
    ac->hmo = atof(tokens[2].c_str());
    ac->hmax = atof(tokens[3].c_str());
    ac->gt = atof(tokens[4].c_str());
}

void AdbOPFParser::parseWingAreaLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->ndrst = atoi(tokens[0].c_str());
    ac->s = atof(tokens[1].c_str());
    ac->clbo = atof(tokens[2].c_str());
    ac->k = atof(tokens[3].c_str());
    ac->cm16 = atof(tokens[4].c_str());
}

void AdbOPFParser::parseConfigLine(const string& line, adb_flight_phase_e phase) {
    int widths[6] = {1,3,13,13,13,13};
    string* tokens = new string[6];
    splitFixedWidth(line, 6, widths, &tokens);
    string name = tokens[2];
    double vstall = atof(tokens[3].c_str());
    double cd0 = atof(tokens[4].c_str());
    double cd2 = atof(tokens[5].c_str());

    ac->configName.insert(std::pair<adb_flight_phase_e, string>(phase, name));
    ac->vstall.insert(std::pair<adb_flight_phase_e, double>(phase, vstall));
    ac->cd0.insert(std::pair<adb_flight_phase_e, double>(phase, cd0));
    ac->cd2.insert(std::pair<adb_flight_phase_e, double>(phase, cd2));
    delete [] tokens;
}

void AdbOPFParser::parseCruiseConfigLine(const string& line) {
    parseConfigLine(line, CRUISE);
}

void AdbOPFParser::parseClimbConfigLine(const string& line) {
    parseConfigLine(line, CLIMB);
}

void AdbOPFParser::parseTakeoffConfigLine(const string& line) {
    parseConfigLine(line, TAKEOFF);
}

void AdbOPFParser::parseApproachConfigLine(const string& line) {
    parseConfigLine(line, APPROACH);
}

void AdbOPFParser::parseLandingConfigLine(const string& line) {
    parseConfigLine(line, LANDING);
}

void AdbOPFParser::parseSpoilerRetLine(const string& line) {
	(void)line;
}

void AdbOPFParser::parseSpoilerExtLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
                           // cd0
    ac->cdSpoilerExt[1] = atof(tokens[2].c_str());  // cd2

}

void AdbOPFParser::parseGearUpLine(const string& line) {
	(void)line;
}

void AdbOPFParser::parseGearDownLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->cdGearDown[0] = atof(tokens[2].c_str());  // cd0
    ac->cdGearDown[1] = atof(tokens[3].c_str());  // cd2
}

void AdbOPFParser::parseBrakesOffLine(const string& line) {
	(void)line;
}

void AdbOPFParser::parseBrakesOnLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");

    ac->cdBrakesOn[1] = atof(tokens[2].c_str());  // cd2
}

void AdbOPFParser::parseClimbThrustCoefsLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->ctc1 = atof(tokens[0].c_str());
    ac->ctc2 = atof(tokens[1].c_str());
    ac->ctc3 = atof(tokens[2].c_str());
    ac->ctc4 = atof(tokens[3].c_str());
    ac->ctc5 = atof(tokens[4].c_str());
}

void AdbOPFParser::parseDescentThrustCoefsLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->ctdesLow = atof(tokens[0].c_str());
    ac->ctdesHigh = atof(tokens[1].c_str());
    ac->hpdes = atof(tokens[2].c_str());
    ac->ctdesApp = atof(tokens[3].c_str());
    ac->ctdesLd = atof(tokens[4].c_str());
}

void AdbOPFParser::parseDescentSpeedsLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->vdesRef = atof(tokens[0].c_str()); // descent cas
    ac->mdesRef = atof(tokens[1].c_str()); // descen tmach
}

void AdbOPFParser::parseFuelConsumptionCoefsLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->cf1 = atof(tokens[0].c_str());
    ac->cf2 = atof(tokens[1].c_str());
}

void AdbOPFParser::parseDescentFuelFlowLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->cf3 = atof(tokens[0].c_str());
    ac->cf4 = atof(tokens[1].c_str());
}

void AdbOPFParser::parseCruiseFuelFlowLine(const string& line) {
	deque<string> tokens = tokenize(line, " ");
    ac->cfcr = atof(tokens[0].c_str());
}

void AdbOPFParser::parseGroundMovementsLine(const string& line) {
    double metersToFt = 3.28083989501;

	deque<string> tokens = tokenize(line, " ");
    ac->tol = atof(tokens[0].c_str()) * metersToFt;
    ac->ldl = atof(tokens[1].c_str()) * metersToFt;
    ac->span = atof(tokens[2].c_str()) * metersToFt;
    ac->length = atof(tokens[3].c_str()) * metersToFt;
}

}

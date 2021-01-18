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
 * AdbFileParser.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbFileParser.h"

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include "AdbParserListener.h"

using std::string;
using std::vector;
using std::ifstream;
using std::find;

namespace osi {

// globals declared in AdbFileParser.h
const string ADB_COMMENT_PREFIX = "CC";
const string ADB_DATA_PREFIX = "CD";
const string ADB_EOF_PREFIX = "FI";

static inline bool startsWith(const string& line, const string& prefix) {
	int prefixLength = prefix.length();
	int lineLength = line.length();

	if(lineLength < prefixLength) {
		return false;
	}

	string substr = line.substr(0, prefixLength);
	if(substr == prefix) {
		return true;
	}

	return false;
}

static inline bool isComment(const string& line) {
	return startsWith(line, ADB_COMMENT_PREFIX);
}

static inline bool isData(const string& line) {
	return startsWith(line, ADB_DATA_PREFIX);
}

static inline bool isEof(const string& line) {
	return startsWith(line, ADB_EOF_PREFIX);
}

static inline string& trim(string& str, const string& prefix="") {

	int prefixLength = prefix.length();
	if(prefixLength > 0) {
		str = str.substr(prefixLength);
	}

	string::size_type pos = str.find_last_not_of(' ');
	if(pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) str.erase(0, pos);
	} else {
		str.erase(str.begin(), str.end());
	}
	return str;
}

AdbFileParser::AdbFileParser(const string& fname) :
	fname(fname),
	lineNum(0),
	dataLineNum(0),
	commentLineNum(0),
	eofLineNum(0),
	listeners(vector<AdbParserListener*>()) {
}

AdbFileParser::~AdbFileParser() {
}

int AdbFileParser::parse() {
	ifstream in;
	in.open(fname.c_str());

	if(!in.is_open()) return -1;

	string line;
	while(in.good()) {
		getline(in, line);
		lineNum++;

		if(isComment(line)) {
			commentLineNum++;
			fireParsedCommentEvent(trim(line, ADB_COMMENT_PREFIX));
		} else if(isData(line)) {
			dataLineNum++;
			fireParsedDataEvent(trim(line, ADB_DATA_PREFIX));
		} else if(isEof(line)) {
			eofLineNum++;
			fireParsedEofEvent(trim(line, ADB_EOF_PREFIX));
		}
	}

	in.close();

	return 0;
}

void AdbFileParser::addAdbParserListener(const AdbParserListener* const listener) {
	vector<AdbParserListener*>::iterator it;
	it = find(listeners.begin(), listeners.end(), listener);
	if(it == listeners.end()) {
		listeners.push_back(const_cast<AdbParserListener*>(listener));
	}
}

void AdbFileParser::removeAdbParserListener(const AdbParserListener* const listener) {
	vector<AdbParserListener*>::iterator it;
	it = find(listeners.begin(), listeners.end(), listener);
	if(it != listeners.end()) {
		listeners.erase(it);
	}
}

void AdbFileParser::fireParsedCommentEvent(const string& line) {
	vector<AdbParserListener*>::iterator iter;
	for(iter=listeners.begin(); iter!=listeners.end(); ++iter) {
		AdbParserListener* listener = (*iter);
		listener->parsedCommentLine(line, commentLineNum, lineNum);
	}
}

void AdbFileParser::fireParsedDataEvent(const string& line) {
	vector<AdbParserListener*>::iterator iter;
	for(iter=listeners.begin(); iter!=listeners.end(); ++iter) {
		AdbParserListener* listener = (*iter);
		listener->parsedDataLine(line, dataLineNum, lineNum);
	}
}

void AdbFileParser::fireParsedEofEvent(const string& line) {
	vector<AdbParserListener*>::iterator iter;
	for(iter=listeners.begin(); iter!=listeners.end(); ++iter) {
		AdbParserListener* listener = (*iter);
		listener->parsedEofLine(line, eofLineNum, lineNum);
	}
}

  void AdbFileParser::splitFixedWidth(const string& line, const int& numColumns, const int* const widths, string** const values) const {

	int start = 0;
	for(int i=0; i<numColumns; ++i) {
		(*values)[i] = line.substr(start, widths[i]);
		trim((*values)[i]);
		start += widths[i];
	}
}

}

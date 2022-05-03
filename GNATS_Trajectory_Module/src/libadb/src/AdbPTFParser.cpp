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
 *  AdbPTFParser.cpp
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbPTFParser.h"

#include <string>
#include <fstream>
#include <cstdio>
#include <map>
#include <algorithm>

using std::replace;
using std::string;
using std::ifstream;
using std::pair;

#include <iostream>
using std::cout;
using std::endl;

namespace osi {

static inline bool starts_with(const string& line, const string& prefix) {
  if(line.length() < prefix.length()) return false;
  return (line.substr(0, prefix.length()) == prefix);
}

static inline bool isComment(const string& line) {
  if(starts_with(line, "=")) return true;
  if(starts_with(line, "|")) return true;
  if(starts_with(line, "FL")) return true;
  if(starts_with(line, "ADB PERFORMANCE FILE")) return true;
  if(starts_with(line, "Speeds")) return true;
  if(starts_with(line, "Source")) return true;
  if(line.length() < 1) return true;
  return false;
}

static inline string& trim(string& line) {
  if(line.length() < 1) return line;
  line.erase(0, line.find_first_not_of(" "));
  unsigned int end=line.find_last_not_of(" ")+1;
  if(end < line.length()) {
    line.erase(end);
  }
  return line;
}

AdbPTFParser::AdbPTFParser(AdbPTFModel& ac, const string& fname):
  AdbFileParser(fname),
  AdbParserListener(),
  ac(&ac) {
  
  addAdbParserListener(this);
}

AdbPTFParser::~AdbPTFParser() {
  removeAdbParserListener(this);
}

int AdbPTFParser::parse() {
  ifstream in;
  in.open(fname.c_str());
  
  if(!in.is_open()) return -1;
  
  string line;
  while(in.good()) {
    getline(in, line);
    lineNum++;

    trim(line);
    
    if(isComment(line)) {
      commentLineNum++;
      fireParsedCommentEvent(trim(line));
    } else {
      dataLineNum++;
      fireParsedDataEvent(trim(line));
    }

    eofLineNum++;
    fireParsedEofEvent(trim(line));

  }
  
  in.close();
  
  return 0;
}

void AdbPTFParser::parsedCommentLine(const string& line, int commentNum, int lineNum) {
  (void)line;
  (void)commentNum;
  (void)lineNum;
}

void AdbPTFParser::parsedDataLine(const string& line, int dataNum, int lineNum) {
  (void)lineNum;

  // use sscanf to read in values
  // for the default PTF files, the cruise data starts at FL30, dataNum 6.
  int fl = 0;
  double cruiseTas = 0.0;
  double cruiseFuelFlowLo = 0.0;
  double cruiseFuelFlowNom = 0.0;
  double cruiseFuelFlowHi = 0.0;
  double climbTas = 0.0;
  double climbRocdLo = 0.0;
  double climbRocdNom = 0.0;
  double climbRocdHi = 0.0;
  double climbFuelFlowNom = 0.0;
  double descentTas = 0.0;
  double descentRocdNom = 0.0;
  double descentFuelFlowNom = 0.0;

  if (dataNum <= 5) {
    // parse the actype, climb, and cruise lines.
    if(starts_with(line, "AC/Type")) {
      ac->actype = line.substr(9);
      trim(ac->actype);
      replace(ac->actype.begin(), ac->actype.end(), '_', '\0');
    } else if(starts_with(line, "climb")) {
      sscanf(line.c_str(),
	     "%*s %*s %lf/%lf %lf %*s %*s %lf",
	     &ac->climbCasLow, &ac->climbCasHi, &ac->climbMach, &ac->massLow);
    } else if(starts_with(line, "cruise")) {
      sscanf(line.c_str(),
	     "%*s %*s %lf/%lf %lf %*s %*s %lf %*s %*s %*s %lf",
	     &ac->cruiseCasLow, &ac->cruiseCasHi, &ac->cruiseMach, &ac->massNom, &ac->maxAltitude);
    } else if(starts_with(line, "descent")) {
      sscanf(line.c_str(),
	     "%*s %*s %lf/%lf %lf %*s %*s %lf",
	     &ac->descentCasLow, &ac->descentCasHi, &ac->descentMach, &ac->massHi);
    }
  } else if (dataNum > 5 && dataNum < 11) {
    // start parsing the table, climb/descent columns only
    // "FL | | tas lo nom hi nom | tas nom nom
    sscanf(line.c_str(),
	   "%d %*s %*s %lf %lf %lf %lf %lf %*s %lf %lf %lf",
	   &fl, &climbTas, &climbRocdLo, &climbRocdNom, &climbRocdHi,
	   &climbFuelFlowNom, &descentTas, &descentRocdNom, 
	   &descentFuelFlowNom);

    double altitude = fl*100.;
    ac->altitudes.push_back(altitude);

    ac->climbTas.insert(pair<double, double>(altitude, climbTas));
    ac->climbRateLow.insert(pair<double, double>(altitude, climbRocdLo));
    ac->climbRateNom.insert(pair<double, double>(altitude, climbRocdNom));
    ac->climbRateHi.insert(pair<double, double>(altitude, climbRocdHi));
    ac->climbFuelFlowNom.insert(pair<double, double>(altitude, climbFuelFlowNom));

    ac->descentTas.insert(pair<double, double>(altitude, descentTas));
    ac->descentRateNom.insert(pair<double, double>(altitude, descentRocdNom));
    ac->descentFuelFlowNom.insert(pair<double, double>(altitude, descentFuelFlowNom));
  } else {
    // parse table, cruise, climb, and descent columns
    // "FL | tas lo nom hi | tas lo nom hi nom | tas nom nom
    sscanf(line.c_str(),
	   "%d %*s %lf %lf %lf %lf %*s %lf %lf %lf %lf %lf %*s %lf %lf %lf", 
	   &fl, &cruiseTas, &cruiseFuelFlowLo, &cruiseFuelFlowNom,
	   &cruiseFuelFlowHi, &climbTas, &climbRocdLo, &climbRocdNom, 
	   &climbRocdHi, &climbFuelFlowNom, &descentTas, &descentRocdNom,
	   &descentFuelFlowNom);

    double altitude = fl*100.;
    ac->altitudes.push_back(altitude);

    ac->climbTas.insert(pair<double, double>(altitude, climbTas));
    ac->climbRateLow.insert(pair<double, double>(altitude, climbRocdLo));
    ac->climbRateNom.insert(pair<double, double>(altitude, climbRocdNom));
    ac->climbRateHi.insert(pair<double, double>(altitude, climbRocdHi));
    ac->climbFuelFlowNom.insert(pair<double, double>(altitude, climbFuelFlowNom));
    
    ac->cruiseTas.insert(pair<double, double>(altitude, cruiseTas));
    ac->cruiseFuelFlowLow.insert(pair<double, double>(altitude, cruiseFuelFlowLo));
    ac->cruiseFuelFlowNom.insert(pair<double, double>(altitude, cruiseFuelFlowNom));
    ac->cruiseFuelFlowHi.insert(pair<double, double>(altitude, cruiseFuelFlowHi));

    ac->descentTas.insert(pair<double, double>(altitude, descentTas));
    ac->descentRateNom.insert(pair<double, double>(altitude, descentRocdNom));
    ac->descentFuelFlowNom.insert(pair<double, double>(altitude, descentFuelFlowNom));
  }
}

void AdbPTFParser::parsedEofLine(const string& line, int eofNum, int lineNum) {
  (void)line;
  (void)eofNum;
  (void)lineNum;
  // generate aux data tables
  if (ac != NULL) {
	  ac->generateAuxiliaryTables();
  }
}

} /* namespace osi */

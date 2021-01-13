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
 * AdbOPFModel.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbOPFModel.h"

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include "adb.h"

using std::string;
using std::map;
using std::ostream;
using std::stringstream;

namespace osi {

static const double ZERO_ARRAY_2[2] = {0,0};

AdbOPFModel::AdbOPFModel() :
	type(""),
	numEngines(0),
	engineType(JET),
	wakeCategory(MEDIUM),
	mref(0),
	mmin(0),
	mmax(0),
	mpyld(0),
	gw(0),
	vmo(0),
	mmo(0),
	hmo(0),
	hmax(0),
	gt(0),
	ndrst(0),
	s(0),
	clbo(0),
	k(0),
	cm16(0),
	configName(map<adb_flight_phase_e, string>()),
	vstall(map<adb_flight_phase_e, double>()),
	cd0(map<adb_flight_phase_e, double>()),
	cd2(map<adb_flight_phase_e, double>()),
	ctc1(0),
	ctc2(0),
	ctc3(0),
	ctc4(0),
	ctc5(0),
	ctdesLow(0),
	ctdesHigh(0),
	hpdes(0),
	ctdesApp(0),
	ctdesLd(0),
	vdesRef(0),
	mdesRef(0),
	cf1(0),
	cf2(0),
	cf3(0),
	cf4(0),
	cfcr(0),
	tol(0),
	ldl(0),
	span(0),
	length(0) {
}

AdbOPFModel::~AdbOPFModel() {
}

static inline string getEngineTypeStr(adb_engine_type_e type) {
    if(type == JET) {
        return "JET";
    } else if(type == PISTON) {
        return "PISTON";
    } else {
        return "TURBOPROP";
    }
}

static inline string getWakeCategoryStr(adb_wake_category_e category) {
    if(category == JUMBO) {
        return "JUMBO";
    } else if(category == HEAVY) {
        return "HEAVY";
    } else if(category == MEDIUM) {
        return "MEDIUM";
    } else {
        return "LIGHT";
    }
}

static inline string getMapStrS(const map<adb_flight_phase_e, string>& m) {
	(void)m;
	return "";
}

static inline string getMapStrD(const map<adb_flight_phase_e, double>& m) {
	stringstream ss;
	map<adb_flight_phase_e, double>::const_iterator iter;
	ss << "{";
	for(iter=m.begin(); iter!=m.end(); ++iter) {
		ss << iter->first << "=" << iter->second << ",";
	}
	ss << "}";
	return ss.str();
}

ostream& operator<<(ostream& out, const AdbOPFModel& o) {
	out << "{"
	    << "type=" << o.type << ","
	    << "numEngines=" << o.numEngines << ","
	    << "engineType=" << getEngineTypeStr(o.engineType) << ","
	    << "wakeCategory=" << getWakeCategoryStr(o.wakeCategory) << ","
	    << "mref=" << o.mref << ","
	    << "mmin=" << o.mmin << ","
	    << "mmax=" << o.mmax << ","
	    << "mpyld=" << o.mpyld << ","
	    << "gw=" << o.gw << ","
	    << "configName=" << getMapStrS(o.configName) << ","
	    << "vstall=" << getMapStrD(o.vstall) << ","
	    << "cd0=" << getMapStrD(o.cd0) << ","
	    << "cd2=" << getMapStrD(o.cd2)
	    << "}";
	return out;
}

}

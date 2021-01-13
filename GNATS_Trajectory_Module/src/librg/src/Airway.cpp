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
 * Airway.cpp
 *
 *  Created on: May 10, 2012
 *      Author: jason
 */

#include "Airway.h"

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

namespace osi {

// global airways map declared extern in Airway.h
map<string, Airway*> rg_airways;

Airway::Airway() :
	name(""),
	description(""),
	altLevel(""),
	routeType(""),
	waypointNames(vector<string>()),
	minAltitude1(vector<double>()),
	minAltitude2(vector<double>()),
	maxAltitude(vector<double>()) {
}

Airway::Airway(const string& name,
		       const string& description,
		       const string& altLevel,
		       const string& routeType,
		       const vector<string>& waypointNames,
		       const vector<double>& minAltitude1,
		       const vector<double>& minAltitude2,
		       const vector<double>& maxAltitude) :
	name(name),
	description(description),
	altLevel(altLevel),
	routeType(routeType),
	waypointNames(waypointNames),
	minAltitude1(minAltitude1),
	minAltitude2(minAltitude2),
	maxAltitude(maxAltitude) {
}

Airway::Airway(const Airway& that) :
	name(that.name),
	description(that.description),
	altLevel(that.altLevel),
	routeType(that.routeType),
	waypointNames(that.waypointNames),
	minAltitude1(that.minAltitude1),
	minAltitude2(that.minAltitude2),
	maxAltitude(that.maxAltitude) {
}

Airway::~Airway() {
}

Airway& Airway::operator=(const Airway& that) {
	if(this == &that) {
		return *this;
	}
	this->name = that.name;
	this->description = that.description;
	this->altLevel = that.altLevel;
	this->routeType = that.routeType;

	this->waypointNames.clear();
	this->waypointNames.insert(this->waypointNames.begin(), that.waypointNames.begin(), that.waypointNames.end());

	this->minAltitude1.clear();
	this->minAltitude1.insert(this->minAltitude1.begin(), that.minAltitude1.begin(), that.minAltitude1.end());

	this->minAltitude2.clear();
	this->minAltitude2.insert(this->minAltitude2.begin(), that.minAltitude2.begin(), that.minAltitude2.end());

	this->maxAltitude.clear();
	this->maxAltitude.insert(this->maxAltitude.begin(), that.maxAltitude.begin(), that.maxAltitude.end());

	return *this;
}

}

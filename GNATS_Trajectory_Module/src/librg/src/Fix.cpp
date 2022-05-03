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
 * Fix.cpp
 *
 *  Created on: May 10, 2012
 *      Author: jason
 */

#include "Fix.h"

#include <map>
#include <string>

using std::map;
using std::string;

namespace osi {

// global fix map declared extern in Fix.h
map<string, Fix*> rg_fixes = map<string, Fix*>();
map<int, Fix*> rg_fixes_by_id = map<int, Fix*>();
map<string, int> rg_fix_ids = map<string, int>();
map<int, string> rg_fix_names = map<int, string>();

Fix::Fix() {
}

Fix::~Fix() {
}

Navaid::Navaid() :
	name(""),
	description(""),
	latitude(0),
	longitude(0),
	frequency(0),
	dmeLatitude(0),
	dmeLongitude(0),
	dmeElevation(0) {
}

Navaid::Navaid(const string& name,
		       const string& description,
		       const double& latitude,
		       const double& longitude,
		       const double& frequency,
		       const double& dmeLatitude,
		       const double& dmeLongitude,
		       const double& dmeElevation) :
    name(name),
    description(description),
    latitude(latitude),
    longitude(longitude),
    frequency(frequency),
    dmeLatitude(dmeLatitude),
    dmeLongitude(dmeLongitude),
    dmeElevation(dmeElevation) {
}

Navaid::Navaid(const Navaid& that) :
    name(that.name),
    description(that.description),
    latitude(that.latitude),
    longitude(that.longitude),
    frequency(that.frequency),
    dmeLatitude(that.dmeLatitude),
    dmeLongitude(that.dmeLongitude),
    dmeElevation(that.dmeElevation) {
}

Navaid::~Navaid() {

}

string& Navaid::getName() {
	return name;
}

double& Navaid::getLatitude() {
	return latitude;
}

double& Navaid::getLongitude() {
	return longitude;
}

Navaid& Navaid::operator=(const Navaid& that) {
	if(this == &that) {
		return *this;
	}
	this->name = that.name;
	this->description = that.description;
	this->latitude = that.latitude;
	this->longitude = that.longitude;
	this->frequency = that.frequency;
	this->dmeLatitude = that.dmeLatitude;
	this->dmeLongitude = that.dmeLongitude;
	this->dmeElevation = that.dmeElevation;
	return *this;
}

Waypoint::Waypoint() :
	name(""),
	description(""),
	latitude(0),
	longitude(0) {
}

Waypoint::Waypoint(const string& name,
		 const string& description,
		 const double& latitude,
		 const double& longitude) :
	name(name),
	description(description),
	latitude(latitude),
	longitude(longitude) {
}

Waypoint::Waypoint(const Waypoint& that) :
	name(that.name),
	description(that.description),
	latitude(that.latitude),
	longitude(that.longitude) {
}

Waypoint::~Waypoint() {

}

string& Waypoint::getName() {
	return name;
}

double& Waypoint::getLatitude() {
	return latitude;
}

double& Waypoint::getLongitude() {
	return longitude;
}

Waypoint& Waypoint::operator=(const Waypoint& that) {
	if(this == &that) {
		return *this;
	}
	this->name = that.name;
	this->description = that.description;
	this->latitude = that.latitude;
	this->longitude = that.longitude;
	return *this;
}

}

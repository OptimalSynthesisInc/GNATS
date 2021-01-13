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
 * RouteElement.cpp
 *
 *  Created on: Nov 5, 2013
 *      Author: jason
 */

#include "RouteElement.h"
#include <vector>
#include <string>

using namespace std;


FpRouteElement::FpRouteElement() :
	identifier(""),
	terminator(""),
	sequence(vector<string>()) {
}

FpRouteElement::FpRouteElement(const string& id, const string& terminator,
		                       const vector<string>& sequence) :
	identifier(id),
	terminator(terminator),
	sequence(sequence) {
}

FpRouteElement::FpRouteElement(const FpRouteElement& that) :
	identifier(that.identifier),
	terminator(that.terminator),
	sequence(that.sequence) {
}

FpRouteElement::~FpRouteElement() {

}

const string& FpRouteElement::get_identifier() const {
	return identifier;
}

string FpRouteElement::get_terminator() const {
	// if this is a route and next is route, return intersection point
	// if this is a route and next is waypoint, return intersection
	// if this is a waypoint and next is a route, return this point
	// if this is a waypoint and next is a waypoint, return this point
	return terminator.substr(0, terminator.find_first_of("0123456789"));
}


#if 0
FpDepartureRoute::FpDepartureRoute() :
	FpRouteElement() {
}

FpDepartureRoute::FpDepartureRoute(const FpDepartureRoute& that) :
	FpRouteElement(that.identifier, that.sequence) {
}

FpDepartureRoute::~FpDepartureRoute() {

}


FpArrivalRoute::FpArrivalRoute() :
	FpRouteElement(),
	par_flag(false) {
}

FpArrivalRoute::FpArrivalRoute(const FpArrivalRoute& that) :
	FpRouteElement(that.identifier, that.sequence),
	par_flag(false) {
}

FpArrivalRoute::~FpArrivalRoute() {

}

bool FpArrivalRoute::isPar() const {
	return par_flag;
}


FpAirway::FpAirway() :
	FpRouteElement() {
}

FpAirway::FpAirway(const FpAirway& that) :
	FpRouteElement(that.identifier, that.sequence) {
}

FpAirway::~FpAirway() {

}


FpAirport::FpAirport() :
	FpRouteElement() {
}

FpAirport::FpAirport(const FpAirport& that) :
	FpRouteElement(that.identifier, that.sequence) {
}

FpAirport::~FpAirport() {

}


FpWaypoint::FpWaypoint() :
	FpRouteElement() {
}

FpWaypoint::FpWaypoint(const FpWaypoint& that) :
	FpRouteElement(that.identifier, that.sequence) {
}

FpWaypoint::~FpWaypoint() {

}

#endif

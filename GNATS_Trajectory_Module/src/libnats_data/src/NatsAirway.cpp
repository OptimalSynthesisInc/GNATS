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
 * NatsAirway.cpp
 * 
 * Airway data loaded from NATS data files
 * C++ host-side class
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "NatsAirway.h"

#include <algorithm>
#include <iostream>
using std::cout;
using std::endl;
using std::find;

NatsAirway::NatsAirway() :
  name(""),
  route(vector<string>()) {
}

NatsAirway::NatsAirway(const NatsAirway& that) :
  name(that.name),
  route(vector<string>()) {
  route.insert(route.begin(), that.route.begin(), that.route.end());
}

NatsAirway::~NatsAirway() {
}

bool NatsAirway::operator<(const NatsAirway& that) const {
  return this->name < that.name;
}

void NatsAirway::getRouteSegment(const string& start,
				  const string& end, 
				  vector<string>* const segment) const {

	if(!segment) return;

	// the route is unsorted so we can't use binary search. need to iterate.
	if(route.size() < 1) return;

	vector<string>::const_iterator it;

	it = find(route.begin(), route.end(), start);
	int startIndex = it - route.begin();

	it = find(route.begin(), route.end(), end);
	int endIndex = it - route.begin();

	// if startIndex==route.size() then we need to verify that the last
	// element value is the same as start value or if the value wasn't found.
	// ditto for endIndex==route.size().
	// if startIndex==route.size() but endIndex not found then
	// use the whole route from startIndex to 0.
	// if startIndex==0 but endIndex not found then
	// use the whole route from startIndex to route.size()-1
	// if startIndex!=0 OR startIndex!=route.size() and endIndex not found
	// then we don't know which direction along the route to go, so ignore
	// the segment.
	if(endIndex == (int)route.size()) {
		if(startIndex==0) {
			// use whole route, starting at front
			endIndex = route.size()-1;
		} else if(startIndex == (int)route.size()-1) {
			// use whole route, starting at back
			endIndex = 0;
		} else {
			// don't know what to do, return empty segment
			return;
		}
	}

	// copy the elements from startIndex to endIndex into output vector
	segment->clear();
	if(startIndex <= endIndex) {
		segment->insert(segment->end(), route.begin()+startIndex, route.begin()+endIndex);
	} else {
		// iterate backward from startIndex to endIndex
		for(int i=startIndex; i>=endIndex; --i) {
			if(i < (int)route.size()) {
				segment->push_back(route.at(i));
			}
		}

	}
}

size_t NatsAirway::size() const {
	return sizeof(NatsAirway) + route.size()*sizeof(string);
}


bool NatsAirway::operator==(const NatsAirway& that) const{

	return  (this->name == that.name);

}

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
 * FlightPlan.cpp
 *
 * Flight plan representation, host-side only 
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "FlightPlan.h"
#include "PointWGS84.h"

#include "geometry_utils.h"

#include "pub_trajectory.h"

#include <cmath>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include <iostream>

using std::cout;
using std::endl;

using namespace osi;

#define PI                      3.14159265359

FlightPlan::FlightPlan() :
  routeString(""),
  departing_taxiPlanString(""),
  landing_taxiPlanString(""),
  flightPlanString(""),
  departing_runwayString(""),
  landing_runwayString(""),
  origin(""),
  origin_name(""),
  origin_latitude(0.0),
  origin_longitude(0.0),
  origin_altitude(0.0),
  destination(""),
  destination_name(""),
  destination_latitude(0.0),
  destination_longitude(0.0),
  destination_altitude(0.0),
  initial_target(""),
  route(vector<PointWGS84>()),
  tocIndex(-1),
  todIndex(-1),
  pathLength(-1){
}

FlightPlan::FlightPlan(const FlightPlan& that) :
  routeString(that.routeString),
  departing_taxiPlanString(that.departing_taxiPlanString),
  landing_taxiPlanString(that.landing_taxiPlanString),
  flightPlanString(that.flightPlanString),
  departing_runwayString(that.departing_runwayString),
  landing_runwayString(that.landing_runwayString),
  origin(that.origin),
  origin_name(that.origin_name),
  origin_latitude(that.origin_latitude),
  origin_longitude(that.origin_longitude),
  origin_altitude(that.origin_altitude),
  destination(that.destination),
  destination_name(that.origin_name),
  destination_latitude(that.origin_latitude),
  destination_longitude(that.origin_longitude),
  destination_altitude(that.origin_altitude),
  initial_target(that.initial_target),
  route(vector<PointWGS84>(that.route)),

  tocIndex(that.tocIndex),
  todIndex(that.todIndex),
  pathLength(that.pathLength) {

}

FlightPlan::~FlightPlan() {
}

bool FlightPlan::operator<(const FlightPlan& that) const {
  // sort by route string
  return this->routeString < that.routeString;
}

FlightPlan& FlightPlan::operator=(const FlightPlan& that) {
	if(this == &that) return *this;

	this->routeString = that.routeString;
	this->departing_taxiPlanString = that.departing_taxiPlanString;
	this->landing_taxiPlanString = that.landing_taxiPlanString;
	this->flightPlanString = that.flightPlanString;
	this->departing_runwayString = that.departing_runwayString;
	this->landing_runwayString = that.landing_runwayString;

	this->origin = that.origin;
	this->origin_name = that.origin_name;
	this->origin_latitude = that.origin_latitude;
	this->origin_longitude = that.origin_longitude;
	this->origin_altitude = that.origin_altitude;

	this->destination = that.destination;
	this->destination_name = that.destination_name;
	this->destination_latitude = that.destination_latitude;
	this->destination_longitude = that.destination_longitude;
	this->destination_altitude = that.destination_altitude;

	this->initial_target = that.initial_target;

	this->route.clear();
	if (that.route.size() > 0) {
		this->route.insert(this->route.begin(),
					   that.route.begin(), that.route.end());
	}

	this->tocIndex = that.tocIndex;
	this->todIndex = that.todIndex;
	this->pathLength = that.pathLength;

	return *this;
}

int FlightPlan::insertTopOfClimb(const double& climbDist) {
	if ((route.size() == 0) || (route[0].proctype == "ENROUTE") || (route[0].proctype == "STAR") || (route[0].proctype == "APPROACH"))
		return 1;

	tocIndex = 0; // Reset

	vector<PointWGS84>::iterator iter_route;

	// Clean up TOP_OF_CLIMB_PT waypoint
	for (iter_route = route.begin(); iter_route != route.end(); iter_route++) {
		if (iter_route->wpname.find(string(TOP_OF_CLIMB_PT), 0) != string::npos) {
			route.erase(iter_route);

			break;
		}
	}

	if (route.size() < 3) {
		tocIndex = 1;
		double fromLat = route[0].latitude;
		double fromLon = route[0].longitude;
		double toLat = route[1].latitude;
		double toLon = route[1].longitude;
		// compute the location of the top-of-climb
		double range = climbDist;
		double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
		double tocLat,tocLon;
		compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);
		vector<PointWGS84>::iterator iter = route.insert(route.begin()+1, PointWGS84(tocLat, tocLon));
		iter->wpname = string(TOP_OF_CLIMB_PT);
		iter->wp_cat = 4;
	} else {
		// find the route points that bracket the top of climb
		// by computing the cumulative path length starting from the
		// beginning of the route.
		double s = 0;
		for (unsigned int j=1; j<route.size(); ++j) {
			double fromLat = route[j-1].latitude;
			double fromLon = route[j-1].longitude;
			double toLat = route[j].latitude;
			double toLon = route[j].longitude;
			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);

			if (s >= climbDist) {
				// compute the location of the top-of-climb
				double range = s - climbDist;
				double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
				double tocLat,tocLon;
				compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);

				// insert the toc at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(tocLat, tocLon));
				iter->wpname = string(TOP_OF_CLIMB_PT);
				iter->wp_cat = 4;
				tocIndex = j;

				break;
			}
		}
	}

	return tocIndex;
}

int FlightPlan::insertTopOfDescent(const double& descentDist) {
	if ((route.size() == 0) || (route[0].proctype == "STAR") || (route[0].proctype == "APPROACH"))
		return 1;

	if(route.size() < 3) {
		todIndex = 1;
		double fromLat = route[1].latitude;
		double fromLon = route[1].longitude;
		double toLat = route[0].latitude;
		double toLon = route[0].longitude;
		// compute the location of the top-of-descent
		double range = descentDist;
		double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
		double todLat,todLon;
		compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);
		vector<PointWGS84>::iterator iter =
				route.insert(route.begin()+1, PointWGS84(todLat, todLon));
		iter->wpname = "TOP_OF_DESCENT_PT";
		iter->wp_cat = 5;
	} else {
		// find the route points that bracket the top of descent
		// by computing the cumulative path length starting from the
		// end of the route
		double s = 0;
		for(int j=(int)route.size()-1; j>0; --j) {
			double fromLat = route[j].latitude;
			double fromLon = route[j].longitude;
			double toLat = route[j-1].latitude;
			double toLon = route[j-1].longitude;
			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);
			if(s >= descentDist) {

				// compute the location of the top-of-descent
				double range = s - descentDist;
				double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
				double todLat, todLon;
				compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

				// insert the tod at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(todLat,todLon));
				iter->wpname = "TOP_OF_DESCENT_PT";
				iter->wp_cat = 5;
				todIndex = j;

				break;
			}
		}
	}
	return todIndex;
}

int FlightPlan::insertTopOfDescent(const double& descentDist, const int index_first_STAR_with_altitude) {
	if ((route.size() == 0) || (route[0].proctype == "APPROACH"))
		return 1;

	todIndex = 0; // Reset

	vector<PointWGS84>::iterator iter_route;

	// Clean up TOP_OF_DESCENT_PT waypoint
	for (iter_route = route.begin(); iter_route != route.end(); iter_route++) {
		if (iter_route->wpname.find(string(TOP_OF_DESCENT_PT), 0) != string::npos) {
			route.erase(iter_route);

			break;
		}
	}

	if (route.size() < 3) {
		todIndex = 1;
		double fromLat = route[1].latitude;
		double fromLon = route[1].longitude;
		double toLat = route[0].latitude;
		double toLon = route[0].longitude;

		// compute the location of the top-of-descent
		double range = descentDist;
		double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
		double todLat, todLon;

		compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

		vector<PointWGS84>::iterator iter =
				route.insert(route.begin()+1, PointWGS84(todLat, todLon));
		iter->wpname = "TOP_OF_DESCENT_PT";
		iter->wp_cat = 5;
	} else {
		// Find the route points that bracket the top of descent
		// by computing the cumulative path length starting from the
		// first STAR waypoint with altitude
		double s = 0;
		for (int j = index_first_STAR_with_altitude; j > 0; --j) {
			double fromLat = route[j].latitude;
			double fromLon = route[j].longitude;
			double toLat = route[j-1].latitude;
			double toLon = route[j-1].longitude;

			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);

			if (s >= descentDist) {
				// Compute the location of the top-of-descent
				double range = s - descentDist;
				double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
				double todLat, todLon;

				compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

				// Insert the TOD at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(todLat,todLon));
				iter->wpname = "TOP_OF_DESCENT_PT";
				iter->wp_cat = 5;
				todIndex = j;

				break;
			}
		}
	}

	return todIndex;
}

int FlightPlan::insertTopOfClimb_geoStyle(const double& climbDist, const double cruiseAltitude) {
	if (route.size() == 0) {
		return 1;
	} else {
		ENUM_Flight_Phase tmpPhase = getFlight_Phase(route[0].phase.c_str());
		if ((route[0].phase == "CRUISE") || (isFlightPhase_in_descending(tmpPhase))) {
			return 1;
		}
	}

	tocIndex = 0; // Reset

	vector<PointWGS84>::iterator iter_route;

	// Clean up TOP_OF_CLIMB_PT waypoint
	for (iter_route = route.begin(); iter_route != route.end(); iter_route++) {
		if (iter_route->wpname.find(string(TOP_OF_CLIMB_PT), 0) != string::npos) {
			route.erase(iter_route);

			break;
		}
	}

	if (route.size() < 3) {
		tocIndex = 1;
		double fromLat = route[0].latitude;
		double fromLon = route[0].longitude;
		double toLat = route[1].latitude;
		double toLon = route[1].longitude;
		// compute the location of the top-of-climb
		double range = climbDist;
		double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
		double tocLat,tocLon;
		compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);
		vector<PointWGS84>::iterator iter = route.insert(route.begin()+1, PointWGS84(tocLat, tocLon));
		iter->wpname = string(TOP_OF_CLIMB_PT);
		iter->alt = cruiseAltitude;
	} else {
		// find the route points that bracket the top of climb
		// by computing the cumulative path length starting from the
		// beginning of the route.
		double s = 0;
		for (unsigned int j=1; j<route.size(); ++j) {
			double fromLat = route[j-1].latitude;
			double fromLon = route[j-1].longitude;
			double toLat = route[j].latitude;
			double toLon = route[j].longitude;
			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);

			if (s >= climbDist) {
				// compute the location of the top-of-climb
				double range = s - climbDist;
				double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
				double tocLat,tocLon;
				compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);

				// insert the toc at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(tocLat, tocLon));
				iter->wpname = string(TOP_OF_CLIMB_PT);
				iter->alt = cruiseAltitude;

				tocIndex = j;

				break;
			}
		}
	}

	return tocIndex;
}

int FlightPlan::insertTopOfClimb_geoStyle(const double& climbDist, const double cruiseAltitude, const int wp_index_start) {
	if (route.size() == 0) {
		return 1;
	} else {
		string tmpStr("FLIGHT_PHASE_");
		tmpStr.append(route[0].phase);

		ENUM_Flight_Phase tmpPhase = getFlight_Phase(tmpStr.c_str());
		if ((route[0].phase == "CRUISE") || (isFlightPhase_in_descending(tmpPhase))) {
			return 1;
		}
	}

	tocIndex = 0; // Reset

	vector<PointWGS84>::iterator iter_route;

	// Clean up TOP_OF_CLIMB_PT waypoint
	for (iter_route = route.begin(); iter_route != route.end(); iter_route++) {
		if (iter_route->wpname.find(string(TOP_OF_CLIMB_PT), 0) != string::npos) {
			route.erase(iter_route);

			break;
		}
	}

	if (route.size() < 3) {
		tocIndex = 1;
		double fromLat = route[0].latitude;
		double fromLon = route[0].longitude;
		double toLat = route[1].latitude;
		double toLon = route[1].longitude;
		// compute the location of the top-of-climb
		double range = climbDist;
		double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
		double tocLat,tocLon;
		compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);
		vector<PointWGS84>::iterator iter = route.insert(route.begin()+1, PointWGS84(tocLat, tocLon));
		iter->wpname = string(TOP_OF_CLIMB_PT);
		iter->alt = cruiseAltitude;
	} else {
		// find the route points that bracket the top of climb
		// by computing the cumulative path length starting from the
		// beginning of the route.
		double s = 0;
		for (unsigned int j = wp_index_start+1; j < route.size(); ++j) {
			double fromLat = route[j-1].latitude;
			double fromLon = route[j-1].longitude;
			double toLat = route[j].latitude;
			double toLon = route[j].longitude;
			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);

			if (s >= climbDist) {
				// compute the location of the top-of-climb
				double range = 0.0;
				if (j == wp_index_start+1) {
					range = climbDist;
				} else {
					range = s - climbDist;
				}

				double heading = compute_heading_gc(fromLat, fromLon, toLat, toLon);
				double tocLat,tocLon;

				compute_location_gc(fromLat, fromLon, range, heading, &tocLat, &tocLon);

				// insert the toc at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(tocLat, tocLon));
				iter->wpname = string(TOP_OF_CLIMB_PT);
				iter->alt = cruiseAltitude;

				tocIndex = j;

				break;
			}
		}
	}

	return tocIndex;
}

int FlightPlan::insertTopOfDescent_geoStyle(const double& descentDist) {
	if (route.size() == 0) {
		return 1;
	} else {
		ENUM_Flight_Phase tmpPhase = getFlight_Phase(route[0].phase.c_str());
		if ((route[0].phase == "CRUISE") || (isFlightPhase_in_descending(tmpPhase))) {
			return 1;
		}
	}

	if(route.size() < 3) {
		todIndex = 1;
		double fromLat = route[1].latitude;
		double fromLon = route[1].longitude;
		double toLat = route[0].latitude;
		double toLon = route[0].longitude;
		// compute the location of the top-of-descent
		double range = descentDist;
		double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
		double todLat,todLon;
		compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);
		vector<PointWGS84>::iterator iter =
				route.insert(route.begin()+1, PointWGS84(todLat, todLon));
		iter->wpname = "TOP_OF_DESCENT_PT";
	} else {
		// find the route points that bracket the top of descent
		// by computing the cumulative path length starting from the
		// end of the route
		double s = 0;
		for(int j=(int)route.size()-1; j>0; --j) {
			double fromLat = route[j].latitude;
			double fromLon = route[j].longitude;
			double toLat = route[j-1].latitude;
			double toLon = route[j-1].longitude;
			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);
			if(s >= descentDist) {

				// compute the location of the top-of-descent
				double range = s - descentDist;
				double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
				double todLat, todLon;
				compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

				// insert the tod at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(todLat,todLon));
				iter->wpname = "TOP_OF_DESCENT_PT";

				todIndex = j;

				break;
			}
		}
	}
	return todIndex;
}

int FlightPlan::insertTopOfDescent_geoStyle(const double& descentDist, const int index_first_waypoint_after_cruise) {
	if ((route.size() == 0) || (route[0].phase == "APPROACH"))
		return 1;

	todIndex = 0; // Reset

	vector<PointWGS84>::iterator iter_route;

	// Clean up TOP_OF_DESCENT_PT waypoint
	for (iter_route = route.begin(); iter_route != route.end(); iter_route++) {
		if (iter_route->wpname.find(string(TOP_OF_DESCENT_PT), 0) != string::npos) {
			route.erase(iter_route);

			break;
		}
	}

	if (route.size() < 3) {
		todIndex = 1;
		double fromLat = route[1].latitude;
		double fromLon = route[1].longitude;
		double toLat = route[0].latitude;
		double toLon = route[0].longitude;

		// compute the location of the top-of-descent
		double range = descentDist;
		double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
		double todLat, todLon;

		compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

		vector<PointWGS84>::iterator iter =
				route.insert(route.begin()+1, PointWGS84(todLat, todLon));
		iter->wpname = "TOP_OF_DESCENT_PT";
	} else {
		// Find the route points that bracket the top of descent
		// by computing the cumulative path length starting from the
		// first STAR waypoint with altitude
		double s = 0;
		for (int j = index_first_waypoint_after_cruise; j > 0; --j) {
			double fromLat = route[j].latitude;
			double fromLon = route[j].longitude;
			double toLat = route[j-1].latitude;
			double toLon = route[j-1].longitude;

			s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);

			if (s >= descentDist) {
				// Compute the location of the top-of-descent
				double range = s - descentDist;
				double heading = compute_heading_gc(toLat, toLon, fromLat, fromLon);
				double todLat, todLon;

				compute_location_gc(toLat, toLon, range, heading, &todLat, &todLon);

				// Insert the TOD at j
				vector<PointWGS84>::iterator iter =
						route.insert(route.begin()+j, PointWGS84(todLat,todLon));
				iter->wpname = "TOP_OF_DESCENT_PT";

				todIndex = j;

				break;
			}
		}
	}

	return todIndex;
}

double FlightPlan::getPathLength() {
	if(pathLength >= 0) return pathLength;
	double s = 0;
	for(unsigned int j=1; j<route.size(); ++j) {
		double fromLat = route[j-1].latitude;
		double fromLon = route[j-1].longitude;
		double toLat = route[j].latitude;
		double toLon = route[j].longitude;
		s += compute_distance_gc(fromLat, fromLon, toLat, toLon, 0);
	}
	pathLength = s;
	return pathLength;
}

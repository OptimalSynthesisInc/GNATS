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
 * RouteElement.h
 *
 *  Created on: Nov 5, 2013
 *      Author: jason
 */

#ifndef ROUTEELEMENT_H_
#define ROUTEELEMENT_H_

#include <vector>
#include <string>

using namespace std;

typedef enum _FpRouteElementType {
	FP_ROUTE_ELEMENT_WAYPOINT,
	FP_ROUTE_ELEMENT_ROUTE
} FpRouteElementType;

/*
 * Abstract base class for route elements
 */
class FpRouteElement {
public:
	FpRouteElement();
	FpRouteElement(const string& id, const string& terminator, const vector<string>& sequence);
	FpRouteElement(const FpRouteElement& that);
	virtual ~FpRouteElement();

	const string& get_identifier() const;
	string get_terminator() const;

	string identifier;       // name of the element
	string terminator;       // name of termination waypoint
	vector<string> sequence; // waypoint name sequence
};

#if 0
/*
 * Departure route.
 */
class FpDepartureRoute : public FpRouteElement {
public:
	FpDepartureRoute();
	FpDepartureRoute(const FpDepartureRoute& that);
	virtual ~FpDepartureRoute();
};

/*
 * Arrival route. May be PAR or STAR
 */
class FpArrivalRoute : public FpRouteElement {
public:
	FpArrivalRoute();
	FpArrivalRoute(const FpArrivalRoute& that);
	virtual ~FpArrivalRoute();

	bool isPar() const;

protected:
	bool par_flag;
};

/*
 * Airway
 */
class FpAirway : public FpRouteElement {
public:
	FpAirway();
	FpAirway(const FpAirway& that);
	virtual ~FpAirway();
};

/*
 * Airport
 */
class FpAirport : public FpRouteElement {
public:
	FpAirport();
	FpAirport(const FpAirport& that);
	virtual ~FpAirport();
};

/*
 * Waypoint
 */
class FpWaypoint : public FpRouteElement {
public:
	FpWaypoint();
	FpWaypoint(const FpWaypoint& that);
	virtual ~FpWaypoint();
};

#endif

#endif /* ROUTEELEMENT_H_ */

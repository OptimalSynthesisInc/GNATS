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
 * tg_Point.cpp
 *
 *  Created on: Aug 15, 2017
 *      Author: pdutta
 */


#include "tg_Point.h"



TrajPoint::TrajPoint ():
	type(0),
	waypointId("NONE"),
	x(0),
	y(0),
	course(0),
	length(0),
	waypointType(0),
	latitude(0),
	longitude(0),
	turnRadius(0),
	eta(0),
	rta(0),
	calibratedAirSpeed(0),
	speedRestriction(0),
	altitude(0),
	altitudeRestriction(0),
	geoAltitude(0),
	fuelRemaining(0),
	outerAirTemperature(0),
	windDirection(0),
	windSpeed(0),
	trueAirSpeed(0),
	trueCourseIntoPoint(0),
	distanceToPoint(0),
	predictedGrossWeight(0),
	constraint(0),
	groundSpeed(0),
	xEndTurn(0),
	yEndTurn(0),
	xStartTurn(0),
	yStartTurn(0),
	xTurnCenter(0),
	yTurnCenter(0),
	turnDistFromWaypoint(0),
	trajSegmentFlags(0),
	distanceToNextHorzPoint(0),
	distanceFromLastHorzPoint(0),
	outboundGroundSpeed(0),
	timeMsForSegment(0),
	alongTrackTurnDistance(0),
	printEnabled(false),
	prId(0){
}

TrajPoint::TrajPoint (const TrajPoint& that):
	type(that.type),
	waypointId(that.waypointId),
	x(that.x),
	y(that.y),
	course(that.course),
	length(that.length),
	waypointType(that.waypointType),
	latitude(that.latitude),
	longitude(that.longitude),
	turnRadius(that.turnRadius),
	eta(that.eta),
	rta(that.rta),
	calibratedAirSpeed(that.calibratedAirSpeed),
	speedRestriction(that.speedRestriction),
	altitude(that.altitude),
	altitudeRestriction(that.altitudeRestriction),
	geoAltitude(that.geoAltitude),
	fuelRemaining(that.fuelRemaining),
	outerAirTemperature(that.outerAirTemperature),
	windDirection(that.windDirection),
	windSpeed(that.windSpeed),
	trueAirSpeed(that.trueAirSpeed),
	trueCourseIntoPoint(that.trueCourseIntoPoint),
	distanceToPoint(that.distanceToPoint),
	predictedGrossWeight(that.predictedGrossWeight),
	constraint(that.constraint),
	groundSpeed(that.groundSpeed),
	xEndTurn(that.xEndTurn),
	yEndTurn(that.yEndTurn),
	xStartTurn(that.xStartTurn),
	yStartTurn(that.yStartTurn),
	xTurnCenter(that.xTurnCenter),
	yTurnCenter(that.yTurnCenter),
	turnDistFromWaypoint(that.turnDistFromWaypoint),
	trajSegmentFlags(that.trajSegmentFlags),
	distanceToNextHorzPoint(that.distanceToNextHorzPoint),
	distanceFromLastHorzPoint(that.distanceFromLastHorzPoint),
	outboundGroundSpeed(that.outboundGroundSpeed),
	timeMsForSegment(that.timeMsForSegment),
	alongTrackTurnDistance(that.alongTrackTurnDistance),
	printEnabled(that.printEnabled),
	prId(that.prId){
}

TrajPoint::~TrajPoint(){
}

bool TrajPoint::operator==(const TrajPoint& that) const{
	return (this->waypointId == that.waypointId) &&
			(this->latitude == that.latitude) &&
			(this->longitude == that.longitude);
}


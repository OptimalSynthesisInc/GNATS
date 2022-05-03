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
 * tg_Point.h
 *
 *  Created on: Aug 15, 2017
 *      Author: pdutta
 *
 *      TRAJECTORY POINT AS DEFINED IN MACS
 */



#ifndef SRC_LIBTG_SRC_TG_POINT_H_
#define SRC_LIBTG_SRC_TG_POINT_H_

#include <vector>
#include <map>
#include <string>

using std::vector;
using std::map;
using std::string;

typedef enum _MACS_waypoint_type {

TRAJ_TYPE_WP = 0,    /*
 * waypoint
 */

TRAJ_TYPE_HP = 1,    /*
 * holding pattern
 */

TRAJ_TYPE_PH = 2,    /*
 * proc hold
 */

TRAJ_TYPE_PT = 3,    /*
 * proc turn
 */

TRAJ_TYPE_RF = 4,    /*
 * rf leg
 */

TRAJ_TYPE_TC = 5,    /*
 * TOC
 */

TRAJ_TYPE_TD = 6,    /*
 * TOD
 */

TRAJ_TYPE_SL = 7,    /*
 * start of level
 */

TRAJ_TYPE_CA = 8,    /*
 * crossover altitude
 */

TRAJ_TYPE_TA = 9,    /*
 * transition altitude
 */

TRAJ_TYPE_AC = 10,   /*
 * Aircraft position
 */

TRAJ_TYPE_CS = 11,   /*
 * only constraint
 */

TRAJ_TYPE_RT = 12,   /*
 * part of current rte
 */

TRAJ_TYPE_AP = 13,   /*
 * Airport DATA
 */

TRAJ_TYPE_SC = 14   /*
 * Speed Change Point
 */
}MACS_Waypoint_type;


//TODO:PARIKSHIT ADDER
typedef enum _flight_mode_MACS{
 TRAJ_SEG_MACH = 1,
 TRAJ_SEG_CAS = 2,
 TRAJ_SEG_IDLE_DESCENT = 3,
 TRAJ_SEG_CLIMB = 4,
 TRAJ_SEG_CRUISE = 5,
 TRAJ_SEG_DESCENT = 6
}flight_mode_MACS;

class TrajPoint{
public:
	TrajPoint();
	TrajPoint(const TrajPoint& that);
	virtual ~TrajPoint();

	int type; // DO NOT KNOW WHAT THIS IS
	string waypointId; //NAME OF THE WAYPOINT
	double x; // IF STEREO PROJECTION IS USED THEN THIS IS NECESSARY ELSE NOT
	double y;// IF STEREO PROJECTION IS USED THEN THIS IS NECESSARY ELSE NOT
	double course;// INBOUND MAGNETIC COURSE
	double length;// DISTANCE TO POINT
	int waypointType;// MACS WAYPOINT TYPE AS GIVEN IN Macs.java
	double latitude;// LATITUDE OF THE POINT
	double longitude;// LONGITUDE OF THE POINT
	double turnRadius;// TURN RADIUS AT THE WP. SEE TrajPoint.getTurnRadiusDefault FOR MORE INFO
	long eta;// ESTIMATED TIME OF ARRIVAL POINT
	long rta; //REQUIRED TIME OF ARRIVAL AT THE POINT
	double calibratedAirSpeed; // CAS AT THE WP.
	double speedRestriction;// SPEED LIMIT (GET FROM SID)
	double altitude;// CURRENT ALTITUDE
	double altitudeRestriction;//ALTITUDE CONSTRAINTS
	double geoAltitude;// CURRENT GEO ALTITUDE
	double fuelRemaining;// FUEL REMAINING
	double outerAirTemperature;// CURRENT AIR TEM
	double windDirection;// CURRENT AIR DIR
	double windSpeed;// CURRENT WIND SPEED
	double trueAirSpeed;// TAS AT THE WP
	double trueCourseIntoPoint;// TRACK ANGLE NEEDED TO REACH WP
	double distanceToPoint;//DISTANCE FROM THE CURRENT STATE TO WP.
	double predictedGrossWeight;// AC WEIGHT AT WP
	int constraint;// IF THERE IS A CONSTRAINT
	double groundSpeed;// GROUND SPEED AT WP
	double xEndTurn;// IF THERE IS A TURN AT WP THE X POS OF WHERE TO END THE TURN
	double yEndTurn;// IF THERE IS A TURN AT WP THE Y POS OF WHERE TO END THE TURN
	double xStartTurn;// IF THERE IS A TURN AT WP THE X POS OF WHERE TO START THE TURN
	double yStartTurn;// IF THERE IS A TURN AT WP THE Y POS OF WHERE TO START THE TURN
	double xTurnCenter;// IF THERE IS A TURN AT WP THE X POS OF TURN CENTER
	double yTurnCenter;// IF THERE IS A TURN AT WP THE X POS OF TURN CENTER
	double turnDistFromWaypoint;// NOT SURE
	int trajSegmentFlags;// WHAT TYPE OF SEGMENT IS THIS SEE FLIGHT MODE MACS
	double distanceToNextHorzPoint;// DISTANCE TO NEXT HORZ POINT
	double distanceFromLastHorzPoint;// DISTANCE TO NEXT HORZ POINT
	double outboundGroundSpeed;//GROUND SPEED VALUE AFTER DEPARTING WP
	long timeMsForSegment;//(3600. * 1000. * POINT LENGTH / GROUNDSPEED);
	double alongTrackTurnDistance;// NOT SURE HOW IS IT DIFFERENT FROM TURN DIST FROM WP



private:
	bool printEnabled;
	int prId;    // Used as private index whenever required

public:

	bool operator==(const TrajPoint& that) const;

};




#endif /* SRC_LIBTG_SRC_TG_POINT_H_ */

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
 * tg_trajectory.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: jason
 */

#include "tg_trajectory.h"
#include <string>
#include <vector>

using namespace std;

Trajectory::Trajectory() :
	flight_index(0),
	callsign(""),
	actype(""),
	origin_airport(""),
	destination_airport(""),
	start_time(0),
	interval_ground(0),
	interval_airborne(0),
	cruise_altitude_ft(0.0),
	cruise_tas_knots(0.0),
	origin_airport_elevation_ft(0.0),
	destination_airport_elevation_ft(0.0),
	flag_externalAircraft(false),
	latitude_deg(vector<real_t>()),
	longitude_deg(vector<real_t>()),
	altitude_ft(vector<real_t>()),
	rocd_fps(vector<real_t>()),
	tas_knots(vector<real_t>()),
	tas_knots_ground(vector<real_t>()),
	course_deg(vector<real_t>()),
	fpa_deg(vector<real_t>()),
	flight_phase(vector<ENUM_Flight_Phase>()) {
	latitude_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	longitude_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	altitude_ft.reserve(INIT_TRAJECTORY_CAPACITY);
	rocd_fps.reserve(INIT_TRAJECTORY_CAPACITY);
	tas_knots.reserve(INIT_TRAJECTORY_CAPACITY);
	tas_knots_ground.reserve(INIT_TRAJECTORY_CAPACITY);
	course_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	fpa_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	flight_phase.reserve(INIT_TRAJECTORY_CAPACITY);
}

Trajectory::Trajectory(const int& flight_index,
	   const string& callsign,
	   const string& actype,
	   const string& origin_airport,
	   const string& destination_airport,
	   const float& start_time,
	   const float& interval_ground,
	   const float& interval_airborne,
	   const float& cruise_altitude_ft,
	   const float& cruise_tas_knots,
	   const float& origin_airport_elevation_ft,
	   const float& destination_airport_elevation_ft,
	   const bool flag_externalAircraft) :
	flight_index(flight_index),
	callsign(callsign),
	actype(actype),
	origin_airport(origin_airport),
	destination_airport(destination_airport),
	start_time(start_time),
	interval_ground(interval_ground),
	interval_airborne(interval_airborne),
	cruise_altitude_ft(cruise_altitude_ft),
	cruise_tas_knots(cruise_tas_knots),
	origin_airport_elevation_ft(origin_airport_elevation_ft),
	destination_airport_elevation_ft(destination_airport_elevation_ft),
	flag_externalAircraft(flag_externalAircraft),
	latitude_deg(vector<real_t>()),
	longitude_deg(vector<real_t>()),
	altitude_ft(vector<real_t>()),
	rocd_fps(vector<real_t>()),
	tas_knots(vector<real_t>()),
	tas_knots_ground(vector<real_t>()),
	course_deg(vector<real_t>()),
	fpa_deg(vector<real_t>()),
	flight_phase(vector<ENUM_Flight_Phase>()),
	timestamp(vector<float>())
	{
	latitude_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	longitude_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	altitude_ft.reserve(INIT_TRAJECTORY_CAPACITY);
	rocd_fps.reserve(INIT_TRAJECTORY_CAPACITY);
	tas_knots.reserve(INIT_TRAJECTORY_CAPACITY);
	tas_knots_ground.reserve(INIT_TRAJECTORY_CAPACITY);
	course_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	fpa_deg.reserve(INIT_TRAJECTORY_CAPACITY);
	flight_phase.reserve(INIT_TRAJECTORY_CAPACITY);
	timestamp.reserve(INIT_TRAJECTORY_CAPACITY);
}


Trajectory::Trajectory(const Trajectory& that) :
	flight_index(that.flight_index),
	callsign(that.callsign),
	actype(that.actype),
	origin_airport(that.origin_airport),
	destination_airport(that.destination_airport),
	start_time(that.start_time),
	interval_ground(that.interval_ground),
	interval_airborne(that.interval_airborne),
	cruise_altitude_ft(that.cruise_altitude_ft),
	cruise_tas_knots(that.cruise_tas_knots),
	origin_airport_elevation_ft(that.origin_airport_elevation_ft),
	destination_airport_elevation_ft(that.destination_airport_elevation_ft),
	flag_externalAircraft(that.flag_externalAircraft),
	latitude_deg(that.latitude_deg),
	longitude_deg(that.longitude_deg),
	altitude_ft(that.altitude_ft),
	rocd_fps(that.rocd_fps),
	tas_knots(that.tas_knots),
	tas_knots_ground(that.tas_knots_ground),
	course_deg(that.course_deg),
	fpa_deg(that.fpa_deg),
	flight_phase(that.flight_phase),
	timestamp(that.timestamp) {
}

Trajectory::~Trajectory() {
}

size_t Trajectory::size() const {
	int n = latitude_deg.size();
	return sizeof(Trajectory) + n*7*sizeof(real_t) + n*sizeof(int) +
	        n*sizeof(string) +
			n*sizeof(ENUM_Flight_Phase) + sizeof(long) + sizeof(int);
}

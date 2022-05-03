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
 * tg_trajectory.h
 *
 *  Created on: Oct 29, 2013
 *      Author: jason
 */

#ifndef TG_TRAJECTORY_H_
#define TG_TRAJECTORY_H_

#include <string>
#include <vector>

#include "real_t.h"
#include "pub_trajectory.h"

using std::string;
using std::vector;

#define INIT_TRAJECTORY_CAPACITY 50000


class Trajectory {
public:
	Trajectory();

	Trajectory(const int& flight_index,
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
			   const bool flag_externalAircraft);

	Trajectory(const Trajectory& that);

	virtual ~Trajectory();

	int flight_index;
	string callsign;
	string actype;
	string origin_airport;
	string destination_airport;
	float start_time;
	float interval_ground;
	float interval_airborne;
	float cruise_altitude_ft;
	float cruise_tas_knots;
	float origin_airport_elevation_ft;
	float destination_airport_elevation_ft;
	bool flag_externalAircraft;

	vector<real_t> latitude_deg;
	vector<real_t> longitude_deg;
	vector<real_t> altitude_ft;
	vector<real_t> rocd_fps;
	vector<real_t> tas_knots;
	vector<real_t> tas_knots_ground;
	vector<real_t> course_deg;
	vector<real_t> fpa_deg;
	vector<ENUM_Flight_Phase> flight_phase;
	vector<float> timestamp;

	size_t size() const;
};

#endif /* TG_TRAJECTORY_H_ */

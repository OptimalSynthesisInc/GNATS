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

#include "pub_aircraft.h"

#include <stdio.h>
#include <stdlib.h>

NewMU_Aircraft::NewMU_Aircraft():
	flag_external_aircraft(vector<bool>()),
	flag_data_initialized(vector<bool>()),

	latitude_deg(vector<real_t>()),
	longitude_deg(vector<real_t>()),
	latitude_deg_traj(vector<real_t>()),
	longitude_deg_traj(vector<real_t>()),
	altitude_ft(vector<real_t>()),
	rocd_fps(vector<real_t>()),
	tas_knots(vector<real_t>()),
	tas_knots_ground(vector<real_t>()),
	course_rad(vector<real_t>()),
	fpa_rad(vector<real_t>()),
	flight_phase(vector<ENUM_Flight_Phase>()),

	latitude_deg_pre_pause(vector<real_t>()),
	longitude_deg_pre_pause(vector<real_t>()),
	altitude_ft_pre_pause(vector<real_t>()),
	rocd_fps_pre_pause(vector<real_t>()),
	tas_knots_pre_pause(vector<real_t>()),
	course_rad_pre_pause(vector<real_t>()),
	fpa_rad_pre_pause(vector<real_t>()),
	cruise_alt_ft_pre_pause(vector<real_t>()),
	cruise_tas_knots_pre_pause(vector<real_t>()),

	departure_time_sec(vector<real_t>()),
	cruise_alt_ft(vector<real_t>()),
	cruise_tas_knots(vector<real_t>()),

	origin_airport_elevation_ft(vector<real_t>()),
	destination_airport_elevation_ft(vector<real_t>()),

	landed_flag(vector<int>()),

	adb_aircraft_type_index(vector<int>()),

	holding_started(vector<bool>()),
	holding_stopped(vector<bool>()),
	has_holding_pattern(vector<bool>()),
	hold_start_index(vector<int>()),
	hold_end_index(vector<int>()),
	holding_tas_knots(vector<real_t>()),

	target_waypoint_index(vector<int>()),
	target_altitude_ft(vector<real_t>()),
	toc_index(vector<int>()),
	tod_index(vector<int>()),

	runway_name_departing(vector<char*>()),
	runway_name_landing(vector<char*>()),

	target_waypoint_node_ptr(vector<waypoint_node_t*>()),
	flag_target_waypoint_change(vector<bool>()),
	last_WaypointNode_ptr(vector<waypoint_node_t*>()),
	flag_reached_meterfix_point(vector<bool>()),

	V_horizontal(vector<real_t>()),
	acceleration_aiming_waypoint_node_ptr(vector<waypoint_node_t*>()),
	acceleration(vector<real_t>()),
	V2_point_latitude_deg(vector<real_t>()),
	V2_point_longitude_deg(vector<real_t>()),
	estimate_touchdown_point_latitude_deg(vector<real_t>()),
	estimate_touchdown_point_longitude_deg(vector<real_t>()),

	t_takeoff(vector<float>()), // Record the time when the aircraft take off
	t_landing(vector<float>()), // Record the time when the aircraft land

	hold_flight_phase(vector<ENUM_Flight_Phase>()), // Variable indicating the flight phase to be held

	course_rad_runway(vector<real_t>()),
	course_rad_taxi(vector<real_t>())
{}

void NewMU_Aircraft::clear() {
	char* tmpCharArray = NULL;

	flag_external_aircraft.clear();
	flag_data_initialized.clear();

	latitude_deg.clear();
	longitude_deg.clear();
	latitude_deg_traj.clear();
	longitude_deg_traj.clear();
	altitude_ft.clear();
	rocd_fps.clear();
	tas_knots.clear();
	tas_knots_ground.clear();
	course_rad.clear();
	fpa_rad.clear();
	flight_phase.clear();

	latitude_deg_pre_pause.clear();
	longitude_deg_pre_pause.clear();
	altitude_ft_pre_pause.clear();
	rocd_fps_pre_pause.clear();
	tas_knots_pre_pause.clear();
	course_rad_pre_pause.clear();
	fpa_rad_pre_pause.clear();
	cruise_alt_ft_pre_pause.clear();
	cruise_tas_knots_pre_pause.clear();

	departure_time_sec.clear();
	cruise_alt_ft.clear();
	cruise_tas_knots.clear();

	origin_airport_elevation_ft.clear();
	destination_airport_elevation_ft.clear();

	landed_flag.clear();

	adb_aircraft_type_index.clear();

	holding_started.clear();
	holding_stopped.clear();
	has_holding_pattern.clear();
	hold_start_index.clear();
	hold_end_index.clear();
	holding_tas_knots.clear();

	target_waypoint_index.clear();
	target_altitude_ft.clear();
	toc_index.clear();
	tod_index.clear();

	if (runway_name_departing.size() > 0) {
		vector<char*>::iterator ite_runwayNameDep;
		for (ite_runwayNameDep = runway_name_departing.begin(); ite_runwayNameDep != runway_name_departing.end(); ite_runwayNameDep++) {
			tmpCharArray = *ite_runwayNameDep;
			if (tmpCharArray != NULL) {
				free(tmpCharArray);
			}
		}

	}
	runway_name_departing.clear();

	if (runway_name_landing.size() > 0) {
		vector<char*>::iterator ite_runwayNameLand;
		for (ite_runwayNameLand = runway_name_landing.begin(); ite_runwayNameLand != runway_name_landing.end(); ite_runwayNameLand++) {
			tmpCharArray = *ite_runwayNameLand;
			if (tmpCharArray != NULL) {
				free(tmpCharArray);
			}
		}

	}
	runway_name_landing.clear();

	target_waypoint_node_ptr.clear();
	flag_target_waypoint_change.clear();
	last_WaypointNode_ptr.clear();
	flag_reached_meterfix_point.clear();

	V_horizontal.clear();
	acceleration_aiming_waypoint_node_ptr.clear();
	acceleration.clear();
	V2_point_latitude_deg.clear();
	V2_point_longitude_deg.clear();
	estimate_touchdown_point_latitude_deg.clear();
	estimate_touchdown_point_longitude_deg.clear();

	t_takeoff.clear();
	t_landing.clear();

	hold_flight_phase.clear();

	course_rad_runway.clear();
	course_rad_taxi.clear();
}

void NewMU_Aircraft::resize(int size) {
	flag_external_aircraft.resize(size);
	flag_data_initialized.resize(size);

	latitude_deg.resize(size);
	longitude_deg.resize(size);
	latitude_deg_traj.resize(size);
	longitude_deg_traj.resize(size);
	altitude_ft.resize(size);
	rocd_fps.resize(size);
	tas_knots.resize(size);
	tas_knots_ground.resize(size);
	course_rad.resize(size);
	fpa_rad.resize(size);
	flight_phase.resize(size);

	latitude_deg_pre_pause.resize(size);
	longitude_deg_pre_pause.resize(size);
	altitude_ft_pre_pause.resize(size);
	rocd_fps_pre_pause.resize(size);
	tas_knots_pre_pause.resize(size);
	course_rad_pre_pause.resize(size);
	fpa_rad_pre_pause.resize(size);
	cruise_alt_ft_pre_pause.resize(size);
	cruise_tas_knots_pre_pause.resize(size);

	departure_time_sec.resize(size);
	cruise_alt_ft.resize(size);
	cruise_tas_knots.resize(size);

	origin_airport_elevation_ft.resize(size);
	destination_airport_elevation_ft.resize(size);

	landed_flag.resize(size);

	adb_aircraft_type_index.resize(size);

	holding_started.resize(size);
	holding_stopped.resize(size);
	has_holding_pattern.resize(size);
	hold_start_index.resize(size);
	hold_end_index.resize(size);
	holding_tas_knots.resize(size);

	target_waypoint_index.resize(size);
	target_altitude_ft.resize(size);
	toc_index.resize(size);
	tod_index.resize(size);

	runway_name_departing.resize(size);
	runway_name_landing.resize(size);

	target_waypoint_node_ptr.resize(size);
	flag_target_waypoint_change.resize(size);
	last_WaypointNode_ptr.resize(size);
	flag_reached_meterfix_point.resize(size);

	V_horizontal.resize(size);
	acceleration_aiming_waypoint_node_ptr.resize(size);
	acceleration.resize(size);
	V2_point_latitude_deg.resize(size);
	V2_point_longitude_deg.resize(size);
	estimate_touchdown_point_latitude_deg.resize(size);
	estimate_touchdown_point_longitude_deg.resize(size);

	t_takeoff.resize(size);
	t_landing.resize(size);

	hold_flight_phase.resize(size);

	course_rad_runway.resize(size);
	course_rad_taxi.resize(size);
}

NewMU_Aircraft::~NewMU_Aircraft()
{
	clear();
}

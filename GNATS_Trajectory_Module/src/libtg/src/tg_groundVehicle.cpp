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

#include "tg_groundVehicle.h"

#include "pub_groundVehicle.h"
#include "tg_api.h"
#include <vector>
#include <cmath>
#include "TrxInputStream_GroundVehicle.h"

vector<TrxRecord_GroundVehicle> g_groundVehicleRecords;

vector<GroundVehicle> g_groundVehicles;

vector<GroundVehicle> groundVehicleStates;

vector<GroundVehicle> groundVehiclePreviousStates;

map<string, int> map_VehicleId_Seq;

map<string, string> map_groundVehicle_owner;

bool compute_groundVehicle_data(long departure_time,
		TrxRecord_GroundVehicle& record,
		GroundVehicle& groundVehicle) {
	bool retValue = false;

	string route_str(record.route_str);

	int pos_1st_dot = route_str.find(".");
	int pos_left_arrow = route_str.find("<");
	int pos_right_arrow = route_str.find(">");

	if ((pos_1st_dot == string::npos) || (pos_left_arrow == string::npos) || (pos_right_arrow == string::npos)
			|| (pos_left_arrow < pos_1st_dot) || (pos_right_arrow < pos_1st_dot)) {


		printf("Ground vehicle %s: wrong drive plan\n", record.vehicle_id.c_str());

		return retValue;
	}

	string str_airport_code = route_str.substr(0, pos_1st_dot);

	string str_drivePlan = route_str.substr(pos_left_arrow+1, (pos_right_arrow - pos_left_arrow - 1));

	waypoint_node_t* drivePlan_ptr = produce_surface_waypoint_node_linkedList(record.vehicle_id, str_airport_code, str_drivePlan);

	// Debug vehicle driving waypoint node list
	/*
	if (drivePlan_ptr != NULL) {
		waypoint_node_t* tmp_wp_ptr = drivePlan_ptr;
		while (tmp_wp_ptr != NULL) {
			printf("compute_groundVehicle_data() --> waypoint = %s, lat/lon = %f, %f\n", (tmp_wp_ptr->wpname != NULL ? tmp_wp_ptr->wpname : ""), tmp_wp_ptr->latitude, tmp_wp_ptr->longitude);
			tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
		}
	}
	*/

	groundVehicle.vehicle_id.assign(record.vehicle_id);
	groundVehicle.airport_id.assign(record.airport_id);
	groundVehicle.aircraft_id.assign(record.aircraft_id);
	groundVehicle.latitude = record.latitude;
	groundVehicle.longitude = record.longitude;
	groundVehicle.altitude = record.altitude;
	groundVehicle.speed = record.speed;
	groundVehicle.course = record.course * M_PI / 180;
	groundVehicle.departure_time = record.timestamp;

	groundVehicle.drive_plan_ptr = drivePlan_ptr;

	int tmpCountWaypoint = 0;
	waypoint_node_t* tmp_wp_ptr = groundVehicle.drive_plan_ptr;
	while (tmp_wp_ptr != NULL) {
		tmpCountWaypoint++;

		groundVehicle.drive_plan_final_node_ptr = tmp_wp_ptr;

		tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
	}

	groundVehicle.drive_plan_length = tmpCountWaypoint;

	retValue = true;

	return retValue;
}

int load_groundVehicle(const string& trx_file) {
	int retValue = 1;

	printf("  Loading ground vehicle data\n");

	// Load the TRX data
	int err = 0;
	TrxHandler_GroundVehicle handler;
	TrxInputStream_GroundVehicle in(trx_file);
	in.addTrxHandler(&handler);
	err = in.parse(g_groundVehicleRecords);
	if (err) {
		printf("Error loading TRX file\n");

		return retValue;
	}

	GroundVehicle* newGroundVehicle;

	for (unsigned int i = 0; i < g_groundVehicleRecords.size(); i++) {
		TrxRecord_GroundVehicle& record = g_groundVehicleRecords.at(i);

		long departure_time = record.timestamp - g_start_time;

		newGroundVehicle = new GroundVehicle();

		bool result = compute_groundVehicle_data(departure_time,
				record,
				*newGroundVehicle);

		if (result) {
			g_groundVehicles.push_back(*newGroundVehicle);

			map_VehicleId_Seq.insert(pair<string, int>(record.vehicle_id, g_groundVehicles.size()-1));
		} else {
			delete newGroundVehicle;
		}
	}
	groundVehicleStates = g_groundVehicles;

/*
	// Debug
	for (unsigned int i = 0; i < g_groundVehicles.size(); i++) {
		GroundVehicle& tmpGroundVehicle = g_groundVehicles.at(i);
		printf("\ni = %d, Vehicle = %s, aircraft_id = %s, lat/lon = %f, %f, speed = %f, altitude = %f, course_rad = %f, departure_time = %ld, drive_plan_length = %d, target_waypoint_index = %d\n",
				i,
				tmpGroundVehicle.vehicle_id.c_str(), tmpGroundVehicle.aircraft_id.c_str(), tmpGroundVehicle.latitude_deg, tmpGroundVehicle.longitude_deg, tmpGroundVehicle.speed, tmpGroundVehicle.altitude_ft, tmpGroundVehicle.course_rad, tmpGroundVehicle.departure_time_sec,
				tmpGroundVehicle.drive_plan_length,
				tmpGroundVehicle.target_waypoint_index);

		waypoint_node_t* tmpWaypoint_node_ptr = tmpGroundVehicle.drive_plan_ptr;
		while (tmpWaypoint_node_ptr != NULL) {
			printf("	i = %d, drive waypoint = %s, lat/lon = %f, %f\n", i, tmpWaypoint_node_ptr->wpname, tmpWaypoint_node_ptr->latitude, tmpWaypoint_node_ptr->longitude);

			tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr;
		}
	}
*/
	retValue = 0;

	return retValue;
}

int release_groundVehicle() {
	map_VehicleId_Seq.clear();

	if (0 < g_groundVehicles.size()) {
		waypoint_node_t* tmp_cur_waypointNode_ptr;
		waypoint_node_t* tmp_next_waypointNode_ptr;

		for (unsigned int i = 0; i < g_groundVehicles.size(); i++) {
			tmp_next_waypointNode_ptr = g_groundVehicles.at(i).drive_plan_ptr;

			while (tmp_next_waypointNode_ptr != NULL) {
				tmp_cur_waypointNode_ptr = tmp_next_waypointNode_ptr;

				releaseWaypointNodeContent(tmp_cur_waypointNode_ptr);

				tmp_cur_waypointNode_ptr->prev_node_ptr = NULL;
				tmp_next_waypointNode_ptr = tmp_cur_waypointNode_ptr->next_node_ptr;
				tmp_cur_waypointNode_ptr->next_node_ptr = NULL;

				free(tmp_cur_waypointNode_ptr);
			}

			g_groundVehicles.at(i).drive_plan_ptr = NULL;
			g_groundVehicles.at(i).drive_plan_final_node_ptr = NULL;
		}
	}

	g_groundVehicles.clear();

	g_groundVehicleRecords.clear();

	printf("Ground vehicle data released.\n");

	return 0;
}


int select_groundVehicleSeq_by_groundVehicleId(string gvid) {
	int ret_GroundVehicleSeq = -1;

	map<string, int>::iterator iterator_map_Gvid_GroundVehicleSeq;
	iterator_map_Gvid_GroundVehicleSeq = map_VehicleId_Seq.find(gvid);
	if (iterator_map_Gvid_GroundVehicleSeq != map_VehicleId_Seq.end()) {
		ret_GroundVehicleSeq = iterator_map_Gvid_GroundVehicleSeq->second; // Got ground vehicle sequence
	}

	return ret_GroundVehicleSeq;
}

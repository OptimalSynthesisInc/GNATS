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

#include "tg_api.h"

#include "tg_rap.h"

#include "tg_simulation.h"

#include <unistd.h>


int main(int argc, char* argv[]) {
	//printf("test_monte_carlo starting\n");
/*
//printf("size of ENUM_Flight_Phase = %d\n", sizeof(ENUM_Flight_Phase_String));
	int i = 0;
//while (ENUM_Flight_Phase_String[i] != NULL) {
while ((ENUM_Flight_Phase_String[i] != NULL) &&
		//(strcmp(ENUM_Flight_Phase_String[i], "") != 0)) {
		(sizeof(ENUM_Flight_Phase_String[i]) != 0)) {
printf("i = %d, string = %s\n", i, ENUM_Flight_Phase_String[i]);
	i++;
}
//printf("ENUM_Flight_Phase_String = %s\n", ENUM_Flight_Phase_String[100]);
printf("Totol i = %d\n", i);
//printf("ENUM_Flight_Phase_String 75 = %s\n", ENUM_Flight_Phase_String[75]);

//	flight_mode_e a = flight_mode_e(1);
//printf("flight_mode_e a = %d\n", a);
//printf("(a == flight_mode_e::FLIGHT_MODE_CLIMB) ? %d\n", (a == FLIGHT_MODE_CLIMB));
//printf("(a == flight_mode_e::FLIGHT_MODE_DESCENT) ? %d\n", (a == FLIGHT_MODE_DESCENT));

//	ENUM_Fruit b = banana; //(apple);
//printf("b String = %s\n", ENUM_Fruit_String[b]);



ENUM_Flight_Phase fp = ENUM_Flight_Phase(3);
printf("(fp == FLIGHT_PHASE_RAMP_DEPARTING) ? %d\n", (fp == FLIGHT_PHASE_RAMP_DEPARTING));
return 0;
*/


	int count = 0;

	tg_init("share", 0, 0);

	while (count < 2) {
		count++;
printf("*********************************************************************\n");
printf("Loading wind and aircraft --> count = %d *****************************\n", count);
printf("*********************************************************************\n");

		load_rap("share/tg/rap");

		//load_aircraft("share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate.trx", "share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx");

		load_aircraft("share/tg/trx/TRX_DEMO_10rec_beta1.5.trx", "share/tg/trx/TRX_DEMO_10rec_mfl_beta1.5.trx");








		//user_defined_waypoint_ids_departing = ["Gate_07_005", "Ramp_07_006", "Txy_B_B5", "Txy_B_B2", "Txy_B_B1", "Txy_B_003", "Txy_B_K", "Txy_B_D", "Txy_E_B", "Txy_A_009", "Txy_A_F1", "Txy_A_M", "Txy_A_003", "Txy_A_002", "Txy_A_A1", "Txy_A_001", "Rwy_02_001", "Rwy_02_007"]
		/*
		vector<string> vector_user_defined_waypoint_ids_departing;
		vector_user_defined_waypoint_ids_departing.push_back("Gate_07_005");
		vector_user_defined_waypoint_ids_departing.push_back("Ramp_07_006");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_B5");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_B2");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_B1");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_003");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_K");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_B_D");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_E_B");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_009");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_F1");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_M");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_003");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_002");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_A1");
		vector_user_defined_waypoint_ids_departing.push_back("Txy_A_001");
		vector_user_defined_waypoint_ids_departing.push_back("Rwy_02_001");
		vector_user_defined_waypoint_ids_departing.push_back("Rwy_02_007");
		*/

		//set_user_defined_surface_taxi_plan(0, "KSFO", vector_user_defined_waypoint_ids_departing);





		//user_defined_waypoint_ids_landing = ["Rwy_03_001", "Rwy_03_009", "Txy_G7_001", "Txy_F_009", "Txy_F_010", "Txy_F_011", "Txy_F_012", "Txy_F11_001", "Rwy_02_015", "Rwy_02_017", "Txy_E12_001", "Txy_E_021", "Txy_E_022", "Txy_D13_001", "Ramp_10_005", "Ramp_10_006", "Gate_10_006"]
		/*
		vector<string> vector_user_defined_waypoint_ids_landing;
		vector_user_defined_waypoint_ids_landing.push_back("Rwy_03_001");
		vector_user_defined_waypoint_ids_landing.push_back("Rwy_03_009");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_G7_001");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_F_009");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_F_010");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_F_011");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_F_012");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_F11_001");
		vector_user_defined_waypoint_ids_landing.push_back("Rwy_02_015");
		vector_user_defined_waypoint_ids_landing.push_back("Rwy_02_017");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_E12_001");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_E_021");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_E_022");
		vector_user_defined_waypoint_ids_landing.push_back("Txy_D13_001");
		vector_user_defined_waypoint_ids_landing.push_back("Ramp_10_005");
		vector_user_defined_waypoint_ids_landing.push_back("Ramp_10_006");
		vector_user_defined_waypoint_ids_landing.push_back("Gate_10_006");
		*/
		//set_user_defined_surface_taxi_plan(0, "KPHX", vector_user_defined_waypoint_ids_landing);



	    //flag_enable_strategic_weather_avoidance = true;








		propagate_flights(86400, 30);



		nats_simulation_operator(NATS_SIMULATION_STATUS_START);
/*
		while (true) {
		    int server_runtime_sim_status = get_runtime_sim_status();
		    if (server_runtime_sim_status == NATS_SIMULATION_STATUS_PAUSE) {
		        break;
			} else {
		        usleep(1000);
			}
		}

		d_aircraft_soa.latitude_deg[0] = (36.0f);
		d_aircraft_soa.longitude_deg[0] = (-120.0f);
		d_aircraft_soa.altitude_ft[0] = (32000.0f);
		d_aircraft_soa.tas_knots[0] = (400.0f);
		d_aircraft_soa.course_rad[0] = (110.0f * 3.1415926 / 180.0f);
	    d_aircraft_soa.rocd_fps[0] = (50.0f);

	    nats_simulation_operator(NATS_SIMULATION_STATUS_RESUME);
*/
		while (true) {
		    int server_runtime_sim_status = get_runtime_sim_status();
		    if (server_runtime_sim_status == NATS_SIMULATION_STATUS_ENDED) {
		        break;
			} else {
		        usleep(1000);
			}
		}



printf("test_monte_carlo --> main() --> About to destroy_aircraft()\n");
		destroy_aircraft();
printf("test_monte_carlo --> main() --> About to destroy_rap()\n");
		destroy_rap();
printf("test_monte_carlo --> main() ending\n");
	} // end - while

	tg_shutdown();
}

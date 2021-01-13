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
#include "tg_aircraft.h"
#include "tg_rap.h"

#include "tg_simulation.h"

#include <cstring>
#include <string>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
	printf("test_user_upload_aircraft starting\n");





	tg_init("share", 0, 0);

	load_rap("share/tg/rap");

	collect_user_upload_aircraft("TRACK_TIME 1121238067",
			"TRACK SWA1897 B733 373638 1222286 130 0 28 ZOA ZOA46",
			"FP_ROUTE KSFO.<{\"id\": \"Gate_F_086\"}, {\"id\": \"Ramp_06_011\"}, {\"id\": \"Txy_A_B1\"}, {\"id\": \"Txy_A_016\"}, {\"id\": \"Txy_A_015\"}, {\"id\": \"Txy_A_012\"}, {\"id\": \"Txy_A_011\"}, {\"id\": \"Txy_A_D\"}, {\"id\": \"Txy_A_010\"}, {\"id\": \"Txy_A_E\"}, {\"id\": \"Txy_A_009\"}, {\"id\": \"Txy_A_F1\"}, {\"id\": \"Txy_A_008\"}, {\"id\": \"Txy_A_G\"}, {\"id\": \"Txy_A_007\"}, {\"id\": \"Txy_A_006\"}, {\"id\": \"Txy_A_005\"}, {\"id\": \"Txy_A_H\"}, {\"id\": \"Txy_A_004\"}, {\"id\": \"Txy_A_M\"}, {\"id\": \"Txy_A_003\"}, {\"id\": \"Txy_A_002\"}, {\"id\": \"Txy_A_A1\"}, {\"id\": \"Txy_A_001\"}, {\"id\": \"Rwy_02_001\"}, {\"id\": \"Rwy_02_002\"}>.RW01R.SSTIK3.LOSHN..BOILE..BLH.HYDRR1.I07R.RW07R.<{\"id\": \"Rwy_03_009\"}, {\"id\": \"Txy_G7_001\"}, {\"id\": \"Txy_F_009\"}, {\"id\": \"Txy_F_010\"}, {\"id\": \"Txy_F_011\"}, {\"id\": \"Txy_F_012\"}, {\"id\": \"Txy_F_013\"}, {\"id\": \"Txy_F13_001\"}, {\"id\": \"Rwy_02_018\"}, {\"id\": \"Txy_E13_001\"}, {\"id\": \"Txy_E_022\"}, {\"id\": \"Txy_D13_001\"}, {\"id\": \"Ramp_10_005\"}, {\"id\": \"Ramp_10_006\"} {\"id\": \"Gate_04_C16\"}>.KPHX",
			33000);








	propagate_flights(11000, 30);

	destroy_aircraft();
	destroy_rap();

	tg_shutdown();
}

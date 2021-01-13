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
#include "util_time.h"

#include <cmath>

#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include "../../libgpu/src/gpuSimulation.h.bak.20200717"

using namespace std;

static double delta_timeval(struct timeval& tvend, struct timeval& tvstart) {
	double dsec = tvend.tv_sec - tvstart.tv_sec;
	double dusec = tvend.tv_usec - tvstart.tv_usec;
	return (dsec + dusec / 1000000.); // seconds
}

//void switchAddr(aircraft_t* x, aircraft_t* y) {
//	X
//}

int main(int argc, char* argv[]) {
	aircraft_t hoa;
	aircraft_t doa;
	aircraft_t* hoa_ptr;
	aircraft_t* doa_ptr;
	aircraft_t* tmp_ptr;
	printf("addr of hoa = %d, addr of doa = %d\n", &hoa, &doa);

	//hoa_ptr = &hoa;
	//doa_ptr = &doa;
	tmp_ptr = &hoa;
	*(&hoa) = *(&doa);
	*(&doa) = *tmp_ptr;
	printf("(After) addr of hoa = %d, addr of doa = %d\n", &hoa, &doa);

	struct tm* tm_utc = getUTCTime();
	time_t utctime = mktime(tm_utc);
	printf("utctime: %X,  %ld\n", utctime, (long)utctime);
	printf("time and date: %s\n", asctime(tm_utc));

	usleep(1000000);

	tm_utc = getUTCTime();
	utctime = mktime(tm_utc);
	printf("(2) utctime: %X,  %ld\n", utctime, (long)utctime);



	printf("(3) currentCpuTime_milliSec: %ld\n", getCurrentCpuTime_milliSec());

	struct timeval tv;
	    struct timezone tz;
	    gettimeofday(&tv, &tz);
	time_t test_time_t = tv.tv_sec;
	printf("(4) utctime: %X,  %ld\n", test_time_t, (long)test_time_t);


// test ===================================











	double cur_cycle_execution_period = 0.0;
	double accu_cycle_execution_period = 0.0;

	int num_loops = 1;

	int count = 0;

	tg_init("share", 0, 0);

	while (count < num_loops) {
		struct timeval tv_start, tv_end;
		count++;
printf("*********************************************************************\n");
printf("Loading wind and aircraft --> count = %d *****************************\n", count);
printf("*********************************************************************\n");

		load_rap("share/tg/rap");

	    //load_aircraft("share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate.trx", "share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx");
	    load_aircraft("share/tg/trx/TRX_DEMO_beta1.0_100rec.trx", "share/tg/trx/TRX_DEMO_beta1.0_100rec_mfl.trx");

		propagate_flights(3600, 30);

		// ======================================

		gettimeofday(&tv_start, NULL);

		nats_simulation_operator(NATS_SIMULATION_STATUS_START);

		while (true) {
		    int server_runtime_sim_status = get_runtime_sim_status();
		    if (server_runtime_sim_status == NATS_SIMULATION_STATUS_ENDED) {
		        break;
			} else {
		        usleep(1000);
			}
		}

		gettimeofday(&tv_end, NULL);

		// ======================================

/*
		ostringstream tmpSS;
		tmpSS << count;

		char* strInt;
		string filename = "AAA_trajectory_";
		filename.append(tmpSS.str().c_str());
		filename.append(".csv");

		tg_write_trajectories(filename.c_str() , g_trajectories);
*/

//printf("test_monte_carlo --> main() --> About to destroy_aircraft()\n");
		destroy_aircraft();
//printf("test_monte_carlo --> main() --> About to destroy_rap()\n");
		destroy_rap();
//printf("test_monte_carlo --> main() ending\n");

		cur_cycle_execution_period = delta_timeval(tv_end, tv_start);
		accu_cycle_execution_period += cur_cycle_execution_period;
	} // end - while








//	while (count < num_loops) {
//		struct timeval tv_start, tv_end;
//		count++;
//printf("*********************************************************************\n");
//printf("Loading wind and aircraft --> count = %d *****************************\n", count);
//printf("*********************************************************************\n");
//
//		// ======================================
//
//double tmpLat = 35.0003357;
//double tmpLon = -115.1521911621;
//
//double tmpLat_rad = 0.610871;
//double tmpLon_rad = -2.009785;
//
////printf("lat rad = %f\n", (tmpLat * M_PI / 180.));
////printf("lon rad = %f\n", (tmpLon * M_PI / 180.));
//
//		gettimeofday(&tv_start, NULL);
//
//double value = cos(tmpLat_rad) + sin(tmpLon_rad);
////double value = cos(tmpLat * M_PI / 180.) + sin(tmpLon * M_PI / 180.);
//
//		gettimeofday(&tv_end, NULL);
//
//		// ======================================
//
//
//
//
//
//		cur_cycle_execution_period = delta_timeval(tv_end, tv_start);
//		accu_cycle_execution_period += cur_cycle_execution_period;
//	} // end - while

	tg_shutdown();

	printf("\n");
	printf("Number of loops = %d\n", num_loops);
	printf("Accumulated execution period = %f\n", accu_cycle_execution_period);
}

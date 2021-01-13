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

#include "rg_api.h"
#include "rg_exec.h"

#include "tg_api.h"

#include "tg_rap.h"

#include "tg_simulation.h"
#include "util_time.h"

#include "Fix.h"

#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>

using namespace std;
using namespace osi;

static double delta_timeval(struct timeval& tvend, struct timeval& tvstart) {
	double dsec = tvend.tv_sec - tvstart.tv_sec;
	double dusec = tvend.tv_usec - tvstart.tv_usec;
	return (dsec + dusec / 1000000.); // seconds
}

int main(int argc, char* argv[]) {
	ResultTrajs t_results;
	vector<string> routes;
	vector<double> costs;
	vector< vector<double> > latitudes;
	vector< vector<double> > longitudes;
	vector< vector<double> > polygonLatitudes;
	vector< vector<double> > polygonLongitudes;

/*	runRgOptimized_Orig("KSFO",
			//"BOILE",
			//"LOSHN",
			"KPHX",
			"HYDRR",
            35000,
            370,
			//"share/tg/rap/rap.t21z.awp130pgrbf00.h5",
			//"share/rg/nas/CIFP_201809",
            "share/rg/polygons/MACS_scenario.dat",
            "NONE", //sigmetFile,
            "share/tg/weather/20180511_161413.airep",
			"share/rg/config/airport.config", //airportConfigFile,
			"share/rg/config/airway.config", //airwayConfigFile,
			"share/rg/config/enrtfix.config", //fixConfigFile,
			"share/rg/config/Proc.config", //procConfigFile,
            true, //rerouteFlag,
            true, //windOptimalFlag,
            true, //filterAirports,
            3, //numOpts,
            0, //costIndex,
            t_results,
            routes,
            costs,
            latitudes,
            longitudes,
            polygonLatitudes,
            polygonLongitudes);*/






	load_rg_data_files_NATS("share/tg/rap", //"share/tg/rap/rap.t21z.awp130pgrbf00.h5",
			"share/rg/nas/CIFP_201809/FAACIFP18",

			//"share/rg/polygons/MACS_scenario.dat",
			//"NONE", //"share/tg/weather/20180511_161413.sigmet",
			//"NONE", //"share/tg/weather/20180511_161413.airep",

			"share/rg/config/airport.config",
			"share/rg/config/airway.config",
			"share/rg/config/enrtfix.config");




// Test.  Oliver Chen  2018.12.04
waypoint_node_t* test_new_waypoint_node_ptr;

test_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
test_new_waypoint_node_ptr->wpname = "KAYEX";
test_new_waypoint_node_ptr->proctype = "SID";
test_new_waypoint_node_ptr->alt_1 = -10000.00;
test_new_waypoint_node_ptr->alt_2 = -10000.00;

waypoint_node_t* test_root_waypoint_node_ptr = test_new_waypoint_node_ptr;
waypoint_node_t* test_waypoint_node_ptr = test_new_waypoint_node_ptr;

test_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
test_new_waypoint_node_ptr->wpname = "LOSHN";
test_new_waypoint_node_ptr->proctype = "ENROUTE";
test_new_waypoint_node_ptr->alt_1 = -10000.00;
test_new_waypoint_node_ptr->alt_2 = -10000.00;

	test_waypoint_node_ptr->next_node_ptr = test_new_waypoint_node_ptr;
	test_new_waypoint_node_ptr->prev_node_ptr = test_waypoint_node_ptr;
	test_waypoint_node_ptr = test_new_waypoint_node_ptr;

test_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
test_new_waypoint_node_ptr->wpname = "BOILE";
test_new_waypoint_node_ptr->proctype = "ENROUTE";
test_new_waypoint_node_ptr->alt_1 = -10000.00;
test_new_waypoint_node_ptr->alt_2 = -10000.00;

	test_waypoint_node_ptr->next_node_ptr = test_new_waypoint_node_ptr;
	test_new_waypoint_node_ptr->prev_node_ptr = test_waypoint_node_ptr;
	test_waypoint_node_ptr = test_new_waypoint_node_ptr;

test_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
test_new_waypoint_node_ptr->wpname = "BLH";
test_new_waypoint_node_ptr->proctype = "ENROUTE";
test_new_waypoint_node_ptr->alt_1 = -10000.00;
test_new_waypoint_node_ptr->alt_2 = -10000.00;

	test_waypoint_node_ptr->next_node_ptr = test_new_waypoint_node_ptr;
	test_new_waypoint_node_ptr->prev_node_ptr = test_waypoint_node_ptr;
	test_waypoint_node_ptr = test_new_waypoint_node_ptr;

test_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
test_new_waypoint_node_ptr->wpname = "TOP_OF_DESCENT_PT";
test_new_waypoint_node_ptr->proctype = "";
test_new_waypoint_node_ptr->alt_1 = -10000.00;
test_new_waypoint_node_ptr->alt_2 = -10000.00;

	test_waypoint_node_ptr->next_node_ptr = test_new_waypoint_node_ptr;
	test_new_waypoint_node_ptr->prev_node_ptr = test_waypoint_node_ptr;
	test_waypoint_node_ptr = test_new_waypoint_node_ptr;

// **********
	supplement_airways_by_flightPlans(test_root_waypoint_node_ptr);
// end - Test.  Oliver Chen  2018.12.04




//	load_rg_weather_files_NATS("share/tg/rap/rap.t21z.awp130pgrbf00.h5",
//			g_cifp_file,
//
//			"share/rg/polygons/MACS_scenario.dat",
//			//"NONE",
//			"share/tg/weather/20180511_161413.sigmet",
//
//			//"NONE"
//			"share/tg/weather/20180511_161413.airep"
//			);




	runRgOptimized_NATS("TEST123",
			"KSFO",
			//"LOSHN",
			"KPHX",
			"NONE",
            33000,
            430,
			//"share/tg/rap/rap.t21z.awp130pgrbf00.h5",
			//"share/rg/nas/CIFP_201809",

            "share/rg/polygons/MACS_scenario.dat",
            "share/tg/weather/20180511_161413.sigmet",
            "share/tg/weather/20180511_161413.airep",

			//"share/rg/config/airport.config", //airportConfigFile,
			//"share/rg/config/airway.config", //airwayConfigFile,
			"share/rg/config/enrtfix.config", //fixConfigFile,
			"share/rg/config/Proc.config", //procConfigFile,

            true, //rerouteFlag,
            true, //windOptimalFlag,
            true, //filterAirports,
            3, //numOpts,
            0, //costIndex,
            t_results,
            routes,
            costs,
            latitudes,
            longitudes,
            polygonLatitudes,
            polygonLongitudes);



cout << endl;
    cout << "(After) routes=" << endl;
    for(unsigned int i=0; i<routes.size(); ++i) {
        cout << "(" << i << ") " << routes.at(i) << endl << endl;
    }





}

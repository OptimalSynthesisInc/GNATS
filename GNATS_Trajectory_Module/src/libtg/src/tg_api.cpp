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
 * tg_api.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#include "AirportLayoutDataLoader.h"

#include "tg_api.h"
#include "tg_adb.h"
#include "tg_airports.h"
#include "tg_waypoints.h"
#include "tg_airways.h"
#include "tg_pars.h"
#include "tg_sidstars.h"
#include "tg_sectors.h"
#include "tg_rap.h"
#include "tg_aircraft.h"
#include "tg_simulation.h"

#include "cuda_compat.h"

#include "geometry_utils.h"

#include "hdf5.h"
#include "pub_logger.h"
#include "util_string.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <omp.h>
#include <zlib.h>

#include <set>
#include <sstream>
#include <string>
#include <cstdio>
#include <dirent.h>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <vector>

#include "util_time.h"

#define GEN_RTG_MAP 0


using namespace std;
using namespace rapidxml;
using namespace osi;

/*
 * host-side globals declared in tg_api.h
 */
char* GNATS_HOME;
char* GNATS_SERVER_HOME;
string g_data_dir = "../share";
string g_trx_file = "";
string g_mfl_file = "";
string g_cifp_file = "";
string g_wind_dir = "";
string g_out_file = "tg_out.h5";
string g_share_dir = "share";
real_t g_perturbation = 0;
long g_horizon = 24*3600; // 24hrs converted to seconds
long g_step_size = 1;
long g_step_size_airborne = 30;
int g_device_id = 0;
bool g_verbose = true;
bool g_output_kml = false;
long g_start_time = 0;
long g_start_time_realTime_simulation = 0;
bool g_flag_standalone_mode = false;
bool g_flag_gdb = false;

map<string, NatsAirport*> map_airport;
map<string, FlightPlan*> map_flightplan;

static const int MAX_SECTOR_NAME_LEN = 12;

/*
 * private helper structs for saving/loading trajectories to HDF5
 */
typedef struct _native_trajectory_t {
	int     flight_index;
	char*   callsign;
	char*   actype;
    char*   origin_airport;
    char*   destination_airport;
	long    start_time;
	int     interval_ground;
	int     interval_airborne;
	float   cruise_altitude_ft;
	float   cruise_tas_knots;
	float   origin_airport_elevation_ft;
	float   destination_airport_elevation_ft;
	bool    flag_externalAircraft;

	long*   timestamp;
	double* latitude;
	double* longitude;
	double* altitude;
	double* hdot;
	double* tas;
	double* tas_ground;
	double* course;
	double* fpa;

	int*    phase;
} native_trajectory_t;

typedef struct _hdf5_trajectory_t {
	int     flight_index;
	char*   callsign;
	char*   actype;
	char*   origin_airport;
	char*   destination_airport;
	long    start_time;
	int     interval_ground;
	int     interval_airborne;
	float   cruise_altitude_ft;
	float   cruise_tas_knots;
	float   origin_airport_elevation_ft;
	float   destination_airport_elevation_ft;
	bool    flag_externalAircraft;

	hvl_t   timestamp;
	hvl_t   latitude;
	hvl_t   longitude;
	hvl_t   altitude;
	hvl_t   hdot;
	hvl_t   tas;
	hvl_t   tas_ground;
	hvl_t   course;
	hvl_t   fpa;

	hvl_t   phase;
} hdf5_trajectory_t;

/*
 * private file-scope variables
 */


/*
 * private file-scope prototypes
 */
static int do_init(adb_ptf_column_e col);


int tg_info() {
	printf("    ADB data: %s\n", (flag_adb_available ? "Yes" : "No"));

	printf("    RAP data: %s\n", (flag_rap_available ? "Yes" : "No"));
	printf("    Waypoint data: %s\n", (flag_waypoint_available ? "Yes" : "No"));
	printf("    Airport data: %s\n", (flag_airport_available ? "Yes" : "No"));
	printf("    Airway data: %s\n", (flag_airway_available ? "Yes" : "No"));
	printf("    SID/STAR/Approach data: %s\n", (flag_SidStarApp_available ? "Yes" : "No"));

	return 0;
}


/**
 * Create a directory and the required parent directories.
 * Similar to bash mkdir -p
 */
static bool mkpath( std::string path ) {
    bool bSuccess = false;
#ifndef _INC__MINGW_H
    int nRC = ::mkdir( path.c_str(), 0775 );
#else
    int nRC = ::mkdir( path.c_str() );
#endif
    if( nRC == -1 )
    {
        switch( errno )
        {
            case ENOENT:
                //parent didn't exist, try to create it
                if( mkpath( path.substr(0, path.find_last_of('/')) ) )
                    //Now, try to create again.
#ifndef _INC__MINGW_H
                    bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
#else
                	bSuccess = 0 == ::mkdir( path.c_str() );
#endif
                else
                    bSuccess = false;
                break;
            case EEXIST:
                //Done!
                bSuccess = true;
                break;
            default:
                bSuccess = false;
                break;
        }
    }
    else
        bSuccess = true;
    return bSuccess;
}

string getCIFPPath(const string& data_dir) {
	string retString;

	struct dirent *dirEnt;
	vector<string> vector_DirectoryName;

	string tmpDir = data_dir;
	tmpDir.append("/rg/nas");

	DIR *dir = opendir(tmpDir.c_str());

	if (dir == NULL) {
		printf("Could not open directory %s", tmpDir.c_str());
		return "";
	}

	vector_DirectoryName.clear();

	while ((dirEnt = readdir(dir)) != NULL) {
		struct stat path_stat;
		char* dirEnt_name = dirEnt->d_name;

		if ((strcmp(dirEnt_name, ".") == 0) || (strcmp(dirEnt_name, "..") == 0)) {
			continue;
		}

		string curPath(data_dir);
		curPath.append("/rg/nas");
		curPath.append("/");
		curPath.append(dirEnt_name);

		if (stat(curPath.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
			if (indexOf(dirEnt_name, "CIFP_") == 0) {
				vector_DirectoryName.push_back(string(dirEnt_name));
			}
		}
	}

	if (vector_DirectoryName.size() > 0) {
		std::sort(vector_DirectoryName.begin(), vector_DirectoryName.end());

		tmpDir.append("/");
		tmpDir.append(vector_DirectoryName.back().c_str());

		retString.assign(tmpDir);
	}

	closedir(dir);

	return retString;
}

/*
 * Initialize the trajectory generator with the data locations and
 * speed perturbation amount (default 0).
 */
int tg_init(const string& data_dir,
		const real_t& cruise_tas_perturbation,
		const int& device_id) {
	setbuf(stdout, NULL);

	logger_printLog(LOG_LEVEL_INFO, 0, "LOG_LEVEL_INFO enabled\n\n");
	logger_printLog(LOG_LEVEL_DEBUG, 0, "LOG_LEVEL_DEBUG enabled\n\n");

	GNATS_HOME = getenv("GNATS_HOME");

	GNATS_SERVER_HOME = getenv("GNATS_SERVER_HOME");

	if ((GNATS_SERVER_HOME == NULL) || (strlen(GNATS_SERVER_HOME) == 0)) {
		// No GNATS_SERVER_HOME defined
		// If the runtime is in Standalone mode, we use the current path as GNATS_SERVER_HOME
		if (g_flag_standalone_mode) {
			char cwd[PATH_MAX];
			if (getcwd(cwd, sizeof(cwd)) != NULL) {
				GNATS_SERVER_HOME = (char*)calloc(strlen(cwd)+1, sizeof(char));
				strcpy(GNATS_SERVER_HOME, cwd);
				GNATS_SERVER_HOME[strlen(cwd)] = '\0';
			}
		} else {
			printf("System environment GNATS_SERVER_HOME does not exist.\n");
		}
	}

#if USE_GPU
    cudaSetDevice(device_id);
#else
    (void)device_id;
#endif

	g_data_dir = data_dir;

	adb_ptf_column_e col;
	if(cruise_tas_perturbation < 0) col = LO;
	else if(cruise_tas_perturbation > 0) col = HI;
	else col = NOM;

	int err = do_init(col);

	return err;
}

int tg_load_rap(const string& grib_file) {

	// load rap data (host and device)
	return load_rap(grib_file);
}

int tg_load_trx(const string& trx_file, const string& mfl_file) {
	// use libtrx to load the trx and mfl files into aircraft objects
	// expand the flight plans
	load_aircraft(trx_file, mfl_file);
	return 0;
}

int tg_generate(const long& t_horizon_minutes,
		        const long& t_step_sec,
		        vector<Trajectory>* const trajectories) {
	int err = propagate_flights(t_horizon_minutes, t_step_sec);
	if(!err && trajectories) {
		std::copy(g_trajectories.begin(), g_trajectories.end(),
				  trajectories->end());
	}
	return err;
}

int tg_reset_aircraft() {
	return 0;
}

int tg_reset() {
	return 0;
}

int tg_shutdown() {
	// destroy device memory and heap-allocated host memory
	// only adb, sectors, and aircraft have device memory.
	// only adb, sectors, aircraft, and ruc have heap-allocated host memory
	// all others are stack allocated and don't need explicit memory freeing

	destroy_adb_performance_tables();
	destroy_sectors();
	destroy_rap();
	destroy_aircraft();

	g_airports.clear();
	g_airways.clear();

	g_pars.clear();
	g_sids.clear();
	g_stars.clear();
	g_approaches.clear();
	g_waypoints.clear();

	g_trajectories.clear();
	g_flightplans.clear();

	if (map_ground_waypoint_connectivity.size() > 0) {
		map<string, GroundWaypointConnectivity>::iterator ite;
		for (ite = map_ground_waypoint_connectivity.begin(); ite != map_ground_waypoint_connectivity.end(); ite++) {
			GroundWaypointConnectivity tmpGroundWaypointConnectivity = ite->second;

			if (tmpGroundWaypointConnectivity.connectivity != NULL) {
				for (int i = 0; i < tmpGroundWaypointConnectivity.map_waypoint_node.size(); i++) {
					if (tmpGroundWaypointConnectivity.connectivity[i] != NULL) {
						free(tmpGroundWaypointConnectivity.connectivity[i]);
						tmpGroundWaypointConnectivity.connectivity[i] = NULL;
					}
				}

				if (tmpGroundWaypointConnectivity.connectivity != NULL) {
					free(tmpGroundWaypointConnectivity.connectivity);
					tmpGroundWaypointConnectivity.connectivity = NULL;
				}
			}

			if (tmpGroundWaypointConnectivity.path_costs != NULL) {
				for (int i = 0; i < tmpGroundWaypointConnectivity.map_waypoint_node.size(); i++) {
					if (tmpGroundWaypointConnectivity.path_costs[i] != NULL) {
						free(tmpGroundWaypointConnectivity.path_costs[i]);
						tmpGroundWaypointConnectivity.path_costs[i] = NULL;
					}
				}

				if (tmpGroundWaypointConnectivity.path_costs != NULL) {
					free(tmpGroundWaypointConnectivity.path_costs);
					tmpGroundWaypointConnectivity.path_costs = NULL;
				}
			}
		}

		map_ground_waypoint_connectivity.clear();
	}

	if (GNATS_HOME != NULL)
		free(GNATS_HOME);

	if (GNATS_SERVER_HOME != NULL)
		free(GNATS_SERVER_HOME);

#if USE_GPU
    cudaDeviceReset();
#endif
	return 0;
}

int tg_get_trajectories(vector<Trajectory>* const trajectories) {
	if(!trajectories) return -1;
	trajectories->insert(trajectories->end(), g_trajectories.begin(),
			             g_trajectories.end());
	return 0;
}

static int tg_write_trajectories_h5(const string& fname, const vector<Trajectory>& trajectories) {

    // rank (num dims) of the array of output trajectories: 1
    int rank = 1;

    // dimension of the array of output trajectories: num_flights
    hsize_t dims[1] = {trajectories.size()};

    // create the array of hdf5 trajectory structs
    native_trajectory_t* native_trajectories =
            (native_trajectory_t*)calloc(dims[0], sizeof(native_trajectory_t));
    hdf5_trajectory_t* hdf5_trajectories =
            (hdf5_trajectory_t*)calloc(dims[0], sizeof(hdf5_trajectory_t));

    // create the array of native trajectory structs, possibly converting
    // from float to double.
    for (hsize_t i=0; i<dims[0]; ++i) {
        size_t traj_len = trajectories.at(i).latitude_deg.size();

        native_trajectories[i].flight_index = trajectories.at(i).flight_index;
        native_trajectories[i].callsign = const_cast<char*>(trajectories.at(i).callsign.c_str());
        native_trajectories[i].actype = const_cast<char*>(trajectories.at(i).actype.c_str());
        native_trajectories[i].origin_airport = const_cast<char*>(trajectories.at(i).origin_airport.c_str());
        native_trajectories[i].destination_airport = const_cast<char*>(trajectories.at(i).destination_airport.c_str());
        native_trajectories[i].start_time = trajectories.at(i).start_time;
        native_trajectories[i].interval_ground = trajectories.at(i).interval_ground;
        native_trajectories[i].interval_airborne = trajectories.at(i).interval_airborne;
        native_trajectories[i].cruise_altitude_ft = trajectories.at(i).cruise_altitude_ft;
        native_trajectories[i].cruise_tas_knots = trajectories.at(i).cruise_tas_knots;
        native_trajectories[i].origin_airport_elevation_ft = trajectories.at(i).origin_airport_elevation_ft;
        native_trajectories[i].destination_airport_elevation_ft = trajectories.at(i).destination_airport_elevation_ft;

        native_trajectories[i].timestamp = (long*)calloc(traj_len, sizeof(long));
        native_trajectories[i].latitude = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].longitude = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].altitude = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].hdot = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].tas = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].tas_ground = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].course = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].fpa = (double*)calloc(traj_len, sizeof(double));
        native_trajectories[i].phase = (int*)calloc(traj_len, sizeof(int));

        // we must do an element-wise copy since data-types may not be the
        // same size.  the output structs use double, but real_t may be float.
        for (size_t j=0; j<traj_len; ++j) {
            native_trajectories[i].timestamp[j] = trajectories.at(i).timestamp[j];
            native_trajectories[i].latitude[j] = trajectories.at(i).latitude_deg[j];
            native_trajectories[i].longitude[j] = trajectories.at(i).longitude_deg[j];
            native_trajectories[i].altitude[j] = trajectories.at(i).altitude_ft[j];
            native_trajectories[i].hdot[j] = trajectories.at(i).rocd_fps[j];
            native_trajectories[i].tas[j] = trajectories.at(i).tas_knots[j];
            native_trajectories[i].tas_ground[j] = trajectories.at(i).tas_knots_ground[j];
            native_trajectories[i].course[j] = trajectories.at(i).course_deg[j];
            native_trajectories[i].fpa[j] = trajectories.at(i).fpa_deg[j];

            native_trajectories[i].phase[j] = trajectories.at(i).flight_phase[j];
        }

        // Copy to the hdf5 struct
        hdf5_trajectories[i].flight_index = native_trajectories[i].flight_index;
        hdf5_trajectories[i].callsign = native_trajectories[i].callsign;
        hdf5_trajectories[i].actype = native_trajectories[i].actype;
        hdf5_trajectories[i].origin_airport = native_trajectories[i].origin_airport;
        hdf5_trajectories[i].destination_airport = native_trajectories[i].destination_airport;
        hdf5_trajectories[i].start_time = native_trajectories[i].start_time;
        hdf5_trajectories[i].interval_ground = native_trajectories[i].interval_ground;
        hdf5_trajectories[i].interval_airborne = native_trajectories[i].interval_airborne;
        hdf5_trajectories[i].cruise_altitude_ft = native_trajectories[i].cruise_altitude_ft;
        hdf5_trajectories[i].cruise_tas_knots = native_trajectories[i].cruise_tas_knots;
        hdf5_trajectories[i].origin_airport_elevation_ft = native_trajectories[i].origin_airport_elevation_ft;
        hdf5_trajectories[i].destination_airport_elevation_ft = native_trajectories[i].destination_airport_elevation_ft;

        hdf5_trajectories[i].timestamp.len = traj_len;
        hdf5_trajectories[i].timestamp.p = native_trajectories[i].timestamp;
        hdf5_trajectories[i].latitude.len = traj_len;
        hdf5_trajectories[i].latitude.p = native_trajectories[i].latitude;
        hdf5_trajectories[i].longitude.len = traj_len;
        hdf5_trajectories[i].longitude.p = native_trajectories[i].longitude;
        hdf5_trajectories[i].altitude.len = traj_len;
        hdf5_trajectories[i].altitude.p = native_trajectories[i].altitude;
        hdf5_trajectories[i].hdot.len = traj_len;
        hdf5_trajectories[i].hdot.p = native_trajectories[i].hdot;
        hdf5_trajectories[i].tas.len = traj_len;
        hdf5_trajectories[i].tas.p = native_trajectories[i].tas;
        hdf5_trajectories[i].tas_ground.len = traj_len;
        hdf5_trajectories[i].tas_ground.p = native_trajectories[i].tas_ground;
        hdf5_trajectories[i].course.len = traj_len;
        hdf5_trajectories[i].course.p = native_trajectories[i].course;
        hdf5_trajectories[i].fpa.len = traj_len;
        hdf5_trajectories[i].fpa.p = native_trajectories[i].fpa;

        hdf5_trajectories[i].phase.len = traj_len;
        hdf5_trajectories[i].phase.p = native_trajectories[i].phase;
    }

    // define the variable-length double array type
    hid_t mem_double_array_type = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    hid_t mem_int_array_type = H5Tvlen_create(H5T_NATIVE_INT);
    hid_t mem_long_array_type = H5Tvlen_create(H5T_NATIVE_LONG);

    hid_t file_double_array_type = H5Tvlen_create(H5T_IEEE_F64LE);
    hid_t file_int_array_type = H5Tvlen_create(H5T_STD_I32LE);
    hid_t file_long_array_type = H5Tvlen_create(H5T_STD_I64LE);
    hid_t string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string_type, H5T_VARIABLE);

    int offset = 0;

    // define the compound data type for memory
    hid_t mem_traj_type = H5Tcreate(H5T_COMPOUND, sizeof(hdf5_trajectory_t));
    H5Tinsert(mem_traj_type, "flight_index", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, callsign);
    H5Tinsert(mem_traj_type, "callsign", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, actype);
    H5Tinsert(mem_traj_type, "actype", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, origin_airport);
    H5Tinsert(mem_traj_type, "origin_airport", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, destination_airport);
    H5Tinsert(mem_traj_type, "destination_airport", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, start_time);
    H5Tinsert(mem_traj_type, "start_time", offset, H5T_NATIVE_LONG);
    offset = HOFFSET(hdf5_trajectory_t, interval_ground);
    H5Tinsert(mem_traj_type, "interval_ground", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, interval_airborne);
    H5Tinsert(mem_traj_type, "interval_airborne", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, cruise_altitude_ft);
    H5Tinsert(mem_traj_type, "cruise_altitude_ft", offset, H5T_NATIVE_FLOAT);
    offset = HOFFSET(hdf5_trajectory_t, cruise_tas_knots);
    H5Tinsert(mem_traj_type, "cruise_tas_knots", offset, H5T_NATIVE_FLOAT);
    offset = HOFFSET(hdf5_trajectory_t, origin_airport_elevation_ft);
    H5Tinsert(mem_traj_type, "origin_airport_elevation_ft", offset, H5T_NATIVE_FLOAT);
    offset = HOFFSET(hdf5_trajectory_t, destination_airport_elevation_ft);
    H5Tinsert(mem_traj_type, "destination_airport_elevation_ft", offset, H5T_NATIVE_FLOAT);

    offset = HOFFSET(hdf5_trajectory_t, timestamp);
    H5Tinsert(mem_traj_type, "timestamp", offset, mem_long_array_type);
    offset = HOFFSET(hdf5_trajectory_t, latitude);
    H5Tinsert(mem_traj_type, "latitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, longitude);
    H5Tinsert(mem_traj_type, "longitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, altitude);
    H5Tinsert(mem_traj_type, "altitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, hdot);
    H5Tinsert(mem_traj_type, "hdot", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, tas);
    H5Tinsert(mem_traj_type, "tas", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, tas_ground);
    H5Tinsert(mem_traj_type, "tas_ground", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, course);
    H5Tinsert(mem_traj_type, "course", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, fpa);
    H5Tinsert(mem_traj_type, "fpa", offset, mem_double_array_type);
    //offset = HOFFSET(hdf5_trajectory_t, sector_index);
    //H5Tinsert(mem_traj_type, "sector_index", offset, mem_int_array_type);
    //offset = HOFFSET(hdf5_trajectory_t, sector_name);
    //H5Tinsert(mem_traj_type, "sector_name", offset, mem_str_array_type);
    offset = HOFFSET(hdf5_trajectory_t, phase);
    H5Tinsert(mem_traj_type, "phase", offset, mem_int_array_type);

    // Compute the size of the compound type
    // Notice
    // The number of elements in the calculation must match the number of the fields in the following code statements about "H5Tinsert"
    hsize_t file_compound_size = H5Tget_size(H5T_STD_I32LE) +
                                 4*H5Tget_size(string_type) +
                                 H5Tget_size(H5T_STD_I64LE) +
                                 2*H5Tget_size(H5T_STD_I32LE) +
								 4*H5Tget_size(H5T_IEEE_F32LE) +
								 H5Tget_size(file_long_array_type) +
                                 8*H5Tget_size(file_double_array_type) +
                                 H5Tget_size(file_int_array_type);

    // define the compound data type for the file.  we manually calculate
    // the offsets of each member because the standard types we use for the
    // file may have different sizes than the corresponding native types.
    // we use 64-bit big-endian for int and double types
    hid_t file_traj_type = H5Tcreate(H5T_COMPOUND, file_compound_size);

    offset = 0;
    H5Tinsert(file_traj_type, "flight_index", offset, H5T_STD_I32LE);
    offset += H5Tget_size(H5T_STD_I32LE);
    H5Tinsert(file_traj_type, "callsign", offset, string_type);
    offset += H5Tget_size(string_type);
    H5Tinsert(file_traj_type, "actype", offset, string_type);
    offset += H5Tget_size(string_type);
    H5Tinsert(file_traj_type, "origin_airport", offset, string_type);
    offset += H5Tget_size(string_type);
    H5Tinsert(file_traj_type, "destination_airport", offset, string_type);
    offset += H5Tget_size(string_type);
    H5Tinsert(file_traj_type, "start_time", offset, H5T_STD_I64LE);
    offset += H5Tget_size(H5T_STD_I64LE);
    H5Tinsert(file_traj_type, "interval_ground", offset, H5T_STD_I32LE);
    offset += H5Tget_size(H5T_STD_I32LE);
    H5Tinsert(file_traj_type, "interval_airborne", offset, H5T_STD_I32LE);
    offset += H5Tget_size(H5T_STD_I32LE);
    H5Tinsert(file_traj_type, "cruise_altitude_ft", offset, H5T_IEEE_F32LE);
    offset += H5Tget_size(H5T_IEEE_F32LE);
    H5Tinsert(file_traj_type, "cruise_tas_knots", offset, H5T_IEEE_F32LE);
    offset += H5Tget_size(H5T_IEEE_F32LE);
    H5Tinsert(file_traj_type, "origin_airport_elevation_ft", offset, H5T_IEEE_F32LE);
    offset += H5Tget_size(H5T_IEEE_F32LE);
    H5Tinsert(file_traj_type, "destination_airport_elevation_ft", offset, H5T_IEEE_F32LE);

    offset += H5Tget_size(H5T_IEEE_F32LE);
    H5Tinsert(file_traj_type, "timestamp", offset, file_long_array_type);
    offset += H5Tget_size(file_long_array_type);
    H5Tinsert(file_traj_type, "latitude", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "longitude", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "altitude", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "hdot", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "tas", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "tas_ground", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "course", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "fpa", offset, file_double_array_type);
    offset += H5Tget_size(file_double_array_type);
    H5Tinsert(file_traj_type, "mode", offset, file_int_array_type);

    // create the file id
    hid_t file = H5Fcreate(fname.c_str(),
    		H5F_ACC_TRUNC,
            H5P_DEFAULT,
			H5P_DEFAULT);

    // create the dataspace.
    hid_t dataspace = H5Screate_simple(rank, dims, NULL);

    // create the dataset and write the compound data to it
    hid_t dataset = H5Dcreate2(file, "/trajectories",
    		file_traj_type,
			dataspace,
		    H5P_DEFAULT,
		    H5P_DEFAULT,
		    H5P_DEFAULT);
    H5Dwrite(dataset, mem_traj_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             hdf5_trajectories);

    // close hdf5 resources
    H5Dclose(dataset);
    H5Sclose(dataspace);
    H5Tclose(string_type);
    H5Tclose(mem_double_array_type);
    H5Tclose(mem_int_array_type);
    H5Tclose(mem_long_array_type);

    H5Tclose(file_double_array_type);
    H5Tclose(file_int_array_type);
    H5Tclose(file_long_array_type);

    H5Tclose(file_traj_type);
    H5Tclose(mem_traj_type);
    H5Fclose(file);

    // free the hdf5 trajectory structs
    for (hsize_t i=0; i<dims[0]; ++i) {
    	free(native_trajectories[i].timestamp);
        free(native_trajectories[i].latitude);
        free(native_trajectories[i].longitude);
        free(native_trajectories[i].altitude);
        free(native_trajectories[i].hdot);
        free(native_trajectories[i].tas);
        free(native_trajectories[i].tas_ground);
        free(native_trajectories[i].course);
        free(native_trajectories[i].fpa);
        free(native_trajectories[i].phase);
    }

    free(hdf5_trajectories);

    return 0;
}

static int tg_write_trajectories_xml(xml_document<>& doc, const vector<Trajectory>& trajectories) {
    xml_node<>* trajectories_node = doc.allocate_node(node_element, "trajectories");

    stringstream tstart_ss;
    if (g_start_time_realTime_simulation == 0) {
		tstart_ss << g_start_time;
	} else {
		tstart_ss << g_start_time_realTime_simulation;
	}

    xml_attribute<>* simstart_attr = doc.allocate_attribute("simulation_start_time", doc.allocate_string(tstart_ss.str().c_str()));

    trajectories_node->append_attribute(simstart_attr);

    for (unsigned int i=0; i<trajectories.size(); ++i) {
        const Trajectory& t = trajectories.at(i);

        stringstream flight_index_ss;
        flight_index_ss << t.flight_index;

        stringstream start_time_ss;
        start_time_ss << t.start_time;

        stringstream interval_ground_ss;
        interval_ground_ss << t.interval_ground;

        stringstream interval_airborne_ss;
        interval_airborne_ss << t.interval_airborne;

        xml_node<>* trajectory_node = doc.allocate_node(node_element, "trajectory");

        // Prepare profile data
        xml_attribute<>* index_attr = doc.allocate_attribute("flight_index", doc.allocate_string(flight_index_ss.str().c_str()));
        xml_attribute<>* callsign_attr = doc.allocate_attribute("callsign", doc.allocate_string(t.callsign.c_str()));
        xml_attribute<>* actype_attr = doc.allocate_attribute("actype", doc.allocate_string(t.actype.c_str()));
        xml_attribute<>* origin_attr = doc.allocate_attribute("origin_airport", doc.allocate_string(t.origin_airport.c_str()));
        xml_attribute<>* destination_attr = doc.allocate_attribute("destination_airport", doc.allocate_string(t.destination_airport.c_str()));
        xml_attribute<>* start_time_attr = doc.allocate_attribute("start_time", doc.allocate_string(start_time_ss.str().c_str()));
        xml_attribute<>* interval_ground_attr = doc.allocate_attribute("interval_ground", doc.allocate_string(interval_ground_ss.str().c_str()));
        xml_attribute<>* interval_airborne_attr = doc.allocate_attribute("interval_airborne", doc.allocate_string(interval_airborne_ss.str().c_str()));

        trajectory_node->append_attribute(index_attr);
        trajectory_node->append_attribute(callsign_attr);
        trajectory_node->append_attribute(actype_attr);
        trajectory_node->append_attribute(origin_attr);
        trajectory_node->append_attribute(destination_attr);
        trajectory_node->append_attribute(start_time_attr);
        trajectory_node->append_attribute(interval_ground_attr);
        trajectory_node->append_attribute(interval_airborne_attr);

        int n = t.latitude_deg.size();

        //long timestamp = t.start_time;
        long timestamp = 0;

        // Loop and retrieve flight state data at every timestamp
        for (int j=0; j<n; ++j) {
        	timestamp = t.timestamp.at(j);

            stringstream lat_ss;
            lat_ss << t.latitude_deg.at(j);

            stringstream lon_ss;
            lon_ss << t.longitude_deg.at(j);

            stringstream alt_ss;
            alt_ss << t.altitude_ft.at(j);

            stringstream rocd_ss;
            rocd_ss << t.rocd_fps.at(j);

            stringstream tas_ss;
            tas_ss << t.tas_knots.at(j);

            stringstream tas_ground_ss;
            tas_ground_ss << t.tas_knots_ground.at(j);

            stringstream course_ss;
            course_ss << t.course_deg.at(j);

            stringstream fpa_ss;
            fpa_ss << t.fpa_deg.at(j);

            const char* phase = ENUM_Flight_Phase_String[t.flight_phase.at(j)];

            stringstream timestamp_ss;
            timestamp_ss << timestamp;

            xml_node<>* point_node = doc.allocate_node(node_element, "trajectory_point");

            xml_attribute<>* t_attr = doc.allocate_attribute("timestamp", doc.allocate_string(timestamp_ss.str().c_str()));

            xml_node<>* lat_node = doc.allocate_node(node_element, "latitude", doc.allocate_string(lat_ss.str().c_str()));
            xml_node<>* lon_node = doc.allocate_node(node_element, "longitude", doc.allocate_string(lon_ss.str().c_str()));
            xml_node<>* alt_node = doc.allocate_node(node_element, "altitude_ft", doc.allocate_string(alt_ss.str().c_str()));
            xml_node<>* rocd_node = doc.allocate_node(node_element, "rocd_fps", doc.allocate_string(rocd_ss.str().c_str()));
            xml_node<>* tas_node = doc.allocate_node(node_element, "tas_knots", doc.allocate_string(tas_ss.str().c_str()));
            xml_node<>* tas_ground_node = doc.allocate_node(node_element, "tas_knots_ground", doc.allocate_string(tas_ground_ss.str().c_str()));
            xml_node<>* course_node = doc.allocate_node(node_element, "course", doc.allocate_string(course_ss.str().c_str()));
            xml_node<>* fpa_node = doc.allocate_node(node_element, "fpa", doc.allocate_string(fpa_ss.str().c_str()));
            xml_node<>* phase_node = doc.allocate_node(node_element, "flight_phase", doc.allocate_string(phase));

            point_node->append_attribute(t_attr);
            point_node->append_node(lat_node);
            point_node->append_node(lon_node);
            point_node->append_node(alt_node);
            point_node->append_node(rocd_node);
            point_node->append_node(tas_node);
            point_node->append_node(tas_ground_node);
            point_node->append_node(course_node);
            point_node->append_node(fpa_node);
            point_node->append_node(phase_node);

            trajectory_node->append_node(point_node);
        }

        trajectories_node->append_node(trajectory_node);
    }

    doc.append_node(trajectories_node);

    return 0;
}

static int tg_write_trajectories_xml(const string& fname, const vector<Trajectory>& trajectories) {


    // build the dom
    xml_document<> doc;
    tg_write_trajectories_xml(doc, trajectories);

    // write the dom to file
    ofstream out;
    out.open(fname.c_str());
    if(!out.is_open()) {
        cout << "ERROR: could not open the output file for writing (" << fname << ")" << endl;
        return -1;
    }
    out << doc;
    out.close();

    return 0;
}

static int tg_write_trajectories_xml_gz(const string& fname, const vector<Trajectory>& trajectories) {

    // build the dom
    xml_document<> doc;
    tg_write_trajectories_xml(doc, trajectories);

    stringstream ss;
    ss << doc;

    string xmlstr = ss.str();
    size_t sz = xmlstr.length()*sizeof(char);

    gzFile gzf = gzopen(fname.c_str(), "wb");
    int err = gzwrite(gzf, xmlstr.c_str(), sz);
    gzclose(gzf);

    return err;
}

static int tg_write_trajectories_csv(const string& fname, const vector<Trajectory>& trajectories) {
	stringstream oss_doc;
	ostringstream oss_tmp;

	stringstream ss_header;
	ss_header << "********* TRAJECTORY OUTPUT DATA *********" << endl;
	ss_header << "** Output Format:" << endl;
	ss_header << "** simulation_start_time" << endl;
	ss_header << "** " << endl;
	ss_header << "** AC,flight_index,callsign,actype,origin_airport,destination_airport,start_time,simulation_interval_ground,simulation_interval_airborne,cruise_altitude_ft,cruise_tas_knots,origin_airport_elevation_ft,destination_airport_elevation_ft,number_of_trajectory_rec" << endl;
	ss_header << "** timestamp(UTC sec),latitude,longitude,altitude_ft,rocd_fps,tas_knots,tas_knots_ground,course,fpa,flight_phase" << endl;

	oss_tmp << ss_header.str();
	oss_tmp << endl;

	if (g_start_time_realTime_simulation == 0) {
		oss_tmp << g_start_time << endl;
	} else {
		oss_tmp << g_start_time_realTime_simulation << endl;
	}

	for (unsigned int i = 0; i < trajectories.size(); ++i) {
		const Trajectory& t = trajectories.at(i);

		stringstream ss_ac_profile;
		stringstream ss_ac_state;

		ss_ac_profile << "AC"
				<< "," << t.flight_index
				<< "," << t.callsign
				<< "," << t.actype
				<< "," << t.origin_airport
				<< "," << t.destination_airport
				<< "," << t.start_time
				<< "," << t.interval_ground
				<< "," << t.interval_airborne
				<< "," << t.cruise_altitude_ft
				<< "," << t.cruise_tas_knots
				<< "," << t.origin_airport_elevation_ft
				<< "," << t.destination_airport_elevation_ft
				<< "," << t.latitude_deg.size()
				<< endl;

		oss_tmp << endl;
		oss_tmp << ss_ac_profile.str();

		float timestamp = 0;

		int n = t.latitude_deg.size();

		for (int j = 0; j < n; ++j) {
			timestamp = t.timestamp.at(j);

			ss_ac_state << timestamp
					<< "," << setprecision(18) << t.latitude_deg.at(j)
					<< "," << setprecision(18) << t.longitude_deg.at(j)
					<< "," << t.altitude_ft.at(j)
					<< "," << t.rocd_fps.at(j)
					<< "," << t.tas_knots.at(j)
					<< "," << t.tas_knots_ground.at(j)
					<< "," << t.course_deg.at(j)
					<< "," << t.fpa_deg.at(j);

			const char* phase;
			phase = ENUM_Flight_Phase_String[t.flight_phase.at(j)];
			ss_ac_state << "," << phase << endl;
		}

		oss_tmp << ss_ac_state.str();
	}

    // Write the doc content to file
    ofstream out;
    out.open(fname.c_str());
    if (!out.is_open()) {
        cout << "ERROR: could not open the output file for writing (" << fname << ")" << endl;
        return -1;
    }

    oss_doc << oss_tmp.str();
    out << oss_doc.str();
    out.close();

    return 0;
}

static int tg_write_trajectories_kml(const string& fname, const vector<Trajectory>& trajectories) {
	stringstream kmlOutPath;

	kmlOutPath << fname;
	ofstream kmlOut;
	kmlOut.open( kmlOutPath.str().c_str() );
	//SPLASH
	kmlOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";

	kmlOut << "<Document> \n <name>Trajectories</name>\n<description>Examples of paths. "
		<<"Note that the tessellate tag is by default set to 0."
		<<"If you want to create tessellated lines, they must be authored(or edited) directly in KML.\n</description>\n  "
		<<"<Style id=\"yellowLineGreenPoly1\">\n   <LineStyle>\n  <color>50141414</color>\n"
		<<"<width>2</width>\n</LineStyle>\n</Style>\n"
		<<"<Style id=\"yellowLineGreenPoly2\">\n   <LineStyle>\n  <color>501400FF</color>\n"
		<<"<width>2</width>\n</LineStyle>\n</Style>\n"
		<<"<Style id=\"yellowLineGreenPoly3\">\n   <LineStyle>\n  <color>50143C00</color>\n"
		<<"<width>2</width>\n</LineStyle>\n</Style>\n"
		<<"<Style id=\"yellowLineGreenPoly4\">\n   <LineStyle>\n  <color>5014F000</color>\n"
		<<"<width>2</width>\n</LineStyle>\n</Style>\n"
		<<"<Style id=\"yellowLineGreenPoly5\">\n   <LineStyle>\n  <color>501478AA</color>\n"
		<<"<width>2</width>\n</LineStyle>\n</Style>\n";

	for (size_t s = 0; s < trajectories.size(); ++s) {
		const Trajectory* traj = &(trajectories.at(s));
		vector<real_t> lats = traj->latitude_deg;
		vector<real_t> lons = traj->longitude_deg;
		vector<real_t> alts = traj->altitude_ft;
		size_t num_t = s%5;

		kmlOut << fixed;
		kmlOut << "<Placemark> \n <name>" << traj->origin_airport << " " << traj->destination_airport
			<< "</name> \n <description>Transparent green wall "
			<< "with yellow outlines</description> \n"
			<< "<styleUrl>#yellowLineGreenPoly" << num_t + 1 << "</styleUrl> \n "
			<< "<LineString>\n  <extrude>1</extrude>\n <tessellate>1</tessellate>\n"
			<< "<altitudeMode>absolute</altitudeMode>\n<coordinates>\n";

		for (size_t k=0; k<lats.size(); ++k) {
			kmlOut << setprecision(10) << lons.at(k) << ","
				    << setprecision(10) << lats.at(k) << ","
					<< setprecision(10) << alts.at(k) << endl;
		}
		kmlOut << "</coordinates>\n </LineString>\n </Placemark>\n";

	}

	kmlOut << "</Document>\n	</kml>" << endl;
	kmlOut.close();

	return 0;
}

/*
 * Write the vector of trajectories to an HDF5 file using a compound
 * data type.
 */
int tg_write_trajectories(const string& fname, const vector<Trajectory>& trajectories) {
    size_t dot_pos = fname.find_last_of(".");
    string basename = fname.substr(0, dot_pos);
    string extension = fname.substr(dot_pos);
    transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Start writing ground vehicle trajectory writing to CSV
    if(groundVehicleSimulationData.size() > 0) {
		ofstream groundTrajectories;
		groundTrajectories.open (basename + "_groundVehicleSimulation.csv");
		groundTrajectories << "Header Format: \nGroundVehicle_ID\nTime,AircraftInService,Latitude,Longitude,Altitude,Speed,Heading\n\n";
		for(map<string, vector<string> >::const_iterator it = groundVehicleSimulationData.begin();
			it != groundVehicleSimulationData.end(); ++it)
		{
			groundTrajectories << it->first + "\n";
			for(int i = 0; i < it->second.size(); i++)
				groundTrajectories << it->second.at(i) + "\n";

			groundTrajectories << "\n";

		}

		groundTrajectories.close();
    }

    // Clear trajectories
    groundVehicleSimulationData.clear();

    // End writing ground vehicles trajectory writing to CSV end

    if (extension == ".xml") {
        return tg_write_trajectories_xml(fname, trajectories);
    } else if (extension == ".gz") {
        return tg_write_trajectories_xml_gz(fname, trajectories);
    } else if (extension == ".h5") {
        return tg_write_trajectories_h5(fname, trajectories);
    } else if (extension == ".csv") {
        return tg_write_trajectories_csv(fname, trajectories);
    } else if (extension == ".kml") {
        return tg_write_trajectories_kml(fname, trajectories);
    }

    return 0;
}

template<typename T, typename U>
void hvl_to_vector(const hvl_t& vl, vector<U>* const vec) {
	if(!vec) return;
	T* array = (T*)vl.p;
	vec->insert(vec->begin(), array, array+vl.len);
}

void hvl_to_str_vector(const hvl_t& vl, vector<string>* const vec) {
    if(!vec) return;
    char* array = (char*)vl.p;
    for(size_t i=0; i<vl.len; ++i) {
        vec->push_back(string(&array[i*MAX_SECTOR_NAME_LEN]));
    }
}

bool flight_index_comparator(const Trajectory& a, const Trajectory& b) {
	return a.flight_index < b.flight_index;
}

// 2018.02.09 Oliver Chen
// This function must be tested to see if it can read h5 files.
static int tg_read_trajectories_h5(const string& fname, vector<Trajectory>* const trajectories, const set<string>& callsign_filter) {
    // open the file
    hid_t file_id = H5Fopen(fname.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

    // open the dataset
    hid_t dataset = H5Dopen2(file_id, "/trajectories", H5P_DEFAULT);

    // get the dataspace and allocate memory for the read
    hsize_t dims[1] = {0};
    hid_t dataspace = H5Dget_space(dataset);
    H5Sget_simple_extent_dims(dataspace, dims, NULL);

    // define the variable-length double array type
    hid_t mem_double_array_type = H5Tvlen_create(H5T_NATIVE_DOUBLE);
    hid_t mem_int_array_type = H5Tvlen_create(H5T_NATIVE_INT);
    hid_t mem_long_array_type = H5Tvlen_create(H5T_NATIVE_LONG);
    hid_t string_type = H5Tcopy(H5T_C_S1);
    H5Tset_size(string_type, H5T_VARIABLE);

    // build the memtype struct
    int offset = 0;

    // define the compound data type for memory
    hid_t mem_traj_type = H5Tcreate(H5T_COMPOUND, sizeof(hdf5_trajectory_t));
    H5Tinsert(mem_traj_type, "flight_index", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, callsign);
    H5Tinsert(mem_traj_type, "callsign", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, actype);
    H5Tinsert(mem_traj_type, "actype", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, origin_airport);
    H5Tinsert(mem_traj_type, "origin_airport", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, destination_airport);
    H5Tinsert(mem_traj_type, "destination_airport", offset, string_type);
    offset = HOFFSET(hdf5_trajectory_t, start_time);
    H5Tinsert(mem_traj_type, "start_time", offset, H5T_NATIVE_LONG);
    offset = HOFFSET(hdf5_trajectory_t, interval_ground);
    H5Tinsert(mem_traj_type, "interval_ground", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, interval_airborne);
    H5Tinsert(mem_traj_type, "interval_airborne", offset, H5T_NATIVE_INT);
    offset = HOFFSET(hdf5_trajectory_t, latitude);
    H5Tinsert(mem_traj_type, "latitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, longitude);
    H5Tinsert(mem_traj_type, "longitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, altitude);
    H5Tinsert(mem_traj_type, "altitude", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, hdot);
    H5Tinsert(mem_traj_type, "hdot", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, tas);
    H5Tinsert(mem_traj_type, "tas", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, course);
    H5Tinsert(mem_traj_type, "course", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, fpa);
    H5Tinsert(mem_traj_type, "fpa", offset, mem_double_array_type);
    offset = HOFFSET(hdf5_trajectory_t, phase);
    H5Tinsert(mem_traj_type, "phase", offset, mem_int_array_type);
    offset = HOFFSET(hdf5_trajectory_t, timestamp);
    H5Tinsert(mem_traj_type, "timestamp", offset, mem_long_array_type);

    hdf5_trajectory_t* hdf5_trajectories =
            (hdf5_trajectory_t*)calloc(dims[0], sizeof(hdf5_trajectory_t));

    // read the data into memtype
    H5Dread(dataset, mem_traj_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            hdf5_trajectories);

    // copy the data into the output vector
    for (hsize_t i=0; i<dims[0]; ++i) {
        hdf5_trajectory_t h5_traj = hdf5_trajectories[i];

        int flight_index = h5_traj.flight_index;
        string callsign(h5_traj.callsign);
        string actype(h5_traj.actype);
        string origin_airport(h5_traj.origin_airport);
        string destination_airport(h5_traj.destination_airport);
        long start_time = h5_traj.start_time;
        int interval_ground = h5_traj.interval_ground;
        int interval_airborne = h5_traj.interval_airborne;
        float cruise_altitude_ft = h5_traj.cruise_altitude_ft;
        float cruise_tas_knots = h5_traj.cruise_tas_knots;
        float origin_airport_elevation_ft = h5_traj.origin_airport_elevation_ft;
        float destination_airport_elevation_ft = h5_traj.destination_airport_elevation_ft;
        bool flag_externalAircraft = h5_traj.flag_externalAircraft;

        Trajectory trajectory(flight_index,
        					  callsign,
							  actype,
							  origin_airport,
                              destination_airport,
							  start_time,
							  interval_ground,
							  interval_airborne,
							  cruise_altitude_ft,
							  cruise_tas_knots,
							  origin_airport_elevation_ft,
							  destination_airport_elevation_ft,
							  flag_externalAircraft);

        hvl_to_vector<double, real_t>(h5_traj.latitude, &(trajectory.latitude_deg));
        hvl_to_vector<double, real_t>(h5_traj.longitude, &trajectory.longitude_deg);
        hvl_to_vector<double, real_t>(h5_traj.altitude, &trajectory.altitude_ft);
        hvl_to_vector<double, real_t>(h5_traj.hdot, &trajectory.rocd_fps);
        hvl_to_vector<double, real_t>(h5_traj.tas, &trajectory.tas_knots);
        hvl_to_vector<double, real_t>(h5_traj.course, &trajectory.course_deg);
        hvl_to_vector<double, real_t>(h5_traj.fpa, &trajectory.fpa_deg);
        hvl_to_vector<float, float>(h5_traj.timestamp, &trajectory.timestamp);

        // Convert integer flight phase to enum flight phase
        for (size_t j=0; j<h5_traj.phase.len; ++j) {
        	ENUM_Flight_Phase phase;
        	int h5_phase = ((int*)(h5_traj.phase.p))[j];;
        	phase = ENUM_Flight_Phase(h5_phase);

        	trajectory.flight_phase.push_back(phase);
        }

        // if the user specified a set of callsigns to filter, then add
        // only the trajectories with the desired callsigns. otherwise
        // add all trajectories.
        if (callsign_filter.size() > 0) {
            set<string>::iterator filter_iter = callsign_filter.find(callsign);
            if (filter_iter != callsign_filter.end()) {
                trajectories->push_back(trajectory);
            }
        } else {
            trajectories->push_back(trajectory);
        }
    }

    // sort the trajectories vector by fight index so that we get the
    // same ordering as the original simulation.
    sort(trajectories->begin(), trajectories->end(), flight_index_comparator);

    H5Tclose(mem_traj_type);
    H5Tclose(string_type);
    H5Tclose(mem_int_array_type);
    H5Tclose(mem_long_array_type);
    H5Tclose(mem_double_array_type);
    H5Sclose(dataspace);
    H5Dclose(dataset);
    H5Fclose(file_id);

    // free mem
    free(hdf5_trajectories);

    return 0;
}

static int tg_read_trajectories_xml(xml_document<>& doc, vector<Trajectory>* const trajectories, const set<string>& callsign_filter) {
    xml_node<>* trajectories_node = doc.first_node("trajectories");
    if (trajectories_node) {
        xml_node<>* trajectory_node = trajectories_node->first_node("trajectory");
        while (trajectory_node) {
            xml_attribute<>* flight_index_attr = trajectory_node->first_attribute("flight_index");
            xml_attribute<>* callsign_attr = trajectory_node->first_attribute("callsign");
            xml_attribute<>* actype_attr = trajectory_node->first_attribute("actype");
            xml_attribute<>* origin_airport_attr = trajectory_node->first_attribute("origin_airport");
            xml_attribute<>* destination_airport_attr = trajectory_node->first_attribute("destination_airport");
            xml_attribute<>* start_time_attr = trajectory_node->first_attribute("start_time");
            xml_attribute<>* interval_ground_attr = trajectory_node->first_attribute("interval_ground");
            xml_attribute<>* interval_airborne_attr = trajectory_node->first_attribute("interval_airborne");
            xml_attribute<>* cruise_altitude_ft_attr = trajectory_node->first_attribute("cruise_altitude_ft");
            xml_attribute<>* cruise_tas_knots_attr = trajectory_node->first_attribute("cruise_tas_knots");
            xml_attribute<>* origin_airport_elevation_ft_attr = trajectory_node->first_attribute("origin_airport_elevation_ft");
            xml_attribute<>* destination_airport_elevation_ft_attr = trajectory_node->first_attribute("destination_airport_elevation_ft");
            xml_attribute<>* flag_externalAircraft_attr = trajectory_node->first_attribute("flag_externalAircraft");

            int flight_index = atoi(flight_index_attr->value());
            string callsign = string(callsign_attr->value());
            string actype = string(actype_attr->value());
            string origin = string(origin_airport_attr->value());
            string dest = string(destination_airport_attr->value());
            long start_time = atol(start_time_attr->value());
            int interval_ground = atoi(interval_ground_attr->value());
            int interval_airborne = atoi(interval_airborne_attr->value());
            float cruise_altitude_ft = atof(cruise_altitude_ft_attr->value());
            float cruise_tas_knots = atof(cruise_tas_knots_attr->value());
            float origin_airport_elevation_ft = atof(origin_airport_elevation_ft_attr->value());
            float destination_airport_elevation_ft = atof(destination_airport_elevation_ft_attr->value());
            bool flag_externalAircraft = false;
            if (indexOf(flag_externalAircraft_attr->value(), "true") == 0) {
            	flag_externalAircraft = true;
            }

            // if a callsign filter was supplied then check the filter
            if (callsign_filter.size() > 0) {
                set<string>::iterator filter_iter = callsign_filter.find(callsign);
                if (filter_iter == callsign_filter.end()) {
                    // not found in the filter set so ignore this flight
                    trajectory_node = trajectory_node->next_sibling("trajectory");

                    continue;
                }
            }

            Trajectory trajectory(flight_index, callsign, actype, origin, dest, start_time, interval_ground, interval_ground, cruise_altitude_ft, cruise_tas_knots, origin_airport_elevation_ft, destination_airport_elevation_ft, flag_externalAircraft);

            xml_node<>* trajectory_point_node = trajectory_node->first_node("trajectory_point");
            while (trajectory_point_node) {
                xml_node<>* lat_node = trajectory_point_node->first_node("latitude_deg");
                xml_node<>* lon_node = trajectory_point_node->first_node("longitude_deg");
                xml_node<>* alt_node = trajectory_point_node->first_node("altitude_ft");
                xml_node<>* rocd_node = trajectory_point_node->first_node("rocd_fps");
                xml_node<>* tas_node = trajectory_point_node->first_node("tas_knots");
                xml_node<>* hdg_node = trajectory_point_node->first_node("course_deg");
                xml_node<>* fpa_node = trajectory_point_node->first_node("fpa_deg");
                xml_node<>* phase_node = trajectory_point_node->first_node("flight_phase");
                xml_node<>* timestamp_node = trajectory_point_node->first_node("timestamp");

                double lat = atof(lat_node->value());
                double lon = atof(lon_node->value());
                double alt = atof(alt_node->value());
                double rocd = atof(rocd_node->value());
                double tas = atof(tas_node->value());
                double hdg = atof(hdg_node->value());
                double fpa = atof(fpa_node->value());


                char* phasestr = phase_node->value();
                long timestamp = atol(timestamp_node->value());

                ENUM_Flight_Phase phase = getFlight_Phase(phasestr);

                trajectory.latitude_deg.push_back(lat);
                trajectory.longitude_deg.push_back(lon);
                trajectory.altitude_ft.push_back(alt);
                trajectory.rocd_fps.push_back(rocd);
                trajectory.tas_knots.push_back(tas);
                trajectory.course_deg.push_back(hdg);
                trajectory.fpa_deg.push_back(fpa);
                trajectory.flight_phase.push_back(phase);
                trajectory.timestamp.push_back(timestamp);

                trajectory_point_node = trajectory_point_node->next_sibling("trajectory_point");
            }

            trajectories->push_back(trajectory);

            trajectory_node = trajectory_node->next_sibling("trajectory");
        }
    }

    return 0;
}

static int tg_read_trajectories_xml(const string& fname, vector<Trajectory>* const trajectories, const set<string>& callsign_filter) {

    if(!trajectories) {
        return -1;
    }

    ifstream in;
    in.open(fname.c_str());
    stringstream ss;
    while(in.good()) {
        string line = "";
        getline(in, line);
        ss << line;
    }
    in.close();

    string xmlstr = ss.str();
    xml_document<> doc;
    doc.parse<0>(const_cast<char*>(xmlstr.c_str()));

    tg_read_trajectories_xml(doc, trajectories, callsign_filter);

    return 0;
}

static int tg_read_trajectories_xml_gz(const string& fname, vector<Trajectory>* const trajectories, const set<string>& callsign_filter) {

    // WARNING: untested.

    // inflate the file
    int buflen = 65536;
    char buf[65536];

    stringstream ss;

    gzFile gzf = gzopen(fname.c_str(), "rb");
    gzbuffer(gzf, buflen);
    // gzread returns uncompressed bytes actually read,
    // will be less than len for eof.
    while(gzread(gzf, buf, buflen) == buflen) {
        ss << buf;
    }
    gzclose(gzf);

    string xmlstr = ss.str();
    xml_document<> doc;
    doc.parse<0>(const_cast<char*>(xmlstr.c_str()));

    tg_read_trajectories_xml(doc, trajectories, callsign_filter);

    return 0;
}

/*
 * Read the trajectories from the specified HDF5 file into the output
 * vector of Trajectories.
 */
int tg_read_trajectories(const string& fname, vector<Trajectory>* const trajectories, const set<string>& callsign_filter) {

    size_t dot_pos = fname.find_last_of(".");
    string basename = fname.substr(0, dot_pos);
    string extension = fname.substr(dot_pos);
    transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if(extension == ".xml") {
        return tg_read_trajectories_xml(fname, trajectories, callsign_filter);
    } else if(extension == ".gz") {
        return tg_read_trajectories_xml_gz(fname, trajectories, callsign_filter);
    } else {
        return tg_read_trajectories_h5(fname, trajectories, callsign_filter);
    }
}

static void generate_map_airport() {
	string cur_airport_id;
	NatsAirport* cur_airport_ptr;

	NatsSid tmp_sid;
	NatsStar tmp_star;
	NatsApproach tmp_approach;

	// Build up map_airport elements.
	for (unsigned int i = 0; i < g_airports.size(); i++) {
		cur_airport_id = g_airports.at(i).code;
		trim(cur_airport_id);
		if (map_airport.find(cur_airport_id) == map_airport.end()) {
			map_airport.insert(pair<string, NatsAirport*>(cur_airport_id, &g_airports.at(i)));
		}
	}

	// Fill SID data in every map_airport element content
	for (unsigned int i = 0; i < g_sids.size(); i++) {
		cur_airport_id = g_sids.at(i).id;
		trim(cur_airport_id);
		if (map_airport.find(cur_airport_id) != map_airport.end()) {
			cur_airport_ptr = map_airport.at(cur_airport_id);

			tmp_sid = g_sids.at(i);

			cur_airport_ptr->avail_sids.push_back(&g_sids.at(i));
			cur_airport_ptr->avail_runways.insert(tmp_sid.runway);
		}
	}

	// Fill STAR data in every map_airport element content
	for (unsigned int i = 0; i < g_stars.size(); i++) {
		cur_airport_id = g_stars.at(i).id;

		trim(cur_airport_id);
		if (map_airport.find(cur_airport_id) != map_airport.end()) {
			cur_airport_ptr = map_airport.at(cur_airport_id);

			tmp_star = g_stars.at(i);

			cur_airport_ptr->avail_stars.push_back(&g_stars.at(i));
			cur_airport_ptr->avail_runways.insert(tmp_star.runway);
		}
	}

	// Fill Approach data in every map_airport element content
	for (unsigned int i = 0; i < g_approaches.size(); i++) {
		cur_airport_id = g_approaches.at(i).id;

		trim(cur_airport_id);
		if (map_airport.find(cur_airport_id) != map_airport.end()) {
			cur_airport_ptr = map_airport.at(cur_airport_id);

			tmp_approach = g_approaches.at(i);

			cur_airport_ptr->avail_approaches.push_back(&g_approaches.at(i));
			cur_airport_ptr->avail_runways.insert(tmp_approach.runway);
		}
	}
}

#if GEN_RTG_MAP
static void generate_rtg_map(const string& adb_dir,const double base_alt = 10000,
		const double knots_to_fpm = 101.26859142607){

	string adb_rtg_path = adb_dir+"/RTG_models";
	mkpath(adb_rtg_path);
	for( size_t k=0;k< g_adb_ptf_models.size(); ++k){
		stringstream filepath;
		filepath << adb_rtg_path << "/"<< g_adb_ptf_models.at(k).actype.c_str() << "__.RTG";

		map<double,double> descentTas = g_adb_ptf_models.at(k).descentTas;
		map<double, double> descentRateNom = g_adb_ptf_models.at(k).descentRateNom;


		ofstream out;
		out.open(filepath.str().c_str());
		out << fixed;
		out <<" Range to go table for aircraft type " << g_adb_ptf_models.at(k).actype.c_str() << endl;
		out << "|----------------------------------------------------------------------------------------------|" << endl;
		out << "|   h_initial (ft) | h_final (ft)     | v_initial (knots)|  v_final (knots) | range to go (nmi)|" << endl;
		out << "|----------------------------------------------------------------------------------------------|" << endl;
		vector<double> altitudes = g_adb_ptf_models.at(k).altitudes;
		double v_base_alt = descentTas[base_alt]*knots_to_fpm;
		double prev_alt = base_alt,v_prev_alt = v_base_alt;

		double rtg_nmi = 0;

		for (size_t j=0;j<altitudes.size(); ++j){


			if (altitudes.at(j) <= base_alt){
				v_prev_alt = descentTas[altitudes.at(j)]*knots_to_fpm;
				prev_alt = altitudes.at(j);
				continue;
			}


			double rod = -1.0*descentRateNom[altitudes.at(j)];
			double v_curr_alt = descentTas[altitudes.at(j)]*knots_to_fpm;

			double del_t = (prev_alt-altitudes.at(j))/rod;

			double accel = (v_prev_alt - v_curr_alt)/del_t;

			double rtg =0;
			if (accel != 0.0 ){
				rtg =  (v_prev_alt*v_prev_alt - v_curr_alt*v_curr_alt) / ( 2*accel );
			}
			else{
				rtg = v_prev_alt*del_t;
			}

			rtg_nmi = rtg_nmi + rtg/(knots_to_fpm*60.0);

			out <<"| " << setprecision(8) <<  setw(16) << altitudes.at(j)
				<< " | " << setprecision(8) <<  setw(16) << base_alt
				<< " | " << setprecision(8) <<  setw(16) << v_curr_alt/knots_to_fpm
				<< " | " << setprecision(8) <<  setw(16) << v_prev_alt/knots_to_fpm
				<< " | " << setprecision(8) <<  setw(16) << rtg_nmi
				<< " | " << endl;

			v_prev_alt = v_curr_alt;
			prev_alt = altitudes.at(j);

		}

		out.close();
	}
}
#endif

static void debug_list_variables() {
/*
// Debug
// Show all airports data
	for (size_t k = 0; k < g_airports.size(); ++k) {
		cout << "g_airports --> airport code = " << g_airports.at(k).code << ", elevation = " << g_airports.at(k).elevation << endl;
	}
*/

/*
// Debug
// Show all waypoints data
		for (size_t k = 0; k < g_waypoints.size(); ++k) {
			cout << "waypoint name = " << g_waypoints.at(k).name << ", latitude = " << g_waypoints.at(k).latitude << ", longitude = " << g_waypoints.at(k).longitude << endl;
		}
*/

/*
// Debug
// Show all SIDs data
		for (size_t k = 0; k < g_sids.size(); ++k) {
			cout << "sid.id = " << g_sids.at(k).id << ", name = " << g_sids.at(k).name << ", runway = " << g_sids.at(k).runway << endl;
//			for (size_t j = 0 ; j < g_sids.at(k).waypoints.size(); ++j)
//				cout << "	waypoint " << j << " is " << g_sids.at(k).waypoints.at(j) << endl;
		}
*/

/*
// Debug
// Show all STARs data
		for (size_t k = 0; k < g_stars.size(); ++k) {
			cout << "star.id = " << g_stars.at(k).id << ", name = " << g_stars.at(k).name << ", runway = " << g_stars.at(k).runway << endl;
			for (size_t j = 0 ; j < g_stars.at(k).waypoints.size(); ++j)
				cout << "	waypoint " << j << " is " << g_stars.at(k).waypoints.at(j) << endl;

//			map<string, vector<string> >::iterator ite;
//			for (ite = g_stars.at(k).wp_map.begin(); ite != g_stars.at(k).wp_map.end(); ite++) {
//				for (int m = 0; m < ite->second.size(); m++)
//					cout << "	wp_map->first = " << ite->first << ", wp_map->second = " << ite->second.at(m) << endl;
//			}
		}
*/

/*
// Debug
// Show all APPROACHes data
		for (size_t k = 0; k < g_approaches.size(); ++k) {
			cout << "g_approaches.id = " << g_approaches.at(k).id << ", name = " << g_approaches.at(k).name << ", runway = " << g_approaches.at(k).runway << endl;

			map<string, vector<string> >& tmp_wp_map = g_approaches.at(k).wp_map;

			map<string, vector<string> >::iterator ite_wp_map;
			for (ite_wp_map = tmp_wp_map.begin(); ite_wp_map != tmp_wp_map.end(); ite_wp_map++) {
				for (int m = 0; m < ite_wp_map->second.size(); m++) {
					printf("	map key = %s, vector value = %s\n", ite_wp_map->first.c_str(), ite_wp_map->second.at(m).c_str());
				}
			}

			//for (size_t j = 0 ; j < g_approaches.at(k).waypoints.size(); ++j)
			//	cout << "	waypoint " << j << " is " << g_approaches.at(k).waypoints.at(j) << endl;
		}
*/

/*
	for (int i = 0; i < g_adb_ptf_models.size(); i++) {
		printf("i = %d, g_adb_ptf_models.actype = %s\n", i, g_adb_ptf_models.at(i).actype.c_str());
	}
*/
}

/*
 * This function loads static data files:
 *   ADB performance tables
 *   Nats airport data
 *   Nats waypoint data
 *   Nats airway data
 *   Nats par data
 *   Nats sid/star data
 *   Nats sector data
 */
static int do_init(adb_ptf_column_e col) {
	if (g_flag_standalone_mode) {
		g_share_dir.assign("../GNATS_Server/share");
	}

	if ((GNATS_HOME != NULL) && (0 < strlen(GNATS_HOME))) {
		g_share_dir = string(GNATS_HOME) + "/" + g_share_dir;
	}

	string adb_dir = g_share_dir + "/libadb";
	string nats_data_dir = g_share_dir + "/libnats_data";

	string cifp_path = getCIFPPath(g_share_dir);
	if (0 < cifp_path.length()) {
		g_cifp_file.assign(cifp_path);
		g_cifp_file.append("/");
		g_cifp_file.append("FAACIFP18");
	} else {
		printf("  CIFP data files are not available\n\n");
	}

#pragma omp parallel num_threads(6)
	{
		int thread_id = omp_get_thread_num();

		if(0 == thread_id) {

		}
		else if(1 == thread_id) {
			// load ADB (host and device)
			load_adb_performance_tables(adb_dir, col);
		}
		else if(2 == thread_id) {
			if (0 < cifp_path.length()) {
				// load nats airport data (host only)
				load_airports(nats_data_dir, g_cifp_file);
			}
		}
		else if(3 == thread_id) {
			if (0 < cifp_path.length()) {
				// load nats waypoint data (host only)
				load_waypoints(nats_data_dir, g_cifp_file);
			}
		}
		else if(4 == thread_id) {
			if (0 < cifp_path.length()) {
				// load nats airways data (host only)
				load_airways(nats_data_dir, g_cifp_file);
			}
		}
		else if(5 == thread_id) {
			if (0 < cifp_path.length()) {
				// load nats sid/star data (host only)
				load_sidstars(nats_data_dir, g_cifp_file);
			}
		}
	}
#pragma omp barrier

	//ONLY USE IT TO GENERATE THE RTG MAP
#if GEN_RTG_MAP
	generate_rtg_map(adb_dir);
#endif
	generate_map_airport();

	//debug_list_variables();

//	load_user_config();

	return 0;
}

int tg_get_num_flights(int* const num_flights) {
	if(!num_flights) return -1;
	*num_flights = get_num_flights();
	return 0;
}

int tg_get_flightplans(map<int, FlightPlan>* const flightplans) {
	if(!flightplans) return -1;
	flightplans->insert(g_flightplans.begin(), g_flightplans.end());
	return 0;
}

int tg_get_flightplan(const int& flight_index, FlightPlan* const fp) {
	if(!fp) return -1;
	*fp = g_flightplans.at(flight_index);
	return 0;
}

int tg_get_cruise_tas(const int& flight_index, real_t* const cruise_tas) {
	if(!cruise_tas) return -1;
	*cruise_tas = h_aircraft_soa.cruise_tas_knots[flight_index];
	return 0;
}

int tg_get_cruise_altitude(const int& flight_index, real_t* const cruise_alt) {
	if(!cruise_alt) return -1;
	*cruise_alt = h_aircraft_soa.cruise_alt_ft[flight_index];
	return 0;
}

int tg_get_origin_airport(const int& flight_index, string* const airport) {
	if(!airport) return -1;
	*airport = g_flightplans.at(flight_index).origin;
	return 0;
}

int tg_get_origin_elevation(const int& flight_index, real_t* const elev) {
	if(!elev) return -1;
	*elev = h_aircraft_soa.origin_airport_elevation_ft[flight_index];
	return 0;
}

int tg_get_destination_airport(const int& flight_index, string* const airport) {
	if(!airport) return -1;
	*airport = g_flightplans.at(flight_index).destination;
	return 0;
}

int tg_get_destination_elevation(const int& flight_index, real_t* const elev) {
	if(!elev) return -1;
	*elev = h_aircraft_soa.destination_airport_elevation_ft[flight_index];
	return 0;
}

int tg_get_aircraft_type(const int& flight_index, string* const actype) {
	if(!actype) return -1;
	*actype = g_trx_records.at(flight_index).actype;
	return 0;
}

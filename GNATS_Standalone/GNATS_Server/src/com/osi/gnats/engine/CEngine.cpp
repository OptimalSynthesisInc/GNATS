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

/**
 * This file contains all C/C++ codes to implement the actual logic of functions in CEngine.h
 */

#include <map>

#include "CEngine.h"

#include "pub_logger.h"

#include "tg_api.h"
#include "tg_rap.h"
#include "tg_aircraft.h"
#include "tg_airports.h"
#include "tg_sidstars.h"
#include "tg_simulation.h"
#include "tg_waypoints.h"
#include "tg_weather.h"
#include "tg_weatherWaypoint.h"

#include "rg_exec.h"

#include "AirportLayoutDataLoader.h"
#include "Controller.h"
#include "geometry_utils.h"

#include "tg_groundVehicle.h"
#include "tg_incidentFlightPhase.h"

#include "tg_riskMeasures.h"

#include "util_string.h"
#include "util_time.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>
#include <set>

#include <float.h>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <zip.h>

using namespace std;
using namespace osi;

#define RADIUS_EARTH_FT         20925524.9
#define PI                      3.14159265359

jobjectArray allAircraftIdArray;

// Terrain Interface variables
double startLat = -56;
double endLat = 75;
double startLon = -180;
double endLon = 180;
double resolution = 0.1;
bool terrDataLoaded = false;
map<int,vector<string>> terrData;
map<string, vector<string>> airportMapData;
map<int,int> usgsMetadata;

ENUM_Flight_Phase getFlightPhase(int flight_phase) {
	ENUM_Flight_Phase retPhase;

	retPhase = ENUM_Flight_Phase(flight_phase);

	return retPhase;
}

void tokenize(std::string const &str, const char delim,
            std::vector<std::string> &out)
{
    size_t start;
    size_t end = 0;
 
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

vector<string> intersection(vector<string> &v1,vector<string> &v2){
    std::vector<std::string> v3;

    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(), back_inserter(v3));
    return v3;
}

static void preprocessStarsAars() {

	string airportList[] = {"KABQ", "KATL", "KBDL", "KBOI", "KBOS", "KBUR", "KBWI", "KCLE", "KCLT", "KCVG", "KDCA", "KDEN", "KDFW", "KDTW", "KEWR", "KGYY", "KHPN", "KIAD", "KIAH", "KISP", "KJAX", "KJFK", "KLAS", "KLAX", "KLGA", "KLGB", "KMCO", "KMDW", "KMEM", "KMHT", "KMIA", "KMSP", "KOAK", "KONT", "KPBI", "KPDK", "KPDX", "KPHL", "KPHX", "KPIT", "KSAN", "KSDF", "KSFO", "KSJC", "KSLC", "KSNA", "KSTL", "KSWF", "KTEB", "KTPA", "KVGT", "PHNL", "PANC", "KORD", "KPVD", "KSEA", "KDAL"};

}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_set_1flag_1gdb
  (JNIEnv *jniEnv, jobject jobj, jboolean j_flag_gdb) {
	g_flag_gdb = j_flag_gdb;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_set_1log_1level
  (JNIEnv *jniEnv, jobject jobj, jstring j_logLevel) {
	const char *c_logLevel;

	if (j_logLevel) {
		c_logLevel = (char*)jniEnv->GetStringUTFChars( j_logLevel, 0 );

		logger_set_console_log_level(c_logLevel);
	}
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_info
  (JNIEnv *jniEnv, jobject jobj) {
	tg_info();
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_tg_1init
  (JNIEnv *jniEnv,
		  jobject jobj,
		  jstring j_data_dir,
		  jint j_perturbation,
		  jint j_device_id,
		  jboolean j_flag_standalone_mode) {
	const char *c_data_dir = (char*)jniEnv->GetStringUTFChars( j_data_dir, 0 );

	if (g_flag_gdb)
		getchar();

	g_flag_standalone_mode = j_flag_standalone_mode;

	int initReturn = 0;

	initReturn = tg_init(c_data_dir, (int) j_perturbation, (int) j_device_id);

	return initReturn;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_tg_1shutdown
  (JNIEnv *jniEnv,
		  jobject jobj) {

}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1rap
  (JNIEnv *jniEnv,
		  jobject jobj,
		  jstring j_wind_dir) {
	const char *c_wind_dir = (char*)jniEnv->GetStringUTFChars( j_wind_dir, 0 );

	return load_rap(c_wind_dir);
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_release_1rap
  (JNIEnv *jniEnv,
		  jobject jobj) {
	return destroy_rap();
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1aircraft
  (JNIEnv *jniEnv,
		  jobject jobj,
		  jstring j_trx_file,
		  jstring j_mfl_file) {
	const char *c_trx_file = (char*)jniEnv->GetStringUTFChars( j_trx_file, 0 );
	const char *c_mfl_file = (char*)jniEnv->GetStringUTFChars( j_mfl_file, 0 );

	return load_aircraft(c_trx_file, c_mfl_file);
}

JNIEXPORT jboolean JNICALL Java_com_osi_gnats_engine_CEngine_validate_1flight_1plan_1record
  (JNIEnv *jniEnv, jobject jobj, jstring j_string_track, jstring j_string_fp_route, jint j_mfl_ft) {
	jboolean retValue = false;

	const char *c_string_track = (char*)jniEnv->GetStringUTFChars( j_string_track, 0 );
	const char *c_string_fp_route = (char*)jniEnv->GetStringUTFChars( j_string_fp_route, 0 );
	int c_mfl_ft = j_mfl_ft;

	string str_errorMsg("");

    char className[strlen("java/rmi/RemoteException")+1];

	jclass exClass;

	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	exClass = jniEnv->FindClass(className);

	bool result = collect_user_upload_aircraft("TRACK_TIME 0", // Set default value
			string(c_string_track),
			string(c_string_fp_route),
			c_mfl_ft,
			true,
			str_errorMsg);

	// If result is false
	if (!result) {
		if ((exClass != NULL) && (str_errorMsg.length() > 0)) {
			jniEnv->ThrowNew(exClass, str_errorMsg.c_str());
		}
	} else {
		retValue = result;
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_release_1aircraft
  (JNIEnv *jniEnv, jobject jobj) {
	return destroy_aircraft();
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1groundVehicle
  (JNIEnv *jniEnv, jobject jobj, jstring j_trx_file) {
	const char *c_trx_file = (char*)jniEnv->GetStringUTFChars( j_trx_file, 0 );

	return load_groundVehicle(c_trx_file);
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_release_1groundVehicle
  (JNIEnv *jniEnv, jobject jobj) {
	return release_groundVehicle();
}

JNIEXPORT jlong JNICALL Java_com_osi_gnats_engine_CEngine_get_1sim_1id
  (JNIEnv *jniEnv, jobject jobj) {
	jlong retValue = sim_id;

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_propagate_1flights__FF
  (JNIEnv *jniEnv,
		  jobject jobj,
		  jfloat j_t_total_propagation_period,
		  jfloat j_t_step) {
	const float c_t_total_propagation_period = (float)j_t_total_propagation_period;
	const float c_t_step = (float)j_t_step;

	return propagate_flights(c_t_total_propagation_period, c_t_step);
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_propagate_1flights__FFFF
  (JNIEnv *jniEnv,
		  jobject jobj,
		  jfloat j_t_total_propagation_period,
		  jfloat j_t_step,
		  jfloat j_t_step_terminal,
		  jfloat j_t_step_airborne) {
	const float c_t_total_propagation_period = (float)j_t_total_propagation_period;
	const float c_t_step = (float)j_t_step;
	const float c_t_step_terminal = (float)j_t_step_terminal;
	const float c_t_step_airborne = (float)j_t_step_airborne;

	return propagate_flights(c_t_total_propagation_period, c_t_step, c_t_step_terminal, c_t_step_airborne);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_enableConflictDetectionAndResolution
  (JNIEnv *jniEnv, jobject jobj, jboolean j_flag) {
	const bool c_flag = j_flag;

	flag_enable_cdnr = c_flag;

	if (flag_enable_cdnr) {
		printf("Conflict detection and resolution: Enabled\n");
	} else {
		printf("Conflict detection and resolution: Disabled\n");
	}
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getCDR_1status
  (JNIEnv *jniEnv, jobject jobj) {
	jobjectArray retArray = NULL;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
	jclass jcls_Float = jniEnv->FindClass("Ljava/lang/Float;");
	jclass jcls_Object = jniEnv->FindClass("Ljava/lang/Object;");

	jmethodID methodId_Float_constructor = jniEnv->GetMethodID(jcls_Float, "<init>", "(F)V");
	jmethodID methodId_String_constructor = jniEnv->GetMethodID(jcls_String, "<init>", "(Ljava/lang/String;)V");

	jclass jcls_ObjectArray_1D = jniEnv->FindClass("[Ljava/lang/Object;");

	if (0 < map_CDR_status.size()) {
		retArray = (jobjectArray)jniEnv->NewObjectArray(map_CDR_status.size(), jcls_ObjectArray_1D, NULL);

		jobjectArray layer_2_Array;

		int tmpIdx = 0;

		std::map<string, pair<string, float>>::iterator ite_map_CDR_status;
		for (ite_map_CDR_status = map_CDR_status.begin(); ite_map_CDR_status != map_CDR_status.end(); ite_map_CDR_status++) {
			jobject jObj_ac1 = (jobject)jniEnv->NewObject(jcls_String, methodId_String_constructor, jniEnv->NewStringUTF(ite_map_CDR_status->first.c_str()));
			jobject jObj_ac2 = (jobject)jniEnv->NewObject(jcls_String, methodId_String_constructor, jniEnv->NewStringUTF(ite_map_CDR_status->second.first.c_str()));

			jobject jObj_heldSeconds_ac1 = (jobject)jniEnv->NewObject(jcls_Float, methodId_Float_constructor, ite_map_CDR_status->second.second);

			layer_2_Array = (jobjectArray)jniEnv->NewObjectArray(3, jcls_Object, NULL);

			jniEnv->SetObjectArrayElement(layer_2_Array, 0, jObj_ac1);
			jniEnv->SetObjectArrayElement(layer_2_Array, 1, jObj_ac2);
			jniEnv->SetObjectArrayElement(layer_2_Array, 2, jObj_heldSeconds_ac1);

			jniEnv->SetObjectArrayElement(retArray, tmpIdx, layer_2_Array);

			tmpIdx++;
		}
	}

	return retArray;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1initiation_1distance_1ft_1surface
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_initiation_distance_ft_surface = c_distance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1initiation_1distance_1ft_1terminal
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_initiation_distance_ft_terminal = c_distance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1initiation_1distance_1ft_1enroute
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_initiation_distance_ft_enroute = c_distance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1separation_1distance_1ft_1surface
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_separation_distance_ft_surface = c_distance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1separation_1distance_1ft_1terminal
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_separation_distance_ft_terminal = c_distance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setCDR_1separation_1distance_1ft_1enroute
  (JNIEnv *jniEnv, jobject jobj, jfloat j_distance) {
	float c_distance = j_distance;

	cdr_separation_distance_ft_enroute = c_distance;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_get_1runtime_1sim_1status
  (JNIEnv *jniEnv, jobject jobj) {
	return get_runtime_sim_status();
}

JNIEXPORT jfloat JNICALL Java_com_osi_gnats_engine_CEngine_get_1curr_1sim_1time
  (JNIEnv *jniEnv, jobject jobj) {
	return get_curr_sim_time();
}

JNIEXPORT jlong JNICALL Java_com_osi_gnats_engine_CEngine_get_1curr_1utc
  (JNIEnv *jniEnv, jobject jobj) {
	return getCurrentCpuTime_milliSec();
}

JNIEXPORT jlong JNICALL Java_com_osi_gnats_engine_CEngine_get_1nextPropagation_1utc_1time
  (JNIEnv *jniEnv, jobject jobj) {
	return nextPropagation_utc_time_realTime_simulation;
}

JNIEXPORT jboolean JNICALL Java_com_osi_gnats_engine_CEngine_isRealTime_1simulation
  (JNIEnv *jniEnv, jobject jobj) {
	bool retValue = flag_realTime_simulation;

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_get_1realTime_1simulation_1time_1step
  (JNIEnv *jniEnv, jobject jobj) {
	return time_step_surface_realTime_simulation;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1start__
  (JNIEnv *jniEnv, jobject jobj) {
	sim_id = getCurrentCpuTime_milliSec();

	nats_simulation_operator(NATS_SIMULATION_STATUS_START);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1start__J
  (JNIEnv *jniEnv, jobject jobj, jlong j_duration) {
	sim_id = getCurrentCpuTime_milliSec();

	set_nats_simulation_duration((long)j_duration);
	nats_simulation_operator(NATS_SIMULATION_STATUS_START);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1start__F
  (JNIEnv *jniEnv, jobject jobj, jfloat j_duration) {
	sim_id = getCurrentCpuTime_milliSec();

	set_nats_simulation_duration(j_duration);
	nats_simulation_operator(NATS_SIMULATION_STATUS_START);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1startRealTime
  (JNIEnv *jniEnv, jobject jobj) {
	struct timezone tz;
	// Obtain synchronized timestamp for real-time simulation
	gettimeofday(&timeval_realTime_simulation_synchronized, &tz);

	flag_realTime_simulation = true;

	time_step_surface_realTime_simulation = 30; // Default
	time_step_terminal_realTime_simulation = 30; // Default
	time_step_airborne_realTime_simulation = 30; // Default

	propagate_flights(0, time_step_surface_realTime_simulation);

	g_start_time_realTime_simulation = 0; // Reset

	sim_id = getCurrentCpuTime_milliSec();

	nats_simulation_operator(NATS_SIMULATION_STATUS_START);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1pause
  (JNIEnv *jniEnv, jobject jobj) {
	nats_simulation_operator(NATS_SIMULATION_STATUS_PAUSE);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1resume__
  (JNIEnv *jniEnv, jobject jobj) {
	nats_simulation_operator(NATS_SIMULATION_STATUS_RESUME);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1resume__J
  (JNIEnv *jniEnv, jobject jobj, jlong j_duration) {
	set_nats_simulation_duration((long)j_duration);
	nats_simulation_operator(NATS_SIMULATION_STATUS_RESUME);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1resume__F
  (JNIEnv *jniEnv, jobject jobj, jfloat j_duration) {
	set_nats_simulation_duration(j_duration);
	nats_simulation_operator(NATS_SIMULATION_STATUS_RESUME);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_sim_1stop
  (JNIEnv *jniEnv, jobject jobj) {
	flag_realTime_simulation = false;

	nats_simulation_operator(NATS_SIMULATION_STATUS_STOP);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_write_1trajectories
  (JNIEnv *jniEnv, jobject jobj,
		  jstring j_output_file) {
	const char *c_output_file = (char*)jniEnv->GetStringUTFChars( j_output_file, 0 );

	tg_write_trajectories(c_output_file, g_trajectories);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_clear_1trajectories
  (JNIEnv *jniEnv, jobject jobj) {
	clear_trajectory();
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_request_1aircraft
  (JNIEnv *jniEnv, jobject jobj, jstring j_assigned_auth_id, jstring j_ac_id) {
	const char *c_assigned_auth_id = (char*)jniEnv->GetStringUTFChars( j_assigned_auth_id, 0 );
	const char *c_ac_id = (char*)jniEnv->GetStringUTFChars( j_ac_id, 0 );

    char className[strlen("java/rmi/RemoteException")+1];

	jclass exClass;

	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	exClass = jniEnv->FindClass(className);

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	string string_assigned_auth_id(c_assigned_auth_id);
	string string_ac_id(c_ac_id);

	string stringErrorMsg;

	if (map_Acid_FlightSeq.find(string_ac_id) == map_Acid_FlightSeq.end()) {
		if (exClass != NULL) {
			stringErrorMsg.assign("The aircraft id ");
			stringErrorMsg.append(string_ac_id);
			stringErrorMsg.append(" does not exist.");

			jniEnv->ThrowNew(exClass, stringErrorMsg.c_str());
		}
	} else {
		if (map_aircraft_owner.find(string_ac_id) != map_aircraft_owner.end()) {
			string tmpOwner(map_aircraft_owner.at(string_ac_id));

			if (string_assigned_auth_id.find(tmpOwner) == string::npos) {
				if (exClass != NULL) {
					stringErrorMsg.assign("The aircraft ");
					stringErrorMsg.append(string_ac_id);
					stringErrorMsg.append(" is already assigned.");

					jniEnv->ThrowNew(exClass, stringErrorMsg.c_str());
				}
			}
		} else {
			map_aircraft_owner.insert(pair<string, string>(string_ac_id, string_assigned_auth_id));
		}
	}
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_request_1groundVehicle
  (JNIEnv *jniEnv, jobject jobj, jstring j_assigned_auth_id, jstring j_gv_id) {
	const char *c_assigned_auth_id = (char*)jniEnv->GetStringUTFChars( j_assigned_auth_id, 0 );
	const char *c_gv_id = (char*)jniEnv->GetStringUTFChars( j_gv_id, 0 );

    char className[strlen("java/rmi/RemoteException")+1];

	jclass exClass;

	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	exClass = jniEnv->FindClass(className);

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	string string_assigned_auth_id(c_assigned_auth_id);
	string string_gv_id(c_gv_id);

	string stringErrorMsg;

	if (map_VehicleId_Seq.find(string_gv_id) == map_VehicleId_Seq.end()) {
		if (exClass != NULL) {
			stringErrorMsg.assign("The ground vehicle id ");
			stringErrorMsg.append(string_gv_id);
			stringErrorMsg.append(" does not exist.");

			jniEnv->ThrowNew(exClass, stringErrorMsg.c_str());
		}
	} else {
		if (map_groundVehicle_owner.find(string_gv_id) != map_groundVehicle_owner.end()) {
			string tmpOwner(map_groundVehicle_owner.at(string_gv_id));

			if (string_assigned_auth_id.find(tmpOwner) == string::npos) {
				if (exClass != NULL) {
					stringErrorMsg.assign("The ground vehicle ");
					stringErrorMsg.append(string_gv_id);
					stringErrorMsg.append(" is already assigned.");

					jniEnv->ThrowNew(exClass, stringErrorMsg.c_str());
				}
			}
		} else {
			map_groundVehicle_owner.insert(pair<string, string>(string_gv_id, string_assigned_auth_id));
		}
	}
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_externalAircraft_1create_1trajectory_1profile
  (JNIEnv *jniEnv, jobject jobj, jstring j_userName, jstring j_ac_id, jstring j_ac_type, jstring j_origin_airport, jstring j_destination_airport, jfloat j_cruise_altitude_ft, jfloat j_cruise_tas_knots, jdouble j_latitude_deg, jdouble j_longitude_deg, jdouble j_altitude_ft, jdouble j_rocd_fps, jdouble j_tas_knots, jdouble j_course_deg, jstring j_flight_phase) {
	const char *c_userName = (char*)jniEnv->GetStringUTFChars( j_userName, 0 );
	const char *c_ac_id = (char*)jniEnv->GetStringUTFChars( j_ac_id, 0 );
	const char *c_ac_type = (char*)jniEnv->GetStringUTFChars( j_ac_type, 0 );
	const char *c_origin_airport = (char*)jniEnv->GetStringUTFChars( j_origin_airport, 0 );
	const char *c_destination_airport = (char*)jniEnv->GetStringUTFChars( j_destination_airport, 0 );
	const float c_start_time = 0;
	const float c_cruise_altitude_ft = (float)j_cruise_altitude_ft;
	const float c_cruise_tas_knots = (float)j_cruise_tas_knots;
	float c_origin_airport_elevation_ft = 0.0;
	float c_destination_airport_elevation_ft = 0.0;

	string c_string_userName(c_userName);

	string c_string_origin_airport(c_origin_airport);
	c_origin_airport_elevation_ft = get_airport_elevation(c_string_origin_airport);

	string c_string_destination_airport(c_destination_airport);
	c_destination_airport_elevation_ft = get_airport_elevation(c_string_destination_airport);

	string c_string_acid(c_ac_id);

	if (map_Acid_FlightSeq.find(c_string_acid) != map_Acid_FlightSeq.end()) {
		char className[strlen("java/rmi/RemoteException")+1];

		jclass exClass;

		strcpy(className, "java/rmi/RemoteException");
		className[strlen("java/rmi/RemoteException")] = '\0';

		exClass = jniEnv->FindClass(className);
		if (exClass != NULL) {
			jniEnv->ThrowNew(exClass, "The same aircraft id already existed.");
		}
	} else {
		int size_of_trajectories = map_Acid_FlightSeq.size();

		newMU_external_aircraft.resize(size_of_trajectories+1);

		newMU_external_aircraft.flag_external_aircraft.at(size_of_trajectories) = true;
		newMU_external_aircraft.flag_data_initialized.at(size_of_trajectories) = false;
		newMU_external_aircraft.adb_aircraft_type_index.at(size_of_trajectories) = get_adb_table_index(string(c_ac_type));
		newMU_external_aircraft.departure_time_sec.at(size_of_trajectories) = c_start_time;
		newMU_external_aircraft.cruise_alt_ft.at(size_of_trajectories) = c_cruise_altitude_ft;
		newMU_external_aircraft.cruise_tas_knots.at(size_of_trajectories) = c_cruise_tas_knots;
		newMU_external_aircraft.origin_airport_elevation_ft.at(size_of_trajectories) = c_origin_airport_elevation_ft;
		newMU_external_aircraft.destination_airport_elevation_ft.at(size_of_trajectories) = c_destination_airport_elevation_ft;

		map_Acid_FlightSeq.insert(pair<string, int>(c_string_acid, size_of_trajectories));

		g_trajectories.push_back(Trajectory(size_of_trajectories,
				c_string_acid,
				string(c_ac_type),
				string(c_origin_airport),
				string(c_destination_airport),
				c_start_time,
				1,
				1,
				c_cruise_altitude_ft,
				c_cruise_tas_knots,
				c_origin_airport_elevation_ft,
				c_destination_airport_elevation_ft,
				true
				));

		map_aircraft_owner.insert(pair<string, string>(c_string_acid, c_string_userName));

		// Handle initial state data
		double c_latitude_deg = j_latitude_deg;
		double c_longitude_deg = j_longitude_deg;
		double c_altitude_ft = j_altitude_ft;
		double c_rocd_fps = j_rocd_fps;
		double c_tas_knots = j_tas_knots;
		double c_course_deg = j_course_deg;
		double c_traj_timestamp = 0;

		double c_fpa_rad = 0;
		double c_fpa_deg = 0;
		if (c_tas_knots > 0) {
			c_fpa_rad = asin(c_rocd_fps / c_tas_knots);
			c_fpa_deg = c_fpa_rad * 180. / PI;
		}

		int tmpSimStatus = get_runtime_sim_status();
		if ((tmpSimStatus == NATS_SIMULATION_STATUS_READY) || (tmpSimStatus == NATS_SIMULATION_STATUS_ENDED)) {
			c_traj_timestamp = 0;
		} else {
			c_traj_timestamp = get_curr_sim_time();
		}

		double c_tas_knots_ground = aircraft_compute_ground_speed(c_traj_timestamp,
				c_latitude_deg,
				c_longitude_deg,
				c_altitude_ft,
				c_tas_knots * cos(c_fpa_rad),
				c_course_deg * PI / 180.);

		const char *c_str_flight_phase = (char*)jniEnv->GetStringUTFChars( j_flight_phase, 0 );
		ENUM_Flight_Phase c_flight_phase = getFlight_Phase(c_str_flight_phase);

		// Write initial state data
		int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
		if (c_flightSeq > -1) {
			newMU_external_aircraft.latitude_deg.at(c_flightSeq) = c_latitude_deg;
			newMU_external_aircraft.longitude_deg.at(c_flightSeq) = c_longitude_deg;
			newMU_external_aircraft.altitude_ft.at(c_flightSeq) = c_altitude_ft;
			newMU_external_aircraft.rocd_fps.at(c_flightSeq) = c_rocd_fps;
			newMU_external_aircraft.tas_knots.at(c_flightSeq) = c_tas_knots;
			newMU_external_aircraft.tas_knots_ground.at(c_flightSeq) = c_tas_knots_ground;
			newMU_external_aircraft.course_rad.at(c_flightSeq) = c_course_deg * PI / 180.0;
			newMU_external_aircraft.fpa_rad.at(c_flightSeq) = c_fpa_deg * PI / 180.0;
			newMU_external_aircraft.flight_phase.at(c_flightSeq) = c_flight_phase;
		}
	}
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_externalAircraft_1inject_1trajectory_1state_1data
  (JNIEnv *jniEnv, jobject jobj, jstring j_ac_id, jdouble j_latitude_deg, jdouble j_longitude_deg, jdouble j_altitude_ft, jdouble j_rocd_fps, jdouble j_tas_knots, jdouble j_course_deg, jstring j_flight_phase, jlong j_timestamp_utc_millisec) {
	jint retValue = -1;

	int sim_status = get_runtime_sim_status();
	if ((sim_status != NATS_SIMULATION_STATUS_READY) && (sim_status != NATS_SIMULATION_STATUS_ENDED)) {
		const char *c_ac_id = (char*)jniEnv->GetStringUTFChars( j_ac_id, 0 );
		double c_latitude_deg = j_latitude_deg;
		double c_longitude_deg = j_longitude_deg;
		double c_altitude_ft = j_altitude_ft;
		double c_rocd_fps = j_rocd_fps;
		double c_tas_knots = j_tas_knots;
		double c_course_deg = j_course_deg;

		double c_fpa_rad = 0;
		double c_fpa_deg = 0;
		if (c_tas_knots > 0) {
			c_fpa_rad = asin(c_rocd_fps / c_tas_knots);
			c_fpa_deg = c_fpa_rad * 180. / PI;
		}

		const char *c_str_flight_phase = (char*)jniEnv->GetStringUTFChars( j_flight_phase, 0 );
		ENUM_Flight_Phase c_flight_phase = getFlight_Phase(c_str_flight_phase);

		double c_traj_timestamp;
		long c_timestamp_utc_millisec;

		c_traj_timestamp = get_curr_sim_time();

		double c_tas_knots_ground = aircraft_compute_ground_speed(c_traj_timestamp,
				c_latitude_deg,
				c_longitude_deg,
				c_altitude_ft,
				c_tas_knots * cos(c_fpa_rad),
				c_course_deg * PI / 180.);

		string string_ac_id(c_ac_id);

		int c_flightSeq = select_flightSeq_by_aircraftId(string_ac_id);
		if (c_flightSeq > -1) {
			newMU_external_aircraft.latitude_deg.at(c_flightSeq) = c_latitude_deg;
			newMU_external_aircraft.longitude_deg.at(c_flightSeq) = c_longitude_deg;
			newMU_external_aircraft.altitude_ft.at(c_flightSeq) = c_altitude_ft;
			newMU_external_aircraft.rocd_fps.at(c_flightSeq) = c_rocd_fps;
			newMU_external_aircraft.tas_knots.at(c_flightSeq) = c_tas_knots;
			newMU_external_aircraft.tas_knots_ground.at(c_flightSeq) = c_tas_knots_ground;
			newMU_external_aircraft.course_rad.at(c_flightSeq) = c_course_deg * PI / 180.0;
			newMU_external_aircraft.fpa_rad.at(c_flightSeq) = c_fpa_deg * PI / 180.0;
			newMU_external_aircraft.flight_phase.at(c_flightSeq) = c_flight_phase;

			retValue = 0;
		}
	}

	return retValue;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftAssignee
  (JNIEnv *jniEnv, jobject jobj, jstring j_ac_id) {
	jstring retString = NULL;

	const char *c_ac_id = (char*)jniEnv->GetStringUTFChars( j_ac_id, 0 );
	string string_acid(c_ac_id);

	map<string, string>::iterator ite_map;
	ite_map = map_aircraft_owner.find(string_acid);
	if (ite_map != map_aircraft_owner.end()) {
		retString = jniEnv->NewStringUTF(ite_map->second.c_str());
	}

	return retString;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_getFlightseq_1by_1Acid
  (JNIEnv *jniEnv, jobject jobj, jstring j_ac_id) {
	jint retValue = -1;

	const char *c_ac_id = (char*)jniEnv->GetStringUTFChars( j_ac_id, 0 );
	string string_acid = c_ac_id;

	int c_flightSeq = select_flightSeq_by_aircraftId(string_acid);
	retValue = c_flightSeq;

	return retValue;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAllAircraftId
  (JNIEnv *jniEnv, jobject jobj) {
	jobjectArray retArray = NULL;

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	retArray = (jobjectArray)jniEnv->NewObjectArray(map_Acid_FlightSeq.size(), jcls, jniEnv->NewStringUTF(""));

	int i = 0;

	map<string, int>::iterator ite_map;
	for (ite_map = map_Acid_FlightSeq.begin(); ite_map != map_Acid_FlightSeq.end(); ite_map++) {
		string cstring_ac_id = ite_map->first;

		jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(cstring_ac_id.c_str()));

		i++;
	}

	return retArray;
}


JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftIds_doubleVals
  (JNIEnv *jniEnv, jobject jobj, double jni_c_minLatitude, double jni_c_maxLatitude, double jni_c_minLongitude, double jni_c_maxLongitude, double jni_c_minAltitude_ft, double jni_c_maxAltitude_ft) {
	jobjectArray retArray = NULL;

	jclass jcls_Float = jniEnv->FindClass("java/lang/Float");
	jfieldID fieldId = jniEnv->GetFieldID(jcls_Float, "value", "F");

	map<int, string> mapCollectedAircrafts;
	int idx_mapCollectedAircrafts = 0;

	if (!isnan(jni_c_minLatitude) && !isnan(jni_c_maxLatitude) && !isnan(jni_c_minLongitude) && !isnan(jni_c_maxLongitude)) {

		int c_num_flights = get_num_flights();
		// Check all flights
		// If its latitude, longitude and/or altitude are in the query range, collect it
		for (int i = 0; i < c_num_flights; i++) {
			if ((jni_c_minLatitude <= h_aircraft_soa.latitude_deg[i]) &&
					(h_aircraft_soa.latitude_deg[i] < jni_c_maxLatitude) &&
					(jni_c_minLongitude <= h_aircraft_soa.longitude_deg[i]) &&
					(h_aircraft_soa.longitude_deg[i] < jni_c_maxLongitude)) {
				if ((jni_c_minAltitude_ft >= 0) && (jni_c_maxAltitude_ft >= 0)) {
					if ((jni_c_minAltitude_ft <= h_aircraft_soa.altitude_ft[i]) &&
							(h_aircraft_soa.altitude_ft[i] < jni_c_maxAltitude_ft)) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else if (jni_c_minAltitude_ft >= 0) {
					if (jni_c_minAltitude_ft <= h_aircraft_soa.altitude_ft[i]) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else if (jni_c_maxAltitude_ft >= 0) {
					if (h_aircraft_soa.altitude_ft[i] < jni_c_maxAltitude_ft) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else {
					mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
					idx_mapCollectedAircrafts++;
				}
			}
		}

		int j = 0;
		jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
		if (idx_mapCollectedAircrafts > 0) {
			retArray = (jobjectArray)jniEnv->NewObjectArray(idx_mapCollectedAircrafts, jcls_String, jniEnv->NewStringUTF(""));
			for (std::map<int, string>::iterator it = mapCollectedAircrafts.begin(); it != mapCollectedAircrafts.end(); ++it)
			{
				jniEnv->SetObjectArrayElement(retArray, j, jniEnv->NewStringUTF(it->second.c_str()));
				j++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftIds
  (JNIEnv *jniEnv, jobject jobj, jobject jobj_minLatitude, jobject jobj_maxLatitude, jobject jobj_minLongitude, jobject jobj_maxLongitude, jobject jobj_minAltitude_ft, jobject jobj_maxAltitude_ft) {
	jobjectArray retArray = NULL;

	float jni_c_minLatitude, jni_c_maxLatitude;
	float jni_c_minLongitude, jni_c_maxLongitude;
	float jni_c_minAltitude_ft, jni_c_maxAltitude_ft;

	jclass jcls_Float = jniEnv->FindClass("java/lang/Float");
	jfieldID fieldId = jniEnv->GetFieldID(jcls_Float, "value", "F");

	map<int, string> mapCollectedAircrafts;
	int idx_mapCollectedAircrafts = 0;

	if ((jobj_minLatitude != NULL) && (jobj_maxLatitude != NULL) &&
					(jobj_minLongitude != NULL) && (jobj_maxLongitude != NULL)) {
		jni_c_minLatitude = jniEnv->GetFloatField(jobj_minLatitude, fieldId);
		jni_c_maxLatitude = jniEnv->GetFloatField(jobj_maxLatitude, fieldId);
		jni_c_minLongitude = jniEnv->GetFloatField(jobj_minLongitude, fieldId);
		jni_c_maxLongitude = jniEnv->GetFloatField(jobj_maxLongitude, fieldId);
		jni_c_minAltitude_ft = jniEnv->GetFloatField(jobj_minAltitude_ft, fieldId);
		jni_c_maxAltitude_ft = jniEnv->GetFloatField(jobj_maxAltitude_ft, fieldId);

		int c_num_flights = get_num_flights();

		// Check all flights
		// If its latitude, longitude and/or altitude are in the query range, collect it
		for (int i = 0; i < c_num_flights; i++) {
			if ((jni_c_minLatitude <= h_aircraft_soa.latitude_deg[i]) &&
					(h_aircraft_soa.latitude_deg[i] < jni_c_maxLatitude) &&
					(jni_c_minLongitude <= h_aircraft_soa.longitude_deg[i]) &&
					(h_aircraft_soa.longitude_deg[i] < jni_c_maxLongitude)) {
				if ((jni_c_minAltitude_ft >= 0) && (jni_c_maxAltitude_ft >= 0)) {
					if ((jni_c_minAltitude_ft <= h_aircraft_soa.altitude_ft[i]) &&
							(h_aircraft_soa.altitude_ft[i] < jni_c_maxAltitude_ft)) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else if (jni_c_minAltitude_ft >= 0) {
					if (jni_c_minAltitude_ft <= h_aircraft_soa.altitude_ft[i]) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else if (jni_c_maxAltitude_ft >= 0) {
					if (h_aircraft_soa.altitude_ft[i] < jni_c_maxAltitude_ft) {
						mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
						idx_mapCollectedAircrafts++;
					}
				} else {
					mapCollectedAircrafts.insert(pair<int, string>(idx_mapCollectedAircrafts, g_trx_records[i].acid));
					idx_mapCollectedAircrafts++;
				}
			}
		}

		int j = 0;
		jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
		if (idx_mapCollectedAircrafts > 0) {
			retArray = (jobjectArray)jniEnv->NewObjectArray(idx_mapCollectedAircrafts, jcls_String, jniEnv->NewStringUTF(""));
			for (std::map<int, string>::iterator it = mapCollectedAircrafts.begin(); it != mapCollectedAircrafts.end(); ++it)
			{
				jniEnv->SetObjectArrayElement(retArray, j, jniEnv->NewStringUTF(it->second.c_str()));
				j++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAssignedAircraftIds
  (JNIEnv *jniEnv, jobject jobj, jstring j_userName) {
	jobjectArray retArray = NULL;

	const char *c_userName = (char*)jniEnv->GetStringUTFChars( j_userName, 0 );
	string c_string_userName(c_userName);

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	set<string> set_assignedAircraft;

	map<string, string>::iterator ite_map;
	for (ite_map = map_aircraft_owner.begin(); ite_map != map_aircraft_owner.end(); ite_map++) {
		if (c_string_userName.find(ite_map->second) != string::npos) {
			set_assignedAircraft.insert(ite_map->first);
		}
	}

	retArray = (jobjectArray)jniEnv->NewObjectArray(set_assignedAircraft.size(), jcls, jniEnv->NewStringUTF(""));

	int tmpIdx = 0;

	set<string>::iterator ite_set;
	for (ite_set = set_assignedAircraft.begin(); ite_set != set_assignedAircraft.end(); ite_set++) {
		jniEnv->SetObjectArrayElement(retArray, tmpIdx, jniEnv->NewStringUTF(ite_set->c_str()));

		tmpIdx++;
	}

	return retArray;
}

JNIEXPORT jobject JNICALL Java_com_osi_gnats_engine_CEngine_select_1aircraft
  (JNIEnv *jniEnv, jobject jobj, jint j_sessionId, jstring j_acid) {
	jobject retObject = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string string_acid(c_acid); // Convert char* to string

	int c_sessionId = j_sessionId;

	jclass jcls = jniEnv->FindClass("com/osi/gnats/aircraft/Aircraft");
	jclass jcls_string = jniEnv->FindClass("Ljava/lang/String;");

	jfieldID fieldId_sessionId = jniEnv->GetFieldID(jcls, "sessionId", "I");

	jfieldID fieldId_acid = jniEnv->GetFieldID(jcls, "acid", "Ljava/lang/String;");
	jfieldID fieldId_flag_external_aircraft = jniEnv->GetFieldID(jcls, "flag_external_aircraft", "Z");
	jfieldID fieldId_latitude_deg = jniEnv->GetFieldID(jcls, "latitude_deg", "F");
	jfieldID fieldId_longitude_deg = jniEnv->GetFieldID(jcls, "longitude_deg", "F");
	jfieldID fieldId_altitude_ft = jniEnv->GetFieldID(jcls, "altitude_ft", "F");
	jfieldID fieldId_rocd_fps = jniEnv->GetFieldID(jcls, "rocd_fps", "F");
	jfieldID fieldId_tas_knots = jniEnv->GetFieldID(jcls, "tas_knots", "F");
	jfieldID fieldId_course_rad = jniEnv->GetFieldID(jcls, "course_rad", "F");
	jfieldID fieldId_fpa_rad = jniEnv->GetFieldID(jcls, "fpa_rad", "F");
	jfieldID fieldId_flight_phase = jniEnv->GetFieldID(jcls, "flight_phase", "I");

	jfieldID fieldId_departure_time_sec = jniEnv->GetFieldID(jcls, "departure_time_sec", "F");
	jfieldID fieldId_cruise_alt_ft = jniEnv->GetFieldID(jcls, "cruise_alt_ft", "F");
	jfieldID fieldId_cruise_tas_knots = jniEnv->GetFieldID(jcls, "cruise_tas_knots", "F");

	jfieldID fieldId_flight_plan_latitude_array = jniEnv->GetFieldID(jcls, "flight_plan_latitude_array", "[F");
	jfieldID fieldId_flight_plan_longitude_array = jniEnv->GetFieldID(jcls, "flight_plan_longitude_array", "[F");
	jfieldID fieldId_flight_plan_length = jniEnv->GetFieldID(jcls, "flight_plan_length", "I");
	jfieldID fieldId_flight_plan_waypoint_name_array = jniEnv->GetFieldID(jcls, "flight_plan_waypoint_name_array", "[Ljava/lang/String;");
	jfieldID fieldId_flight_plan_alt_desc_array = jniEnv->GetFieldID(jcls, "flight_plan_alt_desc_array", "[Ljava/lang/String;");
	jfieldID fieldId_flight_plan_alt_1_array = jniEnv->GetFieldID(jcls, "flight_plan_alt_1_array", "[D");
	jfieldID fieldId_flight_plan_alt_2_array = jniEnv->GetFieldID(jcls, "flight_plan_alt_2_array", "[D");
	jfieldID fieldId_flight_plan_speed_limit_array = jniEnv->GetFieldID(jcls, "flight_plan_speed_limit_array", "[D");
	jfieldID fieldId_flight_plan_speed_limit_desc_array = jniEnv->GetFieldID(jcls, "flight_plan_speed_limit_desc_array", "[Ljava/lang/String;");
	jfieldID fieldId_origin_airport_elevation_ft = jniEnv->GetFieldID(jcls, "origin_airport_elevation_ft", "F");
	jfieldID fieldId_destination_airport_elevation_ft = jniEnv->GetFieldID(jcls, "destination_airport_elevation_ft", "F");

	jfieldID fieldId_landed_flag = jniEnv->GetFieldID(jcls, "landed_flag", "I");

	jfieldID fieldId_target_waypoint_index = jniEnv->GetFieldID(jcls, "target_waypoint_index", "I");
	jfieldID fieldId_target_waypoint_name = jniEnv->GetFieldID(jcls, "target_waypoint_name", "Ljava/lang/String;");
	jfieldID fieldId_airborne_target_waypoint_index = jniEnv->GetFieldID(jcls, "airborne_target_waypoint_index", "I");
	jfieldID fieldId_airborne_target_waypoint_name = jniEnv->GetFieldID(jcls, "airborne_target_waypoint_name", "Ljava/lang/String;");
	jfieldID fieldId_target_altitude_ft = jniEnv->GetFieldID(jcls, "target_altitude_ft", "F");
	jfieldID fieldId_toc_index = jniEnv->GetFieldID(jcls, "toc_index", "I");
	jfieldID fieldId_tod_index = jniEnv->GetFieldID(jcls, "tod_index", "I");

	jmethodID methodId_constructor = jniEnv->GetMethodID(jcls, "<init>", "(Ljava/lang/String;)V");

	retObject = jniEnv->NewObject(jcls, methodId_constructor, j_acid);

	jniEnv->SetIntField(retObject, fieldId_sessionId, c_sessionId);

	waypoint_node_t* waypointNode_ptr = NULL;

	int tmp_target_waypoint_index = -1;

	int c_flightSeq = select_flightSeq_by_aircraftId(string_acid);

	bool c_flag_external_aircraft = false;

	int c_target_waypoint_index = -1;

	if (c_flightSeq > -1) {
		tmp_target_waypoint_index = d_aircraft_soa.target_waypoint_index[c_flightSeq];

		c_flag_external_aircraft = newMU_external_aircraft.flag_external_aircraft.at(c_flightSeq);
		jniEnv->SetBooleanField(retObject, fieldId_flag_external_aircraft, c_flag_external_aircraft);

		if (!c_flag_external_aircraft) {
			c_target_waypoint_index = d_aircraft_soa.target_waypoint_index[c_flightSeq];

			jniEnv->SetFloatField(retObject, fieldId_latitude_deg, d_aircraft_soa.latitude_deg[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_longitude_deg, d_aircraft_soa.longitude_deg[c_flightSeq]);

			jniEnv->SetFloatField(retObject, fieldId_altitude_ft, d_aircraft_soa.altitude_ft[c_flightSeq]);

			jniEnv->SetFloatField(retObject, fieldId_rocd_fps, d_aircraft_soa.rocd_fps[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_tas_knots, d_aircraft_soa.tas_knots[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_course_rad, d_aircraft_soa.course_rad[c_flightSeq]);

			jniEnv->SetFloatField(retObject, fieldId_fpa_rad, d_aircraft_soa.fpa_rad[c_flightSeq]);

			jniEnv->SetIntField(retObject, fieldId_flight_phase, d_aircraft_soa.flight_phase[c_flightSeq]);

			jniEnv->SetFloatField(retObject, fieldId_departure_time_sec, d_aircraft_soa.departure_time_sec[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_cruise_alt_ft, d_aircraft_soa.cruise_alt_ft[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_cruise_tas_knots, d_aircraft_soa.cruise_tas_knots[c_flightSeq]);

			float* tmp_lat_array_data;
			float* tmp_long_array_data;
			int tmp_flight_plan_length = array_Airborne_Flight_Plan_Waypoint_length[c_flightSeq];

			tmp_lat_array_data = (float*)malloc(tmp_flight_plan_length * sizeof(float));
			tmp_long_array_data = (float*)malloc(tmp_flight_plan_length * sizeof(float));
			jfloatArray tmp_fp_latitude_array = jniEnv->NewFloatArray(tmp_flight_plan_length);
			jfloatArray tmp_fp_longitude_array = jniEnv->NewFloatArray(tmp_flight_plan_length);
			jobjectArray tmp_waypoint_name_data = (jobjectArray)jniEnv->NewObjectArray(tmp_flight_plan_length, jcls_string, jniEnv->NewStringUTF(""));
			jobjectArray tmp_flight_plan_alt_desc_data = (jobjectArray)jniEnv->NewObjectArray(tmp_flight_plan_length, jcls_string, jniEnv->NewStringUTF(""));
			double* tmp_flight_plan_alt_1_array_data = (double*)calloc(tmp_flight_plan_length, sizeof(double));
			double* tmp_flight_plan_alt_2_array_data = (double*)calloc(tmp_flight_plan_length, sizeof(double));
			double* tmp_flight_plan_speed_limit_array_data = (double*)calloc(tmp_flight_plan_length, sizeof(double));
			jobjectArray tmp_flight_plan_speed_limit_desc_data = (jobjectArray)jniEnv->NewObjectArray(tmp_flight_plan_length, jcls_string, jniEnv->NewStringUTF(""));

			waypointNode_ptr = getWaypointNodePtr_by_flightSeq(c_flightSeq, 0);
			for (int j; j < tmp_flight_plan_length; j++) {
				if (waypointNode_ptr != NULL) {
					tmp_lat_array_data[j] = waypointNode_ptr->latitude;
					tmp_long_array_data[j] = waypointNode_ptr->longitude;
					jniEnv->SetObjectArrayElement(tmp_waypoint_name_data, j, jniEnv->NewStringUTF(waypointNode_ptr->wpname));
					jniEnv->SetObjectArrayElement(tmp_flight_plan_alt_desc_data, j, jniEnv->NewStringUTF(waypointNode_ptr->alt_desc));
					tmp_flight_plan_alt_1_array_data[j] = waypointNode_ptr->alt_1;
					tmp_flight_plan_alt_2_array_data[j] = waypointNode_ptr->alt_2;
					tmp_flight_plan_speed_limit_array_data[j] = waypointNode_ptr->speed_lim;
					jniEnv->SetObjectArrayElement(tmp_flight_plan_speed_limit_desc_data, j, jniEnv->NewStringUTF(waypointNode_ptr->spdlim_desc));

					waypointNode_ptr = waypointNode_ptr->next_node_ptr;
				}
			}
			jniEnv->SetFloatArrayRegion(tmp_fp_latitude_array, 0, tmp_flight_plan_length, tmp_lat_array_data);
			jniEnv->SetFloatArrayRegion(tmp_fp_longitude_array, 0, tmp_flight_plan_length, tmp_long_array_data);
			free(tmp_lat_array_data);
			free(tmp_long_array_data);
			jniEnv->SetObjectField(retObject, fieldId_flight_plan_latitude_array, tmp_fp_latitude_array);
			jniEnv->SetObjectField(retObject, fieldId_flight_plan_longitude_array, tmp_fp_longitude_array);

			jniEnv->SetIntField(retObject, fieldId_flight_plan_length, tmp_flight_plan_length);

			jniEnv->SetObjectField(retObject, fieldId_flight_plan_waypoint_name_array, tmp_waypoint_name_data);

			jniEnv->SetObjectField(retObject, fieldId_flight_plan_alt_desc_array, tmp_flight_plan_alt_desc_data);

			// Handle flight_plan_alt_1_array variable
			jdoubleArray tmp_flight_plan_alt_1_array = jniEnv->NewDoubleArray(tmp_flight_plan_length);

			jniEnv->SetDoubleArrayRegion(tmp_flight_plan_alt_1_array, 0 , tmp_flight_plan_length, tmp_flight_plan_alt_1_array_data);
			free(tmp_flight_plan_alt_1_array_data);
			jniEnv->SetObjectField(retObject, fieldId_flight_plan_alt_1_array, tmp_flight_plan_alt_1_array);
			// end - Handle flight_plan_alt_1_array variable

			// Handle flight_plan_alt_2_array variable
			jdoubleArray tmp_flight_plan_alt_2_array = jniEnv->NewDoubleArray(tmp_flight_plan_length);

			jniEnv->SetDoubleArrayRegion(tmp_flight_plan_alt_2_array, 0 , tmp_flight_plan_length, tmp_flight_plan_alt_2_array_data);
			free(tmp_flight_plan_alt_2_array_data);
			jniEnv->SetObjectField(retObject, fieldId_flight_plan_alt_2_array, tmp_flight_plan_alt_2_array);
			// end - Handle flight_plan_alt_2_array variable

			// Handle flight_plan_speed_limit_array variable
			jdoubleArray tmp_flight_plan_speed_limit_array = jniEnv->NewDoubleArray(tmp_flight_plan_length);

			jniEnv->SetDoubleArrayRegion(tmp_flight_plan_speed_limit_array, 0 , tmp_flight_plan_length, tmp_flight_plan_speed_limit_array_data);
			free(tmp_flight_plan_speed_limit_array_data);
			jniEnv->SetObjectField(retObject, fieldId_flight_plan_speed_limit_array, tmp_flight_plan_speed_limit_array);
			// end - Handle flight_plan_speed_limit_array variable

			jniEnv->SetObjectField(retObject, fieldId_flight_plan_speed_limit_desc_array, tmp_flight_plan_speed_limit_desc_data);

			jniEnv->SetFloatField(retObject, fieldId_origin_airport_elevation_ft, d_aircraft_soa.origin_airport_elevation_ft[c_flightSeq]);
			jniEnv->SetFloatField(retObject, fieldId_destination_airport_elevation_ft, d_aircraft_soa.destination_airport_elevation_ft[c_flightSeq]);

			jniEnv->SetIntField(retObject, fieldId_landed_flag, d_aircraft_soa.landed_flag[c_flightSeq]);

			jniEnv->SetIntField(retObject, fieldId_target_waypoint_index, c_target_waypoint_index);
			if (isFlightPhase_in_airborne(d_aircraft_soa.flight_phase[c_flightSeq])) {
				jniEnv->SetIntField(retObject, fieldId_airborne_target_waypoint_index, c_target_waypoint_index);
			} else {
				jniEnv->SetIntField(retObject, fieldId_airborne_target_waypoint_index, -1);
			}
			jniEnv->SetFloatField(retObject, fieldId_target_altitude_ft, d_aircraft_soa.target_altitude_ft[c_flightSeq]);
			jniEnv->SetIntField(retObject, fieldId_toc_index, d_aircraft_soa.toc_index[c_flightSeq]);
			jniEnv->SetIntField(retObject, fieldId_tod_index, d_aircraft_soa.tod_index[c_flightSeq]);

			waypointNode_ptr = h_aircraft_soa.target_waypoint_node_ptr[c_flightSeq];
			if ((waypointNode_ptr != NULL) && (waypointNode_ptr->wpname != NULL)) {
				jniEnv->SetObjectField(retObject, fieldId_target_waypoint_name, jniEnv->NewStringUTF(waypointNode_ptr->wpname));
			}
			if (isFlightPhase_in_airborne(d_aircraft_soa.flight_phase[c_flightSeq])) {
				jniEnv->SetObjectField(retObject, fieldId_airborne_target_waypoint_name, jniEnv->NewStringUTF(waypointNode_ptr->wpname));
			} else {
				jniEnv->SetObjectField(retObject, fieldId_airborne_target_waypoint_name, NULL);
			}
		} else {
			jniEnv->SetFloatField(retObject, fieldId_latitude_deg, newMU_external_aircraft.latitude_deg.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_longitude_deg, newMU_external_aircraft.longitude_deg.at(c_flightSeq));

			jniEnv->SetFloatField(retObject, fieldId_altitude_ft, newMU_external_aircraft.altitude_ft.at(c_flightSeq));

			jniEnv->SetFloatField(retObject, fieldId_rocd_fps, newMU_external_aircraft.rocd_fps.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_tas_knots, newMU_external_aircraft.tas_knots.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_course_rad, newMU_external_aircraft.course_rad.at(c_flightSeq));

			jniEnv->SetFloatField(retObject, fieldId_fpa_rad, newMU_external_aircraft.fpa_rad.at(c_flightSeq));

			jniEnv->SetIntField(retObject, fieldId_flight_phase, newMU_external_aircraft.flight_phase.at(c_flightSeq));

			jniEnv->SetFloatField(retObject, fieldId_departure_time_sec, newMU_external_aircraft.departure_time_sec.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_cruise_alt_ft, newMU_external_aircraft.cruise_alt_ft.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_cruise_tas_knots, newMU_external_aircraft.cruise_tas_knots.at(c_flightSeq));

			jniEnv->SetFloatField(retObject, fieldId_origin_airport_elevation_ft, newMU_external_aircraft.origin_airport_elevation_ft.at(c_flightSeq));
			jniEnv->SetFloatField(retObject, fieldId_destination_airport_elevation_ft, newMU_external_aircraft.destination_airport_elevation_ft.at(c_flightSeq));
		}
	} else {
		retObject = NULL;
	}

	jniEnv->ReleaseStringUTFChars(j_acid, c_acid);

	return retObject;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_synchronize_1aircraft_1to_1server
  (JNIEnv *jniEnv, jobject jobj, jobject jobj_aircraft) {
	int retValue = 1; // Initial value = 1

	jclass jcls = jniEnv->FindClass("com/osi/gnats/aircraft/Aircraft");
	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_String, "toString", "()Ljava/lang/String;");

	jfieldID fieldId_acid = jniEnv->GetFieldID(jcls, "acid", "Ljava/lang/String;");

	jfieldID fieldId_latitude_deg = jniEnv->GetFieldID(jcls, "latitude_deg", "F");
	jfieldID fieldId_longitude_deg = jniEnv->GetFieldID(jcls, "longitude_deg", "F");
	jfieldID fieldId_altitude_ft = jniEnv->GetFieldID(jcls, "altitude_ft", "F");
	jfieldID fieldId_rocd_fps = jniEnv->GetFieldID(jcls, "rocd_fps", "F");
	jfieldID fieldId_tas_knots = jniEnv->GetFieldID(jcls, "tas_knots", "F");
	jfieldID fieldId_course_rad = jniEnv->GetFieldID(jcls, "course_rad", "F");
	jfieldID fieldId_fpa_rad = jniEnv->GetFieldID(jcls, "fpa_rad", "F");
	jfieldID fieldId_flight_phase = jniEnv->GetFieldID(jcls, "flight_phase", "I");

	jfieldID fieldId_departure_time_sec = jniEnv->GetFieldID(jcls, "departure_time_sec", "F");
	jfieldID fieldId_cruise_alt_ft = jniEnv->GetFieldID(jcls, "cruise_alt_ft", "F");
	jfieldID fieldId_cruise_tas_knots = jniEnv->GetFieldID(jcls, "cruise_tas_knots", "F");

	jfieldID fieldId_flight_plan_length = jniEnv->GetFieldID(jcls, "flight_plan_length", "I");
	jfieldID fieldId_origin_airport_elevation_ft = jniEnv->GetFieldID(jcls, "origin_airport_elevation_ft", "F");
	jfieldID fieldId_destination_airport_elevation_ft = jniEnv->GetFieldID(jcls, "destination_airport_elevation_ft", "F");

	jfieldID fieldId_landed_flag = jniEnv->GetFieldID(jcls, "landed_flag", "I");

	jfieldID fieldId_target_waypoint_index = jniEnv->GetFieldID(jcls, "target_waypoint_index", "I");
	jfieldID fieldId_target_altitude_ft = jniEnv->GetFieldID(jcls, "target_altitude_ft", "F");
	jfieldID fieldId_toc_index = jniEnv->GetFieldID(jcls, "toc_index", "I");
	jfieldID fieldId_tod_index = jniEnv->GetFieldID(jcls, "tod_index", "I");

	jfieldID fieldId_fp_latitude_index_to_modify = jniEnv->GetFieldID(jcls, "fp_latitude_index_to_modify", "I");
	jfieldID fieldId_fp_latitude_deg_to_modify = jniEnv->GetFieldID(jcls, "fp_latitude_deg_to_modify", "F");
	jfieldID fieldId_fp_longitude_index_to_modify = jniEnv->GetFieldID(jcls, "fp_longitude_index_to_modify", "I");
	jfieldID fieldId_fp_longitude_deg_to_modify = jniEnv->GetFieldID(jcls, "fp_longitude_deg_to_modify", "F");

	jfieldID fieldId_flight_plan_waypoint_name_array = jniEnv->GetFieldID(jcls, "flight_plan_waypoint_name_array", "[Ljava/lang/String;");

	jstring jstring_acid = (jstring)jniEnv->GetObjectField(jobj_aircraft, fieldId_acid);

	const char *cTmpStr = jniEnv->GetStringUTFChars(jstring_acid, NULL);
	string string_acid = cTmpStr;

	int tmp_flight_plan_length = -1;

	int c_flightSeq = select_flightSeq_by_aircraftId(string_acid);
	
	if (c_flightSeq > -1) {
		d_aircraft_soa.latitude_deg[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_latitude_deg);
		d_aircraft_soa.longitude_deg[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_longitude_deg);
		d_aircraft_soa.altitude_ft[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_altitude_ft);
		d_aircraft_soa.rocd_fps[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_rocd_fps);
		d_aircraft_soa.tas_knots[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_tas_knots);
		d_aircraft_soa.course_rad[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_course_rad);
		d_aircraft_soa.fpa_rad[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_fpa_rad);

		int c_int_flight_phase = jniEnv->GetIntField(jobj_aircraft, fieldId_flight_phase);
		d_aircraft_soa.flight_phase[c_flightSeq] = getFlightPhase(c_int_flight_phase);

		d_aircraft_soa.cruise_alt_ft[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_cruise_alt_ft);
		d_aircraft_soa.cruise_tas_knots[c_flightSeq] = jniEnv->GetFloatField(jobj_aircraft, fieldId_cruise_tas_knots);

		tmp_flight_plan_length = array_Airborne_Flight_Plan_Waypoint_length[c_flightSeq];

		d_aircraft_soa.toc_index[c_flightSeq] = jniEnv->GetIntField(jobj_aircraft, fieldId_toc_index);
		d_aircraft_soa.tod_index[c_flightSeq] = jniEnv->GetIntField(jobj_aircraft, fieldId_tod_index);

		int tmp_fp_latitude_index_to_modify = jniEnv->GetIntField(jobj_aircraft, fieldId_fp_latitude_index_to_modify);
		if ((tmp_fp_latitude_index_to_modify > -1) && (tmp_fp_latitude_index_to_modify+1 <= tmp_flight_plan_length)) {
			float new_flight_plan_latitude_deg = jniEnv->GetFloatField(jobj_aircraft, fieldId_fp_latitude_deg_to_modify);

			waypoint_node_t* tmp_waypoint_node_ptr = array_Airborne_Flight_Plan_ptr[c_flightSeq];
			if (tmp_waypoint_node_ptr != NULL) {
				int cnt_waypoint_increment = 0;

				while ((tmp_waypoint_node_ptr != NULL) && (cnt_waypoint_increment < tmp_fp_latitude_index_to_modify)) {
					tmp_waypoint_node_ptr = tmp_waypoint_node_ptr->next_node_ptr;

					cnt_waypoint_increment++;
				}
			}

			// If flight plan waypoint latitude changes, set waypoint name to empty string
			if ((tmp_waypoint_node_ptr != NULL) && (tmp_waypoint_node_ptr->latitude != new_flight_plan_latitude_deg)) {
					if (tmp_waypoint_node_ptr->wpname != NULL) {
						free(tmp_waypoint_node_ptr->wpname);

						tmp_waypoint_node_ptr->wpname = (char*)malloc(2 * sizeof(char));
						strcpy(tmp_waypoint_node_ptr->wpname, "");
						tmp_waypoint_node_ptr->wpname[1] = '\0';
					}

				tmp_waypoint_node_ptr->latitude = new_flight_plan_latitude_deg;
			}
		}

		jniEnv->SetIntField(jobj_aircraft, fieldId_fp_latitude_index_to_modify, (jint)-1); // Reset
		jniEnv->SetFloatField(jobj_aircraft, fieldId_fp_latitude_deg_to_modify, (jfloat)0.0); // Reset

		int tmp_fp_longitude_index_to_modify = jniEnv->GetIntField(jobj_aircraft, fieldId_fp_longitude_index_to_modify);
		if ((tmp_fp_longitude_index_to_modify > -1) && (tmp_fp_longitude_index_to_modify+1 <= tmp_flight_plan_length)) {
			float new_flight_plan_longitude_deg = jniEnv->GetFloatField(jobj_aircraft, fieldId_fp_longitude_deg_to_modify);

			waypoint_node_t* tmp_waypoint_node_ptr = array_Airborne_Flight_Plan_ptr[c_flightSeq];
			if (tmp_waypoint_node_ptr != NULL) {
				int cnt_waypoint_increment = 0;

				while ((tmp_waypoint_node_ptr != NULL) && (cnt_waypoint_increment < tmp_fp_longitude_index_to_modify)) {
					tmp_waypoint_node_ptr = tmp_waypoint_node_ptr->next_node_ptr;

					cnt_waypoint_increment++;
				}
			}

			// If flight plan waypoint longitude changes, set waypoint name to empty string
			if ((tmp_waypoint_node_ptr != NULL) && (tmp_waypoint_node_ptr->longitude != new_flight_plan_longitude_deg)) {
				if (tmp_waypoint_node_ptr->wpname != NULL) {
					free(tmp_waypoint_node_ptr->wpname);

					tmp_waypoint_node_ptr->wpname = (char*)malloc(2 * sizeof(char));
					strcpy(tmp_waypoint_node_ptr->wpname, "");
					tmp_waypoint_node_ptr->wpname[1] = '\0';
				}

				tmp_waypoint_node_ptr->longitude = new_flight_plan_longitude_deg;
			}
		}

		jniEnv->SetIntField(jobj_aircraft, fieldId_fp_longitude_index_to_modify, (jint)-1); // Reset
		jniEnv->SetFloatField(jobj_aircraft, fieldId_fp_longitude_deg_to_modify, (jfloat)0.0); // Reset

		retValue = 0;
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_delay_1departure
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jint j_seconds) {
	jint retValue = 1; // Initial value.  1 means error.

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int c_seconds = j_seconds;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	int sim_status = get_runtime_sim_status();

	if (!(
		  ((sim_status == NATS_SIMULATION_STATUS_START)
				|| (sim_status == NATS_SIMULATION_STATUS_PAUSE)
				|| (sim_status == NATS_SIMULATION_STATUS_RESUME)
				|| (sim_status == NATS_SIMULATION_STATUS_STOP))
			&&
		  (get_curr_sim_time() >= d_aircraft_soa.departure_time_sec[c_flightSeq])
		 )) {
		int tmp_departure_time_sec = d_aircraft_soa.departure_time_sec[c_flightSeq];
		tmp_departure_time_sec += c_seconds;

		h_aircraft_soa.departure_time_sec[c_flightSeq] = tmp_departure_time_sec;
		d_aircraft_soa.departure_time_sec[c_flightSeq] = tmp_departure_time_sec;

		retValue = 0;
	}

	return retValue;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getADB_1cruiseTas
  (JNIEnv *jniEnv, jobject jobj, jstring j_ac_type, jdouble j_altitude_ft) {
	jdouble retValue = 0;

	char className[strlen("java/rmi/RemoteException")+1];
	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	jclass exClass = jniEnv->FindClass(className);

	AdbPTFModel adbPTFModel;

	const char *c_ac_type = (char*)jniEnv->GetStringUTFChars( j_ac_type, NULL );
	string string_ac_type(c_ac_type);
	if (g_adb_indices.find(string_ac_type) == g_adb_indices.end()) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Aircraft type is not valid.");
	    }
	}

	int adb_index = g_adb_indices.at(string_ac_type);
	adbPTFModel = g_adb_ptf_models.at(adb_index);

	double c_altitude_ft = j_altitude_ft;
	if (c_altitude_ft < 0) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Altitude value is not valid.");
	    }
	}

	retValue = adbPTFModel.getCruiseTas(c_altitude_ft);

	return retValue;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getADB_1climb_1descent_1Rate
  (JNIEnv *jniEnv, jobject jobj, jboolean j_flag_climbing, jstring j_ac_type, jdouble j_flight_level, jstring j_adb_mass) {
	jdouble retValue = 0;

	char className[strlen("java/rmi/RemoteException")+1];
	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	jclass exClass = jniEnv->FindClass(className);

	AdbPTFModel adbPTFModel;

	bool c_flag_climbing = j_flag_climbing;

	const char *c_ac_type = (char*)jniEnv->GetStringUTFChars( j_ac_type, NULL );
	string string_ac_type(c_ac_type);
	if (g_adb_indices.find(string_ac_type) == g_adb_indices.end()) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Aircraft type is not valid.");
	    }
	}

	int adb_index = g_adb_indices.at(string_ac_type);
	adbPTFModel = g_adb_ptf_models.at(adb_index);

	double c_flight_level = j_flight_level;

	if (c_flight_level < 0) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Flight level is not valid.");
	    }
	}

	const char *c_tmp_adb_mass = (char*)jniEnv->GetStringUTFChars( j_adb_mass, NULL );
	char* c_adb_mass = (char*)malloc(strlen(c_tmp_adb_mass) * sizeof(char));
	strcpy(c_adb_mass, c_tmp_adb_mass);
	toUppercase(c_adb_mass);

	if (strcmp(c_adb_mass, "LOW") == 0) {
		if (c_flag_climbing) {
			retValue = adbPTFModel.getClimbRate(c_flight_level, LOW);
		} else {
			retValue = adbPTFModel.getDescentRate(c_flight_level, LOW);
		}
	} else if (strcmp(c_adb_mass, "NOMINAL") == 0) {
		if (c_flag_climbing) {
			retValue = adbPTFModel.getClimbRate(c_flight_level, NOMINAL);
		} else {
			retValue = adbPTFModel.getDescentRate(c_flight_level, NOMINAL);
		}
	} else if (strcmp(c_adb_mass, "HIGH") == 0) {
		if (c_flag_climbing) {
			retValue = adbPTFModel.getClimbRate(c_flight_level, HIGH);
		} else {
			retValue = adbPTFModel.getDescentRate(c_flight_level, HIGH);
		}
	} else {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "ADB mass value can only be LOW, NOMINAL, HIGH.");
	    }
	}

	free(c_adb_mass);

	return retValue;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getADB_1climb_1descent_1Tas
  (JNIEnv *jniEnv, jobject jobj, jboolean j_flag_climbing, jstring j_ac_type, jdouble j_altitude_ft) {
	jdouble retValue = 0;

	char className[strlen("java/rmi/RemoteException")+1];
	strcpy(className, "java/rmi/RemoteException");
	className[strlen("java/rmi/RemoteException")] = '\0';

	jclass exClass = jniEnv->FindClass(className);

	AdbPTFModel adbPTFModel;

	bool c_flag_climbing = j_flag_climbing;

	const char *c_ac_type = (char*)jniEnv->GetStringUTFChars( j_ac_type, NULL );
	string string_ac_type(c_ac_type);
	if (g_adb_indices.find(string_ac_type) == g_adb_indices.end()) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Aircraft type is not valid.");
	    }
	}

	int adb_index = g_adb_indices.at(string_ac_type);
	adbPTFModel = g_adb_ptf_models.at(adb_index);

	double c_altitude_ft = j_altitude_ft;
	if (c_altitude_ft < 0) {
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Altitude value is not valid.");
	    }
	}

	if (c_flag_climbing) {
		retValue = adbPTFModel.getClimbTas(c_altitude_ft);
	} else {
		retValue = adbPTFModel.getDescentTas(c_altitude_ft);
	}

	return retValue;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getCurrentSidStarApproach
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_proc_type) {
	jstring retString = NULL;

	string tmp_ret_str = "None"; // Default
	char* tmp_char_array;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );

	FlightPlan curFlightPlan;
	vector<PointWGS84> curRoute;
	string c_string_sid_name;
	string c_string_star_name;
	string c_string_approach_name;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		curFlightPlan = g_flightplans.at(c_flightSeq);

		curRoute = curFlightPlan.route;
		if (curRoute.size() > 0) {
			for (int i = 0; i < curRoute.size(); i++) {
				if (strcmp("SID", c_proc_type) == 0) {
					if (strcmp("SID", curRoute.at(i).proctype.c_str()) == 0) {
						c_string_sid_name = curRoute.at(i).procname;
						c_string_sid_name = trim(c_string_sid_name);
						if (c_string_sid_name.size() > 0) {
							tmp_ret_str = c_string_sid_name;

							break;
						}
					}
				} else if (strcmp("STAR", c_proc_type) == 0) {
					if (strcmp("STAR", curRoute.at(i).proctype.c_str()) == 0) {
						c_string_star_name = curRoute.at(i).procname;
						c_string_star_name = trim(c_string_star_name);
						if (c_string_star_name.size() > 0) {
							tmp_ret_str = c_string_star_name;

							break;
						}
					}
				} else if (strcmp("APPROACH", c_proc_type) == 0) {
					if (strcmp("APPROACH", curRoute.at(i).proctype.c_str()) == 0) {
						c_string_approach_name = curRoute.at(i).procname;
						c_string_approach_name = trim(c_string_approach_name);
						if (c_string_approach_name.size() > 0) {
							tmp_ret_str = c_string_approach_name;

							break;
						}
					}
				}
			}
		}
	} else {
		tmp_ret_str = "Error: Can not find aircraft: ";
		tmp_ret_str.append(c_acid);
	}

	retString = jniEnv->NewStringUTF(tmp_ret_str.c_str());

	return retString;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAllSidsStarsApproaches
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code, jstring j_proc_type) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	vector<NatsApproach*> avail_approaches;
	vector<NatsSid*> avail_sids;
	vector<NatsStar*> avail_stars;

	if (map_airport.find(c_string_airport_code) != map_airport.end()) {
		NatsAirport* cur_airport_ptr = map_airport.at(c_string_airport_code);
		if (strcmp("SID", c_proc_type) == 0) {
			avail_sids = cur_airport_ptr->avail_sids;
			if (avail_sids.size() > 0) {
				retArray = (jobjectArray)jniEnv->NewObjectArray(avail_sids.size(), jcls_String, jniEnv->NewStringUTF(""));
				for (int i = 0; i < avail_sids.size(); ++i)
				{
					jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(avail_sids.at(i)->name.c_str()));
				}
			}
		} else if (strcmp("STAR", c_proc_type) == 0) {
			avail_stars = cur_airport_ptr->avail_stars;
			if (avail_stars.size() > 0) {
				retArray = (jobjectArray)jniEnv->NewObjectArray(avail_stars.size(), jcls_String, jniEnv->NewStringUTF(""));
				for (int i = 0; i < avail_stars.size(); ++i)
				{
					jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(avail_stars.at(i)->name.c_str()));
				}
			}
		} else if (strcmp("APPROACH", c_proc_type) == 0) {
			avail_approaches = cur_airport_ptr->avail_approaches;
			if (avail_approaches.size() > 0) {
				retArray = (jobjectArray)jniEnv->NewObjectArray(avail_approaches.size(), jcls_String, jniEnv->NewStringUTF(""));
				for (int i = 0; i < avail_approaches.size(); ++i)
				{
					jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(avail_approaches.at(i)->name.c_str()));
				}
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1leg_1names
  (JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (map<string, vector<string> >::iterator iter = cur_proc.wp_map.begin(); iter != cur_proc.wp_map.end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(iter->first.c_str()));

				i++;
			}
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (map<string, vector<string> >::iterator iter = cur_proc.wp_map.begin(); iter != cur_proc.wp_map.end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(iter->first.c_str()));

				i++;
			}
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (map<string, vector<string> >::iterator iter = cur_proc.wp_map.begin(); iter != cur_proc.wp_map.end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(iter->first.c_str()));

				i++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getWaypoints_1in_1procedure_1leg
  (JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name) {
	jobjectArray retArray = NULL;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.at(c_string_proc_leg_name).size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (vector<string>::iterator iter = cur_proc.wp_map.at(c_string_proc_leg_name).begin(); iter != cur_proc.wp_map.at(c_string_proc_leg_name).end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF((*iter).c_str()));

				i++;
			}
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.at(c_string_proc_leg_name).size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (vector<string>::iterator iter = cur_proc.wp_map.at(c_string_proc_leg_name).begin(); iter != cur_proc.wp_map.at(c_string_proc_leg_name).end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF((*iter).c_str()));

				i++;
			}
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_proc.wp_map.at(c_string_proc_leg_name).size(), jcls_String, jniEnv->NewStringUTF(""));
			int i = 0;
			for (vector<string>::iterator iter = cur_proc.wp_map.at(c_string_proc_leg_name).begin(); iter != cur_proc.wp_map.at(c_string_proc_leg_name).end(); ++iter) {
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF((*iter).c_str()));

				i++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getWaypoint_1Latitude_1Longitude_1deg
  (JNIEnv *jniEnv, jobject jobj, jstring j_waypoint_name) {
	jdoubleArray retArray = NULL;

	const char *c_waypoint_name = (char*)jniEnv->GetStringUTFChars( j_waypoint_name, NULL );
	string c_string_waypoint_name = c_waypoint_name;

	NatsWaypoint wp;
	wp.name = c_string_waypoint_name;

	vector<NatsWaypoint>::iterator itwp = find(g_waypoints.begin(), g_waypoints.end(), wp);
	if (itwp != g_waypoints.end()) {
		retArray = jniEnv->NewDoubleArray(2);

		jdouble tmp_array[2];
		tmp_array[0] = itwp->latitude;
		tmp_array[1] = itwp->longitude;

		jniEnv->SetDoubleArrayRegion(retArray, 0 , 2, tmp_array);
	} else {
		NatsAirport ap;
		ap.code = c_string_waypoint_name;

		vector<NatsAirport>::iterator itap = find(g_airports.begin(), g_airports.end(), ap);
		if (itap != g_airports.end()){
			retArray = jniEnv->NewDoubleArray(2);

			jdouble tmp_array[2];
			tmp_array[0] = itwp->latitude;
			tmp_array[1] = itwp->longitude;

			jniEnv->SetDoubleArrayRegion(retArray, 0 , 2, tmp_array);
		}
	}

	return retArray;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1alt_11
  (JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name, jstring j_proc_wp_name) {
	jdouble retValue = -1.0;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	const char *c_proc_wp_name = (char*)jniEnv->GetStringUTFChars( j_proc_wp_name, NULL );
	string c_string_proc_wp_name = c_proc_wp_name;

	map<string, vector<pair<string,double> > > tmpMap_alt_1;

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			tmpMap_alt_1 = cur_proc.alt_1;
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			tmpMap_alt_1 = cur_proc.alt_1;
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			tmpMap_alt_1 = cur_proc.alt_1;
		}
	}

	if (tmpMap_alt_1.size() > 0) {
		vector<pair<string,double> > tmpVector = tmpMap_alt_1.at(c_string_proc_leg_name);
		if (tmpVector.size() > 0) {
			for (unsigned int i = 0; i < tmpVector.size(); i++) {
				if (tmpVector.at(i).first.compare(c_string_proc_wp_name) == 0) {
					retValue = tmpVector.at(i).second;
					break;
				}
			}
		}
	}

	return retValue;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1alt_12
  (JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name, jstring j_proc_wp_name) {
	jdouble retValue = -1.0;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	const char *c_proc_wp_name = (char*)jniEnv->GetStringUTFChars( j_proc_wp_name, NULL );
	string c_string_proc_wp_name = c_proc_wp_name;

	map<string, vector<pair<string,double> > > tmpMap_alt_2;

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			tmpMap_alt_2 = cur_proc.alt_2;
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			tmpMap_alt_2 = cur_proc.alt_2;
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			tmpMap_alt_2 = cur_proc.alt_2;
		}
	}

	if (tmpMap_alt_2.size() > 0) {
		vector<pair<string,double> > tmpVector = tmpMap_alt_2.at(c_string_proc_leg_name);
		if (tmpVector.size() > 0) {
			for (unsigned int i = 0; i < tmpVector.size(); i++) {
				if (tmpVector.at(i).first.compare(c_string_proc_wp_name) == 0) {
					retValue = tmpVector.at(i).second;
					break;
				}
			}
		}
	}

	return retValue;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1speed_1limit
  (JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name, jstring j_proc_wp_name) {
	jdouble retValue = -1.0;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	const char *c_proc_wp_name = (char*)jniEnv->GetStringUTFChars( j_proc_wp_name, NULL );
	string c_string_proc_wp_name = c_proc_wp_name;

	map<string, vector<pair<string,double> > > tmpMap_spd_limit;

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			tmpMap_spd_limit = cur_proc.spd_limit;
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			tmpMap_spd_limit = cur_proc.spd_limit;
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			tmpMap_spd_limit = cur_proc.spd_limit;
		}
	}

	if (tmpMap_spd_limit.size() > 0) {
		vector<pair<string,double> > tmpVector = tmpMap_spd_limit.at(c_string_proc_leg_name);
		if (tmpVector.size() > 0) {
			for (unsigned int i = 0; i < tmpVector.size(); i++) {
				if (tmpVector.at(i).first.compare(c_string_proc_wp_name) == 0) {
					retValue = tmpVector.at(i).second;
					break;
				}
			}
		}
	}

	return retValue;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1alt_1desc
	(JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name, jstring j_proc_wp_name) {
	jstring retString = NULL;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	const char *c_proc_wp_name = (char*)jniEnv->GetStringUTFChars( j_proc_wp_name, NULL );
	string c_string_proc_wp_name = c_proc_wp_name;

	map<string, vector<pair<string,string> > > tmpMap_alt_desc;

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			tmpMap_alt_desc = cur_proc.alt_desc;
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			tmpMap_alt_desc = cur_proc.alt_desc;
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			tmpMap_alt_desc = cur_proc.alt_desc;
		}
	}

	if (tmpMap_alt_desc.size() > 0) {
		vector<pair<string,string> > tmpVector = tmpMap_alt_desc.at(c_string_proc_leg_name);
		if (tmpVector.size() > 0) {
			for (unsigned int i = 0; i < tmpVector.size(); i++) {
				if (tmpVector.at(i).first.compare(c_string_proc_wp_name) == 0) {
					retString = jniEnv->NewStringUTF(tmpVector.at(i).second.c_str());;
					break;
				}
			}
		}
	}

	return retString;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getProcedure_1speed_1limit_1desc
	(JNIEnv *jniEnv, jobject jobj, jstring j_proc_type, jstring j_proc_name, jstring j_airport_code, jstring j_proc_leg_name, jstring j_proc_wp_name) {
	jstring retString = NULL;

	const char *c_proc_type = (char*)jniEnv->GetStringUTFChars( j_proc_type, NULL );
	string c_string_proc_type = c_proc_type;

	const char *c_proc_name = (char*)jniEnv->GetStringUTFChars( j_proc_name, NULL );
	string c_string_proc_name = c_proc_name;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_proc_leg_name = (char*)jniEnv->GetStringUTFChars( j_proc_leg_name, NULL );
	string c_string_proc_leg_name = c_proc_leg_name;

	const char *c_proc_wp_name = (char*)jniEnv->GetStringUTFChars( j_proc_wp_name, NULL );
	string c_string_proc_wp_name = c_proc_wp_name;

	map<string, vector<pair<string,string> > > tmpMap_spdlim_desc;

	if (strcmp("SID", c_proc_type) == 0) { // SID
		NatsSid key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsSid>::const_iterator it = find(g_sids.begin(), g_sids.end(), key_proc);
		NatsSid cur_proc;
		if (it != g_sids.end()) {
			cur_proc = *it;

			tmpMap_spdlim_desc = cur_proc.spdlim_desc;
		}
	} else if (strcmp("STAR", c_proc_type) == 0) { // STAR
		NatsStar key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsStar>::const_iterator it = find(g_stars.begin(), g_stars.end(), key_proc);
		NatsStar cur_proc;
		if (it != g_stars.end()) {
			cur_proc = *it;

			tmpMap_spdlim_desc = cur_proc.spdlim_desc;
		}
	} else if (strcmp("APPROACH", c_proc_type) == 0) { // APPROACH
		NatsApproach key_proc;
		key_proc.id = c_string_airport_code;
		key_proc.name = c_string_proc_name;

		std::vector<NatsApproach>::const_iterator it = find(g_approaches.begin(), g_approaches.end(), key_proc);
		NatsApproach cur_proc;
		if (it != g_approaches.end()) {
			cur_proc = *it;

			tmpMap_spdlim_desc = cur_proc.spdlim_desc;
		}
	}

	if (tmpMap_spdlim_desc.size() > 0) {
		vector<pair<string,string> > tmpVector = tmpMap_spdlim_desc.at(c_string_proc_leg_name);
		if (tmpVector.size() > 0) {
			for (unsigned int i = 0; i < tmpVector.size(); i++) {
				if (tmpVector.at(i).first.compare(c_string_proc_wp_name) == 0) {
					retString = jniEnv->NewStringUTF(tmpVector.at(i).second.c_str());;
					break;
				}
			}
		}
	}

	return retString;
}

JNIEXPORT jfloatArray JNICALL Java_com_osi_gnats_engine_CEngine_getGroundWaypointLocation
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring groundWaypointId) {
	jfloatArray retArray = jniEnv->NewFloatArray(2);

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( airportId, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string

	const char *c_groundWaypoint_id = (char*)jniEnv->GetStringUTFChars( groundWaypointId, NULL );
	string c_string_groundWaypointId = c_groundWaypoint_id; // Convert char* to string

	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;
	float tmpArray[2];
	if (map_ground_waypoint_connectivity.find(c_string_airport_code) != map_ground_waypoint_connectivity.end()) {
			cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(c_string_airport_code);
			cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;
			tmpArray[0] = cur_map_waypoint_node[c_string_groundWaypointId].latitude;
			tmpArray[1] = cur_map_waypoint_node[c_string_groundWaypointId].longitude;
			jniEnv->SetFloatArrayRegion(retArray, 0, 2, tmpArray);
	}
	return retArray;
}


JNIEXPORT jobject JNICALL Java_com_osi_gnats_engine_CEngine_select_1airport
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jobject retObject = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string
	string c_string_airport_name;
	double c_airport_latitude;
	double c_airport_longitude;
	double c_airport_elevation;

	jstring j_airport_name;
	jdouble j_airport_latitude;
	jdouble j_airport_longitude;
	jdouble j_airport_elevation;

	jclass jcls = jniEnv->FindClass("com/osi/gnats/airport/Airport");

	if (map_airport.find(c_string_airport_code) != map_airport.end()) {
		NatsAirport* cur_airport_ptr = map_airport.at(c_string_airport_code);
		c_string_airport_name = cur_airport_ptr->name;
		c_airport_latitude = cur_airport_ptr->latitude;
		c_airport_longitude = cur_airport_ptr->longitude;
		c_airport_elevation = cur_airport_ptr->elevation;

		j_airport_name = jniEnv->NewStringUTF(c_string_airport_name.c_str());
		j_airport_latitude = c_airport_latitude;
		j_airport_longitude = c_airport_longitude;
		j_airport_elevation = c_airport_elevation;

		jmethodID methodId_constructor = jniEnv->GetMethodID(jcls, "<init>", "(Ljava/lang/String;Ljava/lang/String;DDD)V"); // The method signature must be a consecutive string without empty spaces

		retObject = jniEnv->NewObject(jcls, methodId_constructor, j_airport_code, j_airport_name, j_airport_latitude, j_airport_longitude, j_airport_elevation);
	}

	return retObject;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getArrivalAirport
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid) {
	jstring retString = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		string tmpAirportCode;

		tmpAirportCode.assign(g_flightplans.at(c_flightSeq).destination);
		if (tmpAirportCode.length() < 4) {
			if ((tmpAirportCode.find("ANC") != string::npos) || (tmpAirportCode.find("HNL") != string::npos)) {
				tmpAirportCode.insert(0, "P");
			} else {
				tmpAirportCode.insert(0, "K");
			}
		}
		retString = jniEnv->NewStringUTF(tmpAirportCode.c_str());
	}

	return retString;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getDepartureAirport
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid) {
	jstring retString = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		string tmpAirportCode;

		tmpAirportCode.assign(g_flightplans.at(c_flightSeq).origin);
		if (tmpAirportCode.length() < 4) {
			if ((tmpAirportCode.find("ANC") != string::npos) || (tmpAirportCode.find("HNL") != string::npos)) {
				tmpAirportCode.insert(0, "P");
			} else {
				tmpAirportCode.insert(0, "K");
			}
		}
		retString = jniEnv->NewStringUTF(tmpAirportCode.c_str());
	}

	return retString;
}

JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getLocation
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jdoubleArray retArray = jniEnv->NewDoubleArray(2);

	jdouble tmp_array[2];

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	if (map_airport.find(c_string_airport_code) != map_airport.end()) {
		NatsAirport* cur_airport_ptr = map_airport.at(c_string_airport_code);
		tmp_array[0] = cur_airport_ptr->latitude;
		tmp_array[1] = cur_airport_ptr->longitude;

		jniEnv->SetDoubleArrayRegion(retArray, 0 , 2, tmp_array);
	}

	return retArray;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getClosestAirport
  (JNIEnv *jniEnv, jobject jobj, jdouble j_latitude, jdouble j_longitude) {
	jstring retString = NULL;

	double c_latitude = j_latitude;
	double c_longitude = j_longitude;

	string tmp_closest_airport;
	double tmp_min_distance = -1; // Reset

	// Check all airports in map_airport
	map<string, NatsAirport*>::iterator ite;
	for (ite = map_airport.begin(); ite != map_airport.end(); ite++) {
		NatsAirport* cur_airport_ptr = ite->second;
		// Calculate distance to the airport
		double cur_dist_to_airport = compute_distance_gc(c_latitude, c_longitude, cur_airport_ptr->latitude, cur_airport_ptr->longitude, 0);

		if (tmp_min_distance == -1) {
			tmp_min_distance = cur_dist_to_airport;
		} else if (cur_dist_to_airport < tmp_min_distance) {
			tmp_min_distance = cur_dist_to_airport; // Update
			tmp_closest_airport = ite->first;
		}
	}

	retString = jniEnv->NewStringUTF(tmp_closest_airport.c_str());

	return retString;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAirportsWithinMiles
  (JNIEnv *jniEnv, jobject jobj, jdouble j_latitude, jdouble j_longitude, jdouble j_miles) {
	jobjectArray retArray = NULL;

	double c_latitude = j_latitude;
	double c_longitude = j_longitude;
	double c_miles = j_miles;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	int count = 0;
	vector<string> collectedAirports;

	if (c_miles > 0) {
		double rangeFeet = 5280 * c_miles;

		// Check all airports in map_airport
		map<string, NatsAirport*>::iterator ite;
		for (ite = map_airport.begin(); ite != map_airport.end(); ite++) {
			NatsAirport* cur_airport_ptr = ite->second;
			// Calculate distance to the airport
			double cur_dist_to_airport = compute_distance_gc(c_latitude, c_longitude, cur_airport_ptr->latitude, cur_airport_ptr->longitude, 0);
			if (cur_dist_to_airport <= rangeFeet) {
				collectedAirports.push_back(ite->first);
			}
		}

		if (collectedAirports.size() > 0) {
			int i = 0;

			retArray = (jobjectArray)jniEnv->NewObjectArray(collectedAirports.size(), jcls_String, jniEnv->NewStringUTF(""));
			vector<string>::iterator ite2;
			for (ite2 = collectedAirports.begin(); ite2 != collectedAirports.end(); ite2++)
			{
				string tmpAirport = *ite2;
				jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(tmpAirport.c_str()));
				i++;
			}

			collectedAirports.clear();
		}
	}

	return retArray;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getFullName
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jstring retString = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	if (map_airport.find(c_string_airport_code) != map_airport.end()) {
		NatsAirport* cur_airport_ptr = map_airport.at(c_string_airport_code);
		retString = jniEnv->NewStringUTF(cur_airport_ptr->name.c_str());
	}

	return retString;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAllRunways
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
	jclass jcls_Object = jniEnv->FindClass("Ljava/lang/Object;");
	jclass jcls_ObjectArray = jniEnv->FindClass("[Ljava/lang/Object;");

	vector<AirportNode> resultVector = getVector_AllRunways(c_string_airport_code);
	if (resultVector.size() > 0) {
		retArray = (jobjectArray)jniEnv->NewObjectArray(resultVector.size(), jcls_ObjectArray, NULL);

		jobjectArray innerArray;

		for (unsigned int i = 0; i < resultVector.size(); i++) {
			innerArray = (jobjectArray)jniEnv->NewObjectArray(2, jcls_String, NULL);

			jniEnv->SetObjectArrayElement(innerArray, 0, jniEnv->NewStringUTF(resultVector.at(i).refName1.c_str()));
			jniEnv->SetObjectArrayElement(innerArray, 1, jniEnv->NewStringUTF(resultVector.at(i).id.c_str()));

			jniEnv->SetObjectArrayElement(retArray, i, innerArray);
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getRunwayExits
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code, jstring j_runway_name) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string

	const char *c_runway_name = (char*)jniEnv->GetStringUTFChars( j_runway_name, NULL );
	string c_string_runway_name = c_runway_name; // Convert char* to string

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	vector<string> resultVector = getVector_AllRunwayWaypoints(c_string_airport_code, c_string_runway_name);
	if (resultVector.size() > 0) {
		retArray = (jobjectArray)jniEnv->NewObjectArray(resultVector.size(), jcls_String, jniEnv->NewStringUTF(""));

		for (unsigned int i = 0; i < resultVector.size(); i++) {
			jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(resultVector.at(i).c_str()));
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getRunwayEnds
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code, jstring j_runway_name) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string

	const char *c_runway_name = (char*)jniEnv->GetStringUTFChars( j_runway_name, NULL );
	string c_string_runway_name = c_runway_name; // Convert char* to string

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	pair<string, string> resultPair = getRunwayEnds(c_string_airport_code, c_string_runway_name);
	if (resultPair.first != "" && resultPair.second != "") {
		retArray = (jobjectArray)jniEnv->NewObjectArray(2, jcls_String, jniEnv->NewStringUTF(""));

		jniEnv->SetObjectArrayElement(retArray, 0, jniEnv->NewStringUTF(resultPair.first.c_str()));
		jniEnv->SetObjectArrayElement(retArray, 1, jniEnv->NewStringUTF(resultPair.second.c_str()));
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getLayout_1node_1map
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_Integer = jniEnv->FindClass("java/lang/Integer");
	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
	jclass jcls_Object = jniEnv->FindClass("Ljava/lang/Object;");
	jclass jcls_ObjectArray = jniEnv->FindClass("[Ljava/lang/Object;");

	jmethodID methodId_constructor_Integer = jniEnv->GetMethodID(jcls_Integer, "<init>", "(I)V");

	AirportNodeLink cur_airport_node_link;
	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	jobjectArray innerArray;

	if (map_ground_waypoint_connectivity.find(c_string_airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(c_string_airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;

		if (cur_map_waypoint_node.size() > 0) {
			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_map_waypoint_node.size(), jcls_ObjectArray, NULL);

			int j = 0;
			jint j_nodeSeq;
			jobject jobj_nodeSeq;

			map<string, AirportNode>::iterator ite;
			for (ite = cur_map_waypoint_node.begin(); ite != cur_map_waypoint_node.end(); ite++) {
				innerArray = (jobjectArray)jniEnv->NewObjectArray(2, jcls_Object, NULL);

				jniEnv->SetObjectArrayElement(innerArray, 0, jniEnv->NewStringUTF(ite->first.c_str()));

				j_nodeSeq = ite->second.index;
				jobj_nodeSeq = jniEnv->NewObject(jcls_Integer, methodId_constructor_Integer, j_nodeSeq);
				jniEnv->SetObjectArrayElement(innerArray, 1, jobj_nodeSeq);

				jniEnv->SetObjectArrayElement(retArray, j, innerArray);

				j++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code; // Convert char* to string

	jclass jcls_Double = jniEnv->FindClass("java/lang/Double");
	jclass jcls_Integer = jniEnv->FindClass("java/lang/Integer");
	jclass jcls_Object = jniEnv->FindClass("Ljava/lang/Object;");
	jclass jcls_ObjectArray = jniEnv->FindClass("[Ljava/lang/Object;");

	jmethodID methodId_constructor_Double = jniEnv->GetMethodID(jcls_Double, "<init>", "(D)V");
	jmethodID methodId_constructor_Integer = jniEnv->GetMethodID(jcls_Integer, "<init>", "(I)V");

	AirportNodeLink cur_airport_node_link;
	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	jobjectArray innerArray;

	if (map_ground_waypoint_connectivity.find(c_string_airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(c_string_airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;

		if (cur_map_waypoint_node.size() > 0) {
			retArray = (jobjectArray)jniEnv->NewObjectArray(cur_map_waypoint_node.size(), jcls_ObjectArray, NULL);

			int j = 0;
			jint j_nodeSeq;
			jobject jobj_nodeSeq;

			jdouble j_node_latitude;
			jdouble j_node_longitude;
			jobject jobj_nodeLatitude;
			jobject jobj_nodeLongitude;

			map<string, AirportNode>::iterator ite;
			for (ite = cur_map_waypoint_node.begin(); ite != cur_map_waypoint_node.end(); ite++) {
				innerArray = (jobjectArray)jniEnv->NewObjectArray(7, jcls_Object, NULL);

				j_nodeSeq = ite->second.index;
				jobj_nodeSeq = jniEnv->NewObject(jcls_Integer, methodId_constructor_Integer, j_nodeSeq);
				jniEnv->SetObjectArrayElement(innerArray, 0, jobj_nodeSeq);

				j_node_latitude = ite->second.latitude;
				j_node_longitude = ite->second.longitude;

				jobj_nodeLatitude = jniEnv->NewObject(jcls_Double, methodId_constructor_Double, j_node_latitude);
				jniEnv->SetObjectArrayElement(innerArray, 1, jobj_nodeLatitude);

				jobj_nodeLongitude = jniEnv->NewObject(jcls_Double, methodId_constructor_Double, j_node_longitude);
				jniEnv->SetObjectArrayElement(innerArray, 2, jobj_nodeLongitude);

				jniEnv->SetObjectArrayElement(innerArray, 3, jniEnv->NewStringUTF(ite->second.refName1.c_str()));

				jniEnv->SetObjectArrayElement(innerArray, 4, jniEnv->NewStringUTF(ite->second.type1.c_str()));

				jniEnv->SetObjectArrayElement(innerArray, 5, jniEnv->NewStringUTF(ite->second.refName2.c_str()));

				jniEnv->SetObjectArrayElement(innerArray, 6, jniEnv->NewStringUTF(ite->second.type2.c_str()));

				jniEnv->SetObjectArrayElement(retArray, j, innerArray);
				
				airportMapData[ite->second.id].push_back(to_string(ite->second.latitude) + "," + to_string(ite->second.longitude));
				
				j++;
			}
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getLayout_1links
  (JNIEnv *jniEnv, jobject jobj, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_Double = jniEnv->FindClass("java/lang/Double");
	jclass jcls_Integer = jniEnv->FindClass("java/lang/Integer");
	jclass jcls_Object = jniEnv->FindClass("Ljava/lang/Object;");
	jclass jcls_ObjectArray = jniEnv->FindClass("[Ljava/lang/Object;");

	jmethodID methodId_constructor_Double = jniEnv->GetMethodID(jcls_Double, "<init>", "(D)V");
	jmethodID methodId_constructor_Integer = jniEnv->GetMethodID(jcls_Integer, "<init>", "(I)V");

	AirportNodeLink cur_airport_node_link;
	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	jobjectArray innerArray;

	if (map_ground_waypoint_connectivity.find(c_string_airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(c_string_airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;

		cur_airport_node_link = cur_GroundWaypointConnectivity.airport_node_link;

		retArray = (jobjectArray)jniEnv->NewObjectArray(cur_airport_node_link.n1_id.size(), jcls_ObjectArray, NULL);

		jint j_n1_nodeSeq;
		jint j_n2_nodeSeq;
		jobject jobj_n1_nodeSeq;
		jobject jobj_n2_nodeSeq;

		// Check all cur_airport_node_link.n1_id and cur_airport_node_link.n2_id data
		for (int i = 0; i < cur_airport_node_link.n1_id.size(); i++) {
			innerArray = (jobjectArray)jniEnv->NewObjectArray(2, jcls_Object, NULL);

			j_n1_nodeSeq = cur_map_waypoint_node.at(cur_airport_node_link.n1_id.at(i)).index;
			jobj_n1_nodeSeq = jniEnv->NewObject(jcls_Integer, methodId_constructor_Integer, j_n1_nodeSeq);
			jniEnv->SetObjectArrayElement(innerArray, 0, jobj_n1_nodeSeq);

			j_n2_nodeSeq = cur_map_waypoint_node.at(cur_airport_node_link.n2_id.at(i)).index;
			jobj_n2_nodeSeq = jniEnv->NewObject(jcls_Integer, methodId_constructor_Integer, j_n2_nodeSeq);
			jniEnv->SetObjectArrayElement(innerArray, 1, jobj_n2_nodeSeq);

			jniEnv->SetObjectArrayElement(retArray, i, innerArray);
		}
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getSurface_1taxi_1plan
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code) {
	jobjectArray retArray = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (index_flight > -1) {
		FlightPlan tmpFlightPlan = g_flightplans.at(index_flight);

		string airportOrigin = tmpFlightPlan.origin;
		string airportDestination = tmpFlightPlan.destination;
		if (airportOrigin.compare(0, 1, "K") != 0) {
			airportOrigin.insert(0, "K");
		}
		if (airportDestination.compare(0, 1, "K") != 0) {
			airportDestination.insert(0, "K");
		}

		bool flagDeparting = true;
		if ((c_string_airport_code.compare(airportOrigin) == 0) || (c_string_airport_code.compare(airportDestination) == 0)) {
			if (c_string_airport_code.compare(airportOrigin) == 0) {
				flagDeparting = true;
			} else {
				flagDeparting = false;
			}

			waypoint_node_t* tmpWaypoint_Node_ptr = NULL;

			int cnt_waypoint;

			if (flagDeparting) {
				cnt_waypoint = h_departing_taxi_plan.waypoint_length[index_flight];
				retArray = (jobjectArray)jniEnv->NewObjectArray(cnt_waypoint, jcls_String, jniEnv->NewStringUTF(""));

				int i = 0;

				tmpWaypoint_Node_ptr = h_departing_taxi_plan.waypoint_node_ptr[index_flight];
				while (tmpWaypoint_Node_ptr != NULL) {
					jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(tmpWaypoint_Node_ptr->wpname));

					tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;

					i++;
				}
			} else {
				cnt_waypoint = h_landing_taxi_plan.waypoint_length[index_flight];
				retArray = (jobjectArray)jniEnv->NewObjectArray(cnt_waypoint, jcls_String, jniEnv->NewStringUTF(""));

				int i = 0;

				tmpWaypoint_Node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
				while (tmpWaypoint_Node_ptr != NULL) {
					jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(tmpWaypoint_Node_ptr->wpname));

					tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;

					i++;
				}
			}
		}
	}

	return retArray;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_generate_1surface_1taxi_1plan__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jstring j_startNode_waypoint_id, jstring j_endNode_waypoint_id, jstring j_runway_name) {
	jint retValue = 1; // Initial value.  1 means error.

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_startNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_startNode_waypoint_id, NULL );
	string c_string_startNode_waypoint_id = c_startNode_waypoint_id;

	const char *c_endNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_endNode_waypoint_id, NULL );
	string c_string_endNode_waypoint_id = c_endNode_waypoint_id;

	const char *c_runway_name = (char*)jniEnv->GetStringUTFChars( j_runway_name, NULL );
	string c_string_runway_name(c_runway_name);
	c_string_runway_name = trim(c_string_runway_name);

	int c_int = 1;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		c_int = generate_and_load_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_string_startNode_waypoint_id, c_string_endNode_waypoint_id, c_string_runway_name);
		retValue = c_int;
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_generate_1surface_1taxi_1plan__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jstring j_startNode_waypoint_id, jstring j_endNode_waypoint_id, jstring j_vrNode_waypoint_id, jstring j_touchdownNode_waypoint_id) {
	jint retValue = 1; // Initial value.  1 means error.

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_startNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_startNode_waypoint_id, NULL );
	string c_string_startNode_waypoint_id = c_startNode_waypoint_id;

	const char *c_endNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_endNode_waypoint_id, NULL );
	string c_string_endNode_waypoint_id = c_endNode_waypoint_id;

	const char *c_vrNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_vrNode_waypoint_id, NULL );
	string c_string_vrNode_waypoint_id = c_vrNode_waypoint_id;

	const char *c_touchdownNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_touchdownNode_waypoint_id, NULL );
	string c_string_touchdownNode_waypoint_id = c_touchdownNode_waypoint_id;

	int c_int = 1;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		c_int = generate_and_load_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_string_startNode_waypoint_id, c_string_endNode_waypoint_id, c_string_vrNode_waypoint_id, c_string_touchdownNode_waypoint_id);
		retValue = c_int;
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_generate_1surface_1taxi_1plan__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2_3D_3D
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jstring j_startNode_waypoint_id, jstring j_endNode_waypoint_id, jdoubleArray j_v2Point_lat_lon, jdoubleArray j_touchdownPoint_lat_lon) {
	jint retValue = 1; // Initial value.  1 means error.

	if (((j_v2Point_lat_lon == NULL) && (j_touchdownPoint_lat_lon == NULL))
			||
		((j_v2Point_lat_lon != NULL) && (j_touchdownPoint_lat_lon != NULL))) {
		return retValue;
	}

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_startNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_startNode_waypoint_id, NULL );
	string c_string_startNode_waypoint_id = c_startNode_waypoint_id;

	const char *c_endNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_endNode_waypoint_id, NULL );
	string c_string_endNode_waypoint_id = c_endNode_waypoint_id;

	bool c_flag_departing;

	double c_v2Point_lat_lon[2];

	if (j_v2Point_lat_lon != NULL) {
		jdouble* jdouble_v2Point_lat_lon = jniEnv->GetDoubleArrayElements(j_v2Point_lat_lon, NULL);

		c_v2Point_lat_lon[0] = jdouble_v2Point_lat_lon[0];
		c_v2Point_lat_lon[1] = jdouble_v2Point_lat_lon[1];

		c_flag_departing = true;
	}

	double c_touchdownPoint_lat_lon[2];

	if (j_touchdownPoint_lat_lon != NULL) {
		jdouble* jdouble_touchdownPoint_lat_lon = jniEnv->GetDoubleArrayElements(j_touchdownPoint_lat_lon, NULL);

		c_touchdownPoint_lat_lon[0] = jdouble_touchdownPoint_lat_lon[0];
		c_touchdownPoint_lat_lon[1] = jdouble_touchdownPoint_lat_lon[1];

		c_flag_departing = false;
	}

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	int c_int = 1;

	if (c_flightSeq > -1) {
		if (c_flag_departing) {
			retValue = generate_and_load_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_string_startNode_waypoint_id, c_string_endNode_waypoint_id, c_v2Point_lat_lon, NULL);
		} else {
			retValue = generate_and_load_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_string_startNode_waypoint_id, c_string_endNode_waypoint_id, NULL, c_touchdownPoint_lat_lon);
		}
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setUser_1defined_1surface_1taxi_1plan__Ljava_lang_String_2Ljava_lang_String_2_3Ljava_lang_String_2
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jobjectArray j_user_defined_waypoint_ids) {
	jint retValue = 1;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_String, "toString", "()Ljava/lang/String;");

	jobject tmpJobject;
	jstring tmpJstring;

	vector<string> c_vector_waypoint_ids;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		if (j_user_defined_waypoint_ids) {
			int cnt_waypoint = jniEnv->GetArrayLength(j_user_defined_waypoint_ids);
			if (cnt_waypoint > 0) {
				for (int i = 0; i < cnt_waypoint; i++) {
					tmpJobject = jniEnv->GetObjectArrayElement(j_user_defined_waypoint_ids, i);
					tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);

					const char *c_tmp_waypoint_id = (char*)jniEnv->GetStringUTFChars(tmpJstring, NULL );
					string c_string_tmp_waypoint_id = c_tmp_waypoint_id;

					c_vector_waypoint_ids.push_back(c_string_tmp_waypoint_id);
				}

				retValue = set_user_defined_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_vector_waypoint_ids);

				if (c_vector_waypoint_ids.size() > 0)
					c_vector_waypoint_ids.clear();
			}
		}
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setUser_1defined_1surface_1taxi_1plan__Ljava_lang_String_2Ljava_lang_String_2_3Ljava_lang_String_2_3D
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jobjectArray j_user_defined_waypoint_ids, jdoubleArray j_v2_or_touchdown_point_lat_lon) {
	jint retValue = 1;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_String, "toString", "()Ljava/lang/String;");

	jobject tmpJobject;
	jstring tmpJstring;

	double c_v2_or_touchdown_point_lat_lon[2];

	if (j_v2_or_touchdown_point_lat_lon != NULL) {
		jdouble* jdouble_v2_or_touchdown_point_lat_lon = jniEnv->GetDoubleArrayElements(j_v2_or_touchdown_point_lat_lon, NULL);

		c_v2_or_touchdown_point_lat_lon[0] = jdouble_v2_or_touchdown_point_lat_lon[0];
		c_v2_or_touchdown_point_lat_lon[1] = jdouble_v2_or_touchdown_point_lat_lon[1];
	}

	vector<string> c_vector_waypoint_ids;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (c_flightSeq > -1) {
		if (j_user_defined_waypoint_ids) {
			int cnt_waypoint = jniEnv->GetArrayLength(j_user_defined_waypoint_ids);
			if (cnt_waypoint > 0) {
				for (int i = 0; i < cnt_waypoint; i++) {
					tmpJobject = jniEnv->GetObjectArrayElement(j_user_defined_waypoint_ids, i);
					tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);

					const char *c_tmp_waypoint_id = (char*)jniEnv->GetStringUTFChars(tmpJstring, NULL );
					string c_string_tmp_waypoint_id = c_tmp_waypoint_id;

					c_vector_waypoint_ids.push_back(c_string_tmp_waypoint_id);
				}

				retValue = set_user_defined_surface_taxi_plan(c_flightSeq, c_string_airport_code, c_vector_waypoint_ids, c_v2_or_touchdown_point_lat_lon);

				if (c_vector_waypoint_ids.size() > 0)
					c_vector_waypoint_ids.clear();
			}
		}
	}

	return retValue;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_get_1taxi_1route_1from_1A_1To_1B
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_airport_code, jstring j_startNode_waypoint_id, jstring j_endNode_waypoint_id) {
	jobjectArray retArray = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_airport_code = (char*)jniEnv->GetStringUTFChars( j_airport_code, NULL );
	string c_string_airport_code = c_airport_code;

	const char *c_startNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_startNode_waypoint_id, NULL );
	string c_string_startNode_waypoint_id = c_startNode_waypoint_id;

	const char *c_endNode_waypoint_id = (char*)jniEnv->GetStringUTFChars( j_endNode_waypoint_id, NULL );
	string c_string_endNode_waypoint_id = c_endNode_waypoint_id;

	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	vector<string> resultVector = get_taxi_route_from_A_To_B(
			c_string_airport_code, c_string_startNode_waypoint_id, c_string_endNode_waypoint_id);
	if (resultVector.size() > 0) {
		retArray = (jobjectArray)jniEnv->NewObjectArray(resultVector.size(), jcls_String, jniEnv->NewStringUTF(""));

		for (int i = 0; i < resultVector.size(); i++) {
			string tmpWaypointid = resultVector.at(i);

			jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(tmpWaypointid.c_str()));
		}
	}

	return retArray;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getDepartureRunway
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid) {
	jstring retObject = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (index_flight > -1) {
		if (h_departing_taxi_plan.runway_name[index_flight] != NULL) {
			retObject = jniEnv->NewStringUTF(h_departing_taxi_plan.runway_name[index_flight]);
		}
	}

	return retObject;
}

JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getArrivalRunway
	(JNIEnv *jniEnv, jobject jobj, jstring j_acid) {
	jstring retObject = NULL;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (index_flight > -1) {
		if (h_landing_taxi_plan.runway_name[index_flight] != NULL) {
			retObject = jniEnv->NewStringUTF(h_landing_taxi_plan.runway_name[index_flight]);
		}
	}

	return retObject;
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getTaxi_1tas
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid) {
	jdouble retValue = 0.0;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (index_flight > -1) {
		retValue = h_landing_taxi_plan.taxi_tas_knots[index_flight]; // Same speed is used in departing and landing.
	}

	return retValue;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setTaxi_1tas
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jdouble j_taxi_tas_knots) {
	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	double c_taxi_tas_knots = j_taxi_tas_knots;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if ((index_flight > -1) && (c_taxi_tas_knots >= 0.0)) {
		h_departing_taxi_plan.taxi_tas_knots[index_flight] = c_taxi_tas_knots; // Set new value to departing taxi plan
		h_landing_taxi_plan.taxi_tas_knots[index_flight] = c_taxi_tas_knots; // Set new value to landing taxi plan
	}
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_controller_1setDelayPeriod
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jstring j_aircraft_clearance, jfloat j_seconds) {
	jint retValue = 1; // Default.  Means error

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	const char *c_aircraft_clearance = (char*)jniEnv->GetStringUTFChars( j_aircraft_clearance, NULL );
	string c_string_aircraft_clearance = c_aircraft_clearance;

	const float c_seconds = j_seconds;

	ControllerErrorData_t tmpHumanErrorData_controller;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if ((index_flight > -1) && (c_seconds > 0)) {
		HumanErrorEvent_t tmpHumanErrorEvent_controller;
		tmpHumanErrorEvent_controller.type = CONTROLLER_ERROR_EVENT_TYPE_CLEARANCE;

		tmpHumanErrorEvent_controller.name = (char*)calloc(c_string_aircraft_clearance.length()+1, sizeof(char));
		strcpy(tmpHumanErrorEvent_controller.name, c_string_aircraft_clearance.c_str());
		tmpHumanErrorEvent_controller.name[c_string_aircraft_clearance.length()] = '\0';

		ControllerErrorData_t tmpHumanErrorData_controller;
		tmpHumanErrorData_controller.delaySecond = c_seconds;
		g_humanError_Controller[0][index_flight].insert(pair<HumanErrorEvent_t, ControllerErrorData_t>(tmpHumanErrorEvent_controller, tmpHumanErrorData_controller));

		retValue = 0;
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_insertAirborneWaypoint
  (JNIEnv *jniEnv, jobject jobj, jstring j_acid, jint j_index_to_insert, jstring j_waypoint_type, jstring j_waypoint_name, jfloat j_waypoint_latitude, jfloat j_waypoint_longitude, jfloat j_waypoint_altitude, jfloat j_waypoint_speed_lim, jstring j_waypoint_spdlim_desc) {
	jint retValue = 1; // Default

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( j_acid, NULL );
	string c_string_acid = c_acid;

	int c_index_to_insert = j_index_to_insert;

	const char *c_waypoint_type = (char*)jniEnv->GetStringUTFChars( j_waypoint_type, NULL );
	string c_string_waypoint_type = c_waypoint_type;

	const char *c_waypoint_name = (char*)jniEnv->GetStringUTFChars( j_waypoint_name, NULL );
	string c_string_waypoint_name = c_waypoint_name;

	float c_waypoint_latitude = j_waypoint_latitude;
	float c_waypoint_longitude = j_waypoint_longitude;
	float c_waypoint_altitude = j_waypoint_altitude;
	float c_waypoint_speed_lim = j_waypoint_speed_lim;

	const char *c_waypoint_spdlim_desc = (char*)jniEnv->GetStringUTFChars( j_waypoint_spdlim_desc, NULL );
	string c_string_waypoint_spdlim_desc = c_waypoint_spdlim_desc;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (-1 < index_flight) {
		retValue = insert_airborne_waypointNode(index_flight,
				c_index_to_insert,
				c_string_waypoint_type,
				c_string_waypoint_name,
				c_waypoint_latitude,
				c_waypoint_longitude,
				c_waypoint_altitude,
				c_waypoint_speed_lim,
				c_string_waypoint_spdlim_desc);
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_deleteAirborneWaypoint
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftID, jint waypointIndex) {
	jint retValue = 1; // Default

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftID, NULL );
	string c_string_acid = c_acid;

	int deleteWaypointIndex = waypointIndex;

	int index_flight = select_flightSeq_by_aircraftId(c_string_acid);
	if (-1 < index_flight) {
		retValue = delete_airborne_waypointNode(index_flight, deleteWaypointIndex);
	}

	return retValue;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setTargetWaypoint
  (JNIEnv *jniEnv, jobject jobj, jstring ac_id, jint waypoint_plan_to_use, jint index_of_target) {
	const string string_ac_id = (string) jniEnv->GetStringUTFChars(ac_id, NULL);
	int c_flightSeq = select_flightSeq_by_aircraftId(string_ac_id);
	if ((c_flightSeq > -1) && (-1 < index_of_target)) {
		waypoint_node_t* waypointNode_ptr = NULL;

		if (waypoint_plan_to_use == 0) {
			waypointNode_ptr = h_departing_taxi_plan.waypoint_node_ptr[c_flightSeq];
		} else if (waypoint_plan_to_use == 1) {
			waypointNode_ptr = array_Airborne_Flight_Plan_ptr[c_flightSeq];
		} else if (waypoint_plan_to_use == 2) {
			waypointNode_ptr = h_landing_taxi_plan.waypoint_node_ptr[c_flightSeq];
		}

		int tmpIdx = 0;
		while ((waypointNode_ptr != NULL) && (tmpIdx <= index_of_target)) {
			if (tmpIdx == index_of_target) {
				h_aircraft_soa.target_waypoint_node_ptr[c_flightSeq] = waypointNode_ptr;

				break;
			}

			waypointNode_ptr = waypointNode_ptr->next_node_ptr;
			tmpIdx++;
		}
	}
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setControllerAbsence
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftID, jint timeSteps) {
	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftID, NULL);
	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	controllerAway = timeSteps;
	defaultSpeed = g_trajectories.at(c_flightSeq).tas_knots[g_trajectories.at(c_flightSeq).tas_knots.size() - 1];
	defaultCourse = g_trajectories.at(c_flightSeq).course_deg[g_trajectories.at(c_flightSeq).course_deg.size() - 1] * PI/180.;
	defaultRocd = g_trajectories.at(c_flightSeq).rocd_fps[g_trajectories.at(c_flightSeq).rocd_fps.size() - 1];

	return 0;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_enableMergingAndSpacingAtMeterFix
  (JNIEnv *jniEnv, jobject jobj, jstring centerId, jstring meterFix, jstring spacingType, jfloat spacingDistance) {

	string meterFixString = (string) jniEnv->GetStringUTFChars(meterFix, NULL);
	string centerIdString = (string) jniEnv->GetStringUTFChars(centerId, NULL);
	string spacingTypeString = (string) jniEnv->GetStringUTFChars(spacingType, NULL);

	meterFixMap[centerIdString].push_back(meterFixString);

	spacingDistMap[centerIdString][meterFixString].first = spacingTypeString;
	spacingDistMap[centerIdString][meterFixString].second = spacingDistance;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_disableMergingAndSpacingAtMeterFix
  (JNIEnv *jniEnv, jobject jobj, jstring centerId, jstring meterFix) {

	string meterFixString = (string) jniEnv->GetStringUTFChars(meterFix, NULL);
	string centerIdString = (string) jniEnv->GetStringUTFChars(centerId, NULL);

	for(int meterFixCount = 0; meterFixCount < meterFixMap[centerIdString].size(); meterFixCount++)
	{
	    string currentMeterFix = meterFixMap[centerIdString][meterFixCount];
	    if(currentMeterFix == meterFixString)
	    {
	    	meterFixMap[centerIdString].erase(meterFixMap[centerIdString].begin() + meterFixCount);
	        --meterFixCount;
	    }
	}

	spacingDistMap[centerIdString][meterFixString].first = "";
	spacingDistMap[centerIdString][meterFixString].second = 0.0;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_pilot_1setActionLag(
		JNIEnv *jniEnv, jobject jobj, jstring aircraftID, jstring lagParameter,
		jfloat lagTimeConstant, jfloat percentageError,
		jfloat parameterCurrentValue, jfloat parameterTarget) {
	const string lagParameterString = (string) jniEnv->GetStringUTFChars(
			lagParameter, NULL);
	const string string_ac_id = (string) jniEnv->GetStringUTFChars(
			aircraftID, NULL);
	int c_flightSeq = select_flightSeq_by_aircraftId(string_ac_id);

	if (lagParameterString == "COURSE") {
		lagParams[c_flightSeq].push_back("COURSE");
		float G = log(1 / percentageError) / lagTimeConstant
				* t_data_collection_period_airborne;
		if (parameterCurrentValue < parameterTarget) {
			while((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue < parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue < parameterTarget)) {
					lagParamValues[c_flightSeq]["COURSE"].push_back(parameterCurrentValue);
				}
			}
		}
		else if (parameterCurrentValue > parameterTarget) {
			while((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue > parameterTarget)) {
						parameterCurrentValue = parameterCurrentValue
								+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue > parameterTarget)) {
						lagParamValues[c_flightSeq]["COURSE"].push_back(parameterCurrentValue);
				}
			}
		}
		lagParamValues[c_flightSeq]["COURSE"].push_back(parameterTarget);
	}


	if (lagParameterString == "AIRSPEED") {
		lagParams[c_flightSeq].push_back("AIRSPEED");
		float G = log(1 / percentageError) / lagTimeConstant
				* t_data_collection_period_airborne;
		if (parameterCurrentValue < parameterTarget) {
			while((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue < parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue < parameterTarget)) {
					lagParamValues[c_flightSeq]["AIRSPEED"].push_back(parameterCurrentValue);
				}
			}
		}
		else if (parameterCurrentValue > parameterTarget) {
			while ((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue > parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
								+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue > parameterTarget)) {
						lagParamValues[c_flightSeq]["AIRSPEED"].push_back(parameterCurrentValue);
					}
			}
		}

		lagParamValues[c_flightSeq]["AIRSPEED"].push_back(parameterTarget);
	}

	if (lagParameterString == "VERTICAL_SPEED") {
		lagParams[c_flightSeq].push_back("VERTICAL_SPEED");
		float G = log(1 / percentageError) / lagTimeConstant
				* t_data_collection_period_airborne;
		if (parameterCurrentValue < parameterTarget) {
			while((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue < parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue < parameterTarget)) {
						lagParamValues[c_flightSeq]["VERTICAL_SPEED"].push_back(parameterCurrentValue);
				}
			}
		}
		else if (parameterCurrentValue > parameterTarget) {
			while ((abs(parameterCurrentValue - parameterTarget) > 0.1) && (parameterCurrentValue > parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((c_flightSeq > -1) && (parameterCurrentValue > parameterTarget)) {
						lagParamValues[c_flightSeq]["VERTICAL_SPEED"].push_back(parameterCurrentValue);
				}
			}
		}

		lagParamValues[c_flightSeq]["VERTICAL_SPEED"].push_back(parameterTarget);
	}

	return 0;
}

JNIEXPORT jfloat JNICALL Java_com_osi_gnats_engine_CEngine_pilot_1setActionRepeat(
		JNIEnv *jniEnv, jobject jobj, jstring aircraftID, jstring repeatParameter, jint flag) {
    float retValue;
	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftID, NULL);
	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	string repeatFlightParameter = (string)(char*) jniEnv->GetStringUTFChars(repeatParameter, NULL);
	if ((c_flightSeq > -1) && flag == 1) {
		if (repeatFlightParameter == "COURSE") {
				retValue = g_trajectories.at(c_flightSeq).course_deg[g_trajectories.at(c_flightSeq).course_deg.size() - 2];
		} else if (repeatFlightParameter == "VERTICAL_SPEED") {
			retValue = g_trajectories.at(c_flightSeq).rocd_fps[g_trajectories.at(c_flightSeq).rocd_fps.size() - 2];
		}
		 else if (repeatFlightParameter == "AIRSPEED") {
			retValue = g_trajectories.at(c_flightSeq).tas_knots[g_trajectories.at(c_flightSeq).tas_knots.size() - 2];
		}
	}
	else if ((c_flightSeq > -1) && flag == 0) {
		if (repeatFlightParameter == "COURSE") {
				retValue = g_trajectories.at(c_flightSeq).course_deg[g_trajectories.at(c_flightSeq).course_deg.size() - 1];
		} else if (repeatFlightParameter == "VERTICAL_SPEED") {
			retValue = g_trajectories.at(c_flightSeq).rocd_fps[g_trajectories.at(c_flightSeq).rocd_fps.size() - 1];
		}
		 else if (repeatFlightParameter == "AIRSPEED") {
			retValue = g_trajectories.at(c_flightSeq).tas_knots[g_trajectories.at(c_flightSeq).tas_knots.size() - 1];
		}
	}

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_pilot_1skipFlightPhase(
		JNIEnv *jniEnv, jobject jobj, jstring aircraftID, jstring flightPhase) {
	jstring retString = NULL;
	const char *c_acid = (char*) jniEnv->GetStringUTFChars(aircraftID, NULL);
	string c_string_acid = c_acid;
	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	skipFlightPhase[c_flightSeq] = (string) jniEnv->GetStringUTFChars(
			flightPhase, NULL);

	return 0;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_downloadWeatherFiles
  (JNIEnv *jniEnv, jobject jobj) {
	jint retValue = 1;

    char className[strlen("java/rmi/RemoteException")+1];

	int sim_status = get_runtime_sim_status();

	if ((sim_status == NATS_SIMULATION_STATUS_START)
				|| (sim_status == NATS_SIMULATION_STATUS_PAUSE)
				|| (sim_status == NATS_SIMULATION_STATUS_RESUME)) {
	    jclass exClass;

	    strcpy(className, "java/rmi/RemoteException");
	    className[strlen("java/rmi/RemoteException")] = '\0';

	    exClass = jniEnv->FindClass(className);
	    if (exClass != NULL) {
	        return jniEnv->ThrowNew(exClass, "Simulation in process.  Unable to download weather files.");
	    }
	} else {
		retValue = tg_download_weather_files();
	}

	return retValue;
}

JNIEXPORT jfloatArray JNICALL Java_com_osi_gnats_engine_CEngine_getWind
  (JNIEnv *jniEnv, jobject jobj, jfloat j_timestamp_sec, jfloat j_latitude_deg, jfloat j_longitude_deg, jfloat j_altitude_ft) {
	jfloatArray retArray = NULL;

	float c_timestamp_sec = j_timestamp_sec;
	float c_latitude_deg = j_latitude_deg;
	float c_longitude_deg = j_longitude_deg;
	float c_altitude_ft = j_altitude_ft;

	set_device_ruc_pointers();

	float wind_east_fps = 0, wind_north_fps = 0;
	get_wind_field_components(c_timestamp_sec, c_latitude_deg, c_longitude_deg, c_altitude_ft, &wind_east_fps, &wind_north_fps);

	retArray = jniEnv->NewFloatArray(2);

	jfloat tmp_array[2];
	tmp_array[0] = wind_north_fps;
	tmp_array[1] = wind_east_fps;

	jniEnv->SetFloatArrayRegion(retArray, 0 , 2, tmp_array);

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getWeatherPolygons
  (JNIEnv *jniEnv, jobject jobj, jstring j_ac_id, jdouble j_lat_deg, jdouble j_lon_deg, jdouble j_alt_ft, jdouble j_nauticalMile_radius) {
	jobjectArray retArray = NULL;

	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(j_ac_id, NULL);
	double c_lat_deg = j_lat_deg;
	double c_lon_deg = j_lon_deg;
	double c_alt_ft = j_alt_ft;
	double c_nauticalMile_radius = j_nauticalMile_radius;

	jclass jcls_weatherPolygon = jniEnv->FindClass("com/osi/gnats/weather/WeatherPolygon");

	// FieldID of properties in WeatherPolygon class
	jfieldID fieldId_WeatherPolygon_x_data = jniEnv->GetFieldID(jcls_weatherPolygon, "x_data", "[D");
	jfieldID fieldId_WeatherPolygon_y_data = jniEnv->GetFieldID(jcls_weatherPolygon, "y_data", "[D");
	jfieldID fieldId_WeatherPolygon_num_vertices = jniEnv->GetFieldID(jcls_weatherPolygon, "num_vertices", "I");
	jfieldID fieldId_WeatherPolygon_ccw_flag = jniEnv->GetFieldID(jcls_weatherPolygon, "ccw_flag", "Z");
	jfieldID fieldId_WeatherPolygon_xmin = jniEnv->GetFieldID(jcls_weatherPolygon, "xmin", "D");
	jfieldID fieldId_WeatherPolygon_xmax = jniEnv->GetFieldID(jcls_weatherPolygon, "xmax", "D");
	jfieldID fieldId_WeatherPolygon_ymin = jniEnv->GetFieldID(jcls_weatherPolygon, "ymin", "D");
	jfieldID fieldId_WeatherPolygon_ymax = jniEnv->GetFieldID(jcls_weatherPolygon, "ymax", "D");
	jfieldID fieldId_WeatherPolygon_x_centroid = jniEnv->GetFieldID(jcls_weatherPolygon, "x_centroid", "D");
	jfieldID fieldId_WeatherPolygon_y_centroid = jniEnv->GetFieldID(jcls_weatherPolygon, "y_centroid", "D");
	jfieldID fieldId_WeatherPolygon_poly_type = jniEnv->GetFieldID(jcls_weatherPolygon, "poly_type", "Ljava/lang/String;");
	jfieldID fieldId_WeatherPolygon_start_hr = jniEnv->GetFieldID(jcls_weatherPolygon, "start_hr", "I");
	jfieldID fieldId_WeatherPolygon_end_hr = jniEnv->GetFieldID(jcls_weatherPolygon, "end_hr", "I");
	// end - FieldID of properties in WeatherPolygon class

	jmethodID methodId_constructor_weatherPolygon = jniEnv->GetMethodID(jcls_weatherPolygon, "<init>", "()V");

	bool c_flag_external_aircraft = false;
	waypoint_node_t* waypointNode_ptr = NULL;

	vector<double> vec_fp_lat_deg;
	vector<double> vec_fp_lon_deg;

	vector<Polygon> resultVector;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);
	if (0 <= c_flightSeq) {
		string tmpDir;

		tg_pathFilename_pirep = trim(tg_pathFilename_pirep);
		if (tg_pathFilename_pirep.length() == 0) {
			// Start looking for the latest pirep file

			tmpDir.assign(g_share_dir + "/tg/weather");

			string latest_pirep_filename = get_latestFilename(tmpDir.c_str(), ".airep");

			if (latest_pirep_filename.length() > 0) {
				tg_pathFilename_pirep.assign(g_share_dir + "/tg/weather/");
				tg_pathFilename_pirep.append(latest_pirep_filename);
			}
		}

		tg_pathFilename_polygon = trim(tg_pathFilename_polygon);
		if (tg_pathFilename_polygon.length() == 0) {
			// Start looking for the latest polygon file

			tmpDir.assign(g_share_dir + "/rg/polygons");

			string latest_polygon_filename = get_latestFilename(tmpDir.c_str(), ".dat");

			if (latest_polygon_filename.length() > 0) {
				tg_pathFilename_polygon.assign(g_share_dir + "/rg/polygons/");
				tg_pathFilename_polygon.append(latest_polygon_filename);
			}
		}

		tg_pathFilename_sigmet = trim(tg_pathFilename_sigmet);
		if (tg_pathFilename_sigmet.length() == 0) {
			// Start looking for the latest sigmet file

			tmpDir.assign(g_share_dir + "/tg/weather");

			string latest_sigmet_filename = get_latestFilename(tmpDir.c_str(), ".sigmet");

			if (latest_sigmet_filename.length() > 0) {
				tg_pathFilename_sigmet.assign(g_share_dir + "/tg/weather/");
				tg_pathFilename_sigmet.append(latest_sigmet_filename);
			}
		}

		c_flag_external_aircraft = newMU_external_aircraft.flag_external_aircraft.at(c_flightSeq);
		if (!c_flag_external_aircraft) {
			vec_fp_lat_deg.clear(); // Reset
			vec_fp_lon_deg.clear(); // Reset

			waypointNode_ptr = h_aircraft_soa.target_waypoint_node_ptr[c_flightSeq]; //getWaypointNodePtr_by_flightSeq(c_flightSeq, 0);
			while (waypointNode_ptr != NULL) {
				vec_fp_lat_deg.push_back(waypointNode_ptr->latitude);
				vec_fp_lon_deg.push_back(waypointNode_ptr->longitude);

				waypointNode_ptr = waypointNode_ptr->next_node_ptr;
			}
		}

		resultVector.clear();

		getWeatherPolygons(c_lat_deg, c_lon_deg, c_alt_ft, c_nauticalMile_radius,
				vec_fp_lat_deg,
				vec_fp_lon_deg,
				tg_pathFilename_polygon,
				tg_pathFilename_sigmet,
				tg_pathFilename_pirep,
				g_cifp_file,
				resultVector);
		if (resultVector.size() > 0) {
			retArray = (jobjectArray)jniEnv->NewObjectArray(resultVector.size(), jcls_weatherPolygon, NULL);

			for (unsigned int i = 0; i < resultVector.size(); i++) {
				jobject newObj_weatherPolygon = jniEnv->NewObject(jcls_weatherPolygon, methodId_constructor_weatherPolygon);

				int tmpNumVertices = resultVector.at(i).getNumVertices();

				if (0 < tmpNumVertices) {
					// Handle fieldId_WeatherPolygon_x_data
					jdoubleArray tmp_x_data_array = jniEnv->NewDoubleArray(tmpNumVertices);

					jniEnv->SetDoubleArrayRegion(tmp_x_data_array, 0 , tmpNumVertices, resultVector.at(i).getXData());

					jniEnv->SetObjectField(newObj_weatherPolygon, fieldId_WeatherPolygon_x_data, tmp_x_data_array);
					// end - Handle fieldId_WeatherPolygon_x_data

					// Handle fieldId_WeatherPolygon_y_data
					jdoubleArray tmp_y_data_array = jniEnv->NewDoubleArray(tmpNumVertices);

					jniEnv->SetDoubleArrayRegion(tmp_y_data_array, 0 , tmpNumVertices, resultVector.at(i).getYData());

					jniEnv->SetObjectField(newObj_weatherPolygon, fieldId_WeatherPolygon_y_data, tmp_y_data_array);
					// end - Handle fieldId_WeatherPolygon_y_data
				} else {
					// Handle fieldId_WeatherPolygon_x_data
					jdoubleArray tmp_x_data_array = jniEnv->NewDoubleArray(0);

					jniEnv->SetObjectField(newObj_weatherPolygon, fieldId_WeatherPolygon_x_data, tmp_x_data_array);
					// end - Handle fieldId_WeatherPolygon_x_data

					// Handle fieldId_WeatherPolygon_y_data
					jdoubleArray tmp_y_data_array = jniEnv->NewDoubleArray(0);

					jniEnv->SetObjectField(newObj_weatherPolygon, fieldId_WeatherPolygon_y_data, tmp_y_data_array);
					// end - Handle fieldId_WeatherPolygon_y_data
				}


				jniEnv->SetBooleanField(newObj_weatherPolygon, fieldId_WeatherPolygon_ccw_flag, resultVector.at(i).getCCWFlag());
				jniEnv->SetIntField(newObj_weatherPolygon, fieldId_WeatherPolygon_num_vertices, tmpNumVertices);

				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_xmin, resultVector.at(i).getXMin());
				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_xmax, resultVector.at(i).getXMax());
				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_ymin, resultVector.at(i).getYMin());
				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_ymax, resultVector.at(i).getYMax());

				double* tmpX_centroid = (double*)calloc(1, sizeof(double));
				double* tmpY_centroid = (double*)calloc(1, sizeof(double));
				resultVector.at(i).getCentroid(tmpX_centroid, tmpY_centroid);

				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_x_centroid, *tmpX_centroid);
				jniEnv->SetDoubleField(newObj_weatherPolygon, fieldId_WeatherPolygon_y_centroid, *tmpY_centroid);

				jniEnv->SetObjectField(newObj_weatherPolygon, fieldId_WeatherPolygon_poly_type, jniEnv->NewStringUTF(resultVector.at(i).getPolyType().c_str()));
				jniEnv->SetIntField(newObj_weatherPolygon, fieldId_WeatherPolygon_start_hr, resultVector.at(i).getStartHour());
				jniEnv->SetIntField(newObj_weatherPolygon, fieldId_WeatherPolygon_end_hr, resultVector.at(i).getEndHour());

				jniEnv->SetObjectArrayElement(retArray, i, newObj_weatherPolygon);

				free(tmpX_centroid);
				free(tmpY_centroid);
			}
		}
	}

	return retArray;
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_enableStrategicWeatherAvoidance
  (JNIEnv *jniEnv, jobject jobj, jboolean j_flag) {
	const bool c_flag = j_flag;

	flag_enable_strategic_weather_avoidance = c_flag;

	if (flag_enable_strategic_weather_avoidance) {
		printf("Strategic weather avoidance: Enabled\n");
	} else {
		printf("Strategic weather avoidance: Disabled\n");
	}
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setWeather_1pirepFile
  (JNIEnv *jniEnv, jobject jobj, jstring j_pathFilename) {
	const char *c_pathFilename = (char*) jniEnv->GetStringUTFChars(j_pathFilename, NULL);

	tg_pathFilename_pirep.assign(c_pathFilename);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setWeather_1polygonFile
  (JNIEnv *jniEnv, jobject jobj, jstring j_pathFilename) {
	const char *c_pathFilename = (char*) jniEnv->GetStringUTFChars(j_pathFilename, NULL);

	tg_pathFilename_polygon.assign(c_pathFilename);
}

JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_setWeather_1sigmetFile
	(JNIEnv *jniEnv, jobject jobj, jstring j_pathFilename) {
	const char *c_pathFilename = (char*) jniEnv->GetStringUTFChars(j_pathFilename, NULL);

	tg_pathFilename_sigmet.assign(c_pathFilename);
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setTacticalWeatherAvoidance
  (JNIEnv *jniEnv, jobject jobj, jstring j_waypoint_name, jfloat j_duration) {
	jint retValue = 1; // Default

	const char *c_waypoint_name = (char*) jniEnv->GetStringUTFChars(j_waypoint_name, NULL);

	WeatherWaypoint newWeatherWaypoint;
	newWeatherWaypoint.waypoint_name = string(c_waypoint_name);

	vector<WeatherWaypoint>::iterator ite_waypoint = find(vector_tactical_weather_waypoint.begin(), vector_tactical_weather_waypoint.end(), newWeatherWaypoint);
	if (ite_waypoint != vector_tactical_weather_waypoint.end()) {
		newWeatherWaypoint.durationSecond = j_duration;

		*ite_waypoint = newWeatherWaypoint;
	} else {
		newWeatherWaypoint.durationSecond = j_duration;

		vector_tactical_weather_waypoint.push_back(newWeatherWaypoint);
	}

	retValue = 0; // Success

	return retValue;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_calculateRelativeVelocity
  (JNIEnv *jniEnv, jobject jobj, jdouble refSpeed, jdouble refCourse, jdouble refFpa, jdouble tempSpeed, jdouble tempCourse, jdouble tempFpa) {
  	double refVelocityVertical, refVelocityHorizontal;
	double tempVelocityVertical,tempVelocityHorizontal;
	
	refVelocityVertical = refSpeed * cos(refFpa);
	tempVelocityVertical = tempSpeed * cos(tempFpa);
	refVelocityHorizontal = refVelocityVertical * cos(refCourse);
	tempVelocityHorizontal = tempVelocityVertical * cos(tempCourse);
	
	return (tempVelocityHorizontal - refVelocityHorizontal);
  }

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_calculateBearing
  (JNIEnv *jniEnv, jobject jobj, jdouble lat1, jdouble lon1, jdouble lat2, jdouble lon2) {
  	double longitude1, longitude2, latitude1, latitude2, longitudeDifference, x, y, bearingAngle;
  	longitude1 = lon1;
	longitude2 = lon2;
	latitude1 = lat1 * PI / 180.;
	latitude2 = lat2 * PI / 180.;
	longitudeDifference = (longitude2-longitude1) * PI / 180.;
	y = sin(longitudeDifference)*cos(latitude2);
	x = cos(latitude1)*sin(latitude2)-sin(latitude1)*cos(latitude2)*cos(longitudeDifference);  
	bearingAngle = fmod(atan2(y, x) * 180./PI +360, 360);
	
	return bearingAngle;
  }

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_calculateDistance
  (JNIEnv *jniEnv, jobject jobj, jdouble lat1, jdouble lon1, jdouble alt1, jdouble lat2, jdouble lon2, jdouble alt2) {
  	int R = 6371;
	double latDistance, lonDistance, a, c, distance, altDiff;
	
    latDistance = (lat2 - lat1) * PI / 180.;
    lonDistance = (lon2 - lon1) * PI / 180.;
    a = sin(latDistance / 2) * sin(latDistance / 2)
            + cos(lat1 * PI / 180.) * cos(lat2 * PI / 180.)
            * sin(lonDistance / 2) * sin(lonDistance / 2);
    c = 2 * atan2(sqrt(a), sqrt(1 - a));
    distance = R * c * 1000; //Kilometer to meters

    altDiff = (alt1 - alt2) * 0.3048; // Feet to meters
    distance = pow(distance, 2) + pow(altDiff, 2);
    distance = sqrt(distance) * 0.00062137;
    
    return distance;
  }

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_calculateWaypointDistance
  (JNIEnv *jniEnv, jobject jobj, float lat1, float lng1, float lat2, float lng2) {
	double earthRadius = 6371000; //meters
    lat1 *= PI / 180.0;
    lat2 *= PI / 180.0;
    lng1 *= PI / 180.0;
    lng2 *= PI / 180.0;
    double dLat = lat2-lat1;
    double dLng = lng2-lng1;
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1) * cos(lat2) *
               sin(dLng/2) * sin(dLng/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    float dist = (float) (earthRadius * c);

    return dist * 3.2808;
 }

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring runway) {

	jclass jcls_string = jniEnv->FindClass("Ljava/lang/String;");
	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_string, "toString", "()Ljava/lang/String;");
	string airportString = (string)(char*) jniEnv->GetStringUTFChars(airportId, NULL);

  	
	double runwayThresholdLat = 0.0, runwayThresholdLon = 0.0, runwayEndLat = 0.0, runwayEndLon = 0.0;
	string runwayThreshold, runwayEnd;
	vector<jobject> airportNodeMap, airportNodeData;
	vector<string> runwayExitVector;
	int thresholdNodeID= 0, endNodeID = 0;
	jobject tmpJobject;
	jstring tmpJstring;
	
	jobjectArray runwayExits = Java_com_osi_gnats_engine_CEngine_getRunwayExits(jniEnv, jobj, airportId, runway);
	int cnt_runwayExits = jniEnv->GetArrayLength(runwayExits);
	for (int i = 0; i < cnt_runwayExits; i++) {
		tmpJobject = jniEnv->GetObjectArrayElement(runwayExits, i);
		tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);

		const char *runwayExit = (char*)jniEnv->GetStringUTFChars(tmpJstring, NULL );

		runwayExitVector.push_back(runwayExit);
	}
	runwayThreshold = runwayExitVector.at(0);
	runwayEnd = runwayExitVector.back();

	AirportNodeLink cur_airport_node_link = map_ground_waypoint_connectivity.at(airportString).airport_node_link;

	
	runwayThresholdLat = getGroundWaypointLatLon(&cur_airport_node_link, runwayThreshold.c_str(), 0);
	runwayThresholdLon = getGroundWaypointLatLon(&cur_airport_node_link, runwayThreshold.c_str(), 1);

	runwayEndLat = getGroundWaypointLatLon(&cur_airport_node_link, runwayEnd.c_str(), 0);
	runwayEndLon = getGroundWaypointLatLon(&cur_airport_node_link, runwayEnd.c_str(), 1);

	double runwayEnds[2][2] = {{runwayThresholdLat, runwayThresholdLon}, {runwayEndLat, runwayEndLon}};

	jclass doubleArrayClass = jniEnv->FindClass("[D");
	jobjectArray retObj = jniEnv->NewObjectArray((jsize) 2, doubleArrayClass, NULL);
	
    // Go through the firs dimension and add the second dimension arrays
    for (int i = 0; i < 2; i++)
    {
        jdoubleArray doubleArray = jniEnv->NewDoubleArray(2);
        jniEnv->SetDoubleArrayRegion(doubleArray, (jsize) 0, (jsize) 2, (jdouble*) runwayEnds[i]);
        jniEnv->SetObjectArrayElement(retObj, (jsize) i, doubleArray);
        jniEnv->DeleteLocalRef(doubleArray);
    }


	return retObj;

  }


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_getPassengerCount
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftType) {
 	const char *c_aircraftType = (char*)jniEnv->GetStringUTFChars( aircraftType, NULL );
	string aircraftTypeStr = c_aircraftType;

	int maxPax = -1;
	ifstream aircraftDataFile(g_share_dir + "/AircraftData/AircraftData.csv");

  	string line;
	const char delim = ',';
	
	while (getline(aircraftDataFile, line)) {
			vector<std::string> out;
    		tokenize(line, delim, out);
			if (out.at(0) == aircraftTypeStr)
				maxPax = stoi(out.at(1));
	}

    return maxPax;
  }


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftCost
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftType) {

  	const char *c_aircraftType = (char*)jniEnv->GetStringUTFChars( aircraftType, NULL );
	string aircraftTypeStr = c_aircraftType;

	double aircraftCost = -1;
	ifstream aircraftDataFile(g_share_dir + "/AircraftData/AircraftData.csv");

  	string line;
	const char delim = ',';
	
	while (getline(aircraftDataFile, line)) {
			vector<std::string> out;
    		tokenize(line, delim, out);
			if (out.at(0) == aircraftTypeStr)
				aircraftCost = stod(out.at(2));
	}

    return aircraftCost;
  }



JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getVelocityAlignmentWithRunway
  (JNIEnv *jniEnv, jobject jobj, jint sessionId, jstring aircraftId, jstring procedure) {

  	const char *c_procedure = (char*)jniEnv->GetStringUTFChars( procedure, NULL );
	string procedureStr = c_procedure;

	double alignmentAngle, aircraftCourse, runwayHeading;

	jstring airport = NULL, runway= NULL;

	if(procedureStr == "DEPARTURE") {
		airport = Java_com_osi_gnats_engine_CEngine_getDepartureAirport(jniEnv, jobj, aircraftId);
		runway = Java_com_osi_gnats_engine_CEngine_getDepartureRunway(jniEnv, jobj, aircraftId);
	}
	else if(procedureStr == "ARRIVAL") {
		airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId);
		runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId);
	}

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	aircraftCourse = d_aircraft_soa.course_rad[c_flightSeq] * 180 / PI;

	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, airport, runway);
	jdoubleArray thresholdObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 0);
	jdouble* thresholdPoint = jniEnv->GetDoubleArrayElements(thresholdObj, NULL);
	jdoubleArray endObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 1);
	jdouble* endPoint = jniEnv->GetDoubleArrayElements(endObj, NULL);


	runwayHeading = Java_com_osi_gnats_engine_CEngine_calculateBearing(jniEnv, jobj, thresholdPoint[0], thresholdPoint[1], endPoint[0], endPoint[1]);
	alignmentAngle = aircraftCourse - runwayHeading;

	return alignmentAngle;

  }


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getDistanceToRunwayEnd
  (JNIEnv *jniEnv, jobject jobj, jint sessionId, jstring aircraftId) {
  	double currentLat, currentLon;
	jstring arrivalRunway, arrivalAirport;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	currentLat = d_aircraft_soa.latitude_deg[c_flightSeq];
	currentLon = d_aircraft_soa.longitude_deg[c_flightSeq];

	
	arrivalAirport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId);
	arrivalRunway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId);
	
	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, arrivalAirport, arrivalRunway);

	int cnt_waypoint = jniEnv->GetArrayLength(ends);
	jdoubleArray endObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 1);
	jdouble* endPoint = jniEnv->GetDoubleArrayElements(endObj, NULL);

	return Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, currentLat, currentLon, 0.0, endPoint[0], endPoint[1], 0.0);
  }


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getDistanceToRunwayThreshold
  (JNIEnv *jniEnv, jobject jobj, jint sessionId, jstring aircraftId) {
	double currentLat, currentLon;
	jstring arrivalRunway, arrivalAirport;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	currentLat = d_aircraft_soa.latitude_deg[c_flightSeq];
	currentLon = d_aircraft_soa.longitude_deg[c_flightSeq];

	
	arrivalAirport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId);
	arrivalRunway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId);
	
	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, arrivalAirport, arrivalRunway);

	int cnt_waypoint = jniEnv->GetArrayLength(ends);
	jdoubleArray thresholdObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 0);
	jdouble* thresholdPoint = jniEnv->GetDoubleArrayElements(thresholdObj, NULL);

	return Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, currentLat, currentLon, 0.0, thresholdPoint[0], thresholdPoint[1], 0.0);

}


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setAircraftBookValue
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId, jdouble aircraftBookValue) {
    string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	aircraftAndCargoData[c_string_acid]["AIRCRAFT_BOOK_VALUE"] = aircraftBookValue;
  	return 1;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftBookValue
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId) {
    string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	return aircraftAndCargoData[c_string_acid]["AIRCRAFT_BOOK_VALUE"];
}


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setCargoWorth
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId, jdouble cargoWorth) {
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	aircraftAndCargoData[c_string_acid]["CARGO_WORTH"] = cargoWorth;
  	return 1;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getCargoWorth
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId) {
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	return aircraftAndCargoData[c_string_acid]["CARGO_WORTH"];
}


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setPassengerLoadFactor
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId, jdouble paxLoadFactor) {
    string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	aircraftAndCargoData[c_string_acid]["PAX_LOAD_FACTOR"] = paxLoadFactor;
  	return 1;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getPassengerLoadFactor
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId) {
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	return aircraftAndCargoData[c_string_acid]["PAX_LOAD_FACTOR"];
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setTouchdownPointOnRunway
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId, jdouble latitude, jdouble longitude) {
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	aircraftRunwayData[c_string_acid]["LANDING"].first = latitude;
  	aircraftRunwayData[c_string_acid]["LANDING"].second = longitude;
  	return 1;
}


JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getTouchdownPointOnRunway
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId) {
  	double touchdownPoint[2];
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);

  	touchdownPoint[0] = aircraftRunwayData[c_string_acid]["LANDING"].first;
  	touchdownPoint[1] = aircraftRunwayData[c_string_acid]["LANDING"].second;

    jdoubleArray retDoubleArray = jniEnv->NewDoubleArray(2);
    jniEnv->SetDoubleArrayRegion( retDoubleArray, 0, 2, (const jdouble*) touchdownPoint );

	return retDoubleArray;
}


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setTakeOffPointOnRunway
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId, jdouble latitude, jdouble longitude) {
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);
  	aircraftRunwayData[c_string_acid]["TAKEOFF"].first = latitude;
  	aircraftRunwayData[c_string_acid]["TAKEOFF"].second = longitude;
  	return 1;
}


JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getTakeOffPointOnRunway
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId) {
  	double liftoffPoint[2];
  	string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraftId, NULL);

  	liftoffPoint[0] = aircraftRunwayData[c_string_acid]["TAKEOFF"].first;
  	liftoffPoint[1] = aircraftRunwayData[c_string_acid]["TAKEOFF"].second;

    jdoubleArray retDoubleArray = jniEnv->NewDoubleArray(2);
    jniEnv->SetDoubleArrayRegion( retDoubleArray, 0, 2, (const jdouble*) liftoffPoint );

	return retDoubleArray;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1FlightPhaseSequence
  (JNIEnv *jniEnv, jobject jobj, jstring filename) {
	string c_filename = (string)(char*) jniEnv->GetStringUTFChars(filename, NULL);
	return load_flightSequence (c_filename);
}


JNIEXPORT void JNICALL Java_com_osi_gnats_engine_CEngine_clear_1FlightPhaseSequence
  (JNIEnv *jniEnv, jobject jobj) {
	clear_flightSequence();
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1aviationOccurenceProfile
  (JNIEnv *jniEnv, jobject jobj, jstring j_dirPath) {
	jint retValue = 1;

	const char *c_dirPath = (char*)jniEnv->GetStringUTFChars( j_dirPath, NULL );

	retValue = load_aviationOccurenceProfile(c_dirPath);

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_load_1flightphase_1aviationOccurence_1mapping
  (JNIEnv *jniEnv, jobject jobj, jstring j_dirPath) {
	jint retValue = 1;

	const char *c_dirPath = (char*)jniEnv->GetStringUTFChars( j_dirPath, NULL );

	retValue = load_flightphase_aviationOccurence_mapping(c_dirPath);

	return retValue;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setSampleWeatherHazard
  (JNIEnv *jniEnv, jobject jobj, jdoubleArray weatherRegionBounds) {

  jdouble* weatherRegionBoundsArray = jniEnv->GetDoubleArrayElements(weatherRegionBounds, NULL);

  std::vector<double> weatherRegionsBoundVector (weatherRegionBoundsArray, weatherRegionBoundsArray + 5);

  weatherSample = weatherRegionsBoundVector;
      
  return 1;
}


JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_calculateRisk
  (JNIEnv *jniEnv, jobject jobj, jstring flightData, jboolean cifpExists) {
    
    string c_string_flightdata = (string)(char*) jniEnv->GetStringUTFChars(flightData, NULL);
    istringstream iss1(c_string_flightdata);
	vector<string> flightburst;
	vector<string> flight1;
	vector<string> flight2;
	
    for (string token; getline(iss1, token, '^'); )
        flightburst.push_back(move(token));
        
    istringstream iss2(flightburst.at(0));
    for (string token; getline(iss2, token, ','); )
        flight1.push_back(move(token));
        
    if (flightburst.size() == 2) {
	    istringstream iss3(flightburst.at(1));
	    for (string token; getline(iss3, token, ','); )
	        flight2.push_back(move(token));
	}
	        
	// AOCType, timeToGo, aircraft1, aircraft2, phase1, phase2, latitude, longitude
	vector<string>retVal;
	double lat1, lat2, lon1, lon2, alt1, alt2, course1, course2, speed1, speed2, rocd1, rocd2;
	int phase1, phase2;
	string acid1, acid2;
    
	lat1 = stod(flight1.at(0));
	lon1 = stod(flight1.at(1));
	alt1 = stod(flight1.at(2));
	course1 = stod(flight1.at(3));
	speed1 = stod(flight1.at(4));
	rocd1 = stod(flight1.at(5));
	acid1 = flight1.at(6);
	phase1 = stoi(flight1.at(7));

	if (flightburst.size() == 2) {
		lat2 = stod(flight2.at(0));
		lon2 = stod(flight2.at(1));
		alt2 = stod(flight2.at(2));
		course2 = stod(flight2.at(3));
		speed2 = stod(flight2.at(4));
		rocd2 = stod(flight2.at(5));
		acid2 = flight2.at(6);
		phase2 = stoi(flight2.at(7));
	}

	// Check GCOL
	double distanceToGoToWeather, distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, lat2, lon2, alt2);
	double timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);

	angleConverging = 1;

	if ((flightburst.size() == 2) && distanceToGo < 0.1  && angleConverging) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("GCOL");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(acid2);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(phase2));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check MAC
	if ((flightburst.size() == 2) && distanceToGo < 3 && phase1 > 6 && angleConverging) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("MAC");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(acid2);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(phase2));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check WAKE
	if ((flightburst.size() == 2) && (truncf(course1 * 10) / 10 == truncf(course2 * 10) / 10) && distanceToGo < 3 && (6 < phase1 < 10) &&  (6 < phase2 < 10) ) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("WAKE");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(acid2);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(phase2));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	
	// Check WSTRW
	if (weatherSample.size() > 0) {
		distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, weatherSample.at(0), weatherSample.at(1), weatherSample.at(4));
		timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
		if (distanceToGo < 4.7 && phase1 > 6) {
			retVal.push_back("__AOC_DELIM__");
			retVal.push_back("WSTRW");
			retVal.push_back(to_string(timeToGo));
			retVal.push_back(acid1);
			retVal.push_back(to_string(phase1));
			retVal.push_back(to_string(lat1));
			retVal.push_back(to_string(lon1));
		}
	}

	
	// Check CFIT
	double terrainElevation = Java_com_osi_gnats_engine_CEngine_getElevation(jniEnv, jobj, lat1, lon1, cifpExists);
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, lat1, lon1, 0.0);
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo < 4.7 && 21 > phase1 > 6) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("CFIT");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check TE
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, lat1 + (sin(1/(lat1 * PI / 180.0))), lon1 + (sin(1/(lon1 * PI / 180.0))), alt1);
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo < 86.52 && ((22 > phase1 > 19) || (4 > phase1 > 1))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("TE");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check RE
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, lat1 + (sin(1/(lat1 * PI / 180.0))), lon1 + (sin(1/(lon1 * PI / 180.0))), alt1);
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo < 86.52 && ((20 > phase1 > 17) || (6 > phase1 > 3))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("RE");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check RI
	distanceToGoToWeather, distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, lat2, lon2, alt2);
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if ((flightburst.size() == 2) && distanceToGo < .375  && angleConverging) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("RI");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(acid2);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(phase2));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check OS
	jobject tmpJobject;
	jstring tmpJstring;
	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");
	jstring runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	string c_string_runway = (string)(char*) jniEnv->GetStringUTFChars(runway, NULL);
	jstring airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	string c_string_airport = (string)(char*) jniEnv->GetStringUTFChars(airport, NULL);
	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_String, "toString", "()Ljava/lang/String;");	
	jobjectArray jObjArr = Java_com_osi_gnats_engine_CEngine_getRunwayEnds(jniEnv, jobj, jniEnv->NewStringUTF(c_string_airport.c_str()), jniEnv->NewStringUTF(c_string_runway.c_str()));
	tmpJobject = jniEnv->GetObjectArrayElement(jObjArr, 1);
	tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	string runwayEnd = (string)(char*) jniEnv->GetStringUTFChars(tmpJstring, NULL);
	
	airportMapData.clear();
	jobjectArray airportMap = Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data(jniEnv, jobj, airport);
		
	vector<double> targetPointLoc;
	istringstream issData(airportMapData[runwayEnd].at(0));
    for (string token; getline(issData, token, ','); )
        targetPointLoc.push_back(stod(move(token)));
        	
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, targetPointLoc.at(0), targetPointLoc.at(1), get_airport_elevation(c_string_airport));
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo < .52 && ((20 > phase1 > 17) || (6 > phase1 > 3))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("OS");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check CTOL
	runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_runway = (string)(char*) jniEnv->GetStringUTFChars(runway, NULL);
	airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_airport = (string)(char*) jniEnv->GetStringUTFChars(airport, NULL);
	jObjArr = Java_com_osi_gnats_engine_CEngine_getRunwayEnds(jniEnv, jobj, jniEnv->NewStringUTF(c_string_airport.c_str()), jniEnv->NewStringUTF(c_string_runway.c_str()));
	tmpJobject = jniEnv->GetObjectArrayElement(jObjArr, 0);
	tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	runwayEnd = (string)(char*) jniEnv->GetStringUTFChars(tmpJstring, NULL);
	
	airportMap = Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data(jniEnv, jobj, airport);
	
	targetPointLoc.clear();	
	targetPointLoc;
	istringstream issData1(airportMapData[runwayEnd].at(0));
    for (string token; getline(issData1, token, ','); )
        targetPointLoc.push_back(stod(move(token)));
        	
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, targetPointLoc.at(0), targetPointLoc.at(1), get_airport_elevation(c_string_airport));
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo < 649.34 && ((7 > phase1))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("CTOL");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check TI
	runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_runway = (string)(char*) jniEnv->GetStringUTFChars(runway, NULL);
	airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_airport = (string)(char*) jniEnv->GetStringUTFChars(airport, NULL);
	jObjArr = Java_com_osi_gnats_engine_CEngine_getRunwayEnds(jniEnv, jobj, jniEnv->NewStringUTF(c_string_airport.c_str()), jniEnv->NewStringUTF(c_string_runway.c_str()));
	tmpJobject = jniEnv->GetObjectArrayElement(jObjArr, 0);
	tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	runwayEnd = (string)(char*) jniEnv->GetStringUTFChars(tmpJstring, NULL);
	
	airportMap = Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data(jniEnv, jobj, airport);
	
	targetPointLoc.clear();	
	targetPointLoc;
	istringstream issData2(airportMapData[runwayEnd].at(0));
    for (string token; getline(issData2, token, ','); )
        targetPointLoc.push_back(stod(move(token)));
        	
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, targetPointLoc.at(0), targetPointLoc.at(1), get_airport_elevation(c_string_airport));
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo > .6 && ((18 < phase1))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("TI");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	
	// Check US
	runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_runway = (string)(char*) jniEnv->GetStringUTFChars(runway, NULL);
	airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_airport = (string)(char*) jniEnv->GetStringUTFChars(airport, NULL);
	jObjArr = Java_com_osi_gnats_engine_CEngine_getRunwayEnds(jniEnv, jobj, jniEnv->NewStringUTF(c_string_airport.c_str()), jniEnv->NewStringUTF(c_string_runway.c_str()));
	tmpJobject = jniEnv->GetObjectArrayElement(jObjArr, 0);
	tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	runwayEnd = (string)(char*) jniEnv->GetStringUTFChars(tmpJstring, NULL);
	
	airportMap = Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data(jniEnv, jobj, airport);
	
	targetPointLoc.clear();	
	targetPointLoc;
	istringstream issData3(airportMapData[runwayEnd].at(0));
    for (string token; getline(issData3, token, ','); )
        targetPointLoc.push_back(stod(move(token)));
        	
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, targetPointLoc.at(0), targetPointLoc.at(1), get_airport_elevation(c_string_airport));
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo > .6 && ((18 < phase1))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("US");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	// Check ML
	runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_runway = (string)(char*) jniEnv->GetStringUTFChars(runway, NULL);
	airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, jniEnv->NewStringUTF(acid1.c_str()));
	c_string_airport = (string)(char*) jniEnv->GetStringUTFChars(airport, NULL);
	jObjArr = Java_com_osi_gnats_engine_CEngine_getRunwayEnds(jniEnv, jobj, jniEnv->NewStringUTF(c_string_airport.c_str()), jniEnv->NewStringUTF(c_string_runway.c_str()));
	tmpJobject = jniEnv->GetObjectArrayElement(jObjArr, 0);
	tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	runwayEnd = (string)(char*) jniEnv->GetStringUTFChars(tmpJstring, NULL);
	
	airportMap = Java_com_osi_gnats_engine_CEngine_getLayout_1node_1data(jniEnv, jobj, airport);
	
	targetPointLoc.clear();	
	targetPointLoc;
	istringstream issData4(airportMapData[runwayEnd].at(0));
    for (string token; getline(issData4, token, ','); )
        targetPointLoc.push_back(stod(move(token)));
        	
	distanceToGo = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, lat1, lon1, alt1, targetPointLoc.at(0), targetPointLoc.at(1), get_airport_elevation(c_string_airport));
	timeToGo = (distanceToGo * 1609.34) / (speed1 * 0.514444);
	if (distanceToGo > .6 && ((18 < phase1))) {
		retVal.push_back("__AOC_DELIM__");
		retVal.push_back("ML");
		retVal.push_back(to_string(timeToGo));
		retVal.push_back(acid1);
		retVal.push_back(to_string(phase1));
		retVal.push_back(to_string(lat1));
		retVal.push_back(to_string(lon1));
	}
	
	jobjectArray retArray = NULL;
	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");
	retArray = (jobjectArray)jniEnv->NewObjectArray(retVal.size(), jcls, jniEnv->NewStringUTF(""));
	int i = 0;
	for(auto element = retVal.begin(); element!=retVal.end(); ++element) {
		string aocElement = *element;
		jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(aocElement.c_str()));
		i++;
	}

	return retArray;
	
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setRegionOfRegard
  (JNIEnv *jniEnv, jobject jobj, jstring aircraft, jdoubleArray regionBounds) {

  string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraft, NULL);

  jdouble* regionBoundsArray = jniEnv->GetDoubleArrayElements(regionBounds, NULL);

  std::vector<double> regionsBoundVector (regionBoundsArray, regionBoundsArray + 6);

  regionOfRegard[c_string_acid].push_back(regionsBoundVector);
  
  return 1;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getRegionOfRegard
  (JNIEnv *jniEnv, jobject jobj, jstring aircraft) {
	
  jobjectArray retObj = NULL;
  jclass doubleArrayClass = jniEnv->FindClass("[D");
	
  string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraft, NULL);
  
  double** temp;
  temp = new double*[regionOfRegard[c_string_acid].size()];
  for(unsigned i=0; (i < regionOfRegard[c_string_acid].size()); i++)
  { 
     temp[i] = new double[6];
     for(unsigned j=0; (j < 6); j++)
     {
         temp[i][j] = regionOfRegard[c_string_acid][i][j];
     } 
  }

  retObj = jniEnv->NewObjectArray((jsize) regionOfRegard[c_string_acid].size(), doubleArrayClass, NULL);
	
  for (int i = 0; i < regionOfRegard[c_string_acid].size(); i++)
  {
    jdoubleArray doubleArray = jniEnv->NewDoubleArray(6);
    jniEnv->SetDoubleArrayRegion(doubleArray, (jsize) 0, (jsize) 6, (jdouble*) temp[i]);
    jniEnv->SetObjectArrayElement(retObj, (jsize) i, doubleArray);
    jniEnv->DeleteLocalRef(doubleArray);
  }
  
  return retObj;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAircraftInRegionOfRegard
  (JNIEnv *jniEnv, jobject jobj, jstring aircraft) {
  jclass jcls_string = jniEnv->FindClass("Ljava/lang/String;");
  jmethodID methodID_toString = jniEnv->GetMethodID(jcls_string, "toString", "()Ljava/lang/String;");
  jobjectArray retArray = NULL;
  jobjectArray tmpArray = NULL;
  vector<string> tmpVec;
  jobject tmpJobject;
  jstring tmpJstring;
  
  string c_string_acid = (string)(char*) jniEnv->GetStringUTFChars(aircraft, NULL);
  
  for(vector<double> temp: regionOfRegard[c_string_acid]) {
  	tmpArray = Java_com_osi_gnats_engine_CEngine_getAircraftIds_doubleVals(jniEnv, jobj, temp[0], temp[1], temp[2], temp[3], temp[4], temp[5]);
  	int cnt_aircraft = jniEnv->GetArrayLength(tmpArray);
	if (cnt_aircraft > 0) {
		for (int i = 0; i < cnt_aircraft; i++) {
			tmpJobject = jniEnv->GetObjectArrayElement(tmpArray, i);
			tmpJstring = (jstring)jniEnv->CallObjectMethod(tmpJobject, methodID_toString);
	
			const char *c_tmp_flight = (char*)jniEnv->GetStringUTFChars(tmpJstring, NULL );
			string c_flight = c_tmp_flight;
	
			tmpVec.push_back(c_flight);
	    }
    }
  }
  
  retArray = (jobjectArray)jniEnv->NewObjectArray(tmpVec.size(), jcls_string, jniEnv->NewStringUTF(""));
  int i = 0;
  for(auto element = tmpVec.begin(); element!=tmpVec.end(); ++element) {
	string aircraft = *element;
	jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(aircraft.c_str()));
	i++;
  }
  return retArray;

}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getL2Distance
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring aircraft1, jstring aircraft2) {
  double retVal = -1;
  
  const char *c_airportId = (char*)jniEnv->GetStringUTFChars( airportId, NULL );
  string string_airportId(c_airportId); // Convert char* to string
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraft1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  const char *c_aircraft2 = (char*)jniEnv->GetStringUTFChars( aircraft2, NULL );
  string string_aircraft2(c_aircraft2); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  int c_flightSeq2 = select_flightSeq_by_aircraftId(string_aircraft2);
  
  if (c_flightSeq1 != -1 && c_flightSeq2 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq2], h_aircraft_soa.longitude_deg[c_flightSeq2], h_aircraft_soa.altitude_ft[c_flightSeq2]);
  
  return retVal;
  
}

JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getTimeToObjectOfInterest
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring aircraftId1, jfloat latitude, jfloat longitude) {
 
  double retVal = -1;
  
  const char *c_airportId = (char*)jniEnv->GetStringUTFChars( airportId, NULL );
  string string_airportId(c_airportId); // Convert char* to string
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], latitude, longitude, h_aircraft_soa.altitude_ft[c_flightSeq1]) / d_aircraft_soa.latitude_deg[c_flightSeq1] * 1.15;
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getDistanceToObjectOfInterest
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring aircraftId1, jfloat latitude, jfloat longitude) {
  
  double retVal = -1;
  
  const char *c_airportId = (char*)jniEnv->GetStringUTFChars( airportId, NULL );
  string string_airportId(c_airportId); // Convert char* to string
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], latitude, longitude, h_aircraft_soa.altitude_ft[c_flightSeq1]);
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getTimeToVehicleContact
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring aircraftId2) {

  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  const char *c_aircraft2 = (char*)jniEnv->GetStringUTFChars( aircraftId2, NULL );
  string string_aircraft2(c_aircraft2); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  int c_flightSeq2 = select_flightSeq_by_aircraftId(string_aircraft2);
  
  if (c_flightSeq1 != -1 && c_flightSeq2 != -1) {  
  	FlightPlan curFlightPlan1 = g_flightplans.at(c_flightSeq1);
  	FlightPlan curFlightPlan2 = g_flightplans.at(c_flightSeq2);

	int loopLength = (curFlightPlan1.route.size() >= curFlightPlan2.route.size()) ? curFlightPlan2.route.size() : curFlightPlan1.route.size();
	
  	for (int i=0; i < loopLength; i++) {  	
  		if(curFlightPlan1.route.at(i).wpname == curFlightPlan2.route.at(i).wpname) {
  			    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq2], h_aircraft_soa.longitude_deg[c_flightSeq2], h_aircraft_soa.altitude_ft[c_flightSeq2]);
  		}
  	}
  }
  return retVal;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getTimeToPavementEdge
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1) {
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq1] * sin(d_aircraft_soa.course_rad[c_flightSeq1]), h_aircraft_soa.longitude_deg[c_flightSeq1] * sin(d_aircraft_soa.course_rad[c_flightSeq1]), h_aircraft_soa.altitude_ft[c_flightSeq1]) / (d_aircraft_soa.tas_knots[c_flightSeq1] * 0.514444);

  return retVal;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfLineOfSightChange
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring aircraftId2) {
  
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  const char *c_aircraft2 = (char*)jniEnv->GetStringUTFChars( aircraftId2, NULL );
  string string_aircraft2(c_aircraft2); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  int c_flightSeq2 = select_flightSeq_by_aircraftId(string_aircraft2);
  
  if (c_flightSeq1 != -1 && c_flightSeq2 != -1 && angleConverging)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq2], h_aircraft_soa.longitude_deg[c_flightSeq2], h_aircraft_soa.altitude_ft[c_flightSeq2])  / (d_aircraft_soa.tas_knots[c_flightSeq1] * 0.514444);
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToPavementEdge
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jint timeSteps) {
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq1] * sin(d_aircraft_soa.course_rad[c_flightSeq1]), h_aircraft_soa.longitude_deg[c_flightSeq1] * sin(d_aircraft_soa.course_rad[c_flightSeq1]), h_aircraft_soa.altitude_ft[c_flightSeq1]) / (timeSteps);

  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToVehicle
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring aircraftId2, jint timeSteps) {
 
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  const char *c_aircraft2 = (char*)jniEnv->GetStringUTFChars( aircraftId2, NULL );
  string string_aircraft2(c_aircraft2); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  int c_flightSeq2 = select_flightSeq_by_aircraftId(string_aircraft2);
  
  if (c_flightSeq1 != -1 && c_flightSeq2 != -1) {  
  	FlightPlan curFlightPlan1 = g_flightplans.at(c_flightSeq1);
  	FlightPlan curFlightPlan2 = g_flightplans.at(c_flightSeq2);

	int loopLength = (curFlightPlan1.route.size() >= curFlightPlan2.route.size()) ? curFlightPlan2.route.size() : curFlightPlan1.route.size();
	
  	for (int i=0; i < loopLength; i++) {  	
  		if(curFlightPlan1.route.at(i).wpname == curFlightPlan2.route.at(i).wpname) {
  			    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq2], h_aircraft_soa.longitude_deg[c_flightSeq2], h_aircraft_soa.altitude_ft[c_flightSeq2]) / (timeSteps);
  		}
  	}
  }
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToWaypoint__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring waypoint, jint timeSteps) {

  double retVal = -1;
  jdoubleArray waypointLocation;

  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
 
  waypointLocation = Java_com_osi_gnats_engine_CEngine_getWaypoint_1Latitude_1Longitude_1deg(jniEnv, jobj, waypoint);
  
  jdouble* waypointLocationArray = jniEnv->GetDoubleArrayElements(waypointLocation, NULL);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], waypointLocationArray[0], waypointLocationArray[1], h_aircraft_soa.altitude_ft[c_flightSeq1]) / (timeSteps);

  return retVal;
 
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToWaypoint__Ljava_lang_String_2FFI
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jfloat latitude, jfloat longitude, jint timeSteps) {
 
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], latitude, longitude, h_aircraft_soa.altitude_ft[c_flightSeq1]) / timeSteps;
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToEvent
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jfloat latitude, jfloat longitude, jint timeSteps) {
 
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], latitude, longitude, h_aircraft_soa.altitude_ft[c_flightSeq1]) / timeSteps;
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToWeatherEvent
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jdoubleArray weatherBounds) {
 
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  jdouble* weatherBoundsArray = jniEnv->GetDoubleArrayElements(weatherBounds, NULL);
    
  if (c_flightSeq1 != -1)
    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], weatherBoundsArray[0], weatherBoundsArray[1], h_aircraft_soa.altitude_ft[c_flightSeq1]);
  
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToWakeVortex
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring aircraftId2){
 
  double retVal = -1;
  
  const char *c_aircraft1 = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
  string string_aircraft1(c_aircraft1); // Convert char* to string
  
  const char *c_aircraft2 = (char*)jniEnv->GetStringUTFChars( aircraftId2, NULL );
  string string_aircraft2(c_aircraft2); // Convert char* to string
  
  int c_flightSeq1 = select_flightSeq_by_aircraftId(string_aircraft1);
  int c_flightSeq2 = select_flightSeq_by_aircraftId(string_aircraft2);
  
  if (c_flightSeq1 != -1 && c_flightSeq2 != -1) {  
  	FlightPlan curFlightPlan1 = g_flightplans.at(c_flightSeq1);
  	FlightPlan curFlightPlan2 = g_flightplans.at(c_flightSeq2);

	int loopLength = (curFlightPlan1.route.size() >= curFlightPlan2.route.size()) ? curFlightPlan2.route.size() : curFlightPlan1.route.size();
	
  	for (int i=0; i < loopLength; i++) {  	
  		if(curFlightPlan1.route.at(i).wpname == curFlightPlan2.route.at(i).wpname) {
  			    retVal = Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, h_aircraft_soa.latitude_deg[c_flightSeq1], h_aircraft_soa.longitude_deg[c_flightSeq1], h_aircraft_soa.altitude_ft[c_flightSeq1], h_aircraft_soa.latitude_deg[c_flightSeq2], h_aircraft_soa.longitude_deg[c_flightSeq2], h_aircraft_soa.altitude_ft[c_flightSeq2]);
  		}
  	}
  }
  return retVal;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfVelocityAlignmentToRunway
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jstring procedure, jint timeSteps) {
 
    const char *c_procedure = (char*)jniEnv->GetStringUTFChars( procedure, NULL );
	string procedureStr = c_procedure;

	double alignmentAngle, aircraftCourse, runwayHeading;

	jstring airport = NULL, runway= NULL;

	if(procedureStr == "DEPARTURE") {
		airport = Java_com_osi_gnats_engine_CEngine_getDepartureAirport(jniEnv, jobj, aircraftId1);
		runway = Java_com_osi_gnats_engine_CEngine_getDepartureRunway(jniEnv, jobj, aircraftId1);
	}
	else if(procedureStr == "ARRIVAL") {
		airport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId1);
		runway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId1);
	}

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	aircraftCourse = d_aircraft_soa.course_rad[c_flightSeq] * 180 / PI;

	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, airport, runway);
	jdoubleArray thresholdObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 0);
	jdouble* thresholdPoint = jniEnv->GetDoubleArrayElements(thresholdObj, NULL);
	jdoubleArray endObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 1);
	jdouble* endPoint = jniEnv->GetDoubleArrayElements(endObj, NULL);


	runwayHeading = Java_com_osi_gnats_engine_CEngine_calculateBearing(jniEnv, jobj, thresholdPoint[0], thresholdPoint[1], endPoint[0], endPoint[1]);
	alignmentAngle = aircraftCourse - runwayHeading;

	return alignmentAngle / timeSteps;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToRunwayEnd
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jint timeSteps) {
 
    double currentLat, currentLon;
	jstring arrivalRunway, arrivalAirport;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	currentLat = d_aircraft_soa.latitude_deg[c_flightSeq];
	currentLon = d_aircraft_soa.longitude_deg[c_flightSeq];

	
	arrivalAirport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId1);
	arrivalRunway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId1);
	
	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, arrivalAirport, arrivalRunway);

	int cnt_waypoint = jniEnv->GetArrayLength(ends);
	jdoubleArray endObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 1);
	jdouble* endPoint = jniEnv->GetDoubleArrayElements(endObj, NULL);

	return Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, currentLat, currentLon, 0.0, endPoint[0], endPoint[1], 0.0) / timeSteps;
  
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getRateOfApproachToRunwayThreshold
  (JNIEnv *jniEnv, jobject jobj, jstring aircraftId1, jint timeSteps) {
 
    double currentLat, currentLon;
	jstring arrivalRunway, arrivalAirport;

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftId1, NULL );
	string c_string_acid = c_acid;

	int c_flightSeq = select_flightSeq_by_aircraftId(c_string_acid);

	currentLat = d_aircraft_soa.latitude_deg[c_flightSeq];
	currentLon = d_aircraft_soa.longitude_deg[c_flightSeq];

	
	arrivalAirport = Java_com_osi_gnats_engine_CEngine_getArrivalAirport(jniEnv, jobj, aircraftId1);
	arrivalRunway = Java_com_osi_gnats_engine_CEngine_getArrivalRunway(jniEnv, jobj, aircraftId1);
	
	jobjectArray ends = Java_com_osi_gnats_engine_CEngine_getRunwayEndpoints(jniEnv, jobj, arrivalAirport, arrivalRunway);

	int cnt_waypoint = jniEnv->GetArrayLength(ends);
	jdoubleArray thresholdObj = (jdoubleArray)jniEnv->GetObjectArrayElement(ends, 0);
	jdouble* thresholdPoint = jniEnv->GetDoubleArrayElements(thresholdObj, NULL);

	return Java_com_osi_gnats_engine_CEngine_calculateDistance(jniEnv, jobj, currentLat, currentLon, 0.0, thresholdPoint[0], thresholdPoint[1], 0.0); 
  
}


JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAllGroundVehicleIds
  (JNIEnv *jniEnv, jobject jobj) {
	jobjectArray retArray = NULL;

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	retArray = (jobjectArray)jniEnv->NewObjectArray(map_VehicleId_Seq.size(), jcls, jniEnv->NewStringUTF(""));

	int i = 0;

	map<string, int>::iterator ite_map;
	for (ite_map = map_VehicleId_Seq.begin(); ite_map != map_VehicleId_Seq.end(); ite_map++) {
		string cstring_vehicle_id = ite_map->first;

		jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(cstring_vehicle_id.c_str()));

		i++;
	}

	return retArray;
}

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getAssignedGroundVehicleIds
  (JNIEnv *jniEnv, jobject jobj, jstring j_userName) {
	jobjectArray retArray = NULL;

	const char *c_userName = (char*)jniEnv->GetStringUTFChars( j_userName, 0 );
	string c_string_userName(c_userName);

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	set<string> set_assignedGroundVehicles;

	map<string, string>::iterator ite_map;
	for (ite_map = map_groundVehicle_owner.begin(); ite_map != map_groundVehicle_owner.end(); ite_map++) {
		if (c_string_userName.find(ite_map->second) != string::npos) {
			set_assignedGroundVehicles.insert(ite_map->first);
		}
	}

	retArray = (jobjectArray)jniEnv->NewObjectArray(set_assignedGroundVehicles.size(), jcls, jniEnv->NewStringUTF(""));

	int tmpIdx = 0;

	set<string>::iterator ite_set;
	for (ite_set = set_assignedGroundVehicles.begin(); ite_set != set_assignedGroundVehicles.end(); ite_set++) {
		jniEnv->SetObjectArrayElement(retArray, tmpIdx, jniEnv->NewStringUTF(ite_set->c_str()));

		tmpIdx++;
	}

	return retArray;
}

JNIEXPORT jobject JNICALL Java_com_osi_gnats_engine_CEngine_select_1groundVehicle
  (JNIEnv *jniEnv, jobject jobj, jint j_sessionId, jstring groundVehicleId) {
	jobject retObject = NULL;

	const char *c_gvid = (char*)jniEnv->GetStringUTFChars( groundVehicleId, NULL );
	string string_gvid(c_gvid); // Convert char* to string

	int c_sessionId = j_sessionId;

	jclass jcls = jniEnv->FindClass("com/osi/gnats/groundvehicle/GroundVehicle");
	jclass jcls_string = jniEnv->FindClass("Ljava/lang/String;");

	jfieldID fieldId_sessionId = jniEnv->GetFieldID(jcls, "sessionId", "I");

	jfieldID fieldId_gvid = jniEnv->GetFieldID(jcls, "gvid", "Ljava/lang/String;");
	jfieldID fieldId_aircraftId = jniEnv->GetFieldID(jcls, "acid", "Ljava/lang/String;");
	jfieldID fieldId_airportId = jniEnv->GetFieldID(jcls, "airportId", "Ljava/lang/String;");

	jfieldID fieldId_flag_external_groundvehicle = jniEnv->GetFieldID(jcls, "flag_external_groundvehicle", "Z");

	jfieldID fieldId_latitude = jniEnv->GetFieldID(jcls, "latitude", "F");
	jfieldID fieldId_longitude = jniEnv->GetFieldID(jcls, "longitude", "F");
	jfieldID fieldId_altitude = jniEnv->GetFieldID(jcls, "altitude", "F");
	jfieldID fieldId_speed = jniEnv->GetFieldID(jcls, "speed", "F");
	jfieldID fieldId_course = jniEnv->GetFieldID(jcls, "course", "F");

	jfieldID fieldId_drive_plan_latitude_array = jniEnv->GetFieldID(jcls, "drive_plan_latitude_array", "[F");
	jfieldID fieldId_drive_plan_longitude_array = jniEnv->GetFieldID(jcls, "drive_plan_longitude_array", "[F");
	jfieldID fieldId_drive_plan_waypoint_name_array = jniEnv->GetFieldID(jcls, "drive_plan_waypoint_name_array", "[Ljava/lang/String;");

	jfieldID fieldId_departure_time = jniEnv->GetFieldID(jcls, "departure_time", "F");
	jfieldID fieldId_drive_plan_length = jniEnv->GetFieldID(jcls, "drive_plan_length", "I");

	jfieldID fieldId_drive_target_waypoint_index = jniEnv->GetFieldID(jcls, "target_waypoint_index", "I");
	jfieldID fieldId_drive_target_waypoint_name = jniEnv->GetFieldID(jcls, "target_waypoint_name", "Ljava/lang/String;");

	jmethodID methodId_constructor = jniEnv->GetMethodID(jcls, "<init>", "(Ljava/lang/String;)V");

	retObject = jniEnv->NewObject(jcls, methodId_constructor, groundVehicleId);

	jniEnv->SetIntField(retObject, fieldId_sessionId, c_sessionId);

	waypoint_node_t* waypointNode_ptr = NULL;

	int tmp_target_waypoint_index = -1;

	int c_groundVehicleSeq = select_groundVehicleSeq_by_groundVehicleId(string_gvid);

	bool c_flag_external_groundvehicle = false;

	int c_target_waypoint_index = -1;

	if (c_groundVehicleSeq > -1) {

		jniEnv->SetFloatField(retObject, fieldId_latitude, groundVehicleStates.at(c_groundVehicleSeq).latitude);
		jniEnv->SetFloatField(retObject, fieldId_longitude, groundVehicleStates.at(c_groundVehicleSeq).longitude);
		jniEnv->SetFloatField(retObject, fieldId_altitude, groundVehicleStates.at(c_groundVehicleSeq).altitude);
		jniEnv->SetFloatField(retObject, fieldId_speed, groundVehicleStates.at(c_groundVehicleSeq).speed);
		jniEnv->SetFloatField(retObject, fieldId_course, groundVehicleStates.at(c_groundVehicleSeq).course);
		jniEnv->SetIntField(retObject, fieldId_drive_target_waypoint_index, groundVehicleStates.at(c_groundVehicleSeq).target_waypoint_index);
		jniEnv->SetIntField(retObject, fieldId_drive_plan_length, groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length);
		jniEnv->SetFloatField(retObject, fieldId_departure_time, groundVehicleStates.at(c_groundVehicleSeq).departure_time);
		jniEnv->SetObjectField(retObject, fieldId_drive_target_waypoint_name, jniEnv->NewStringUTF(groundVehicleStates.at(c_groundVehicleSeq).target_waypoint_name.c_str()));
		jniEnv->SetObjectField(retObject, fieldId_aircraftId, jniEnv->NewStringUTF(groundVehicleStates.at(c_groundVehicleSeq).aircraft_id.c_str()));
		jniEnv->SetObjectField(retObject, fieldId_airportId, jniEnv->NewStringUTF(groundVehicleStates.at(c_groundVehicleSeq).airport_id.c_str()));

		float drive_plan_latitude_array[groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length];
		float drive_plan_longitude_array[groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length];
		jclass stringClass = jniEnv->FindClass("Ljava/lang/String;");

		jobjectArray tmp_dp_name_array = (jobjectArray) jniEnv->NewObjectArray(groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length, stringClass, jniEnv->NewStringUTF(""));
		int tmpCountWaypoint = 0;
		waypoint_node_t* tmp_wp_ptr = groundVehicleStates.at(c_groundVehicleSeq).drive_plan_ptr;
		while (tmp_wp_ptr != NULL) {

			drive_plan_latitude_array[tmpCountWaypoint] = tmp_wp_ptr->latitude;
			drive_plan_longitude_array[tmpCountWaypoint] = tmp_wp_ptr->longitude;

			jniEnv->SetObjectArrayElement(tmp_dp_name_array, tmpCountWaypoint, jniEnv->NewStringUTF(tmp_wp_ptr->wpname));

			tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
			tmpCountWaypoint++;

		}

		jfloatArray tmp_dp_latitude_array = jniEnv->NewFloatArray(groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length);
		jniEnv->SetFloatArrayRegion(tmp_dp_latitude_array, 0, groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length, drive_plan_latitude_array);
		jniEnv->SetObjectField(retObject, fieldId_drive_plan_latitude_array, tmp_dp_latitude_array);

		jfloatArray tmp_dp_longitude_array = jniEnv->NewFloatArray(groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length);
		jniEnv->SetFloatArrayRegion(tmp_dp_longitude_array, 0, groundVehicleStates.at(c_groundVehicleSeq).drive_plan_length, drive_plan_longitude_array);
		jniEnv->SetObjectField(retObject, fieldId_drive_plan_longitude_array, tmp_dp_longitude_array);
		jniEnv->SetObjectField(retObject, fieldId_drive_plan_waypoint_name_array, tmp_dp_name_array);

	} else {
		retObject = NULL;
	}

	jniEnv->ReleaseStringUTFChars(groundVehicleId, c_gvid);

	return retObject;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_synchronize_1groundvehicle_1to_1server
  (JNIEnv *jniEnv, jobject jobj, jobject groundVehicle, jstring parameter) {
	int retValue = 1; // Initial value = 1

	jclass jcls = jniEnv->FindClass("com/osi/gnats/groundvehicle/GroundVehicle");
	jclass jcls_String = jniEnv->FindClass("Ljava/lang/String;");

	jmethodID methodID_toString = jniEnv->GetMethodID(jcls_String, "toString", "()Ljava/lang/String;");

	jfieldID fieldId_gvid = jniEnv->GetFieldID(jcls, "gvid", "Ljava/lang/String;");
	jfieldID fieldId_airportId = jniEnv->GetFieldID(jcls, "airportId", "Ljava/lang/String;");

	jfieldID fieldId_flag_external_groundvehicle = jniEnv->GetFieldID(jcls, "flag_external_groundvehicle", "Z");

	jfieldID fieldId_latitude = jniEnv->GetFieldID(jcls, "latitude", "F");
	jfieldID fieldId_longitude = jniEnv->GetFieldID(jcls, "longitude", "F");
	jfieldID fieldId_altitude = jniEnv->GetFieldID(jcls, "altitude", "F");
	jfieldID fieldId_speed = jniEnv->GetFieldID(jcls, "speed", "F");
	jfieldID fieldId_course = jniEnv->GetFieldID(jcls, "course", "F");

	jfieldID fieldId_drive_plan_latitude_array = jniEnv->GetFieldID(jcls, "drive_plan_latitude_array", "[F");
	jfieldID fieldId_drive_plan_longitude_array = jniEnv->GetFieldID(jcls, "drive_plan_longitude_array", "[F");
	jfieldID fieldId_drive_plan_waypoint_name_array = jniEnv->GetFieldID(jcls, "drive_plan_waypoint_name_array", "[Ljava/lang/String;");

	jfieldID fieldId_departure_time = jniEnv->GetFieldID(jcls, "departure_time", "F");
	jfieldID fieldId_dp_latitude_index_to_modify = jniEnv->GetFieldID(jcls, "dp_latitude_index_to_modify", "I");
	jfieldID fieldId_dp_longitude_index_to_modify = jniEnv->GetFieldID(jcls, "dp_longitude_index_to_modify", "I");
	jfieldID fieldId_dp_latitude_to_modify = jniEnv->GetFieldID(jcls, "dp_latitude_to_modify", "F");
	jfieldID fieldId_dp_longitude_to_modify = jniEnv->GetFieldID(jcls, "dp_longitude_to_modify", "F");

	jfieldID fieldId_drive_target_waypoint_index = jniEnv->GetFieldID(jcls, "target_waypoint_index", "I");
	jfieldID fieldId_drive_target_waypoint_name = jniEnv->GetFieldID(jcls, "target_waypoint_name", "Ljava/lang/String;");

	jstring jstring_gvid = (jstring)jniEnv->GetObjectField(groundVehicle, fieldId_gvid);
	const char *cTmpStrGvid = jniEnv->GetStringUTFChars(jstring_gvid, NULL);
	string string_gvid = cTmpStrGvid;

	const char *cTmpStrParameter = jniEnv->GetStringUTFChars(parameter, NULL);
	string string_parameter = cTmpStrParameter;


	int c_groundVehicleSeq = select_groundVehicleSeq_by_groundVehicleId(string_gvid);
	string customWaypoint = "CUSTOM_GROUND_POINT";
	int latChangeIndex = -1, lonChangeIndex = -1;
	float newLat = 0.0, newLon = 0.0;
	if (c_groundVehicleSeq > -1) {
		if(string_parameter == "latitude")
			groundVehicleStates.at(c_groundVehicleSeq).latitude = jniEnv->GetFloatField(groundVehicle, fieldId_latitude);
		if(string_parameter == "longitude")
			groundVehicleStates.at(c_groundVehicleSeq).longitude = jniEnv->GetFloatField(groundVehicle, fieldId_longitude);
		if(string_parameter == "altitude")
			groundVehicleStates.at(c_groundVehicleSeq).altitude = jniEnv->GetFloatField(groundVehicle, fieldId_altitude);
		if(string_parameter == "speed")
			groundVehicleStates.at(c_groundVehicleSeq).speed = jniEnv->GetFloatField(groundVehicle, fieldId_speed);
		if(string_parameter == "course") {
			courseChange.push_back(groundVehicleStates.at(c_groundVehicleSeq).vehicle_id);
			groundVehicleStates.at(c_groundVehicleSeq).course = jniEnv->GetFloatField(groundVehicle, fieldId_course);
		}

		latChangeIndex = jniEnv->GetIntField(groundVehicle, fieldId_dp_latitude_index_to_modify);
		lonChangeIndex = jniEnv->GetIntField(groundVehicle, fieldId_dp_longitude_index_to_modify);

		if (latChangeIndex > -1) {
			newLat = jniEnv->GetFloatField(groundVehicle, fieldId_dp_latitude_to_modify);

			int tmpCountWaypoint = 0;
			waypoint_node_t* tmp_wp_ptr = groundVehicleStates.at(c_groundVehicleSeq).drive_plan_ptr;
			while (tmp_wp_ptr != NULL) {
				if(tmpCountWaypoint == latChangeIndex) {
					tmp_wp_ptr->latitude = newLat;
					tmp_wp_ptr->wpname = strdup(customWaypoint.c_str());
					break;
				}

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
				tmpCountWaypoint++;
			}
		}

		if (lonChangeIndex > -1) {
			newLon = jniEnv->GetFloatField(groundVehicle, fieldId_dp_longitude_to_modify);

			int tmpCountWaypoint = 0;
			waypoint_node_t* tmp_wp_ptr = groundVehicleStates.at(c_groundVehicleSeq).drive_plan_ptr;
			while (tmp_wp_ptr != NULL) {
				if(tmpCountWaypoint == lonChangeIndex) {
					tmp_wp_ptr->longitude = newLon;
					tmp_wp_ptr->wpname = strdup(customWaypoint.c_str());
					break;
				}

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
				tmpCountWaypoint++;
			}
		}


	}
	return 1;

}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_externalGroundVehicle_1create_1trajectory_1profile
  (JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId, jstring aircraftInService, jstring airport, jfloat latitude, jfloat longitude, jfloat speed, jfloat course) {
	int retVal = 0;
	const char *c_gvid = (char*)jniEnv->GetStringUTFChars( groundVehicleId, NULL );
	string string_gvid(c_gvid); // Convert char* to string

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftInService, NULL );
	string string_acid(c_acid); // Convert char* to string

	const char *c_airportId = (char*)jniEnv->GetStringUTFChars( airport, NULL );
	string string_airportId(c_airportId); // Convert char* to string

	// Create new class object for the new ground vehicle being inserted into simulation.

	GroundVehicle groundVehicle;
	groundVehicle.vehicle_id = string_gvid;
	groundVehicle.aircraft_id = string_acid;
	groundVehicle.latitude = latitude;
	groundVehicle.longitude = longitude;
	groundVehicle.course = course;
	groundVehicle.speed = speed;
	groundVehicle.altitude = get_airport_elevation(string_airportId);
	groundVehicle.airport_id = string_airportId;
	groundVehicle.flag_external_groundVehicle = true;

	groundVehicleStates.push_back(groundVehicle);

	return retVal;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_externalGroundVehicle_1inject_1trajectory_1state_1data
  (JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId, jstring aircraftInService, jfloat latitude, jfloat longitude, jfloat speed, jfloat course) {
	int retVal = 1;

	const char *c_gvid = (char*)jniEnv->GetStringUTFChars( groundVehicleId, NULL );
	string string_gvid(c_gvid); // Convert char* to string

	const char *c_acid = (char*)jniEnv->GetStringUTFChars( aircraftInService, NULL );
	string string_acid(c_acid); // Convert char* to string

	int c_groundVehicleSeq = select_groundVehicleSeq_by_groundVehicleId(string_gvid);

	if (c_groundVehicleSeq > -1) {
		groundVehicleStates.at(c_groundVehicleSeq).aircraft_id = string_acid;
		groundVehicleStates.at(c_groundVehicleSeq).latitude = latitude;
		groundVehicleStates.at(c_groundVehicleSeq).longitude = longitude;
		groundVehicleStates.at(c_groundVehicleSeq).course = course;
		groundVehicleStates.at(c_groundVehicleSeq).speed = speed;
		retVal = 0;
	}

	return retVal;
}


JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getCenterCodes
  (JNIEnv *jniEnv, jobject jobj) {
  	vector<string> centerList{"KZAU", "KZBW", "KZDC", "KZDV", "KZFW", "KZHU",
				"KZID", "KZJX", "KZKC", "KZLA", "KZLC", "KZMA", "KZME", "KZMP", "KZNY", "KZOA", "KZOB", "KZSE", "KZTL", "PZAN", "PZHN", "KZAB" };
  
	jobjectArray retArray = NULL;

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	retArray = (jobjectArray)jniEnv->NewObjectArray(centerList.size(), jcls, jniEnv->NewStringUTF(""));

	int i = 0;

	for(auto element = centerList.begin(); element!=centerList.end(); ++element) {

		string center = *element;
		jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(center.c_str()));
		i++;
	}

	return retArray;

  }


JNIEXPORT jstring JNICALL Java_com_osi_gnats_engine_CEngine_getCurrentCenter
  (JNIEnv *, jobject, jstring);


JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getFixesInCenter
  (JNIEnv *jniEnv, jobject jobj, jstring centerId) {

  	const char *c_centerId = (char*)jniEnv->GetStringUTFChars( centerId, NULL );
	string string_centerId(c_centerId); // Convert char* to string

  	ifstream centerFile(g_share_dir + "/artcc/ArtccWaypoints.csv");
  	vector<string> fixes;
  	string line;
	const char delim = ',';
	
	while (getline(centerFile, line)) {
			vector<std::string> out;
    		tokenize(line, delim, out);

			if (out.at(0) == string_centerId)
				fixes.push_back(out.at(1));
	}

    jobjectArray retArray = NULL;

	jclass jcls = jniEnv->FindClass("Ljava/lang/String;");

	retArray = (jobjectArray)jniEnv->NewObjectArray(fixes.size(), jcls, jniEnv->NewStringUTF(""));

	int i = 0;

	for(auto element = fixes.begin(); element!=fixes.end(); ++element) {

		string fix = *element;
		jniEnv->SetObjectArrayElement(retArray, i, jniEnv->NewStringUTF(fix.c_str()));
		i++;
	}

	return retArray;
  }



JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setGroundOperatorAbsence
  (JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId, jint timeSteps) {
	const char *c_gvid = (char*)jniEnv->GetStringUTFChars( groundVehicleId, NULL );
	string string_gvid(c_gvid); // Convert char* to string

	groundOperatorAbsence[string_gvid] = timeSteps;

	return 1;
}


JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_groundVehicle_1setActionLag(
		JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId, jstring lagParameter,
		jfloat lagTimeConstant, jfloat percentageError,
		jfloat parameterCurrentValue, jfloat parameterTarget) {
	const char *c_gvid = (char*) jniEnv->GetStringUTFChars(groundVehicleId,
			NULL);
	string string_gvid(c_gvid); // Convert char* to string

	const string lagParameterString = (string) jniEnv->GetStringUTFChars(
				lagParameter, NULL);

	if (lagParameterString == "COURSE") {
		gvLagParams[string_gvid].push_back("COURSE");
		float G = log(1 / percentageError) / lagTimeConstant
				* t_step_terminal;
		if (parameterCurrentValue < parameterTarget) {
			while ((abs(parameterCurrentValue - parameterTarget) > 0.1)
					&& (parameterCurrentValue < parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((parameterCurrentValue < parameterTarget)) {
					gvLagParamValues[string_gvid]["COURSE"].push_back(
							parameterCurrentValue);
				}
			}
		} else if (parameterCurrentValue > parameterTarget) {
			while ((abs(parameterCurrentValue - parameterTarget) > 0.1)
					&& (parameterCurrentValue > parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((parameterCurrentValue > parameterTarget)) {
					gvLagParamValues[string_gvid]["COURSE"].push_back(
							parameterCurrentValue);
				}
			}
		}
		gvLagParamValues[string_gvid]["COURSE"].push_back(parameterTarget);
	}

	if (lagParameterString == "SPEED") {
		gvLagParams[string_gvid].push_back("SPEED");
		float G = log(1 / percentageError) / lagTimeConstant
				* t_step_terminal;
		if (parameterCurrentValue < parameterTarget) {
			while ((abs(parameterCurrentValue - parameterTarget) > 0.1)
					&& (parameterCurrentValue < parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((parameterCurrentValue < parameterTarget)) {
					gvLagParamValues[string_gvid]["SPEED"].push_back(
							parameterCurrentValue);
				}
			}
		} else if (parameterCurrentValue > parameterTarget) {

			while ((abs(parameterCurrentValue - parameterTarget) > 0.1)
					&& (parameterCurrentValue > parameterTarget)) {
				parameterCurrentValue = parameterCurrentValue
						+ G * (parameterTarget - parameterCurrentValue);
				if ((parameterCurrentValue > parameterTarget)) {
					gvLagParamValues[string_gvid]["SPEED"].push_back(
							parameterCurrentValue);
				}
			}

		}
		gvLagParamValues[string_gvid]["SPEED"].push_back(parameterTarget);
	}

	return 1;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setVehicleContact(
		JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId) {
	const char *c_gvid = (char*) jniEnv->GetStringUTFChars(groundVehicleId,
			NULL);
	string string_gvid(c_gvid); // Convert char* to string

	int c_groundVehicleSeq = select_groundVehicleSeq_by_groundVehicleId(
			string_gvid);
	groundVehicleStates.erase(groundVehicleStates.begin() + c_groundVehicleSeq);
	g_groundVehicles.erase(g_groundVehicles.begin() + c_groundVehicleSeq);

	return 1;
}

JNIEXPORT jfloat JNICALL Java_com_osi_gnats_engine_CEngine_groundVehicle_1setActionRepeat(
		JNIEnv *jniEnv, jobject jobj, jstring groundVehicleId,
		jstring repeatParameter) {
	jfloat retVal = 0.0;
	const char *c_gvid = (char*) jniEnv->GetStringUTFChars(groundVehicleId,
			NULL);
	string string_gvid(c_gvid); // Convert char* to string

	string string_repeatParameter = (string) (char*) jniEnv->GetStringUTFChars(
			repeatParameter, NULL);

	int c_groundVehicleSeq = select_groundVehicleSeq_by_groundVehicleId(
			string_gvid);

	if (string_repeatParameter == "SPEED")
		retVal = groundVehiclePreviousStates.at(c_groundVehicleSeq).speed;
	else if (string_repeatParameter == "COURSE")
		retVal = groundVehiclePreviousStates.at(c_groundVehicleSeq).course;

	return retVal;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setRadarError
  (JNIEnv *jniEnv, jobject jobj, jstring airportId, jstring parameter, jdouble originalValue, jdouble bias, jdouble noise, jint scope) {
	int retVal = 0;

	const char *c_airportId = (char*) jniEnv->GetStringUTFChars(airportId,
				NULL);
	string string_airportId(c_airportId);
	const char *c_parameter = (char*) jniEnv->GetStringUTFChars(parameter,
				NULL);
	string string_parameter(c_parameter);

	if (string_parameter == "RANGE") {
		radarErrorModel[string_airportId].first = "RANGE";
		radarErrorModel[string_airportId].second = originalValue + bias + sqrt(noise);
	}
	else if (string_parameter == "AZIMUTH") {
		radarErrorModel[string_airportId].first = "AZIMUTH";
		radarErrorModel[string_airportId].second = originalValue + bias + sqrt(noise);
	}
	else if (string_parameter == "ELEVATION") {
		radarErrorModel[string_airportId].first = "ELEVATION";
		radarErrorModel[string_airportId].second = originalValue + bias + sqrt(noise);
	}
	else {
		retVal = 1;
	}
	return retVal;
}

void explode(string const & s, char delim, int index)
{
    istringstream iss(s);

    for (string token; getline(iss, token, delim); )
        terrData[index].push_back(move(token));
}

// Method to get index of file that contains elevation data
int getFileIndex(string path, double latitude, double longitude) {

	string line;
	vector<string> latLonLimits;
	int latRange, fileNumber = -1, rowCount = 0;
	double minLat, minLon, maxLat, maxLon;

	// Scan metadata to get the file index
	ifstream file(path);
	while (file.peek() != EOF) {
		getline(file, line);

		stringstream ss(line);
		
		while( ss.good() )
		{
		    string substr;
		    getline( ss, substr, ',' );
		    latLonLimits.push_back( substr );
		}

		// Range check for the data
		minLat = stod(latLonLimits.at(2));
		minLon = stod(latLonLimits.at(0));
		maxLat = stod(latLonLimits.at(3));
		maxLon = stod(latLonLimits.at(1));
		if (((latitude >= minLat) && (latitude <= maxLat))
				&& ((longitude >= minLon) && (longitude <= maxLon))) {
			fileNumber = rowCount;
			usgsMetadata[fileNumber] = trunc(latitude);
			break;
		}

		rowCount++;
		
	}
	
	file.close();
	
	return fileNumber;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_loadTerrainData
  (JNIEnv *jniEnv, jobject jobj, jdouble minLatDeg, jdouble maxLatDeg, jdouble minLonDeg, jdouble maxLonDeg, jboolean cifpExists) {
    int err = 0;

	int fileNumber = 0, range = 0, region = 0;
	const char *name;
    struct zip_stat st;
    char *contents;
    zip_file *f;
	string nameStr;
	zip *z;
	double bufferLat = minLatDeg;
	vector<int> fileList;
	
	while (minLonDeg < maxLonDeg) {
		minLatDeg = bufferLat;
		while (minLatDeg < maxLatDeg) {
			if ((minLatDeg >= 23.937 && minLatDeg <= 48.808) && (minLonDeg >= -125.185 && minLonDeg <= -64.765)) {
				region = 0;
				int latitudeDifferenceAcrossGrid = 24872;
				double longitudeChangeFactor = (int) ((round((minLonDeg - 0.001) * 1000.0) / 1000.0 - (-125.185)) / 0.001);
				if (longitudeChangeFactor < 0)
					longitudeChangeFactor = 0;
				range = longitudeChangeFactor * latitudeDifferenceAcrossGrid + (int) ((minLatDeg - 23.937) / 0.001);
				fileNumber = (int) floor(range / 50092208);
				if ((find(fileList.begin(), fileList.end(), fileNumber) == fileList.end()) && fileNumber != -1)
					fileList.push_back(fileNumber);			
			}
			else if ((minLatDeg >= 53.999 && minLatDeg <= 72.000) && (minLonDeg >= -169.000 && minLonDeg <= -130.999)) {
				fileNumber = getFileIndex(g_share_dir + "/elevation/1/metadata.csv", minLatDeg, minLonDeg);
				if ((find(fileList.begin(), fileList.end(), fileNumber) == fileList.end()) && fileNumber != -1)
					fileList.push_back(fileNumber);		
				region = 1;
			} else if ((minLatDeg >= 18.999 && minLatDeg <= 22) && (minLonDeg >= -159.000 && minLonDeg <= -154.999)) {
				fileNumber = getFileIndex(g_share_dir + "/elevation/2/metadata.csv", minLatDeg, minLonDeg);
				if ((find(fileList.begin(), fileList.end(), fileNumber) == fileList.end()) && fileNumber != -1)
					fileList.push_back(fileNumber);		
				region = 2;
			} else if ((minLatDeg >= startLat && minLatDeg <= endLat) && (minLonDeg >= startLon && minLonDeg <= endLon)) {
				// Custom/Open Terrain Data
				if ((find(fileList.begin(), fileList.end(), fileNumber) == fileList.end()) && fileNumber != -1)
					fileList.push_back(trunc(minLatDeg));
				region = 3;
			}
			minLatDeg += 0.5;
		}
		minLonDeg += 0.5;	
	}

	if (!terrDataLoaded) {
		if (fileList.size() > 0)
			cout<<"Loading terrain data..."<<"\n";
		else 
			cout<<"Terrain data not found for given latitude/longitude range."<<"\n";

		z = zip_open((g_share_dir + "/elevation/" + to_string(region) + "/elevation.zip").c_str(), 0, &err);
		for (int j = 0; j < fileList.size(); j++) {
		    //Search for the file of given name
		    if (cifpExists)
			    nameStr = to_string(region) + "terrain" + to_string(fileList.at(j));
			else
				nameStr = to_string(fileList.at(j));

			name = nameStr.c_str();

		    zip_stat_init(&st);
		    zip_stat(z, name, 0, &st);

			// Check if the entry exists in the zip file
		    if (st.valid != 0) {
				//Alloc memory for its uncompressed contents
				contents = new char[st.size];

				//Read the compressed file
				f = zip_fopen(z, name, 0);

				zip_fread(f, contents, st.size);
				string str(contents);

				explode(str, '\n', fileList.at(j));

				zip_fclose(f);

				//Do something with the contents
				//delete allocated memory
				delete[] contents;
		    }
	    }

	    zip_close(z);

		terrDataLoaded = true;
	}

	if (terrData.size() == 0) {
		cout << "Loaded terrain data: empty" << endl;
	}

	return err;
}


JNIEXPORT jdouble JNICALL Java_com_osi_gnats_engine_CEngine_getElevation
  (JNIEnv *jniEnv, jobject jobj, jdouble latitude, jdouble longitude, jboolean cifpExists) {
  
	double elevation = 0.0;
	int latitudeDifferenceAcrossGrid, longitudeChangeFactor, a, b, latRange, lonDiff, index;
	
	if ((latitude >= 23.937 && latitude <= 48.808) && (longitude >= -125.185 && longitude <= -64.765)) {
	  	latitudeDifferenceAcrossGrid = 24872;
		longitudeChangeFactor = (int) ((round((longitude - 0.001) * 1000.0) / 1000.0 - (-125.185)) / 0.001);
	
		if (longitudeChangeFactor < 0)
			longitudeChangeFactor = 0;
	
		a = longitudeChangeFactor * latitudeDifferenceAcrossGrid + (int) ((latitude - 23.937) / 0.001);
	
		b = (int) floor(a / 50092208);
		
		if ((a - (b) * 50092208) < terrData[b].size()) {
			try {
				elevation = stod(terrData[b].at(a - (b) * 50092208)) * 100;
			}
			catch (const invalid_argument&) {
		    }
	    	catch (const std::out_of_range& e) {
			}
		}
	}
	else if (((latitude >= 53.999 && latitude <= 72.000) && (longitude >= -169.000 && longitude <= -130.999))
		 || ((latitude >= 18.999 && latitude <= 22.000) && (longitude >= -159.000 && longitude <= -154.999))
	) {
			
		latRange = (int) (1.001 / 0.000925925926);
		lonDiff = (int) ((longitude - floor(longitude)) / 0.000925925926);
		index = lonDiff * latRange + (int) ((latitude - floor(latitude)) / 0.000925925926);

		if (index < terrData[usgsMetadata[trunc(latitude)]].size()) {
			try {
				elevation = stod(terrData[usgsMetadata[trunc(latitude)]].at(index)) * 100;
			}
			catch (const invalid_argument&) {
		    }
		    catch (const std::out_of_range& e) {
			}
		}

	} else if ((latitude >= startLat && latitude <= endLat) && (longitude >= startLon && longitude <= endLon)) {
		try {
			elevation = stod(terrData[trunc(latitude)].at(floor((longitude - (startLon)) * 1/resolution))) * 100;
		}
		catch (const invalid_argument&) {
    	}
    	catch (const std::out_of_range& e) {
		}
	}
	else
		elevation = 0.0;

	return elevation;

  }

JNIEXPORT jobjectArray JNICALL Java_com_osi_gnats_engine_CEngine_getElevationMapBounds
  (JNIEnv *jniEnv, jobject jobj, jboolean cifpExists) {
  
	jobjectArray retObj = NULL;
	jclass doubleArrayClass = jniEnv->FindClass("[D");

    double contiguousUSArr[3][4] = { { 23.937, 48.808, -125.185, -64.765 }, 
	{ 53.999, 72.000, -169.000, -130.999 }, 
	{ 18.999, 22.000, -159.000, -154.999 } };
	
	double openArr[1][4] = { {startLat, endLat, startLon, endLon} };
	
	if (!cifpExists) {	// Open GNATS
	    retObj = jniEnv->NewObjectArray((jsize) 1, doubleArrayClass, NULL);
	
        jdoubleArray doubleArray = jniEnv->NewDoubleArray(4);
        jniEnv->SetDoubleArrayRegion(doubleArray, (jsize) 0, (jsize) 4, (jdouble*) openArr[0]);
        jniEnv->SetObjectArrayElement(retObj, (jsize) 0, doubleArray);
        jniEnv->DeleteLocalRef(doubleArray);
	    
	}
	else {				// ULI GNATS
		retObj = jniEnv->NewObjectArray((jsize) 3, doubleArrayClass, NULL);
	
	    // Go through the firs dimension and add the second dimension arrays
	    for (int i = 0; i < 3; i++)
	    {
	        jdoubleArray doubleArray = jniEnv->NewDoubleArray(4);
	        jniEnv->SetDoubleArrayRegion(doubleArray, (jsize) 0, (jsize) 4, (jdouble*) contiguousUSArr[i]);
	        jniEnv->SetObjectArrayElement(retObj, (jsize) i, doubleArray);
	        jniEnv->DeleteLocalRef(doubleArray);
	    }
	}

	return retObj;
  }

// Returns min, max, mean, variance, st dev
JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getElevationAreaStats
  (JNIEnv *jniEnv, jobject jobj, jdouble minLatDeg, jdouble maxLatDeg, jdouble minLonDeg, jdouble maxLonDeg, jboolean cifpExists) {
  	int gridSize = 0;
  	double min = 0, max = 0, mean = 0, variance = 0, stddev = 0, elevation = 0;
	double bufferLat = minLatDeg;
	vector<double> gridElevation;

	// Get all elevation data points
	while (minLonDeg < maxLonDeg) {
		minLatDeg = bufferLat;
		while (minLatDeg < maxLatDeg) {
			elevation = Java_com_osi_gnats_engine_CEngine_getElevation(jniEnv, jobj, minLatDeg, minLonDeg, cifpExists);
			gridElevation.push_back(elevation);
			mean += elevation;
			if (cifpExists)
				minLatDeg += 0.001;
			else
				minLatDeg += resolution;
		}
		if (cifpExists)
			minLonDeg += 0.001;
		else
			minLonDeg += resolution;
	}
	// Calculate stats
	gridSize = gridElevation.size();
	min = *min_element(gridElevation.begin(), gridElevation.end());
	max = *max_element(gridElevation.begin(), gridElevation.end());
	mean /= gridSize;

	for(vector <double> :: iterator it = gridElevation.begin(); it != gridElevation.end(); ++it){
		variance += (*it - mean) * (*it - mean);
	}
	variance /= gridSize;
	stddev = sqrt(variance);
	double retArray[5] = {min, max, mean, variance, stddev};

	jclass doubleArrayClass = jniEnv->FindClass("[D");
    jdoubleArray retObj = jniEnv->NewDoubleArray(5);
    jniEnv->SetDoubleArrayRegion(retObj, (jsize) 0, (jsize) 5, (jdouble*) retArray);
    
    return retObj;
}

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_setTerrainProfile
  (JNIEnv *jniEnv, jobject jobj, jdouble startLat, jdouble endLat, jdouble startLon, jdouble endLon, jdouble resolution) {
  	
  	::startLat = startLat;
	::endLat = endLat;
	::startLon = startLon;
	::endLon = endLon;
	::resolution = resolution;
	
	return 0;
  }

JNIEXPORT jint JNICALL Java_com_osi_gnats_engine_CEngine_clearTerrainData
  (JNIEnv *jniEnv, jobject jobj) {
  	
  	terrDataLoaded = false;	
  	terrData.clear();
  	usgsMetadata.clear();
  	
  	return 0;
  	
  }


JNIEXPORT jdoubleArray JNICALL Java_com_osi_gnats_engine_CEngine_getLineOfSight
  (JNIEnv *jniEnv, jobject jobj, jdouble observerLat, jdouble observerLon, jdouble observerAlt, jdouble targetLat, jdouble targetLon, jdouble targetAlt, jboolean cifpExists) {


	// Returns an array of (Range, Azimuth, Elevation, Masking)
	double retVal[] = {0, 0, 0, 0};
	
	// Radius of Earth in ft
	double R = 20902230.97;
	
	// Convert latitude/longitude to radian
	observerLat = observerLat * (PI / 180);
	observerLon = observerLon * (PI / 180);
	targetLat = targetLat * (PI / 180);
	targetLon = targetLon * (PI / 180);
	
	// Transform Lat, Lon, Alt positinos of observer and target to geoinertial frame
	double xt = (R + targetAlt) * cos(targetLat) * cos(targetLon);
	double yt = (R + targetAlt) * cos(targetLat) * sin(targetLon);
	double zt = (R + targetAlt) * sin(targetLat);
	double xo = (R + observerAlt) * cos(observerLat) * cos(observerLon);
	double yo = (R + observerAlt) * cos(observerLat) * sin(observerLon);
	double zo = (R + observerAlt) * sin(observerLat);

	// Get relative position vectors
	double dxo = xo - xt;
	double dyo = yo - yt;
	double dzo = zo - zt;
	
	// Get transformed relative vector to observer's topocentric frame
	double xl = dxo * sin(observerLat) * cos(observerLon) + dyo * sin(observerLat) * sin(observerLon) - dzo * cos(observerLat);
	double yl = -dxo * sin(observerLat) + dyo * cos(observerLat);
	double zl = dxo * cos(observerLat) * cos(observerLon) + dyo * cos(observerLat) * sin(observerLon) + dzo * sin(observerLat);
	
	
	// Calculate Range
	retVal[0] = sqrt(xl * xl + yl * yl + zl * zl);
	
	// Calculate Azimuth
	retVal[1] = atan2((sin(targetLon - observerLon) * cos(targetLat)), (cos(observerLat) * sin(targetLat) - sin(observerLat) * cos(targetLat) * cos(targetLon - observerLon))) * 180.0 / PI;
	
	// Calculate Elevation
	retVal[2] = zl / sqrt(xl * xl + yl * yl) * 180.0 / PI;
	
	double startLat = observerLat;
	double endLat = targetLat;
	double startLon = observerLon;
	double endLon = targetLon;

	if(startLat > targetLat) {
		startLat = targetLat;
		endLat = observerLat;
		startLon = targetLon;
		endLon = observerLon;
	}
	
	startLat *= 180.0 / PI;
	endLat *= 180.0 / PI;
	startLon *= 180.0 / PI;
	endLon *= 180.0 / PI;
	int altCount = (int)((endLat - startLat) / 0.01);
	
	double buffAlt = observerAlt;
	if(buffAlt > targetAlt)
		buffAlt = targetAlt;
			
	double slope = (endLon - startLon) / (endLat - startLat);
	while(startLat < endLat) {
		double newLon = slope * startLat - slope * observerLat * 180.0 / PI+ observerLon * 180.0 / PI;
		if(Java_com_osi_gnats_engine_CEngine_getElevation(jniEnv, jobj, startLat, newLon, cifpExists) > buffAlt)
				retVal[3] = 1;
			buffAlt += abs(observerAlt - targetAlt) / altCount;

		startLat += 0.01;
	}

	if (retVal[3] == 0) {
		if(retVal[2] < asin(R / ((R + observerAlt)) * 180.0 / PI))
			retVal[3] = 2;
	}
	
	jclass doubleArrayClass = jniEnv->FindClass("[D");
    jdoubleArray retObj = jniEnv->NewDoubleArray(4);
    jniEnv->SetDoubleArrayRegion(retObj, (jsize) 0, (jsize) 4, (jdouble*) retVal);
    
    return retObj;

  }


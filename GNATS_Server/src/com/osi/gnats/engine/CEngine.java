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

package com.osi.gnats.engine;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.groundvehicle.GroundVehicle;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.airport.Airport;
import com.osi.gnats.weather.WeatherPolygon;
import com.osi.util.Utils;

/**
 * Class to hold JNI function declaration
 * 
 * This class is the entry point called by other Java classes into the C functions.
 */
public class CEngine {
	private static CEngine engInstance;
	
	public CEngine(String logLevel, boolean flag_gdb) {
		if (engInstance == null) {
			if (Utils.isWindowsPlatform()) {
				// Here, the OS is Windows system.
				//
				// To ensure all dependency libraries are loaded, we load libraries individually.
				// Some dependency libraries will be automatically loaded.  Not all libraries are handled in the following block.

				System.loadLibrary("kernel32");
				System.loadLibrary("MSVCRT");
				
				System.loadLibrary("libcurl-4");
				System.loadLibrary("zlib1");
				
				System.loadLibrary("grib_api_lib");
				System.loadLibrary("libhdf5");

				System.loadLibrary("libwinpthread-1");
				System.loadLibrary("libgcc_s_seh-1");
				System.loadLibrary("libgomp-1");
				System.loadLibrary("libstdc++-6");

				System.loadLibrary("libxml2-2");
				System.loadLibrary("libxml++-2.6-2");

				System.loadLibrary("libhdf5-0");
				System.loadLibrary("libglibmm-2.4-1");

				System.loadLibrary("libcommon");
				System.loadLibrary("libwind");
				System.loadLibrary("libadb");
				System.loadLibrary("libastar");
				System.loadLibrary("libcuda_compat");
				
				System.loadLibrary("libgeomutils");
				System.loadLibrary("libghthash");
				System.loadLibrary("liblektor");
				System.loadLibrary("libnats_data");
				System.loadLibrary("libairport_layout");
				
				System.loadLibrary("libfp");
				System.loadLibrary("libpilot");
				
				System.loadLibrary("libcontroller");

				System.loadLibrary("libtrx");
				System.loadLibrary("librg");
				
				System.loadLibrary("libtg");
				System.loadLibrary("libgnatsengine");
			} else {
				System.loadLibrary("tg");
				System.loadLibrary("gnatsengine");
			}

			if (logLevel != null) {
				set_log_level(logLevel);
			}

			if (flag_gdb)
				set_flag_gdb(true);
		
			tg_init("share", 0, 0, ServerNATS.isStandalone_mode());
		
			CEngine.engInstance = this;
		}
	}

	// ========== GNATS core functions ==========
	
	public native void set_flag_gdb(boolean flag_gdb);
	
	public native void set_log_level(String logLevel);
	
	public native void info();
	
	private native int tg_init(String data_dir, int perturbation, int device_id, boolean flag_standalone_mode);
	
	private native void tg_shutdown();
	
	public native int load_rap(String wind_dir);
	
	public native int release_rap();
	
	public native int load_aircraft(String trx_file, String mfl_file);
	
	public native boolean validate_flight_plan_record(String string_track, String string_fp_route, int mfl_ft);
	
	public native int release_aircraft();

	
	// ========== GNATS Simulation-related Functions ==========
	
	public native long get_sim_id();
	
	public native int propagate_flights(float t_end, float t_step);
	
	public native int propagate_flights(float t_end, float t_step_surface, float t_step_terminal, float t_step_airborne);
	
	public native void enableConflictDetectionAndResolution(boolean flag);
	
	public native Object[][] getCDR_status();
	
	public native void setCDR_initiation_distance_ft_surface(float distance);
	
	public native void setCDR_initiation_distance_ft_terminal(float distance);
	
	public native void setCDR_initiation_distance_ft_enroute(float distance);
	
	public native void setCDR_separation_distance_ft_surface(float distance);
	
	public native void setCDR_separation_distance_ft_terminal(float distance);
	
	public native void setCDR_separation_distance_ft_enroute(float distance);
	
	public native int get_runtime_sim_status();
	
	public native float get_curr_sim_time();
	
	public native long get_curr_utc_time();
	
	public native long get_nextPropagation_utc_time();
	
	public native boolean isRealTime_simulation();
	
	public native int get_realTime_simulation_time_step();
	
	public native void sim_start();
	
	public native void sim_start(long t_duration);
	
	public native void sim_start(float t_duration);
	
	public native void sim_startRealTime();
	
	public native void sim_pause();
	
	public native void sim_resume();
	
	public native void sim_resume(long t_duration);
	
	public native void sim_resume(float t_duration);
	
	public native void sim_stop();
	
	public native void write_trajectories(String output_file);
	
	public native void clear_trajectories();

	public native void request_aircraft(String assignedAuthId, String ac_id);
	
	public native void request_groundVehicle(String assignedAuthId, String gv_id);

	public native void externalAircraft_create_trajectory_profile(String userName,
			String ac_id,
			String ac_type,
			String origin_airport,
			String destination_airport,
			float cruise_altitude_ft,
			float cruise_tas_knots,
			double latitude,
			double longitude,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course,
			String flight_phase);

	public native int externalAircraft_inject_trajectory_state_data(String ac_id,
			double latitude,
			double longitude,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course,
			String flight_phase,
			long timestamp_utc_millisec);
	
	// ========== GNATS Aircraft Functions ==========
	
	public native String getAircraftAssignee(String acid);
	
	public native int getFlightseq_by_Acid(String acid);
	
	public native String[] getAllAircraftId();
	
	public native String[] getAircraftIds(Float minLatitude, Float maxLatitude, Float minLongitude, Float maxLongitude, Float minAltitude_ft, Float maxAltitude_ft);

	public native String[] getAssignedAircraftIds(String username);
	
	public native Aircraft select_aircraft(int sessionId, String acid);
	
	public native int synchronize_aircraft_to_server(Aircraft aircraft);
	
	public native int delay_departure(String acid, int seconds);
	
	// ========== GNATS ADB Data Functions ==========
	
	public native double getADB_cruiseTas(String ac_type, double altitude_ft);
	
	public native double getADB_climb_descent_Rate(boolean flagClimbing, String ac_type, double flight_level, String adb_mass);
	
	public native double getADB_climb_descent_Tas(boolean flagClimbing, String ac_type, double altitude_ft);
	
	// ========== GNATS TerminalArea Functions ==========
	
	public native String getCurrentSidStarApproach(String acid, String proc_type);
	
	public native String[] getAllSidsStarsApproaches(String airport_code, String proc_type);
	
	public native String[] getProcedure_leg_names(String proc_type, String proc_name, String airport_code);
	
	public native String[] getWaypoints_in_procedure_leg(String proc_type, String proc_name, String airport_code, String proc_leg_name);

	public native double[] getWaypoint_Latitude_Longitude_deg(String waypoint_name);
	
	public native double getProcedure_alt_1(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	public native double getProcedure_alt_2(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	public native double getProcedure_speed_limit(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	public native String getProcedure_alt_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	public native String getProcedure_speed_limit_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	public native float[] getGroundWaypointLocation(String airportId, String groundWaypointId);
	
	// ========== GNATS Airport Functions ==========
	
	public native Airport select_airport(String airport_code);
	
	public native String getArrivalAirport(String acid);
	
	public native String getDepartureAirport(String acid);
	
	public native double[] getLocation(String airport_code);
	
	public native String getClosestAirport(double latitude_deg, double longitude_deg);
	
	public native String[] getAirportsWithinMiles(double latitude_deg, double longitude_deg, double miles);
	
	public native String getFullName(String airport_code);
	
	public native Object[] getAllRunways(String airport_code);
		
	public native String[] getRunwayExits(String airport_code, String runway_id);
	
	public native Object[] getLayout_node_map(String airport_code);
	
	public native Object[] getLayout_node_data(String airport_code);
	
	public native Object[] getLayout_links(String airport_code);
	
	public native String[] getSurface_taxi_plan(String acid, String airport_code);
	
	public native int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String runway_name);
	
	public native int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String vrNode_waypoint_id, String touchdownNode_waypoint_id);
	
	public native int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, double[] v2Point_lat_lon, double[] touchdownPoint_lat_lon);
	
	public native int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids);
	
	public native int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids, double[] v2_or_touchdown_point_lat_lon);
	
	public native String[] get_taxi_route_from_A_To_B(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id);
	
	public native String getDepartureRunway(String acid);
	
	public native String getArrivalRunway(String acid);
	
	public native double getTaxi_tas_knots(String acid);
	
	public native void setTaxi_tas_knots(String acid, double tas_knots);
	
	public native String[] getRunwayEnds(String airportId, String runwayId);
	
	// ========== GNATS Controller Functions ==========
	
	public native int controller_setDelayPeriod(String acid, String aircraft_clearance, float seconds);
	
	public native int insertAirborneWaypoint(String acid,
			int index_to_insert,
			String waypoint_type,
			String waypoint_name,
			float waypoint_latitude,
			float waypoint_longitude,
			float waypoint_altitude,
			float waypoint_speed_lim,
			String spdlim_desc);
	
	public native int deleteAirborneWaypoint(String acid, int index_to_delete);

	public native void setTargetWaypoint(String acid,
			int waypoint_plan_to_use,
			int index_of_target);
	
	public native int setControllerAbsence(String aircraftID, int timeSteps);
	
	public native int releaseAircraftHold(String aircraftID, String approach, String targetWaypoint);
	
	public native void enableMergingAndSpacingAtMeterFix(String centerId, String meterFix, String spacingType, float spacing);

	public native void disableMergingAndSpacingAtMeterFix(String centerId, String meterFix);

	public native void enableStrategicWeatherAvoidance(boolean flag);
	
	public native void setWeather_pirepFile(String pathFilename);
	
	public native void setWeather_polygonFile(String pathFilename);
	
	public native void setWeather_sigmetFile(String pathFilename);
	
	public native int setTacticalWeatherAvoidance(String waypoint_name, float duration_sec);
	
	// ========== GNATS Pilot Functions ==========

	public native int pilot_setActionLag(String aircraftID, String lagParameter, float lagTimeConstant, float percentageError, float parameterCurrentValue, float parameterTarget);
					
	public native float pilot_setActionRepeat(String aircraftID, String repeatParameter, int flag);
			
	public native int pilot_skipFlightPhase(String aircraftID, String flightPhase);
	
	// ========== GNATS Weather Functions ==========
	
	public native int downloadWeatherFiles();
	
	public native float[] getWind(float timestamp_sec,
			float latitude_deg,
			float longitude_deg,
			float altitude_ft);
	
	public native WeatherPolygon[] getWeatherPolygons(String ac_id, double lat_deg, double lon_deg, double alt_ft, double nauticalMile_radius);
	
	// ========== GNATS RiskMeasure Functions =========
	
	public native double calculateRelativeVelocity(double refSpeed, double refCourse, double refFpa, double tempSpeed, double tempCourse, double tempFpa);
	
	public native double calculateBearing(double lat1, double lon1, double lat2, double lon2);
	
	public native double calculateDistance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2);
	
	public native double calculateWaypointDistance(float lat1, float lng1, float lat2, float lng2);
	
	public native double getVelocityAlignmentWithRunway(int sessionId, String aircraftId, String procedure);
	
	public native double getDistanceToRunwayEnd(int sessionId, String aircraftId);
	
	public native double getDistanceToRunwayThreshold(int sessionId, String aircraftId);
	
	public native double[][] getRunwayEndpoints(String airportId, String runway);
	
	public native int getPassengerCount(String aircraftType);
	
	public native double getAircraftCost(String aircraftType);
	
	public native int setAircraftBookValue(String aircraftId, double aircraftBookValue);
	
	public native double getAircraftBookValue(String aircraftId);
	
	public native int setCargoWorth(String aircraftId, double cargoWorth);
	
	public native double getCargoWorth(String aircraftId);
	
	public native int setPassengerLoadFactor(String aircraftId, double paxLoadFactor);
	
	public native double getPassengerLoadFactor(String aircraftId);
	
	public native int setTouchdownPointOnRunway(String aircraftId, double latitude, double longitude);
	
	public native double[] getTouchdownPointOnRunway(String aircraftId);
	
	public native int setTakeOffPointOnRunway(String aircraftId, double latitude, double longitude);
	
	public native double[] getTakeOffPointOnRunway(String aircraftId);
	
	public native int load_FlightPhaseSequence(String filename);
	
	public native void clear_FlightPhaseSequence();
	
	public native int load_aviationOccurenceProfile(String dirPath);
	
	// ==================== GNATS GroundVehicle Functions ===================
	
	public native int load_groundVehicle(String trx_file);
	
	public native int release_groundVehicle();

	public native String[] getAllGroundVehicleIds();
	
	public native String[] getAssignedGroundVehicleIds(String username);
	
	public native GroundVehicle select_groundVehicle(int sessionId, String groundVehicleId);
	
	public native int synchronize_groundvehicle_to_server(GroundVehicle groundVehicle, String parameter);
	
	public native int externalGroundVehicle_create_trajectory_profile(String groundVehicleId, String aircraftInService, String airport, float latitude, float longitude, float speed, float course);

	public native int externalGroundVehicle_inject_trajectory_state_data(String groundVehicleId, String aircraftInService, float latitude, float longitude, float speed, float course);

	// ==================== GNATS Environment Functions ===================
	
	public native String[] getCenterCodes();
	
	public native String getCurrentCenter(String aircraftId);
	
	public native String[] getFixesInCenter(String centerId);
		
	// ==================== GNATS GroundOperator Functions ===================
	
	public native int setGroundOperatorAbsence(String groundVehicleId, int timeSteps);
	
	public native int groundVehicle_setActionLag(String groundVehicleId, String lagParameter, float lagTimeConstant, float percentageError, float parameterCurrentValue, float parameterTarget);
	
	public native int setVehicleContact(String groundVehicleId);
	
	public native float groundVehicle_setActionRepeat(String groundVehicleId, String repeatParameter);
	
	// =================== GNATS TerrainInterface functions ==================
	
	public native int loadTerrainData(double minLatDeg, double maxLatDeg, double minLonDeg, double maxLonDeg, boolean cifpExists);
	
	public native double getElevation(double latDeg, double lonDeg, boolean cifpExists);

	public native double[][] getElevationMapBounds(boolean cifpExists);

	public native double[] getElevationAreaStats(double minLatDeg, double maxLatDeg, double minLonDeg, double maxLonDeg, boolean cifpExists);
	
	public native int setTerrainProfile(double startLat, double endLat, double startLon, double endLon, double resolution);
	
	public native int clearTerrainData();
	
	// ==================== GNATS GroundCommunication Functions ==============
	
	public native int setRadarError(String airportId, String parameter, double originalValue, double bias, double noiseVariance, int scope);

	public native double[] getLineOfSight(double observerLat, double observerLon, double observerAlt, double targetLat, double targetLon, double targetAlt, boolean cifpExists);

	public native int setNavigationLocationError(int sessionId, String aircraftId, String parameter, double bias, double drift, double scaleFactor, double noiseVariance, int scope);
}

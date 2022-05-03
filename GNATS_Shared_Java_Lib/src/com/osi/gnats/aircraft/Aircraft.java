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

package com.osi.gnats.aircraft;

import java.io.Serializable;
import java.io.File;

public class Aircraft extends AircraftSynchronizer implements Serializable {
	private static final long serialVersionUID = 3103179025079369656L;

	private int sessionId = 0;
	
	private String acid = null;
	
	private boolean flag_external_aircraft;
	private String assigned_user;
	
	/* Current state */
	private float latitude_deg;
	private float longitude_deg;
	private float altitude_ft;
	private float rocd_fps;
	private float tas_knots;
	private float course_rad;
	private float fpa_rad;

	private int flight_phase;
	
	// Aircraft static data
	private float departure_time_sec;
	private float cruise_alt_ft;
	private float cruise_tas_knots;
	private float[] flight_plan_latitude_array;  // 2D
	private float[] flight_plan_longitude_array; // 2D
	private int   flight_plan_length;
	private String[] flight_plan_waypoint_name_array;
	private String[] flight_plan_alt_desc_array;
	private double[] flight_plan_alt_1_array;
	private double[] flight_plan_alt_2_array;
	private double[] flight_plan_speed_limit_array;
	private String[] flight_plan_speed_limit_desc_array;
	private float origin_airport_elevation_ft;
	private float destination_airport_elevation_ft;

	private int landed_flag;
	
	// Aircraft intent
	private int target_waypoint_index;
	private String target_waypoint_name;
	private int airborne_target_waypoint_index;
	private String airborne_target_waypoint_name;
	private float target_altitude_ft;
	private int toc_index;
	private int tod_index;
	private float target_waypoint_latitude_deg;
	private float target_waypoint_longitude_deg;
	
	private int fp_latitude_index_to_modify = -1;
	private float fp_latitude_deg_to_modify;
	private int fp_longitude_index_to_modify = -1;
	private float fp_longitude_deg_to_modify;
	
	public Aircraft() {
		this.acid = "";
	}
	
	public Aircraft(String acid) {
		this.acid = acid;
	}
	
	
	private int synchronize() {
		int retValue = -1;

		try {
			if (this.flag_external_aircraft) {
				throw new Exception("This is an external aircraft.  Can't set state to it.");
			}

			if (remoteAircraft != null) {
				retValue = remoteAircraft.synchronize_aircraft_to_server(this);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	/**
	 * Postpone the departure time of the current aircraft for certain seconds.
	 * If the aircraft already departed, the departure time will not be changed.
	 * @param seconds
	 * @return
	 */
	public int delay_departure(int seconds) {
		int retValue = -1;
		
		try {
			retValue = remoteAircraft.delay_departure(acid, seconds);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	/**
	 * Get session ID
	 * @return
	 */
	public int getSessionId() {
		return sessionId;
	}
	
	/**
	 * Get aircraft ID.
	 * @return
	 */
	public String getAcid() {
		return acid;
	}
	
	/**
	 * Get the value of whether the aircraft is an external one
	 * @return
	 */
	public boolean getFlag_external_aircraft() {
		return flag_external_aircraft;
	}

	/**
	 * Get the assigned user
	 * @return
	 */
	public String getAssigned_user() {
		return assigned_user;
	}

	/**
	 * Get current latitude degree.
	 * @return
	 */
	public float getLatitude_deg() {
		return latitude_deg;
	}
	
	/**
	 * Set new value to current latitude.
	 * @param latitude_deg
	 */
	public void setLatitude_deg(float latitude_deg) {
		this.latitude_deg =  latitude_deg;
		synchronize();
	}
	
	/**
	 * Get current longitude degree.
	 * @return
	 */
	public float getLongitude_deg() {
		return longitude_deg;
	}
	
	/**
	 * Set new value to current longitude.
	 * @param longitude_deg
	 */
	public void setLongitude_deg(float longitude_deg) {
		this.longitude_deg = longitude_deg;
		synchronize();
	}
	
	/**
	 * Get current altitude in feet.
	 * @return
	 */
	public float getAltitude_ft() {
		return altitude_ft;
	}
	
	/**
	 * Set altitude in feet.
	 * @param altitude_ft
	 */
	public void setAltitude_ft(float altitude_ft) {
		this.altitude_ft = altitude_ft;
		synchronize();
	}
	
	/**
	 * Get rate of climb or descent in feet per second.
	 * @return
	 */
	public float getRocd_fps() {
		return rocd_fps;
	}
	/**
	 * Set rate of climb or descent in feet per second.
	 * @param rocd_fps
	 */
	public void setRocd_fps(float rocd_fps) {
		this.rocd_fps = rocd_fps;
		synchronize();
	}
	
	/**
	 * Get current speed.
	 * @return
	 */
	public float getTas_knots() {
		return tas_knots;
	}
	/**
	 * Set current speed
	 * @param tas_knots
	 */
	public void setTas_knots(float tas_knots) {
		this.tas_knots = tas_knots;
		synchronize();
	}
	
	/**
	 * Get current course.
	 * @return
	 */
	public float getCourse_rad() {
		return course_rad;
	}
	
	/**
	 * Set new value to current course.
	 * @param course_rad
	 */
	public void setCourse_rad(float course_rad) {
		this.course_rad = course_rad;
		synchronize();
	}
	
	/**
	 * Get flight path angle.
	 * @return
	 */
	public float getFpa_rad() {
		return fpa_rad;
	}
	
	/**
	 * Get current flight phase
	 * @return
	 */
	public int getFlight_phase() {
		return flight_phase;
	}
	
	/**
	 * Set new value to the current flight phase
	 * @param flight_phase
	 */
	public void setFlight_phase(int flight_phase) {
		this.flight_phase = flight_phase;
		synchronize();
	}
	
	/**
	 * Get departure time in second.
	 * @return
	 */
	public float getDeparture_time_sec() {
		return departure_time_sec;
	}
	
	/**
	 * Get cruise altitude in feet.
	 * @return
	 */
	public float getCruise_alt_ft() {
		return cruise_alt_ft;
	}
	
	/**
	 * Set cruise altitude in feet.
	 * @param cruise_alt_ft
	 */
	public void setCruise_alt_ft(float cruise_alt_ft) {
		this.cruise_alt_ft = cruise_alt_ft;
		synchronize();
	}
	
	/**
	 * Get cruise speed.
	 * @return
	 */
	public float getCruise_tas_knots() {
		return cruise_tas_knots;
	}
	
	/**
	 * Set cruise speed.
	 * @param cruise_tas_knots
	 */
	public void setCruise_tas_knots(float cruise_tas_knots) {
		this.cruise_tas_knots = cruise_tas_knots;
		synchronize();
	}
	
	/**
	 * Get array of latitude of the flight plan.
	 * @return
	 */
	public float[] getFlight_plan_latitude_array() {
		return flight_plan_latitude_array;
	}
	
	/**
	 * Get array of longitude of the flight plan.
	 * @return
	 */
	public float[] getFlight_plan_longitude_array() {
		return flight_plan_longitude_array;
	}
	
	/**
	 * Get number of records of the flight plan.
	 * @return
	 */
	public int getFlight_plan_length() {
		return flight_plan_length;
	}
	
	/**
	 * Get array of waypoint names of the flight plan.
	 * @return
	 */
	public String[] getFlight_plan_waypoint_name_array() {
		String[] retValue = null;
		retValue = flight_plan_waypoint_name_array;
		return retValue;
	}
	
	/**
	 * Get array of altitude description of the flight plan.
	 * @return
	 */
	public String[] getFlight_plan_alt_desc_array() {
		return flight_plan_alt_desc_array;
	}

	/**
	 * Get array of upper bound altitude of the flight plan.
	 * @return
	 */
	public double[] getFlight_plan_alt_1_array() {
		return flight_plan_alt_1_array;
	}

	/**
	 * Get array of lower bound altitude of the flight plan.
	 * @return
	 */
	public double[] getFlight_plan_alt_2_array() {
		return flight_plan_alt_2_array;
	}

	/**
	 * Get array of speed limit of the flight plan.
	 * @return
	 */
	public double[] getFlight_plan_speed_limit_array() {
		return flight_plan_speed_limit_array;
	}
	
	/**
	 * Get array of speed limit description of the flight plan.
	 * @return
	 */
	public String[] getFlight_plan_speed_limit_desc_array() {
		return flight_plan_speed_limit_desc_array;
	}
	
	/**
	 * Get elevation of the origin airport.
	 * @return
	 */
	public float getOrigin_airport_elevation_ft() {
		return origin_airport_elevation_ft;
	}
	
	/**
	 * Get elevation of the destination airport.
	 * @return
	 */
	public float getDestination_airport_elevation_ft() {
		return destination_airport_elevation_ft;
	}
	
	/**
	 * Get flag value indicating if the aircraft is landed.
	 * @return
	 */
	public int getLanded_flag() {
		return landed_flag;
	}
	
	/**
	 * Get array index of the flight plan data of the target waypoint.
	 * @return
	 */
	public int getTarget_waypoint_index() {
		return target_waypoint_index;
	}
	
	public String getTarget_waypoint_name() {
		return target_waypoint_name;
	}
	
	/**
	 * Get the flight plan array index of the top-of-climb waypoint.
	 * @return
	 */
	public int getToc_index() {
		return toc_index;
	}
	
	/**
	 * Get the flight plan array index of the top-of-descent waypoint.
	 * @return
	 */
	public int getTod_index() {
		return tod_index;
	}

	/**
	 * Set new value to airborne target waypoint latitude.
	 * @param latitude_deg
	 */
	public void setAirborne_target_waypoint_latitude_deg(float latitude_deg) throws Exception {
		this.target_waypoint_latitude_deg = latitude_deg;
		setFlight_plan_latitude_deg(airborne_target_waypoint_index, latitude_deg);
	}
	
	/**
	 * Set new value to airborne target waypoint longitude.
	 * @param longitude_deg
	 */
	public void setAirborne_target_waypoint_longitude_deg(float longitude_deg) throws Exception {
		this.target_waypoint_longitude_deg = longitude_deg;
		setFlight_plan_longitude_deg(airborne_target_waypoint_index, longitude_deg);
	}
	
	/**
	 * Set latitude of the n-th airborne waypoint.
	 * @param index Index number of the flight plan array.
	 * @param latitude_deg Latitude
	 */
	public void setFlight_plan_latitude_deg(int index, float latitude_deg) throws Exception {
		if (index < 0) {
			throw new Exception("Flight plan index is invalid");
		}
		
		this.fp_latitude_index_to_modify = index;
		this.fp_latitude_deg_to_modify = latitude_deg;
		
		int ret = synchronize();
		if (ret == 0) { // If succeeded
			if (latitude_deg != flight_plan_latitude_array[index]) {
				flight_plan_waypoint_name_array[index] = "";
			}
			this.flight_plan_latitude_array[index] = latitude_deg;
		}
	}
	
	/**
	 * Set longitude of the n-th airborne waypoint.
	 * @param index Index number of the flight plan array.
	 * @param longitude_deg Longitude
	 */
	public void setFlight_plan_longitude_deg(int index, float longitude_deg) throws Exception {
		if (index < 0) {
			throw new Exception("Flight plan index is invalid");
		}
		
		this.fp_longitude_index_to_modify = index;
		this.fp_longitude_deg_to_modify = longitude_deg;
		
		int ret = synchronize();
		if (ret == 0) { // If succeeded
			if (longitude_deg != flight_plan_longitude_array[index]) {
				flight_plan_waypoint_name_array[index] = "";
			}
			this.flight_plan_longitude_array[index] = longitude_deg;
		}
	}
	
}

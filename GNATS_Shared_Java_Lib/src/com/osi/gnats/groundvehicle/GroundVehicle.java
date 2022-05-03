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

package com.osi.gnats.groundvehicle;

import java.io.Serializable;

public class GroundVehicle extends GroundVehicleSynchronizer implements Serializable {
	private static final long serialVersionUID = 3103179025079369656L;

	private int sessionId = 0;
	
	private String gvid = null;
	private String acid = null;
	private String airportId = null;

	private boolean flag_external_groundvehicle;
	private String assigned_user;
	
	/* Current state */
	private float latitude;
	private float longitude;
	private float altitude;
	private float speed;
	private float course;
	
	// GroundVehicle static data
	private float departure_time;
	private float[] drive_plan_latitude_array; 
	private float[] drive_plan_longitude_array;
	private int drive_plan_length;
	private String[] drive_plan_waypoint_name_array;

	// GroundVehicle intent
	private int target_waypoint_index;
	private String target_waypoint_name;
	private float target_waypoint_latitude_deg;
	private float target_waypoint_longitude_deg;
	
	// Waypoint change
	private int dp_latitude_index_to_modify = -1;
	private float dp_latitude_to_modify;
	private int dp_longitude_index_to_modify = -1;
	private float dp_longitude_to_modify;
	
	public GroundVehicle(String gvid) {
		this.gvid = gvid;
	}
	
	
	private int synchronize(String parameter) {
		int retValue = -1;

		try {
			if (this.flag_external_groundvehicle) {
				throw new Exception("This is an external ground vehicle.  Can't set state to it.");
			}
			if (remoteGroundVehicle != null) {
				retValue = remoteGroundVehicle.synchronize_groundvehicle_to_server(this, parameter);
			}
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
	 * Get ground vehicle ID.
	 * @return
	 */
	public String getGvid() {
		return gvid;
	}
	
	/**
	 * Get airport ICAO code.
	 * @return
	 */
	public String getAirportId() {
		return airportId;
	}
	
	/**
	 * Get aircraft ID.
	 * @return
	 */
	public String getAircraftInService() {
		return acid;
	}
	
	
	/**
	 * Get the value of whether the ground vehicle is an external one
	 * @return
	 */
	public boolean getFlag_external_groundvehicle() {
		return flag_external_groundvehicle;
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
	public float getLatitude() {
		return latitude;
	}
	
	/**
	 * Set new value to current latitude.
	 * @param latitude
	 */
	public void setLatitude(float latitude) {
		this.latitude = latitude;
		synchronize("latitude");
	}
	
	/**
	 * Get current longitude degree.
	 * @return
	 */
	public float getLongitude() {
		return longitude;
	}
	
	/**
	 * Set new value to current longitude.
	 * @param longitude
	 */
	public void setLongitude(float longitude) {
		this.longitude = longitude;
		synchronize("longitude");
	}
	
	/**
	 * Get current altitude in feet.
	 * @return
	 */
	public float getAltitude() {
		return altitude;
	}
	
	/**
	 * Get current speed.
	 * @return
	 */
	public float getSpeed() {
		return speed;
	}
	
	/**
	 * Set current speed
	 * @param speed
	 */
	public void setSpeed(float speed) {
		this.speed = speed;
		synchronize("speed");
	}
	
	/**
	 * Get current course.
	 * @return
	 */
	public float getCourse() {
		return course;
	}
	
	/**
	 * Set new value to current course.
	 * @param course
	 */
	public void setCourse(float course) {
		this.course = course;
		synchronize("course");
	}
	
	/**
	 * Get departure time.
	 * @return
	 */
	public float getDeparture_time() {
		return departure_time;
	}
	
	/**
	 * Get array of latitude of the drive plan.
	 * @return
	 */
	public float[] getDrive_plan_latitude_array() {
		return drive_plan_latitude_array;
	}
	
	/**
	 * Get array of longitude of the drive plan.
	 * @return
	 */
	public float[] getDrive_plan_longitude_array() {
		return drive_plan_longitude_array;
	}
	
	/**
	 * Get number of records of the drive plan.
	 * @return
	 */
	public int getDrive_plan_length() {
		return drive_plan_length;
	}
	
	/**
	 * Get array of waypoint names of the drive plan.
	 * @return
	 */
	public String[] getDrive_plan_waypoint_name_array() {
		return drive_plan_waypoint_name_array;
	}
	
	
	/**
	 * Get array index of the drive plan data of the target waypoint.
	 * @return
	 */
	public int getTarget_waypoint_index() {
		return target_waypoint_index;
	}

	
	/**
	 * Get target waypoint name.
	 * @return
	 */
	public String getTarget_waypoint_name() {
		return target_waypoint_name;
	}
	
	/**
	 * Set latitude of the n-th drive plan waypoint.
	 * @param index Index number of the drive plan array.
	 * @param latitude Latitude
	 */
	public void setDrive_plan_latitude(int index, float latitude) throws Exception {
		if (index < 0) {
			throw new Exception("drive plan index is invalid");
		}
		
		this.dp_latitude_index_to_modify = index;
		this.dp_latitude_to_modify = latitude;
		
		int ret = synchronize("");
		if (ret == 0) { // If succeeded
			if (latitude != drive_plan_latitude_array[index]) {
				drive_plan_waypoint_name_array[index] = "";
			}
			this.drive_plan_latitude_array[index] = latitude;

		}
	}
	
	/**
	 * Set longitude of the n-th drive plan waypoint.
	 * @param index Index number of the drive plan array.
	 * @param longitude Longitude
	 */
	public void setDrive_plan_longitude(int index, float longitude) throws Exception {
		if (index < 0) {
			throw new Exception("drive plan index is invalid");
		}
		
		this.dp_longitude_index_to_modify = index;
		this.dp_longitude_to_modify = longitude;
		
		int ret = synchronize("");
		if (ret == 0) { // If succeeded
			if (longitude != drive_plan_longitude_array[index]) {
				drive_plan_waypoint_name_array[index] = "";
			}
			this.drive_plan_longitude_array[index] = longitude;
		}
	}
	
}

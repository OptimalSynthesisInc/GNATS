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

package com.osi.gnats.api.entity;

import java.rmi.RemoteException;

import com.osi.gnats.api.BaseInterface;
import com.osi.util.AircraftClearance;

public interface ControllerInterface extends BaseInterface {

	/**
	 * Set delay period in seconds, for providing clearance to an aircraft.
	 * @param acid The callsign of the aircraft
	 * @param aircraft_clearance AircraftClearance object
	 * @param seconds To be specified in seconds. 10 seconds, as an example.
	 * @return
	 */
	public int setDelayPeriod(String acid, AircraftClearance aircraft_clearance, float seconds);
	
	/**
	 * The controller makes the pilot repeat an action, based on the repeatParameter value.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerRepeatParameter Flight parameter for which action is to be repeated
	 * @return
	 */
	public int setActionRepeat(String controllerAircraftID, String controllerRepeatParameter);

	/**
	 * The controller skips issuing clearance to an aircraft to the next required flight phase.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerFlightPhase Flight parameter for which action is to be skipped
	 * @return
	 */	
	public int skipFlightPhase(String controllerAircraftID, String controllerFlightPhase);

	/**
	 * Instead of clearing the aircraft to the value of one parameter, the controller erroneously clears the aircraft to another value.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerOriginalChangeParameter Original parameter to be changed due to controller action 
	 * @param controllerWrongChangeParameter Erroneous parameter to be changed due to controller action
	 * @return
	 */	
	public int setWrongAction(String controllerAircraftID, String controllerOriginalChangeParameter, String controllerWrongChangeParameter);

	/**
	 * Controller issues clearance to perform reverse of the intended action, by reversing the value of the changeParameter.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerChangeParameter Flight parameter for which action is to be reversed
	 * @return
	 */	
	public int setActionReversal(String controllerAircraftID, String controllerChangeParameter);

	/**
	 * Clears the aircraft to execute only a part of a required action, by providing the original target value of the parameter, and a percentage of its value to be executed.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerChangeParameter Flight parameter for which action is to be partially performed
	 * @param controllerOriginalTarget Originial value for parameter
	 * @param controllerPercentage Percentage of action to be executed
	 * @return
	 */	
	public int setPartialAction(String controllerAircraftID, String controllerChangeParameter, float controllerOriginalTarget, float controllerPercentage);

	/**
	 * Omits issuing the clearance by the controller, resulting in the pilot continuing to maintain current value for the skipParameter.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerSkipParameter Flight parameter for which action is to be skipped
	 * @return
	 */	
	public int skipChangeAction(String controllerAircraftID, String controllerSkipParameter);
	

	/**
	 * Controller issues lagged clearances lagging the aircraft action, by specifying a certain percent of the execution to be completed within a given time period.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param controllerLagParameter Flight parameter for which action is to be lagged
	 * @param controllerLagTimeConstant To be specified in seconds. 10 seconds, as an example.
	 * @param controllerPercentageError Error percentage for the lag. For example, if 95% of the action is to be executed in the lag time constant, percentage error would be 0.05.
	 * @param controllerParameterTarget Original parameter value to be reached.
	 * @return
	 */
	public int setActionLag(String controllerAircraftID, String controllerLagParameter, float controllerLagTimeConstant, float controllerPercentageError, float controllerParameterTarget);	

	/**
	 * Controller advisories can be absent for a given time period, requiring the aircraft to execute default plans while waiting for the controller to provide updates.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param timeSteps Number of time steps for which controller is absent
	 * @return
	 */	
	public int setControllerAbsence(String controllerAircraftID, int timeSteps);
	
	/**
	 * The Controller releases the aircraft from the holding pattern and inserts it into the arrival stream.
	 * @param controllerAircraftID The callsign of the aircraft
	 * @param approach Approach procedure for landing
	 * @param targetWaypoint Waypoint to head to once released from hold pattern
	 * @return
	 */
	public int releaseAircraftHold(String controllerAircraftID, String approach, String targetWaypoint) throws RemoteException, Exception;
	
	/**
	 * Enable/disable "Conflict Detection and Resolution"
	 * @param flag
	 */
	public void enableConflictDetectionAndResolution(boolean flag);
	
	/**
	 * Get current status of CD&R conflicting events 
	 * 
	 * Result data: An array of CD&R status.
	 * 
	 * Each array element is formated in the form of an array.  The content are:
	 *                            aircraft ID of the held aircraft,
	 *                            aircraft ID of the conflicting aircraft,
	 *                            seconds of holding of the held aircraft
     * Format type: [[String, String, float]]
     * Example: [["AC1", "AC_conflicting_with_AC1", heldSeconds_AC1], ["AC2", "AC_conflicting_with_AC2", heldSeconds_AC2]]
	 * @return
	 * @throws RemoteException
	 */
	public Object[][] getCDR_status() throws RemoteException;
	
	/**
	 * Initiation distance of CD&R algorithm when the aircraft is in surface area
	 * @param distance
	 */
	public void setCDR_initiation_distance_ft_surface(float distance);
	
	/**
	 * Initiation distance of CD&R algorithm when the aircraft is in terminal area
	 * @param distance
	 */
	public void setCDR_initiation_distance_ft_terminal(float distance);
	
	/**
	 * Initiation distance of CD&R algorithm when the aircraft is in enroute phase
	 * @param distance
	 */
	public void setCDR_initiation_distance_ft_enroute(float distance);
	
	/**
	 * Separation distance of CD&R algorithm when the aircraft is in surface area
	 * @param distance
	 */
	public void setCDR_separation_distance_ft_surface(float distance);
	
	/**
	 * Separation distance of CD&R algorithm when the aircraft is in terminal area
	 * @param distance
	 */
	public void setCDR_separation_distance_ft_terminal(float distance);
	
	/**
	 * Separation distance of CD&R algorithm when the aircraft is in enroute phase
	 * @param distance
	 */
	public void setCDR_separation_distance_ft_enroute(float distance);
	
	/**
	* Enable merging and spacing at a meter fix waypoint on the arrival stream of aircraft. This helps to space out flights for safety reasons both in air and on ground.
	* @param centerId The ARTCC Id where to which the meter fix belongs.
	* @param meterFix The meter fix point where the spacing needs to be enabled.
	* @param trailAttribute String, with permitted values being "TIME" or "DISTANCE". This defines whether the float input for the last parameter is distance or time for aircraft spacing.
	* @param timeInTrail/distanceInTrail The minimum separation distance or time between aircraft. This input should be consistent with the selection for trailAttribute parameter. timeInTrails 	 is to be supplied in minutes, and distanceInTrail is to be supplied in miles.
	* @return
	*/
	public void enableMergingAndSpacingAtMeterFix(String centerId, String meterFix, String spacingType, float spacingDistance);

	/**
	* Disable merging and spacing at a meter fix waypoint on the arrival stream of aircraft. This helps to space out flights for safety reasons both in air and on ground.
	* @param centerId The ARTCC Id where to which the meter fix belongs.
	* @param meterFix The meter fix point where the spacing needs to be disabled.
	* @return
	*/
	public void disableMergingAndSpacingAtMeterFix(String centerId, String meterFix);

	/**
	 * Enable/disable Strategic Weather Avoidance
	 * @param flag
	 */
	public void enableStrategicWeatherAvoidance(boolean flag);
	
	/**
	 * Set Pirep weather file
	 * @param pathFilename
	 */
	public void setWeather_pirepFile(String pathFilename);
	
	/**
	 * Set polygon file
	 * @param pathFilename
	 */
	public void setWeather_polygonFile(String pathFilename);
	
	/**
	 * Set Sigmet weather file
	 * @param pathFilename
	 */
	public void setWeather_sigmetFile(String pathFilename);
	
	/**
	 * Set to-be-avoided waypoint for the Tactical Weather Avoidance
	 * @param waypoint_name Name of the waypoint to avoid
	 * @param duration_sec Duration in seconds to avoid
	 * @return
	 */
	public int setTacticalWeatherAvoidance(String waypoint_name, float duration_sec);
	
	/**
	 * Insert new waypoint in the airborne flight plan
	 * @param acid Aircraft ID
	 * @param index_to_insert Position to insert this new waypoint.  0 means first.
	 * @param waypoint_type Procedure type.  Value is "SID", "ENROUTE", "STAR", "APPROACH".
	 * @param waypoint_name Name of waypoint
	 * @param waypoint_latitude Latitude of waypoint
	 * @param waypoint_longitude Longitude of waypoint
	 * @param waypoint_altitude Altitude of waypoint
	 * @param waypoint_speed_lim Speed limit of waypoint
	 * @param spdlim_desc Speed limit description of waypoint.  Value is "", "+", "-".
	 * @return
	 * @throws RemoteException
	 */
	public int insertAirborneWaypoint(String acid,
			int index_to_insert,
			String waypoint_type,
			String waypoint_name,
			float waypoint_latitude,
			float waypoint_longitude,
			float waypoint_altitude,
			float waypoint_speed_lim,
			String spdlim_desc) throws RemoteException;

	/**
	 * Delete waypoint in the airborne flight plan
	 * @param acid
	 * @param index_to_delete
	 * @return
	 * @throws RemoteException
	 */
	public int deleteAirborneWaypoint(String acid,
			int index_to_delete) throws RemoteException;
	
	public void setTargetWaypoint(String acid,
			int index_of_target) throws RemoteException;
}

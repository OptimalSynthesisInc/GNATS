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

package com.osi.gnats.server.entity;

import java.rmi.RemoteException;
import java.lang.Math;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.rmi.entity.RemoteController;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.util.AircraftClearance;

public class ServerController extends ServerClass implements RemoteController {

	public ServerController(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}

	/*
	 * Method to check if a String array contains a particular String value.
	 */
	public boolean contains(String[] arr, String item) {
      for (String n : arr) {
         if (item.equals(n)) {
            return true;
         }
      }
      return false;
	}
	
	/*
	 * This function returns the runway points (Threshold and end) in the form ((latitudeThreshold, longitudeThreshold),(latitudeEnd, longitudeEnd))
	 */
	public String getRunwayEnd(String airportId, String runway) {
		String[] runwayExits;
		String runwayEnd;
		
		runwayExits = cEngine.getRunwayExits(airportId, runway);
		runwayEnd = runwayExits[runwayExits.length - 1];
		return runwayEnd;
		
	}
	
	
	public int setDelayPeriod(String acid, AircraftClearance aircraft_clearance, float seconds) {
		return cEngine.controller_setDelayPeriod(acid, aircraft_clearance.name(), seconds);
	}

	/*
	 * Controller issues lagged clearances affecting pilot action, by reaching
	 * certain percent of execution within a given time period. Following are the
	 * parameters: lagParameter: Paremeter to be lagged, can have following values:
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE 
	 * lagTimeConstant: To be provided in seconds. Eg. 10 seconds. 
	 * percentageError: Error percentage for lag. For example, if 95% of the action is to be executed, percentage error would be
	 * 0.05. 
	 * parameterTarget: Original parameter value to be reached.
	 */
	public int setActionLag(int sessionId, String aircraftID, String lagParameter, float lagTimeConstant, float percentageError,
			float parameterTarget) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			float parameterCurrentValue = 0;
			if (lagParameter.equals("AIRSPEED")) {
				parameterCurrentValue = aircraft.getTas_knots();
				cEngine.pilot_setActionLag(aircraftID, lagParameter, lagTimeConstant, percentageError,
						parameterCurrentValue, parameterTarget);
			} else if (lagParameter.equals("VERTICAL_SPEED")) {
				parameterCurrentValue = aircraft.getRocd_fps();
				cEngine.pilot_setActionLag(aircraftID, lagParameter, lagTimeConstant, percentageError,
						parameterCurrentValue, parameterTarget);
			} else if (lagParameter.equals("COURSE")) {
				parameterCurrentValue = aircraft.getCourse_rad();
				parameterCurrentValue *= 180.0 / Math.PI;
				cEngine.pilot_setActionLag(aircraftID, lagParameter, lagTimeConstant, percentageError,
						parameterCurrentValue, parameterTarget);
			} else {
				retVal = 1;
			}

		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * Omit issuing of clearance by controller, resulting in the pilot continuing to
	 * maintain current value for skipParameter. skipParameter can have following
	 * values: 
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE
	 */
	public int skipChangeAction(int sessionId, String aircraftID, String skipParameter) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			float previousValue = cEngine.pilot_setActionRepeat(aircraftID, skipParameter, 0);
			if (skipParameter.equals("AIRSPEED")) {
				aircraft.setTas_knots(previousValue);
			} else if (skipParameter.equals("VERTICAL_SPEED")) {
				aircraft.setRocd_fps(previousValue);
			} else if (skipParameter.equals("COURSE")) {
				previousValue *= Math.PI / 180;
				aircraft.setCourse_rad(previousValue);
			} else {
				retVal = 1;
			}
			cEngine.synchronize_aircraft_to_server(aircraft);
		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * Clear the pilot to execute only part of an action, by providing the original
	 * target value of parameter, and percentage of it to be executed.
	 * changeParameter can have following values: 
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE
	 */
	public int setPartialAction(int sessionId, String aircraftID, String changeParameter, float originalTarget, float percentage) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			float newTarget = percentage * originalTarget / 100;
			if (changeParameter.equals("AIRSPEED")) {
				aircraft.setTas_knots(newTarget);
			} else if (changeParameter.equals("VERTICAL_SPEED")) {
				aircraft.setRocd_fps(newTarget);
			} else if (changeParameter.equals("COURSE")) {
				newTarget *= Math.PI / 180.0;
				aircraft.setCourse_rad(newTarget);
			} else {
				retVal = 1;
			}
			cEngine.synchronize_aircraft_to_server(aircraft);
		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * Controller issues clearance to perform reverse of intended action, by
	 * reverting the value of changeParameter. changeParameter can have following
	 * values: 
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE
	 */
	public int setActionReversal(int sessionId, String aircraftID, String changeParameter) {
		float previousValue = cEngine.pilot_setActionRepeat(aircraftID, changeParameter, 1);
		float currentTas = 0;
		float currentRocd = 0;
		float currentCourse = 0;
		float paramDifference = 0;
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			if (changeParameter.equals("AIRSPEED")) {
				currentTas = aircraft.getTas_knots();
				paramDifference = currentTas - previousValue;
				aircraft.setTas_knots(currentTas + paramDifference * -1);
			} else if (changeParameter.equals("VERTICAL_SPEED")) {
				currentRocd = aircraft.getRocd_fps();
				paramDifference = currentRocd - previousValue;
				aircraft.setRocd_fps(currentRocd + paramDifference * -1);
			} else if (changeParameter.equals("COURSE")) {
				currentCourse = aircraft.getCourse_rad();
				previousValue *= Math.PI / 180;
				paramDifference = currentCourse - previousValue;
				aircraft.setCourse_rad(currentCourse + paramDifference * -1);
			} else {
				retVal = 1;
			}
			cEngine.synchronize_aircraft_to_server(aircraft);
		} else {
			retVal = 1;
		}

		return retVal;
	}

	/*
	 * Clear the pilot to set the value of one parameter, erroneously to another.
	 * For example, the controller can assign the magnitude of airspeed (170 kts) as
	 * course angle (170 degrees). These are following pairs of parameters that can
	 * be mutually interchanged: 
	 * 1. AIRSPEED – COURSE 
	 * 2. FLIGHT_LEVEL – AIRSPEED 
	 * 3. COURSE – FLIGHT_LEVEL
	 */
	public int setWrongAction(int sessionId, String aircraftID, String originalChangeParameter, String wrongChangeParameter) {
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		if (aircraft != null) {
			if (originalChangeParameter.equals("AIRSPEED") && wrongChangeParameter.equals("COURSE")) {
				float currentTas = aircraft.getTas_knots();
				currentTas *= Math.PI / 180;
				aircraft.setCourse_rad(currentTas);
			} else if (originalChangeParameter.equals("COURSE") && wrongChangeParameter.equals("AIRSPEED")) {
				float currentCourse = aircraft.getCourse_rad();
				currentCourse *= 180 / Math.PI;
				aircraft.setTas_knots(currentCourse);
			} else if (originalChangeParameter.equals("AIRSPEED") && wrongChangeParameter.equals("FLIGHT_LEVEL")) {
				float currentTas = aircraft.getTas_knots();
				aircraft.setAltitude_ft(currentTas);
			} else if (originalChangeParameter.equals("FLIGHT_LEVEL") && wrongChangeParameter.equals("AIRSPEED")) {
				float currentFltLvl = aircraft.getAltitude_ft();
				aircraft.setTas_knots(currentFltLvl);
			} else if (originalChangeParameter.equals("COURSE") && wrongChangeParameter.equals("FLIGHT_LEVEL")) {
				float currentCourse = aircraft.getCourse_rad();
				currentCourse *= 180 / Math.PI;
				aircraft.setAltitude_ft(currentCourse);
			} else if (originalChangeParameter.equals("FLIGHT_LEVEL") && wrongChangeParameter.equals("COURSE")) {
				float currentFltLvl = aircraft.getAltitude_ft();
				currentFltLvl *= Math.PI / 180;
				aircraft.setCourse_rad(currentFltLvl);
			} else {
				retVal = 1;
			}
			cEngine.synchronize_aircraft_to_server(aircraft);
		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * The controller makes the pilot repeat an action, based on the repeatParameter
	 * value. repeatParameter can have following values: 
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE
	 */
	public int setActionRepeat(int sessionId, String aircraftID, String repeatParameter) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		float previousValue = cEngine.pilot_setActionRepeat(aircraftID, repeatParameter, 0);
		float currentTas = 0;
		float currentRocd = 0;
		float currentCourse = 0;
		float paramDifference = 0;

		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			if (repeatParameter.equals("AIRSPEED")) {
				currentTas = aircraft.getTas_knots();
				paramDifference = currentTas - previousValue;
				aircraft.setTas_knots(currentTas + paramDifference);
			} else if (repeatParameter.equals("VERTICAL_SPEED")) {
				currentRocd = aircraft.getRocd_fps();
				paramDifference = currentRocd - previousValue;
				aircraft.setRocd_fps(currentRocd + paramDifference);
			} else if (repeatParameter.equals("COURSE")) {
				currentCourse = aircraft.getCourse_rad();
				previousValue *= Math.PI / 180;
				paramDifference = currentCourse - previousValue;
				aircraft.setCourse_rad(currentCourse + paramDifference);
			} else {
				retVal = 1;
			}

			cEngine.synchronize_aircraft_to_server(aircraft);
		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * The controller skips clearing the aircraft to the required flight phase.
	 * flightPhase can have any of the Flight Phase Enum Values. Eg.
	 * FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE
	 */
	public int skipFlightPhase(int sessionId, String aircraftID, String flightPhase) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			retVal = cEngine.pilot_skipFlightPhase(aircraftID, flightPhase);
		} else {
			retVal = 1;
		}
		return retVal;
	}

	/*
	 * Controller advisories can be absent for a given time period, requiring the
	 * pilot to execute default plans while waiting for controller to provide
	 * updates. Parameter timeSteps denotes number of steps that pilot would be
	 * flying without controller intervention.
	 */
	public int setControllerAbsence(int sessionId, String aircraftID, int timeSteps) {
		int retVal = 1; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if ((timeSteps > -1) && (aircraft != null))
			retVal = cEngine.setControllerAbsence(aircraftID, timeSteps);
		return retVal;
	}
	
	/*
	 * The controller commands the pilot to break out of the hold pattern and get
	 * back into the arrival stream.
	 */
	public int releaseAircraftHold(int sessionId, String aircraftID, String approach, String targetWaypoint) throws RemoteException, Exception {
		Aircraft aircraft = null;
		String landingRunway;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
			if (aircraft != null && !targetWaypoint.equals("")) {
			String arrivalAirport = cEngine.getArrivalAirport(aircraftID);
			String arrivalGate;
			if (approach.equals("")) {
				double latitude_deg = cEngine.getWaypoint_Latitude_Longitude_deg(targetWaypoint)[0];
				double longitude_deg = cEngine.getWaypoint_Latitude_Longitude_deg(targetWaypoint)[1];
				aircraft.setAirborne_target_waypoint_latitude_deg((float)latitude_deg);
				aircraft.setAirborne_target_waypoint_longitude_deg((float)longitude_deg);
			}
			else {
				String[] approachLegs = cEngine.getProcedure_leg_names("APPROACH", approach, arrivalAirport);
				String[] approachLegWaypoints = null;
				String[] arrivalTaxiPlan = null;
				int count = 0;
				
				for (String approachLeg: approachLegs) {
				    approachLegWaypoints = cEngine.getWaypoints_in_procedure_leg( "APPROACH", approach, arrivalAirport, approachLeg);
				    if(contains(approachLegWaypoints, targetWaypoint))
				    	break;
				}
				
				String[] landingTaxiPlan = cEngine.getSurface_taxi_plan(aircraftID, arrivalAirport);
				arrivalGate = landingTaxiPlan[landingTaxiPlan.length-1];
				landingRunway = "RW" + approach.substring(approach.length() - 3);
				
				int targetWaypointIndex = aircraft.getTarget_waypoint_index();
				String[] waypointsInFlightPlan = aircraft.getFlight_plan_waypoint_name_array();
				count = waypointsInFlightPlan.length;
				
				int i = 0;
				for (i = count - 1; i > targetWaypointIndex; i--) {
					cEngine.deleteAirborneWaypoint(aircraftID, i);
				}
			
				float latitude = 0.0f;
				float longitude = 0.0f;
				int insertIndexBuffer = targetWaypointIndex + 1;			

				for (i = 0; i < approachLegWaypoints.length; i++) {
					try {
						latitude = (float)cEngine.getWaypoint_Latitude_Longitude_deg(approachLegWaypoints[i])[0];
						longitude = (float)cEngine.getWaypoint_Latitude_Longitude_deg(approachLegWaypoints[i])[1];
						
						insertAirborneWaypoint(aircraftID, insertIndexBuffer, "APPROACH", approachLegWaypoints[i], latitude, longitude, 0.0f, 0.0f, "");
						
						insertIndexBuffer++;
					}
					catch (Exception e) {
						System.out.println(approachLegWaypoints[i] + " waypoint insertion failed.");
					}
					if(approachLegWaypoints[i].toLowerCase().contains("rw"))
						break;
				}
						
				
				String landingRunwayCode = getRunwayEnd(arrivalAirport, landingRunway);
				cEngine.generate_surface_taxi_plan(aircraftID, arrivalAirport,
						landingRunwayCode, arrivalGate,
						landingRunway);
		        
			}
		}
		return 1;
	}

	public int insertAirborneWaypoint(String acid,
			int index_to_insert,
			String waypoint_type,
			String waypoint_name,
			float waypoint_latitude,
			float waypoint_longitude,
			float waypoint_altitude,
			float waypoint_speed_lim,
			String spdlim_desc) throws RemoteException {
		return cEngine.insertAirborneWaypoint(acid, index_to_insert, waypoint_type, waypoint_name, waypoint_latitude, waypoint_longitude, waypoint_altitude, waypoint_speed_lim, spdlim_desc);
	}
	
	public int deleteAirborneWaypoint(String acid,
			int index_to_delete) throws RemoteException {
		return cEngine.deleteAirborneWaypoint(acid, index_to_delete);
	}
	
	public void setTargetWaypoint(String acid,
			int waypoint_plan_to_use,
			int index_of_target) throws RemoteException {
		cEngine.setTargetWaypoint(acid, waypoint_plan_to_use, index_of_target);
	}
	
	public void enableConflictDetectionAndResolution(boolean flag) throws RemoteException {
		cEngine.enableConflictDetectionAndResolution(flag);
	}
	
	public Object[][] getCDR_status() throws RemoteException {
		return cEngine.getCDR_status();
	}
	
	public void setCDR_initiation_distance_ft_surface(float distance) throws RemoteException {
		cEngine.setCDR_initiation_distance_ft_surface(distance);
	}
	
	public void setCDR_initiation_distance_ft_terminal(float distance) throws RemoteException {
		cEngine.setCDR_initiation_distance_ft_terminal(distance);
	}
	
	public void setCDR_initiation_distance_ft_enroute(float distance) throws RemoteException {
		cEngine.setCDR_initiation_distance_ft_enroute(distance);
	}
	
	public void setCDR_separation_distance_ft_surface(float distance) throws RemoteException {
		cEngine.setCDR_separation_distance_ft_surface(distance);
	}
	
	public void setCDR_separation_distance_ft_terminal(float distance) throws RemoteException {
		cEngine.setCDR_separation_distance_ft_terminal(distance);
	}
	
	public void setCDR_separation_distance_ft_enroute(float distance) throws RemoteException {
		cEngine.setCDR_separation_distance_ft_enroute(distance);
	}
	
	public void enableMergingAndSpacingAtMeterFix(String centerId, String meterFix, String spacingType, float spacing) throws RemoteException {
		cEngine.enableMergingAndSpacingAtMeterFix(centerId, meterFix, spacingType, spacing);
	}
	
	public void disableMergingAndSpacingAtMeterFix(String centerId, String meterFix) throws RemoteException {
		cEngine.disableMergingAndSpacingAtMeterFix(centerId, meterFix);
	}
	
	public void enableStrategicWeatherAvoidance(boolean flag) throws RemoteException {
		cEngine.enableStrategicWeatherAvoidance(flag);
	}
	
	public void setWeather_pirepFile(String pathFilename) throws RemoteException {
		cEngine.setWeather_pirepFile(pathFilename);
	}
	
	public void setWeather_polygonFile(String pathFilename) throws RemoteException {
		cEngine.setWeather_polygonFile(pathFilename);
	}
	
	public void setWeather_sigmetFile(String pathFilename) throws RemoteException {
		cEngine.setWeather_sigmetFile(pathFilename);
	}
	
	public int setTacticalWeatherAvoidance(String waypoint_name, float duration_sec) throws RemoteException {
		return cEngine.setTacticalWeatherAvoidance(waypoint_name, duration_sec);
	}
	
}

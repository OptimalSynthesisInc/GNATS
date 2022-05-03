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

import com.osi.gnats.rmi.entity.RemotePilot;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.aircraft.Aircraft;

public class ServerPilot extends ServerClass implements RemotePilot {

	public ServerPilot(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}

	/*
	 * Lag pilot action, by reaching certain percent of execution within a given
	 * time period. Following are the parameters: lagParameter: Paremeter to be
	 * lagged, can have following values: 1. AIRSPEED 2. VERTICAL_SPEED 3. COURSE
	 * lagTimeConstant: To be provided in seconds. Eg. 10 seconds. percentageError:
	 * Error percentage for lag. For example, if 95% of the action is to be
	 * executed, percentage error would be 0.05. parameterTarget: Original parameter
	 * value to be reached.
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
	 * Omit parameter change, by continuing to maintain current value for
	 * skipParameter. skipParameter can have following values: 
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
	 * Execute only part of an action, by providing the original target value of
	 * parameter, and percentage of it to be performed by pilot, for the
	 * changeParameter. changeParameter can have following values: 
	 * 1. AIRSPEED 
	 * 2. VERTICAL_SPEED 
	 * 3. COURSE
	 * 
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
	 * Reverse a pilot action, by reverting the value of changeParameter.
	 * changeParameter can have following values: 
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
	 * Set the value of one parameter, erroneously to another. For example, the
	 * pilot can set magnitude of airspeed (170 kts) as course angle (170 degrees).
	 * These are following pairs of parameters that can be mutually interchanged: 
	 * 1.AIRSPEED – COURSE 
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
	 * Repeat pilot action, based on the repeatParameter value. repeatParameter can
	 * have following values: 
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
	 * If simulation has not started, the flight plan read from TRX can be changed
	 * using this function. This constitutes to error in reading the flight plan.
	 * Following are the parameters: errorParameter: Parameter with erroneous data.
	 * It can have any of the following values: 1. AIRSPEED 2. VERTICAL_SPEED 3.
	 * COURSE correctValue: This is the correct flight plan data that should have
	 * ideally be read.
	 */
	public int setFlightPlanReadError(int sessionId, String aircraftID, String parameter, float updatedValue) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		if (cEngine.get_runtime_sim_status() == 0) {
			Aircraft aircraft = null;
			aircraft = cEngine.select_aircraft(sessionId, aircraftID);
			if (aircraft != null) {
				if (parameter.equals("AIRSPEED")) {
					aircraft.setTas_knots(updatedValue);
				} else if (parameter.equals("VERTICAL_SPEED")) {
					aircraft.setRocd_fps(updatedValue);
				} else if (parameter.equals("COURSE")) {
					aircraft.setCourse_rad(updatedValue);
				} else {
					retVal = 1;
				}
				cEngine.synchronize_aircraft_to_server(aircraft);
			} else {
				retVal = 1;
			}
		}
		return retVal;
	}

	/*
	 * Ignore flight phase transition, by skipping the mentioned flight phase.
	 * flightPhase can have any of the Flight Phase Enum Values. Eg.
	 * FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE
	 */
	public int skipFlightPhase(int sessionId, String aircraftID, String flightPhase) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftID);
		if (aircraft != null) {
			cEngine.pilot_skipFlightPhase(aircraftID, flightPhase);
		} else {
			retVal = 1;
		}
		return retVal;
	}
}

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

package com.osi.gnats.server.entity;

import java.rmi.RemoteException;
import java.lang.Math;

import com.osi.gnats.rmi.entity.RemoteGroundOperator;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.groundvehicle.GroundVehicle;

public class ServerGroundOperator extends ServerClass implements RemoteGroundOperator {

	public ServerGroundOperator(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}

	public int setGroundOperatorAbsence(int sessionId, String groundVehicleId, int timeSteps) {
		int retVal = 1; // Returned 1 implies Error, returned 0 implies Success
		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		if ((timeSteps > 0) && (groundVehicle != null))
			retVal = cEngine.setGroundOperatorAbsence(groundVehicleId, timeSteps);
		return retVal;
	}

	public int setActionRepeat(int sessionId, String groundVehicleId, String repeatParameter) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		float previousValue = cEngine.groundVehicle_setActionRepeat(groundVehicleId, repeatParameter);
		float currentSpeed = 0;
		float currentCourse = 0;
		float paramDifference = 0;

		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		if (groundVehicle != null) {
			if (repeatParameter.equals("SPEED")) {
				currentSpeed = groundVehicle.getSpeed();
				paramDifference = currentSpeed - previousValue;
				groundVehicle.setSpeed(currentSpeed + paramDifference);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "speed");
			} else if (repeatParameter.equals("COURSE")) {
				currentCourse = groundVehicle.getCourse();
				paramDifference = currentCourse - previousValue;
				groundVehicle.setCourse(currentCourse + paramDifference);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "course");
			} else {
				retVal = 1;
			}

		} else {
			retVal = 1;
		}
		return retVal;
	}

	public int setVehicleContact(int sessionId, String groundVehicleId) {
		return cEngine.setVehicleContact(groundVehicleId);
	}

	public int setWrongAction(int sessionId, String groundVehicleId, String originalChangeParameter,
			String wrongChangeParameter) {
		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		if (groundVehicle != null) {
			if (originalChangeParameter.equals("SPEED") && wrongChangeParameter.equals("COURSE")) {
				float currentSpeed = groundVehicle.getSpeed();
				groundVehicle.setCourse(currentSpeed);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "course");
			} else if (originalChangeParameter.equals("COURSE") && wrongChangeParameter.equals("SPEED")) {
				float currentCourse = groundVehicle.getCourse();
				groundVehicle.setSpeed(currentCourse);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "speed");
			} else {
				retVal = 1;
			}
		} else {
			retVal = 1;
		}
		return retVal;
	}

	public int setActionReversal(int sessionId, String groundVehicleId, String changeParameter) {
		float previousValue = cEngine.groundVehicle_setActionRepeat(groundVehicleId, changeParameter);
		float currentSpeed = 0;
		float currentCourse = 0;
		float paramDifference = 0;
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		if (groundVehicle != null) {
			if (changeParameter.equals("SPEED")) {
				currentSpeed = groundVehicle.getSpeed();
				paramDifference = currentSpeed - previousValue;
				groundVehicle.setSpeed(currentSpeed + paramDifference * -1);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "speed");
			} else if (changeParameter.equals("COURSE")) {
				currentCourse = groundVehicle.getCourse();
				paramDifference = currentCourse - previousValue;
				groundVehicle.setCourse(currentCourse + paramDifference * -1);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "course");
			} else {
				retVal = 1;
			}
		} else {
			retVal = 1;
		}

		return retVal;
	}

	public int setPartialAction(int sessionId, String groundVehicleId, String changeParameter, float originalTarget,
			float percentage) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		if (groundVehicle != null) {
			float newTarget = percentage * originalTarget / 100;
			if (changeParameter.equals("SPEED")) {
				groundVehicle.setSpeed(newTarget);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "speed");
			} else if (changeParameter.equals("COURSE")) {
				groundVehicle.setCourse(newTarget);
				cEngine.synchronize_groundvehicle_to_server(groundVehicle, "course");
			} else {
				retVal = 1;
			}
		} else {
			retVal = 1;
		}
		return retVal;
	}

	public int setActionLag(int sessionId, String groundVehicleId, String lagParameter, float lagTimeConstant,
			float percentageError, float parameterTarget) {
		int retVal = 0; // Returned 1 implies Error, returned 0 implies Success
		GroundVehicle groundVehicle = null;
		groundVehicle = cEngine.select_groundVehicle(sessionId, groundVehicleId);

		if (groundVehicle != null) {
			float parameterCurrentValue = 0;
			if (lagParameter.equals("SPEED")) {
				parameterCurrentValue = groundVehicle.getSpeed();
				cEngine.groundVehicle_setActionLag(groundVehicleId, lagParameter, lagTimeConstant, percentageError,
						parameterCurrentValue, parameterTarget);
			} else if (lagParameter.equals("COURSE")) {
				parameterCurrentValue = groundVehicle.getCourse();
				parameterCurrentValue *= 180.0 / Math.PI;
				cEngine.groundVehicle_setActionLag(groundVehicleId, lagParameter, lagTimeConstant, percentageError,
						parameterCurrentValue, parameterTarget);
			} else {
				retVal = 1;
			}

		} else {
			retVal = 1;
		}
		return retVal;
	}

}

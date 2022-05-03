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

public interface GroundOperatorInterface extends BaseInterface {

	/**
	 * Ground operators can be absent for a given time period, requiring the vehicle to stop while waiting for the operator to take back control.
	 * @param groundVehicleId The callsign of the vehicle that the operator is in-charge of.
	 * @param timeSteps Number of time steps for which operator is absent
	 * @return
	 */	
	public int setGroundOperatorAbsence(String groundVehicleId, int timeSteps);
	
	/**
	 * The ground operator repeats an action, based on the repeatParameter value.
	 * @param groundVehicleId The callsign of the aircraft
	 * @param repeatParameter Ground vehicle parameter for which action is to be repeated
	 * @return
	 */
	public int setActionRepeat(String groundVehicleId, String repeatParameter);
	
	/**
	 * Ground operators collides the ground vehicle into another object (Potentially building/aircraft/automobile/person)
	 * @param groundVehicleId The callsign of the vehicle that the operator is in-charge of.
	 * @return
	 */	
	public int setVehicleContact(String groundVehicleId);

	/**
	 * Instead of acting to change value of one parameter, the ground operator erroneously changes another.
	 * @param groundVehicleId The callsign of the ground vehicle
	 * @param originalChangeParameter Original parameter to be changed due to ground operator action 
	 * @param wrongChangeParameter Erroneous parameter to be changed due to ground operator action
	 * @return
	 */	
	public int setWrongAction(String groundVehicleId, String originalChangeParameter, String wrongChangeParameter);
	
	/**
	 * Ground operator executes reverse of the intended action, by reversing the value of the changeParameter.
	 * @param groundVehicleId The callsign of the ground vehicle
	 * @param changeParameter Ground vehicle parameter for which action is to be reversed
	 * @return
	 */	
	public int setActionReversal(String groundVehicleId, String changeParameter);
	
	/**
	 * Ground operator executes part of the originally intended action.
	 * @param groundVehicleId The callsign of the ground vehicle
	 * @param changeParameter Ground Vehicle parameter for which action is to be partially performed
	 * @param originalTarget Originial value for parameter
	 * @param percentage Percentage of action to be executed
	 * @return
	 */	
	public int setPartialAction(String groundVehicleId, String changeParameter, float originalTarget, float percentage);
		
	/**
	 * Ground operator lags vehicle action, therreby a certain percent of the execution getting completed within a given time period.
	 * @param groundVehicleId The callsign of the ground vehicle
	 * @param lagParameter Flight parameter for which action is to be lagged
	 * @param lagTimeConstant To be specified in seconds. 10 seconds, as an example.
	 * @param percentageError Error percentage for the lag. For example, if 95% of the action is to be executed in the lag time constant, percentage error would be 0.05.
	 * @param parameterTarget Original parameter value to be reached.
	 * @return
	 */
	public int setActionLag(String groundVehicleId, String lagParameter, float lagTimeConstant, float percentageError, float parameterTarget);

}

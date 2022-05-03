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

package com.osi.gnats.api;

import java.rmi.RemoteException;

public interface RiskMeasuresInterface extends BaseInterface {
    
	/**
	* This function takes-in the reference aircraft callsign as the input. It then forms a bounding box around the aircraft within which a potential safety hazard may exist. The aircraft callsigns 	 are filtered to find the ones that lie within this box, +/- 2000 ft in altitude of the reference aircraft. These flights are then analyzed for their position and velocity relative to the 		reference aircraft, which are then returned to the user.
	* @param aircraftId The ICAO code for the airport.
	* @return
	*/
	public Object getFlightsInRange(String aircraftID, float minLatitude, float maxLatitude, float minLongitude, float maxLongitude, float minAltitude_ft, float maxAltitude_ft);
	
	/**
	* For an aircraft in its landing phase, this function calculates the distance to the runway threshold.
	* @param airportId The ICAO code for the airport.
	* @return
	*/
	public double getDistanceToRunwayThreshold(String aircraftId);

	/**
	* For an aircraft in its takeoff phase, this function calculates the distance to the end of the runway.
	* @param airportId The ICAO code for the airport.
	* @return
	*/
	public double getDistanceToRunwayEnd(String aircraftId);

	/**
	* For an aircraft either in landing or takeoff phases, this function computes the alignment of the velocity vector relative to the runway centerline.
	* @param airportId The ICAO code for the airport.
	* @param procedure ARRIVAL/DEPARTURE procedure.
	* @return
	*/
	public double getVelocityAlignmentWithRunway(String aircraftId, String procedure);

	/**
	* This function returns the number of passengers occupying a particular aircraft, assuming 100% load factor. Data for all aircraft types in the ADB database are available in NATS. 
	* @param aircraftType Aircraft Type for which the passengerCount is required.
	* @return
	*/
	public int getPassengerCount(String aircraftType);

	/**
	* This function returns the cost (in millions of US Dollars) for a new aircraft of the aircraft type. Data for all aircraft types in the ADB database are available in NATS.
	* @param aircraftType Aircraft Type for which the cost is required.
	* @return
	*/
	public double getAircraftCost(String aircraftType);

	/**
	* This function models a wake vortex hazard envelope to determine wake encounter hazards for trailing flights. The wake generating aircraft is assumed to be located in the center of a rectangular, divergent, descending tube with two wingspan initial breadth and one wingspan thickness.
	* @param refAircraftId: The callsign of aircraft which is producing the wake vortex.
	* @param envelopeStartWidth: The width (in feet) of the envelope at start of wake. (typically twice the aircraft wingspan)
	* @param envelopeStartThickness: The Thickness (in feet) of the envelope at start of the wake. (typically one wingspan of the aircraft)
	* @param envelopeEndWidth: The width (in feet) of the envelope at end of the wake vortex hazard.
	* @param envelopeEndThickness: The thickness (in feet) of the envelope at end of the wake vortex hazard.
	* @param envelopeRange: Impact range(in miles) of the vortex envelope. (4 to 15 nm, depending on the weight class of the aircraft: Super, Heavy, Large)
	* @param envelopeAltitudeDrop: Drop (in feet) of the envelope end relative to the wake generating aircraft.
	* @return
	*/
	public Object getFlightsInWakeVortexRange(String refAircraftId, float envelopeStartLength, float envelopeStartBreadth, float envelopeEndLength, float envelopeEndBreadth, float envelopeRangeLength, float envelopeDecline);


	/**
	* Set the book value of the aircraft in million US$.
	* @param aircraftId: Aircraft callsign.
	* @param aircraftBookValue: Aircraft book value to be set.
	* @return
	*/
	public int setAircraftBookValue(String aircraftId, double aircraftBookValue);
	
	/**
	* Get the book value of the aircraft in million US$.
	* @param aircraftId: Aircraft callsign.
	* @return
	*/
	public double getAircraftBookValue(String aircraftId);
	
	/**
	* Set the value of the cargo in the aircraft, in million US$.
	* @param aircraftId: Aircraft callsign.
	* @param cargoWorth: Cargo worth to be set.
	* @return
	*/
	public int setCargoWorth(String aircraftId, double cargoWorth);
	
	/**
	* Get the value of the cargo in the aircraft, in million US$.
	* @param aircraftId: Aircraft callsign.
	* @return
	*/
	public double getCargoWorth(String aircraftId);
	
	/**
	* Set load factor for (passenger occupancy relative to the total number of seats) in an aircraft instance.
	* @param aircraftId: Aircraft callsign.
	* @param paxLoadFactor: Passenger load factor to be set.
	* @return
	*/
	public int setPassengerLoadFactor(String aircraftId, double paxLoadFactor);
	
	/**
	* Get load factor for passenger occupancy in an aircraft instance.
	* @param aircraftId: Aircraft callsign.
	* @return
	*/
	public double getPassengerLoadFactor(String aircraftId);
	
	/**
	 * Set aircraft touch down point on runway for landing
	 * @param aircraftId Aircraft Callsign
	 * @param latitude The latitude of touchdown point
	 * @param longitude The longitude of touchdown point
	 */
	public int setTouchdownPointOnRunway(String aircraftId, double latitude, double longitude);
	
	/**
	 * Get aircraft touch down point on runway for landing
	 * @param aircraftId Aircraft Callsign
	 */
	public double[] getTouchdownPointOnRunway(String aircraftId);
	
	/**
	 * Set aircraft take off point on runway for liftoff
	 * @param aircraftId Aircraft Callsign
	 * @param latitude The latitude of touchdown point
	 * @param longitude The longitude of touchdown point
	 */
	public int setTakeOffPointOnRunway(String aircraftId, double latitude, double longitude);
	
	/**
	 * Get aircraft take off point on runway for liftoff
	 * @param aircraftId Aircraft Callsign
	 */
	public double[] getTakeOffPointOnRunway(String aircraftId);
	
	/**
	 * Get L1 distance between two aircraft.
	 * @param airportId The ICAO code of airport where aircrafts are in taxi
	 * @param aircraftId1 The callsign of first aircraft
	 * @param aircraftId2 The callsign of second aircraft
	 * @return
	 */
	public double getL1Distance(String airportId, String aircraftId1, String aircraftId2);

	/**
	 * Get distance between aircraft current position and pavement edge in direction of course.
	 * @param airportId The ICAO code of airport where aircraft in taxi
	 * @param aircraftId Callsign of the aircraft
	 * @return
	 */
	public double getDistanceToPavementEdge(String airportId, String aircraftId);
	
	public double getL2Distance(String airportId, String vehicle1, String vehicle2) throws RemoteException;
	public double getTimeToObjectOfInterest(String airportId, String vehicle1, float latitude, float longitude) throws RemoteException;
	public double getDistanceToObjectOfInterest(String airportId, String vehicle1, float latitude, float longitude) throws RemoteException;
	public double getTimeToVehicleContact(String vehicle1, String vehicle2) throws RemoteException;
	public double getTimeToPavementEdge(String vehicleId) throws RemoteException;
	public double getRateOfLineOfSightChange(String aircraftID1, String aircraftID2) throws RemoteException;
	public double getRateOfApproachToPavementEdge(String aircraftID, int timesteps) throws RemoteException;
	public double getRateOfApproachToVehicle(String vehicle1ID, String vehicle2ID, int timesteps) throws RemoteException;
	public double getRateOfApproachToWaypoint(String aircraftID, String Waypoint, int timesteps) throws RemoteException;
	public double getRateOfApproachToWaypoint(String aircraftID, float waypointLatitude, float waypointLongitude, int timesteps) throws RemoteException;
	public double getRateOfApproachToEvent(String aircraftID, float eventCenterLatitude, float eventCenterLongitude, int timesteps) throws RemoteException;
	public double getRateOfApproachToWeatherEvent(String aircraftID, double[] weatherBounds) throws RemoteException;
	public double getRateOfApproachToWakeVortex(String leadingAircraftID, String trailingAircraftID) throws RemoteException;
	public double getRateOfVelocityAlignmentToRunway(String aircraftID, String procedure, int timesteps) throws RemoteException;
	public double getRateOfApproachToRunwayEnd(String aircraftID, int timesteps) throws RemoteException;
	public double getRateOfApproachToRunwayThreshold(String aircraftID, int timesteps) throws RemoteException;
	
	public int load_aviationOccurenceProfile(String dirPath) throws RemoteException;
	public int load_flightphase_aviationOccurence_mapping(String dirPath) throws RemoteException;
	public int setSampleWeatherHazard(double[] weatherRegionBounds) throws RemoteException;
	public String[] calculateRisk(String flightData) throws RemoteException;
	public int setRegionOfRegard(String aircraft, double[] regionBounds) throws RemoteException;
	public double[][] getRegionOfRegard(String aircraft) throws RemoteException;
	public String[] getAircraftInRegionOfRegard(String aircraft) throws RemoteException;
}

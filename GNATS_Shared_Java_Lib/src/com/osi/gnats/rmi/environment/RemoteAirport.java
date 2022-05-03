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

package com.osi.gnats.rmi.environment;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.airport.Airport;

public interface RemoteAirport extends Remote {

	public Airport select_airport(String airport_code) throws RemoteException;
	
	public String getArrivalAirport(String acid) throws RemoteException;
	
	public String getDepartureAirport(String acid) throws RemoteException;
	
	public double[] getLocation(String airport_code) throws RemoteException;
	
	public String getClosestAirport(double latitude_deg, double longitude_deg) throws RemoteException;
	
	public String[] getAirportsWithinMiles(double latitude_deg, double longitude_deg, double miles) throws RemoteException;
	
	public String getFullName(String airport_code) throws RemoteException;
	
	public Object[] getAllRunways(String airport_code) throws RemoteException;
	
	public String[] getAllGates(String airport_code) throws RemoteException;
	
	public String[] getRunwayExits(String airport_code, String runway_id) throws RemoteException;
	
	public Object[] getLayout_node_map(String airport_code) throws RemoteException;
	
	public Object[] getLayout_node_data(String airport_code) throws RemoteException;
	
	public Object[] getLayout_links(String airport_code) throws RemoteException;
	
	public String[] getSurface_taxi_plan(String acid, String airport_code) throws RemoteException;
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String runway_name) throws RemoteException;
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String v2Node_waypoint_id, String touchdownNode_waypoint_id) throws RemoteException;
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, double[] v2Point_lat_lon, double[] touchdownPoint_lat_lon) throws RemoteException;

	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids) throws RemoteException;
	
	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids, double[] v2_or_touchdown_point_lat_lon) throws RemoteException;
	
	public String[] get_taxi_route_from_A_To_B(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id) throws RemoteException;
	
	public String getDepartureRunway(String acid) throws RemoteException;
	
	public String getArrivalRunway(String acid) throws RemoteException;
	
	public double getTaxi_tas_knots(String acid) throws RemoteException;
	
	public void setTaxi_tas_knots(String acid, double tas_knots) throws RemoteException;
	
	public String[] getAllAirportCodesInGNATS() throws RemoteException;
	
	public String[] getRunwayEnds(String airportId, String runwayId) throws RemoteException;
}

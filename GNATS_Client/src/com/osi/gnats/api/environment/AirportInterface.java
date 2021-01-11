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

package com.osi.gnats.api.environment;

import com.osi.gnats.airport.Airport;
import com.osi.gnats.api.BaseInterface;

public interface AirportInterface extends BaseInterface {

	/**
	 * Get an Airport object instance by a given airport code.
	 * @param airport_code
	 * @return
	 */
	public Airport select_airport(String airport_code);
	
	/**
	 * Get arrival airport of the requested aircraft.
	 * @param acid
	 * @return Airport code
	 */
	public String getArrivalAirport(String acid);
	
	/**
	 * Get departure airport of the requested aircraft.
	 * @param acid
	 * @return Airport code
	 */
	public String getDepartureAirport(String acid);
	
	/**
	 * Get latitude and longitude of the requested airport.
	 * @param airport_code
	 * @return 
	 */
	public double[] getLocation(String airport_code);
	
	/**
	 * Get airport closet to the given location.
	 * @param latitude_deg
	 * @param longitude_deg
	 * @return Airport code
	 */
	public String getClosestAirport(double latitude_deg, double longitude_deg);
	
	/**
	 * Get all airports within the given location and mile range.
	 * @param latitude_deg
	 * @param longitude_deg
	 * @param miles
	 * @return
	 */
	public String[] getAirportsWithinMiles(double latitude_deg, double longitude_deg, double miles);
	
	/**
	 * Get full airport name of the given airport code.
	 * @param airport_code
	 * @return
	 */
	public String getFullName(String airport_code);
	
	/**
	 * Get all the runways of the given airport.
	 * @param airport_code
	 * @return
	 */
	public Object[] getAllRunways(String airport_code);
	
	/**
	 * Get all runway exits of the given airport code and runway id.
	 * @param airport_code
	 * @param runway_id
	 * @return
	 */
	public String[] getRunwayExits(String airport_code, String runway_id);
	
	/**
	 * Get mapping of node and sequence number of a given airport.
	 * 
	 * The returned data is an array.  Each array element is an array of "Waypoint node Id" and "Node sequence number".
	 * @param airport_code Airport code
	 * @return
	 */
	public Object[] getLayout_node_map(String airport_code);
	
	/**
	 * Get waypoint node data of a given airport.
	 * 
	 * The returned data is an array.  Each array element is an array of "Node sequence number", "Latitude" and "Longitude".
	 * @param airport_code Airport code
	 * @return
	 */
	public Object[] getLayout_node_data(String airport_code);
	
	/**
	 * Get links of waypoint nodes of a given airport.
	 * 
	 * The returned data is an array.  Each array element is an array of "Node 1 sequence number", "Node 2 sequence number".
	 * @param airport_code Airport code
	 * @return
	 */
	public Object[] getLayout_links(String airport_code);
	
	/**
	 * Get surface taxi plan of the given aircraft Id and airport code.
	 * @param acid
	 * @param airport_code
	 * @return Array of all waypoint Ids in the order of visiting.
	 */
	public String[] getSurface_taxi_plan(String acid, String airport_code);
	
	/**
	 * Generate taxi plan and load it in the NATS engine.
	 * @param acid Aircraft Id
	 * @param airport_code Airport code
	 * @param startNode_waypoint_id Starting waypoint Id
	 * @param endNode_waypoint_id Ending waypoint Id
	 * @param runway_name Name of runway
	 * @return Value indicating if the process runs successfully.  0 means success.  1 means failure.
	 */
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String runway_name);
	
	/**
	 * Generate taxi plan and load it in the NATS engine.
	 * @param acid Aircraft Id
	 * @param airport_code Airport code
	 * @param startNode_waypoint_id Starting waypoint Id
	 * @param endNode_waypoint_id Ending waypoint Id
	 * @param v2Node_waypoint_id V2 waypoint Id(if departing)
	 * @param touchdownNode_waypoint_id Touchdown waypoint Id(if landing)
	 * @return Value indicating if the process runs successfully.  0 means success.  1 means failure.
	 */
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String v2Node_waypoint_id, String touchdownNode_waypoint_id);
	
	/**
	 * Generate taxi plan and load it in the NATS engine.
	 * @param acid Aircraft Id
	 * @param airport_code Airport code
	 * @param startNode_waypoint_id Starting waypoint Id
	 * @param endNode_waypoint_id Ending waypoint Id
	 * @param v2Point_lat_lon V2 point latitude and longitude(if departing)
	 * @param touchdownPoint_lat_lon Touchdown point latitude and longitude(if landing)
	 * @return Value indicating if the process runs successfully.  0 means success.  1 means failure.
	 */
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, double[] v2Point_lat_lon, double[] touchdownPoint_lat_lon);
	
	/**
	 * Set user-defined surface taxi plan and load it in the NATS engine.
	 * @param acid
	 * @param airport_code
	 * @param user_defined_waypoint_ids
	 * @return Response value.  0 means success.  1 means error.
	 */
	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids);
	
	/**
	 * Set user-defined surface taxi plan and load it in the NATS engine.
	 * @param acid
	 * @param airport_code
	 * @param user_defined_waypoint_ids
	 * @param v2_or_touchdown_point_lat_lon
	 * @return
	 */
	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids, double[] v2_or_touchdown_point_lat_lon);
	
	/**
	 * Generate taxi route from waypoint A to B.
	 * @param acid
	 * @param airport_code
	 * @param startNode_waypoint_id
	 * @param endNode_waypoint_id
	 * @return
	 */
	public String[] get_taxi_route_from_A_To_B(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id);
	
	/**
	 * Get departure runway of the given aircraft.
	 * If the departure taxi plan does not exist, no result will be returned.
	 * @param acid
	 * @return
	 */
	public String getDepartureRunway(String acid);
	
	/**
	 * Get arrival runway of the given aircraft.
	 * If the arrival taxi plan does not exist, no result will be returned.
	 * @param acid
	 * @return
	 */
	public String getArrivalRunway(String acid);
	
	/**
	 * Get surface taxi speed in tas knots of the given aircraft.
	 * @param acid
	 * @return
	 */
	public double getTaxi_tas_knots(String acid);
	
	/**
	 * Set surface taxi speed in tas knots of the given aircraft.
	 * @param acid Aircraft Id
	 * @param tas_knots Speed.
	 */
	public void setTaxi_tas_knots(String acid, double tas_knots);
}

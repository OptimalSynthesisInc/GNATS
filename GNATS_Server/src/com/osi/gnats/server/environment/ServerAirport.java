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

package com.osi.gnats.server.environment;

import java.rmi.RemoteException;

import com.osi.gnats.airport.Airport;
import com.osi.gnats.rmi.environment.RemoteAirport;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import java.util.ArrayList;
import java.util.List;
import java.io.File;

public class ServerAirport extends ServerClass implements RemoteAirport {
	
	public ServerAirport(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	public Airport select_airport(String airport_code) {
		Airport retAirport = null;
		
		if(ServerNATS.cifpExists) {	
			if ((airport_code != null) && (!"".equals(airport_code))) {
				retAirport = cEngine.select_airport(airport_code);
			}
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retAirport;
	}
	
	public String getArrivalAirport(String acid) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getArrivalAirport(acid);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getDepartureAirport(String acid) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getDepartureAirport(acid);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double[] getLocation(String airport_code) {
		double[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getLocation(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getClosestAirport(double latitude_deg, double longitude_deg) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getClosestAirport(latitude_deg, longitude_deg);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAirportsWithinMiles(double latitude_deg, double longitude_deg, double miles) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getAirportsWithinMiles(latitude_deg, longitude_deg, miles);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}

	public String getFullName(String airport_code) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getFullName(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public Object[] getAllRunways(String airport_code) {
		Object[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getAllRunways(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAllGates(String airport_code) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			Airport airport = cEngine.select_airport(airport_code);
			List<String> gateList = new ArrayList<String>();
			if(airport != null) {
				Object[] airportLayout = cEngine.getLayout_node_map(airport_code);
				for(int i = 0; i < airportLayout.length; i++) {
					String node = (((Object[])airportLayout[i])[0]).toString();
					if(node.toLowerCase().startsWith("gate") || node.toLowerCase().startsWith("spot") || node.toLowerCase().startsWith("park"))
						gateList.add(node);
				}
			}
			retValue = gateList.toArray(new String[gateList.size()]);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getRunwayExits(String airport_code, String runway_id) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getRunwayExits(airport_code, runway_id);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public Object[] getLayout_node_map(String airport_code) {
		Object[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getLayout_node_map(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public Object[] getLayout_node_data(String airport_code) {
		Object[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getLayout_node_data(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public Object[] getLayout_links(String airport_code) {
		Object[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getLayout_links(airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getSurface_taxi_plan(String acid, String airport_code) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getSurface_taxi_plan(acid, airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String runway_name) {
		int retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id, endNode_waypoint_id, runway_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, String v2Node_waypoint_id, String touchdownNode_waypoint_id) {
		int retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id, endNode_waypoint_id, v2Node_waypoint_id, touchdownNode_waypoint_id);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id, double[] v2Point_lat_lon, double[] touchdownPoint_lat_lon) {
		int retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id, endNode_waypoint_id, v2Point_lat_lon, touchdownPoint_lat_lon);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids) {
		int retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.setUser_defined_surface_taxi_plan(acid, airport_code, user_defined_waypoint_ids);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids, double[] v2_or_touchdown_point_lat_lon) {
		int retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.setUser_defined_surface_taxi_plan(acid, airport_code, user_defined_waypoint_ids, v2_or_touchdown_point_lat_lon);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] get_taxi_route_from_A_To_B(String acid, String airport_code, String startNode_waypoint_id, String endNode_waypoint_id) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.get_taxi_route_from_A_To_B(acid, airport_code, startNode_waypoint_id, endNode_waypoint_id);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getDepartureRunway(String acid) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getDepartureRunway(acid);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getArrivalRunway(String acid) {
		String retValue = "";
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getArrivalRunway(acid);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double getTaxi_tas_knots(String acid) {
		double retValue = -1;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getTaxi_tas_knots(acid);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public void setTaxi_tas_knots(String acid, double tas_knots) {
		
		if(ServerNATS.cifpExists) {
			cEngine.setTaxi_tas_knots(acid, tas_knots);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
	}
	
	public String[] getAllAirportCodesInGNATS() {

		String[] retValue = {"KABQ", "KATL", "KBDL", "KBHM", "KBNA", "KBOI", "KBOS", "KBTV", "KBUR", "KBWI", "KBZN", "KCHS", "KCLE", "KCLT", "KCRW", "KCVG", "KDAL", "KDCA", "KDEN", "KDFW", "KDSM", "KDTW", "KEWR", "KFAR", "KFLL", "KFSD", "KGYY", "KHPN", "KIAD", "KIAH", "KICT", "KILG", "KIND", "KISP", "KJAC", "KJAN", "KJAX", "KJFK", "KLAS", "KLAX", "KLEX", "KLGA", "KLGB", "KLIT", "KMCO", "KMDW", "KMEM", "KMHT", "KMIA", "KMKE", "KMSP", "KMSY", "KOAK", "KOKC", "KOMA", "KONT", "KORD", "KPBI", "KPDX", "KPHL", "KPHX", "KPIT", "KPVD", "KPWM", "KSAN", "KSAT", "KSDF", "KSEA", "KSFO", "KSJC", "KSLC", "KSNA", "KSTL", "KSWF", "KTEB", "KTPA", "KVGT", "PANC", "PHNL"};

		return retValue;
	}
	
	public String[] getRunwayEnds(String airportId, String runwayId) {
		String[] retValue = null;
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.getRunwayEnds(airportId, runwayId);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
}

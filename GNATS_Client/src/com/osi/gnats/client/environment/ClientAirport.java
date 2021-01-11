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

package com.osi.gnats.client.environment;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.airport.Airport;
import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.environment.AirportInterface;
import com.osi.gnats.client.BaseClass;
import com.osi.gnats.rmi.environment.RemoteAirport;

public class ClientAirport extends BaseClass implements AirportInterface {
	private RemoteAirport remoteAirport;

	public ClientAirport(NATSInterface natsInterface, Remote rs) throws RemoteException {
		super(natsInterface, "Airport", "Airport Functions", "Airport Functions");
		this.remoteAirport = (RemoteAirport) rs;
	}

	public ClientAirport(NATSInterface natsInterface, RemoteAirport rs) throws RemoteException {
		super(natsInterface, "Airport", "Airport Functions", "Airport Functions");
		this.remoteAirport = (RemoteAirport) rs;
	}

	public Airport select_airport(String airport_code) {
		Airport retAirport = null;

		try {
			retAirport = remoteAirport.select_airport(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retAirport;
	}

	public String getArrivalAirport(String acid) {
		String retObject = null;

		try {
			retObject = remoteAirport.getArrivalAirport(acid);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String getDepartureAirport(String acid) {
		String retObject = null;

		try {
			retObject = remoteAirport.getDepartureAirport(acid);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public double[] getLocation(String airport_code) {
		double[] retObject = null;

		try {
			retObject = remoteAirport.getLocation(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String getClosestAirport(double latitude_deg, double longitude_deg) {
		String retObject = null;

		try {
			retObject = remoteAirport.getClosestAirport(latitude_deg, longitude_deg);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String[] getAirportsWithinMiles(double latitude_deg, double longitude_deg, double miles) {
		String[] retObject = null;

		try {
			retObject = remoteAirport.getAirportsWithinMiles(latitude_deg, longitude_deg, miles);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String getFullName(String airport_code) {
		String retObject = null;

		try {
			retObject = remoteAirport.getFullName(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public Object[] getAllRunways(String airport_code) {
		Object[] retObject = null;

		try {
			retObject = remoteAirport.getAllRunways(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String[] getAllGates(String airport_code) {
		String[] retObject = null;

		try {
			retObject = remoteAirport.getAllGates(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String[] getRunwayExits(String airport_code, String runway_id) {
		String[] retObject = null;

		try {
			retObject = remoteAirport.getRunwayExits(airport_code, runway_id);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public Object[] getLayout_node_map(String airport_code) {
		Object[] retObject = null;

		try {
			retObject = remoteAirport.getLayout_node_map(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public Object[] getLayout_node_data(String airport_code) {
		Object[] retObject = null;

		try {
			retObject = remoteAirport.getLayout_node_data(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public Object[] getLayout_links(String airport_code) {
		Object[] retObject = null;

		try {
			retObject = remoteAirport.getLayout_links(airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String[] getSurface_taxi_plan(String acid, String airport_code) {
		String[] retObject = null;

		try {
			retObject = remoteAirport.getSurface_taxi_plan(acid, airport_code);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id,
			String endNode_waypoint_id, String runway_name) {
		int retValue = 1;

		try {
			if ((runway_name != null) && (!runway_name.equals(""))) {
				retValue = remoteAirport.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id,
						endNode_waypoint_id, runway_name);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id,
			String endNode_waypoint_id, String vrNode_waypoint_id, String touchdownNode_waypoint_id) {
		int retValue = 1;

		try {
			if (vrNode_waypoint_id == null)
				vrNode_waypoint_id = "";
			if (touchdownNode_waypoint_id == null)
				touchdownNode_waypoint_id = "";

			if (((!vrNode_waypoint_id.equals("")) && (touchdownNode_waypoint_id.equals("")))
					|| ((vrNode_waypoint_id.equals("")) && (!touchdownNode_waypoint_id.equals("")))) {
				retValue = remoteAirport.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id,
						endNode_waypoint_id, vrNode_waypoint_id, touchdownNode_waypoint_id);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public int generate_surface_taxi_plan(String acid, String airport_code, String startNode_waypoint_id,
			String endNode_waypoint_id, double[] v2Point_lat_lon, double[] touchdownPoint_lat_lon) {
		int retValue = 1;

		try {
			if (((v2Point_lat_lon != null) && (touchdownPoint_lat_lon == null))
					|| ((v2Point_lat_lon == null) && (touchdownPoint_lat_lon != null))) {
				retValue = remoteAirport.generate_surface_taxi_plan(acid, airport_code, startNode_waypoint_id,
						endNode_waypoint_id, v2Point_lat_lon, touchdownPoint_lat_lon);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids) {
		int retValue = 1;

		try {
			retValue = remoteAirport.setUser_defined_surface_taxi_plan(acid, airport_code, user_defined_waypoint_ids);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public int setUser_defined_surface_taxi_plan(String acid, String airport_code, String[] user_defined_waypoint_ids,
			double[] v2_or_touchdown_point_lat_lon) {
		int retValue = 1;

		try {
			if (v2_or_touchdown_point_lat_lon != null) {
				retValue = remoteAirport.setUser_defined_surface_taxi_plan(acid, airport_code,
						user_defined_waypoint_ids, v2_or_touchdown_point_lat_lon);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public String[] get_taxi_route_from_A_To_B(String acid, String airport_code, String startNode_waypoint_id,
			String endNode_waypoint_id) {
		String[] retObject = null;

		try {
			retObject = remoteAirport.get_taxi_route_from_A_To_B(acid, airport_code, startNode_waypoint_id,
					endNode_waypoint_id);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String getDepartureRunway(String acid) {
		String retObject = null;

		try {
			retObject = remoteAirport.getDepartureRunway(acid);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public String getArrivalRunway(String acid) {
		String retObject = null;

		try {
			retObject = remoteAirport.getArrivalRunway(acid);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObject;
	}

	public double getTaxi_tas_knots(String acid) {
		double retValue = 1;

		try {
			retValue = remoteAirport.getTaxi_tas_knots(acid);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public void setTaxi_tas_knots(String acid, double tas_knots) {
		try {
			remoteAirport.setTaxi_tas_knots(acid, tas_knots);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public String[] getAllAirportCodesInGNATS() {
		String[] airportList = null;
		try {
			airportList = remoteAirport.getAllAirportCodesInGNATS();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return airportList;
	}
	
	public String[] getRunwayEnds(String airportId, String runwayId) {
		String[] runwayEnds = null;
		try {
			runwayEnds = remoteAirport.getRunwayEnds(airportId, runwayId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		return runwayEnds;
	}
}

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

package com.osi.gnats.client;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.SimulationInterface;
import com.osi.gnats.rmi.RemoteSimulation;
import com.osi.util.Constants;

public class ClientSimulation extends BaseClass implements SimulationInterface {
	private RemoteSimulation remoteSimulation;

	private float t_total_propagation_period;
	private float t_step_surface;
	private float t_step_terminal;
	private float t_step_airborne;

	public ClientSimulation(NATSInterface natsInterface, Remote rs) throws RemoteException {
		super(natsInterface, "Sim", "Simulation Functions", "Simulation Functions");
		this.remoteSimulation = (RemoteSimulation)rs;
	}
	
	public ClientSimulation(NATSInterface natsInterface, RemoteSimulation rs) throws RemoteException {
		super(natsInterface, "Sim", "Simulation Functions", "Simulation Functions");
		this.remoteSimulation = (RemoteSimulation)rs;
	}
	
	public long get_sim_id() throws RemoteException {
		return remoteSimulation.get_sim_id();
	}
	
	public int setupSimulation(int t_total_propagation_period, int t_step) {
		this.t_total_propagation_period = t_total_propagation_period;
		this.t_step_surface = t_step;
		this.t_step_terminal = t_step;
		this.t_step_airborne = t_step;
		
		return setupSimulation((float)t_total_propagation_period, (float)t_step);
	}
	
	public int setupSimulation(float t_total_propagation_period, float t_step) {
		int retValue = -1;

		try {
			this.t_total_propagation_period = t_total_propagation_period;
			this.t_step_surface = t_step;
			this.t_step_terminal = t_step;
			this.t_step_airborne = t_step;
			
			retValue = remoteSimulation.setupSimulation(sessionId, t_total_propagation_period, t_step);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setupSimulation(int t_total_propagation_period, int t_step_surface, int t_step_terminal, int t_step_airborne) {
		this.t_total_propagation_period = t_total_propagation_period;
		this.t_step_surface = t_step_surface;
		this.t_step_terminal = t_step_terminal;
		this.t_step_airborne = t_step_airborne;
		
		return setupSimulation((float)t_total_propagation_period, (float)t_step_surface, (float)t_step_terminal, (float)t_step_airborne);
	}
	
	public int setupSimulation(float t_total_propagation_period, float t_step_surface, float t_step_terminal, float t_step_airborne) {
		int retValue = -1;
		
		try {
			this.t_total_propagation_period = t_total_propagation_period;
			this.t_step_surface = t_step_surface;
			this.t_step_terminal = t_step_terminal;
			this.t_step_airborne = t_step_airborne;
			
			retValue = remoteSimulation.setupSimulation(sessionId, t_total_propagation_period, t_step_surface, t_step_terminal, t_step_airborne);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	
	public int get_runtime_sim_status() {
		int retValue = -1;
		
		try {
			retValue = remoteSimulation.get_runtime_sim_status();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public float get_curr_sim_time() {
		float retValue = -1;
		
		try {
			retValue = remoteSimulation.get_curr_sim_time();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public long get_curr_utc_time() throws RemoteException {
		return remoteSimulation.get_curr_utc_time();
	}
	
	public long get_nextPropagation_utc_time() throws RemoteException {
		return remoteSimulation.get_nextPropagation_utc_time();
	}
	
	public int get_realTime_simulation_time_step() throws RemoteException {
		return remoteSimulation.get_realTime_simulation_time_step();
	}
	
	public void start() {
		try {
			remoteSimulation.start();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void start(long t_duration) {
		try {
			remoteSimulation.start(t_duration);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void start(float t_duration) {
		try {
			remoteSimulation.start(t_duration);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}

	public void startRealTime() {
		try {
			remoteSimulation.startRealTime();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void pause() {
		try {
			remoteSimulation.pause();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void resume() {
		try {
			remoteSimulation.resume();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void resume(long t_duration) {
		try {
			remoteSimulation.resume(t_duration);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void resume(float t_duration) {
		try {
			remoteSimulation.resume(t_duration);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void stop() {
		try {
			remoteSimulation.stop();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void write_trajectories(String output_file) {
		try {
			remoteSimulation.write_trajectories(output_file);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}

	public void clear_trajectory() {
		try {
			remoteSimulation.clear_trajectory();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void request_aircraft(String ac_id) throws RemoteException {
		remoteSimulation.request_aircraft(sessionId, ac_id);
	}

	public void request_groundVehicle(String gv_id) throws RemoteException {
		remoteSimulation.request_groundVehicle(sessionId, gv_id);
	}
	
	public void externalAircraft_create_trajectory_profile(String ac_id,
			String ac_type,
			String origin_airport,
			String destination_airport,
			float cruise_altitude_ft,
			float cruise_tas_knots,
			double latitude_deg,
			double longitude_deg,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course_deg,
			String flight_phase) throws RemoteException {
		try {
			remoteSimulation.externalAircraft_create_trajectory_profile(sessionId, ac_id, ac_type, origin_airport, destination_airport, cruise_altitude_ft, cruise_tas_knots, latitude_deg, longitude_deg, altitude_ft, rocd_fps, tas_knots, course_deg, flight_phase);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}

	public void externalAircraft_inject_trajectory_state_data(String ac_id,
			double latitude_deg,
			double longitude_deg,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course_deg,
			String flight_phase,
			long timestamp_utc_millisec) throws RemoteException {
		try {
			remoteSimulation.externalAircraft_inject_trajectory_state_data(sessionId, ac_id, latitude_deg, longitude_deg, altitude_ft, rocd_fps, tas_knots, course_deg, flight_phase, timestamp_utc_millisec);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void requestDownloadTrajectoryFile() throws RemoteException, Exception {
		if (getNATSClient().isStandaloneMode()) {
			throw new Exception(Constants.MSG_FUNC_INVALID_STANDALONE_MODE);
		}
		
		System.out.println("Downloading trajectory file...");

		remoteSimulation.requestDownloadTrajectoryFile(sessionId);
	}
	
}

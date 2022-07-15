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

package com.osi.gnats.server;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.net.Socket;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Date;

import com.osi.gnats.rmi.RemoteSimulation;
import com.osi.gnats.rmi.ServerSession;
import com.osi.gnats.server.equipment.ServerAircraft;
import com.osi.gnats.user.UserManager;
import com.osi.gnats.user.UserPermission;

/**
 * Actual RMI class which implements RemoteSimulation
 */
public class ServerSimulation extends ServerClass implements RemoteSimulation {
	
	private long latestSimId = 0;
	private ArrayList<Pair_String_String> list_trajectoryFiles = new ArrayList<Pair_String_String>();
	
	/**
	 * Constructor
	 * @param serverNATS
	 * @throws RemoteException
	 */
	public ServerSimulation(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	/**
	 * Set simulation ID
	 */
	public long get_sim_id() throws RemoteException {
		return cEngine.get_sim_id();
	}
	
	/**
	 * Setup simulation
	 */
	public int setupSimulation(int sessionId, float t_total_propagation_period, float t_step) throws RemoteException {
		return cEngine.propagate_flights(t_total_propagation_period, t_step);
	}
	
	/**
	 * Setup simulation
	 */
	public int setupSimulation(int sessionId, float t_total_propagation_period, float t_step_surface, float t_step_terminal, float t_step_airborne) throws RemoteException {
		return cEngine.propagate_flights(t_total_propagation_period, t_step_surface, t_step_terminal, t_step_airborne);
	}
	
	/**
	 * Get simulation status
	 */
	public int get_runtime_sim_status() {
		return cEngine.get_runtime_sim_status();
	}
	
	/**
	 * Get current simulation timestamp
	 */
	public float get_curr_sim_time() {
		return cEngine.get_curr_sim_time();
	}
	
	/**
	 * Get current UTC on NATS Server
	 */
	public long get_curr_utc_time() throws RemoteException {
		return new Date().getTime();
	}

	/**
	 * Get the UTC of the next propagation
	 */
	public long get_nextPropagation_utc_time() throws RemoteException {
		return cEngine.get_nextPropagation_utc_time();
	}
	
	/**
	 * Get simulation time step of the real-time simulation
	 */
	public int get_realTime_simulation_time_step() throws RemoteException {
		return cEngine.get_realTime_simulation_time_step();
	}
	
	/**
	 * Start simulation
	 */
	public void start() throws RemoteException {
		cEngine.sim_start();
		
		latestSimId = cEngine.get_sim_id();
	}
	
	/**
	 * Start simulation for certain duration and pause
	 */
	public void start(long t_duration) throws RemoteException {
		cEngine.sim_start(t_duration);

		latestSimId = cEngine.get_sim_id();
	}
	
	/**
	 * Start simulation for certain duration and pause
	 */
	public void start(float t_duration) throws RemoteException {
		cEngine.sim_start(t_duration);

		latestSimId = cEngine.get_sim_id();
	}
	
	/**
	 * Start real-time simulation
	 */
	public void startRealTime() throws RemoteException {
		cEngine.sim_startRealTime();
		
		latestSimId = cEngine.get_sim_id();
	}
	
	/**
	 * Pause the simulation
	 */
	public void pause() throws RemoteException {
		cEngine.sim_pause();
	}
	
	/**
	 * Resume the simulation from the pause phase
	 */
	public void resume() throws RemoteException {
		cEngine.sim_resume();
	}
	
	/**
	 * Resume the simulation from the pause phase for certain period of time and pause again
	 */
	public void resume(long t_duration) throws RemoteException {
		cEngine.sim_resume(t_duration);
	}
	
	/**
	 * Resume the simulation from the pause phase for certain period of time and pause again
	 */
	public void resume(float t_duration) throws RemoteException {
		cEngine.sim_resume(t_duration);
	}
	
	/**
	 * Stop the simulation
	 */
	public void stop() throws RemoteException {
		cEngine.sim_stop();
	}
	
	/**
	 * Write trajectory to a file
	 */
	public void write_trajectories(String output_file) throws RemoteException {
		cEngine.write_trajectories(output_file);
		
		Pair_String_String newPair = new Pair_String_String();
		newPair.setFirst("" + latestSimId);
		newPair.setSecond(output_file);
		
		list_trajectoryFiles.add(newPair);
	}
	
	/**
	 * Clean up trajectory data
	 */
	public void clear_trajectory() throws RemoteException {
		System.gc();
		
		cEngine.clear_trajectories();
	}
	
	/**
	 * Request the ownership of a aircraft
	 */
	public void request_aircraft(int sessionId, String ac_id) throws RemoteException {
		String tmpAuthid = UserManager.getAuthidFromSession(Integer.valueOf(sessionId));
		if (tmpAuthid != null) {
			cEngine.request_aircraft(tmpAuthid, ac_id);
		}
	}
	
	/**
	 * Request the ownership of a ground vehicle
	 */
	public void request_groundVehicle(int sessionId, String gv_id) throws RemoteException {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {
			String tmpAuthid = UserManager.getAuthidFromSession(Integer.valueOf(sessionId));
			if (tmpAuthid != null) {
				cEngine.request_groundVehicle(tmpAuthid, gv_id);
			}
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
	}

	/**
	 * Create profile data of an external aircraft
	 */
	public void externalAircraft_create_trajectory_profile(int sessionId,
			String ac_id,
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
		String auth_id = UserManager.getAuthidFromSession(sessionId);

		cEngine.externalAircraft_create_trajectory_profile(auth_id, ac_id, ac_type, origin_airport, destination_airport, cruise_altitude_ft, cruise_tas_knots, latitude_deg, longitude_deg, altitude_ft, rocd_fps, tas_knots, course_deg, flight_phase);
	}

	/**
	 * Send state data of an external aircraft to NATS Server
	 */
	public int externalAircraft_inject_trajectory_state_data(int sessionId,
			String ac_id,
			double latitude_deg,
			double longitude_deg,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course_deg,
			String flight_phase,
			long timestamp_utc_millisec) throws RemoteException {
		String auth_id = UserManager.getAuthidFromSession(sessionId);
		
		// Check aircraft ownership
		// If this session user does not own the aircraft, a RemoteException will be thrown
		ServerAircraft.isAircraftAssignee(auth_id, ac_id);

		return cEngine.externalAircraft_inject_trajectory_state_data(ac_id, latitude_deg, longitude_deg, altitude_ft, rocd_fps, tas_knots, course_deg, flight_phase, timestamp_utc_millisec);
	}
	
	/**
	 * Request to download trajectory file
	 */
	public void requestDownloadTrajectoryFile(int sessionId) throws RemoteException {
		Pair_String_String latestPair = null;
		if (list_trajectoryFiles.size() > 0) {
			latestPair = list_trajectoryFiles.get(list_trajectoryFiles.size()-1);
			
			if (latestPair != null) {
				String latestACTrajectoryFilePathName = latestPair.getSecond();
				File fileLatestACTrajectory = new File(latestACTrajectoryFilePathName);
				if (!fileLatestACTrajectory.exists()) {
					throw new RemoteException("Trajectory file " + latestACTrajectoryFilePathName + "does not exist on NATS Server");
				}

				ServerSession tmpServerSession = null;
				tmpServerSession = UserManager.getServerSession(Integer.valueOf(sessionId));
				try {
					if (tmpServerSession != null) {
						Socket tmpSocket = tmpServerSession.getSocket();

						if ((tmpSocket != null) && (!tmpSocket.isClosed())) {
							BufferedOutputStream bos = new BufferedOutputStream(tmpSocket.getOutputStream());

							DataOutputStream dos = new DataOutputStream(bos);

							String jsonStr = "{socketBusy:\"true\"}";
					
							dos.writeUTF(jsonStr);
					        dos.flush();
							
							FileInputStream fileIStream = new FileInputStream(fileLatestACTrajectory);
							BufferedInputStream bis = new BufferedInputStream(fileIStream);

							jsonStr = "{" + "fileTransfer:\"" + latestACTrajectoryFilePathName + "\", fileLength:" + fileLatestACTrajectory.length() + "}";
							
							dos.writeUTF(jsonStr);
					        dos.flush();

					        int theByte = 0;
					        while ((theByte = bis.read()) != -1) bos.write(theByte);

					        bos.flush();

							// Handle Ground Vehicle trajectory file
							String latestGVTrajectoryFilePathName = latestACTrajectoryFilePathName.substring(0, latestACTrajectoryFilePathName.lastIndexOf(".")) + "_groundVehicleSimulation.csv";

							File fileLatestGVTrajectory = new File(latestGVTrajectoryFilePathName);
							if (fileLatestGVTrajectory.exists()) {
								jsonStr = "{" + "fileTransfer:\"" + latestGVTrajectoryFilePathName + "\", fileLength:" + fileLatestGVTrajectory.length() + "}";

						        dos.writeUTF(jsonStr);
						        dos.flush();
						        
								fileIStream = new FileInputStream(fileLatestGVTrajectory);
								bis = new BufferedInputStream(fileIStream);
						        
						        while ((theByte = bis.read()) != -1) bos.write(theByte);
						        
						        bos.flush();
							}

							jsonStr = "{socketBusy:\"false\"}";
							
							dos.writeUTF(jsonStr);
					        dos.flush();
						}
					}
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		}
	}
	
	class Pair_String_String {
		String first;
		String second;
		
		public String getFirst() {
			return first;
		}
		public void setFirst(String first) {
			this.first = first;
		}
		
		public String getSecond() {
			return second;
		}
		public void setSecond(String second) {
			this.second = second;
		}
	}
}

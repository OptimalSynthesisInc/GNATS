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

package com.osi.gnats.api;

import java.rmi.RemoteException;

public interface SimulationInterface extends BaseInterface {
	
	/**
	 * Get ID of the current simulation
	 * 
	 * @return
	 * @throws RemoteException
	 */
	public long get_sim_id() throws RemoteException;
	
	/**
	 * Setup the trajectory propagation.
	 * 
	 * @param t_total_propagation_period Total period of time of propagation in seconds.
	 * @param t_step Time step of surface traffic in seconds.
	 * @return
	 */
	public int setupSimulation(int t_total_propagation_period, int t_step);
	
	/**
	 * Setup the trajectory propagation.
	 * 
	 * @param t_total_propagation_period Total period of time of propagation in seconds.
	 * @param t_step Time step of surface traffic in seconds.
	 * @return
	 */
	public int setupSimulation(float t_total_propagation_period, float t_step);
	
	/**
	 * Setup the trajectory propagation.
	 * 
	 * @param t_total_propagation_period Total period of time of propagation in seconds.
	 * @param t_step_surface Time step of surface traffic in seconds.
	 * @param t_step_terminal Time step of terminal area traffic in seconds.
	 * @param t_step_airborne Time step of traffic(above TRACON) in seconds.
	 * @return
	 */
	public int setupSimulation(int t_total_propagation_period, int t_step_surface, int t_step_terminal, int t_step_airborne);
	
	/**
	 * Setup the trajectory propagation.
	 * 
	 * @param t_total_propagation_period Total period of time of propagation in seconds.
	 * @param t_step_surface Time step of surface traffic in seconds.
	 * @param t_step_terminal Time step of terminal area traffic in seconds.
	 * @param t_step_airborne Time step of traffic(above TRACON) in seconds.
	 * @return
	 */
	public int setupSimulation(float t_total_propagation_period, float t_step_surface, float t_step_terminal, float t_step_airborne);
		
	/**
	 * Get the runtime status of the propagation simulation
	 * 
	 * @return The status code
	 */
	public int get_runtime_sim_status();
	
	/**
	 * Get the current simulation timestamp.
	 * @return
	 */
	public float get_curr_sim_time();
	
	/**
	 * Get the current NATS Server UTC system time in milli seconds
	 * @return
	 * @throws RemoteException
	 */
	public long get_curr_utc_time() throws RemoteException;
	
	/**
	 * Get the UTC in milli seconds of the next trajectory propagation
	 * @return
	 * @throws RemoteException
	 */
	public long get_nextPropagation_utc_time() throws RemoteException;
	
	/**
	 * Get time step in seconds of the real-time simulation
	 * 
	 * @return Time step in seconds
	 * @throws RemoteException
	 */
	public int get_realTime_simulation_time_step() throws RemoteException;
	
	/**
	 * Start the trajectory propagation process
	 */
	public void start();
	
	/**
	 * Start the trajectory propagation and process data for certain seconds of duration time.  The trajectory propagation process executes for certain seconds then the server stays in waiting mode for the further commands from the client.
	 * Notice. Please be sure to issue resume() or stop() function after running this function.  Otherwise, the propagation process will not end by itself.
	 * @param t_duration Duration of seconds to process the propagation.
	 */
	public void start(long t_duration);
	
	/**
	 * Start the trajectory propagation and process data for certain seconds of duration time.  The trajectory propagation process executes for certain seconds then the server stays in waiting mode for the further commands from the client.
	 * Notice. Please be sure to issue resume() or stop() function after running this function.  Otherwise, the propagation process will not end by itself.
	 * @param t_duration Duration of seconds to process the propagation.
	 */
	public void start(float t_duration);
	
	/**
	 * Start the trajectory propagation in real-time mode
	 */
	public void startRealTime();
	
	/**
	 * Pause the trajectory propagation process.
	 */
	public void pause();
	
	/**
	 * Resume the trajectory propagation process.
	 */
	public void resume();
	
	/**
	 * Resume the trajectory propagation process and process data for certain seconds of duration time.  The trajectory propagation process executes for certain seconds then the server stays in waiting mode for the further commands from the client. 
	 * Notice. Please be sure to issue resume() or stop() function after running this function.  Otherwise, the propagation process will not end by itself.
	 * @param t_duration Duration of seconds to process the propagation.
	 */
	public void resume(long t_duration);
	
	/**
	 * Resume the trajectory propagation process and process data for certain seconds of duration time.  The trajectory propagation process executes for certain seconds then the server stays in waiting mode for the further commands from the client. 
	 * Notice. Please be sure to issue resume() or stop() function after running this function.  Otherwise, the propagation process will not end by itself.
	 * @param t_duration Duration of seconds to process the propagation.
	 */
	public void resume(float t_duration);
	
	/**
	 * Stop the trajectory propagation process.
	 */
	public void stop();
	
	/**
	 * Write trajectory data into file.  The outputted file will be saved on NATS_Server.
	 * @param output_file The filename and path to be outputted.
	 */
	public void write_trajectories(String output_file);
	
	/**
	 * Clean up trajectory data
	 */
	public void clear_trajectory();
	
	/**
	 * Request aircrafts from NATS Server
	 * 
	 * @param ac_id Aircraft ID
	 * @throws RemoteException
	 */
	public void request_aircraft(String ac_id) throws RemoteException;
	
	/**
	 * Request ground vehicles from NATS Server
	 * 
	 * @param gv_id Ground Vehicle ID
	 * @throws RemoteException
	 */
	public void request_groundVehicle(String gv_id) throws RemoteException;
	
	/**
	 * Create profile data of an external aircraft
	 * 
	 * @param ac_id
	 * @param ac_type
	 * @param origin_airport
	 * @param destination_airport
	 * @param cruise_altitude_ft
	 * @param cruise_tas_knots
	 * @param latitude_deg
	 * @param longitude_deg
	 * @param altitude_ft
	 * @param rocd_fps
	 * @param tas_knots
	 * @param course_deg
	 * @param flight_phase
	 * @throws RemoteException
	 */
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
			String flight_phase) throws RemoteException;
	
	/**
	 * Send state data of the external aircraft to NATS Server
	 * 
	 * @param ac_id
	 * @param latitude_deg
	 * @param longitude_deg
	 * @param altitude_ft
	 * @param rocd_fps
	 * @param tas_knots
	 * @param course_deg
	 * @param flight_phase
	 * @param timestamp_utc_millisec
	 * @throws RemoteException
	 */
	public void externalAircraft_inject_trajectory_state_data(String ac_id,
			double latitude_deg,
			double longitude_deg,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course_deg,
			String flight_phase,
			long timestamp_utc_millisec) throws RemoteException;
	
	/**
	 * Request to download the latest trajectory file from NATS Server
	 * 
	 * Due to possible big number of simultaneous file-downloading requests or huge file size, the file downloading process may not start immediately.
	 * 
	 * @throws RemoteException
	 */
	public void requestDownloadTrajectoryFile() throws RemoteException, Exception;
}

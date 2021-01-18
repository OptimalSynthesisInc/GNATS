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

package com.osi.gnats.rmi;

import java.rmi.RemoteException;

public interface RemoteSimulation extends BaseRemote {
	
	public long get_sim_id() throws RemoteException;
	
	public int setupSimulation(int sessionId, float t_total_propagation_period, float t_step) throws RemoteException;
	
	public int setupSimulation(int sessionId, float t_total_propagation_period, float t_step_surface, float t_step_terminal, float t_step_airborne) throws RemoteException;
		
	public int get_runtime_sim_status() throws RemoteException;
	
	public float get_curr_sim_time() throws RemoteException;
	
	public long get_curr_utc_time() throws RemoteException;
	
	public long get_nextPropagation_utc_time() throws RemoteException;
	
	public int get_realTime_simulation_time_step() throws RemoteException;
	
	public void start() throws RemoteException;
	
	public void start(long t_duration) throws RemoteException;
	
	public void start(float t_duration) throws RemoteException;
	
	public void startRealTime() throws RemoteException;
	
	public void pause() throws RemoteException;
	
	public void resume() throws RemoteException;
	
	public void resume(long t_duration) throws RemoteException;
	
	public void resume(float t_duration) throws RemoteException;
	
	public void stop() throws RemoteException;
	
	public void write_trajectories(String output_file) throws RemoteException;
	
	public void clear_trajectory() throws RemoteException;
	
	public void request_aircraft(int sessionId, String ac_id) throws RemoteException;
	
	public void request_groundVehicle(int sessionId, String gv_id) throws RemoteException;

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
		String flight_phase) throws RemoteException;

	public int externalAircraft_inject_trajectory_state_data(int sessionId,
			String ac_id,
			double latitude_deg,
			double longitude_deg,
			double altitude_ft,
			double rocd_fps,
			double tas_knots,
			double course_deg,
			String flight_phase,
			long timestamp_utc_millisec) throws RemoteException;
	
	public void requestDownloadTrajectoryFile(int sessionId) throws RemoteException;

}

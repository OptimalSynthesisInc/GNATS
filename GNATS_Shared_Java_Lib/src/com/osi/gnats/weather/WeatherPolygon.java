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

package com.osi.gnats.weather;

import java.io.Serializable;

public class WeatherPolygon implements Serializable {
	private static final long serialVersionUID = 4056570006138311320L;
	
	// vertex data
	private double[] x_data;
	private double[] y_data;
	private int num_vertices;
	private boolean ccw_flag;

	// bounding box definition
	private double xmin;
	private double xmax;
	private double ymin;
	private double ymax;

	// centroid
	private double x_centroid;
	private double y_centroid;

	private String poly_type;
	private int start_hr;
	private int end_hr;
	
	// ====================================================
	
	/**
	 * Get longitude values of vertices in the polygon.
	 * @return
	 */
	public double[] getX_data() {
		return x_data;
	}
	public void setX_data(double[] x_data) {
		this.x_data = x_data;
	}
	
	/**
	 * Get latitude values of vertices in the polygon.
	 * @return
	 */
	public double[] getY_data() {
		return y_data;
	}
	public void setY_data(double[] y_data) {
		this.y_data = y_data;
	}
	
	/**
	 * Get number of vertices in the polygon.
	 * @return
	 */
	public int getNum_vertices() {
		return num_vertices;
	}
	public void setNum_vertices(int num_vertices) {
		this.num_vertices = num_vertices;
	}
	
	/**
	 * Get boolean value indicating whether the vertices are created counter-clockwise in the polygon.
	 * @return
	 */
	public boolean getCcw_flag() {
		return ccw_flag;
	}
	public void setCcw_flag(boolean ccw_flag) {
		this.ccw_flag = ccw_flag;
	}
	
	/**
	 * Get minimum longitude value of all vertices in the polygon.
	 * @return
	 */
	public double getXmin() {
		return xmin;
	}
	public void setXmin(double xmin) {
		this.xmin = xmin;
	}
	
	/**
	 * Get maximum longitude value of all vertices in the polygon.
	 * @return
	 */
	public double getXmax() {
		return xmax;
	}
	public void setXmax(double xmax) {
		this.xmax = xmax;
	}
	
	/**
	 * Get minimum latitude value of all vertices in the polygon.
	 * @return
	 */
	public double getYmin() {
		return ymin;
	}
	public void setYmin(double ymin) {
		this.ymin = ymin;
	}
	
	/**
	 * Get maximum latitude value of all vertices in the polygon.
	 * @return
	 */
	public double getYmax() {
		return ymax;
	}
	public void setYmax(double ymax) {
		this.ymax = ymax;
	}
	
	/**
	 * Get longitude value of the centroid point in the polygon.
	 * @return
	 */
	public double getX_centroid() {
		return x_centroid;
	}
	public void setX_centroid(double x_centroid) {
		this.x_centroid = x_centroid;
	}
	
	/**
	 * Get latitude value of the centroid point in the polygon.
	 * @return
	 */
	public double getY_centroid() {
		return y_centroid;
	}
	public void setY_centroid(double y_centroid) {
		this.y_centroid = y_centroid;
	}
	
	/**
	 * Get polygon type.
	 * @return
	 */
	public String getPoly_type() {
		return poly_type;
	}
	public void setPoly_type(String poly_type) {
		this.poly_type = poly_type;
	}
	
	/**
	 * Get starting hour of the polygon.
	 * @return
	 */
	public int getStart_hr() {
		return start_hr;
	}
	public void setStart_hr(int start_hr) {
		this.start_hr = start_hr;
	}
	
	/**
	 * Get ending hour of the polygon.
	 * @return
	 */
	public int getEnd_hr() {
		return end_hr;
	}
	public void setEnd_hr(int end_hr) {
		this.end_hr = end_hr;
	}
}

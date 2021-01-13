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

#ifndef RG_EXEC_H
#define RG_EXEC_H

#include "rg_api.h"

namespace osi {
extern vector<string> rg_vector_rap_files;
extern set<string> rg_airportFilter;

  int load_rg_data_files_NATS(const string rap_file,
		const string cifp_file,
		const string& airportConfigFile,
		const string& airwayConfigFile,
		const string& fixConfigFile);

  int load_rg_weather_files_NATS(
  		const string cifp_file,
  		const string& polygonFile,
  		const string& sigmetFile,
  		const string& pirepFile);

  void release_rg_resources();

  void supplement_airways_by_flightPlans(waypoint_node_t* waypoint_node_ptr);

  int runRgOptimized(const string& origin,
		     const string& destination,
		     const string& procName,
		     const double& cruiseAlt,
		     const double& cruiseSpd,
		     const string& polygonFile,
		     const string& sigmetFile,
		     const string& pirepFile,
		     const string& airportConfigFile,
		     const string& airwayConfigFile,
		     const string& fixConfigFile,
		     const string& procConfigFile, 
		     const bool& rerouteFlag,
		     const bool& windOptimalFlag,
		     const bool& filterAirports,
		     const int& numOpts,
		     const double& costIndex,
		     ResultTrajs& t_results,
		     vector<string>& routes,
		     vector<double>& costs,
		     vector< vector<double> >& latitudes,
		     vector< vector<double> >& longitudes,
		     vector< vector<double> >& polygonLatitudes,
		     vector< vector<double> >& polygonLongitudes);
		     
  int runRgOptimized_NATS(const string& callsign,
		const string& origin,
  		const string& destination,
  		const string& procName,
  		const double& cruiseAlt,
  		const double& cruiseSpd,
  		const string& polygonFile,
  		const string& sigmetFile,
  		const string& pirepFile,
	    const string& fixConfigFile,
	    const string& procConfigFile,
	    const bool& rerouteFlag,
	    const bool& windOptimalFlag,
	    const bool& filterAirports,
	    const int& numOpts,
	    const double& costIndex,
	    ResultTrajs& t_results,
	    vector<string>& routes,
	    vector<double>& costs,
	    vector< vector<double> >& latitudes,
	    vector< vector<double> >& longitudes,
	    vector< vector<double> >& polygonLatitudes,
	    vector< vector<double> >& polygonLongitudes);


  int runRgForNATS(const string& polygonFile,
  		const string& sigmetFile,
  		const string& pirepFile,
  		const vector<double>& lats_deg,
  		const vector<double>& lons_deg,
  		vector<double>& lat_reroute_deg,
  		vector<double>& lon_reroute_deg,
		vector<pair<int,int> >& similar_idx_wpts,
		const string &CIFPfilepath = "NONE");


  int getWeatherPolygons(const double& lat_deg, const double& lon_deg, const double& alt_ft, const double& nmi_rad,
  					const vector<double>& fp_lat_deg,
  					const vector<double>& fp_lon_deg,
  					const string& polygonFile,
  					const string& sigmetFile,
  					const string& pirepFile,
  					const string &CIFPfilepath,
  					vector<Polygon>& polygons_ahead);

}

#endif

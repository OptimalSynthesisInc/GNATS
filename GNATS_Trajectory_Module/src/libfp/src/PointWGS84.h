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

/**
 * PointWGS84.h
 * 
 * 2D Lat/Lon coordinate
 *
 * Author: jason
 * Date: January 19, 2013
 */

#ifndef __POINTWGS84_H__
#define __POINTWGS84_H__
#include <string>
using std::string;

class PointWGS84 {
  
 public:
  PointWGS84();
  PointWGS84(const double& latitude, const double& longitude);
  PointWGS84(const double& latitude, const double& longitude, const double& alt);
  PointWGS84(const PointWGS84& that);
  virtual ~PointWGS84();

  PointWGS84& operator=(const PointWGS84& that);
  bool operator==(const PointWGS84& that) const;


 public:
  double latitude;
  double longitude;
  double alt;

  //Need to add these such that we can incorporate the use of SID/STARs
  //and waypoint restrictions in the current trajectory generator.
  string path_n_terminator; //ARINC 424.18 SECTION 5.21
  string alt_desc;//ARINC 424.18 SECTION 5.29
  double alt_1;//ARINC 424.18 SECTION 5.30
  double alt_2;//ARINC 424.18 SECTION 5.30
  double speed_lim;//ARINC 424.18 SECTION 5.72
  string wpname;//ARINC 424.18 SECTION 5.13
  string procname;//ARINC 424.18 SECTION 5.10
  string proctype;//ARINC 424.18 SECTION 5.5 (SID/STAR OR APPROACH)
  string recco_navaid;//ARINC 424.18 SECTION 5.23
  double theta;//ARINC 424.18 SECTION 5.24
  double rho;//ARINC 424.18 SECTION 5.25
  double mag_course;//ARINC 424.18 SECTION 5.26
  double rt_dist;//ARINC 424.18 SECTION 5.27
  string spdlim_desc;//ARINC 424.18 SECTION 5.261
  double wp_cat;

  string phase;
  string type;

};

#endif  /* __POINTWGS84_H */

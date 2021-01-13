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
 * NatsSid.h
 * 
 * Sid data loaded from NATS data files
 * C++ host-side class
 *
 * Author: jason
 * Date: January 19, 2013
 */

#ifndef __NATSSID_H__
#define __NATSSID_H__

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;

class NatsSid {
 public:
  NatsSid();
  NatsSid(const NatsSid& that);
  virtual ~NatsSid();

  string id;
  string name;
  vector<string> waypoints;
  vector<double> latitudes;
  vector<double> longitudes;
  //ONLY SID THING
  map<string,pair<double,double> > runway_to_fdf_course_alt;
  //ONLY SID THING

  //PARIKSHIT ADDER
  /*
   * Each variable has been described in the comments. For details of what each
   * variable means, you can look up FAA CIFP database and the navigation data
   * decoding specification standards as per ARINC 424-18 document.
   */
  string runway;//JUST A PLACEHOLDER SUCH THAT THE TEMPLATED DATA LOADER WORKS
  map<string, vector<string> > wp_map; // ROUTE_IN_SID-->VECTOR(WAYPOINT) )
  map<string, pair<string,string> > route_to_trans_rttype; // ROUTE_IN_SID-->PAIR(ROUTE TRANSITION ID,ROUTE TYPE)
  map<string, vector<pair<string,string> > > path_term; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,PATH AND TERMINATION) )
  map<string, vector<pair<string,string> > > alt_desc; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,ALTITUDE CONSTRAINT DESCRIPTION) )
  map<string, vector<pair<string,double> > > alt_1; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,LOW ALT CONSTRAINT) )
  map<string, vector<pair<string,double> > > alt_2; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,HIGH ALT CONSTRAINT) )
  map<string, vector<pair<string,double> > > spd_limit; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,SPEED LIM CONSTRAINT) )
  map<string, vector<pair<string,string> > > recco_nav; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,RECOMMENDED NAVAID) ) SEE SECTION 5.23 IN ARINC 424
  map<string, vector<pair<string,double> > > theta; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,THETA ) ) SEE SECTION 5.24 IN ARINC 424
  map<string, vector<pair<string,double> > > rho; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,RHO ) ) SEE SECTION 5.25 IN ARINC 424
  map<string, vector<pair<string,double> > > mag_course; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,MAGNETIC COURSE AT WPT) ) SEE SECTION 5.26 IN ARINC 424
  map<string, vector<pair<string,double> > > rt_dist; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,ROUTE DISTANCE ) ) SEE SECTION 5.27 IN ARINC 424
  map<string, vector<pair<string,string> > > spdlim_desc; //ROUTE_IN_SID--> VECTOR( PAIR(WAYPOINT,SPEED LIMIT CONSTRAINT DESCRIPTION) )
  size_t size() const;

  bool operator<(const NatsSid& that) const;

  bool operator==(const NatsSid& that) const;
};

#endif  /* __NATSSID_H__ */

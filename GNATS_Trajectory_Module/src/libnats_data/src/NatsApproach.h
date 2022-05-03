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

/*
 * NatsApproach.h
 *
 *  Created on: Jul 19, 2017
 *      Author: pdutta
 */

#ifndef SRC_LIBNATS_DATA_SRC_NATSAPPROACH_H_
#define SRC_LIBNATS_DATA_SRC_NATSAPPROACH_H_

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;

class NatsApproach {
 public:
  NatsApproach();
  NatsApproach(const NatsApproach& that);
  virtual ~NatsApproach();

  string id;
  string name;
  vector<string> waypoints;
  vector<double> latitudes;
  vector<double> longitudes;
  //ONLY SID THING. JUST PUT HERE SUCH THAT THE TEMPLATED PARSING WORKS
  map<string,pair<double,double> > runway_to_fdf_course_alt;
  //ONLY SID THING. JUST PUT HERE SUCH THAT THE TEMPLATED PARSING WORKS


  //PARIKSHIT ADDER
  /*
   * Please refer to NatsSid.h for complete description.
   * Replace SID by APPROACH in them.
   */

  string runway;
  map<string, vector<string> > wp_map;
  map<string, pair<string,string> > route_to_trans_rttype;
  map<string, vector<pair<string,string> > > path_term;
  map<string, vector<pair<string,string> > > alt_desc;
  map<string, vector<pair<string,double> > > alt_1;
  map<string, vector<pair<string,double> > > alt_2;
  map<string, vector<pair<string,double> > > spd_limit;
  map<string, vector<pair<string,string> > > recco_nav;
  map<string, vector<pair<string,double> > > theta;
  map<string, vector<pair<string,double> > > rho;
  map<string, vector<pair<string,double> > > mag_course;
  map<string, vector<pair<string,double> > > rt_dist;
  map<string, vector<pair<string,string> > > spdlim_desc;

  size_t size() const;

  bool operator<(const NatsApproach& that) const;

  bool operator==(const NatsApproach& that) const;
};

#endif /* SRC_LIBNATS_DATA_SRC_NATSAPPROACH_H_ */

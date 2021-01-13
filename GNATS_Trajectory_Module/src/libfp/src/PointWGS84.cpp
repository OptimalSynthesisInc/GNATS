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
 * PointWGS84.cu
 * 
 * WGS84 Lat/Lon Point for device
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "PointWGS84.h"
#include <stdlib.h>

PointWGS84::PointWGS84() :
  latitude(0),
  longitude(0),
  alt(0),
  path_n_terminator(""),
  alt_desc("NONE"),
  alt_1(-1000),
  alt_2(-1000),
  speed_lim(-1000),
  wpname(""),
  procname(""),
  proctype(""),
  recco_navaid("NONE"),
  theta(-1000),
  rho(-1000),
  mag_course(-1000),
  rt_dist(-1000),
  spdlim_desc("NONE"),
  wp_cat(-10),
  phase(""),
  type("") {
}

PointWGS84::PointWGS84(const double& latitude, const double& longitude) :
  latitude(latitude),
  longitude(longitude),
  alt(0),
  path_n_terminator(""),
  alt_desc("NONE"),
  alt_1(-10000),
  alt_2(-10000),
  speed_lim(-10000),
  wpname(""),
  procname(""),
  proctype(""),
  recco_navaid("NONE"),
  theta(-10000),
  rho(-1000),
  mag_course(-1000),
  rt_dist(-1000),
  spdlim_desc("NONE"),
  wp_cat(-10),
  phase(""),
  type("") {
}

PointWGS84::PointWGS84(const double& latitude, const double& longitude, const double& alt) :
  latitude(latitude),
  longitude(longitude),
  alt(alt),
  path_n_terminator(""),
  alt_desc("NONE"),
  alt_1(-10000),
  alt_2(-10000),
  speed_lim(-10000),
  wpname(""),
  procname(""),
  proctype(""),
  recco_navaid("NONE"),
  theta(-10000),
  rho(-1000),
  mag_course(-1000),
  rt_dist(-1000),
  spdlim_desc("NONE"),
  wp_cat(-10),
  phase(""),
  type("") {
}

PointWGS84::PointWGS84(const PointWGS84& that) :
  latitude(that.latitude),
  longitude(that.longitude),
  alt(that.alt),
  path_n_terminator(that.path_n_terminator),
  alt_desc(that.alt_desc),
  alt_1(that.alt_1),
  alt_2(that.alt_2),
  speed_lim(that.speed_lim),
  wpname(that.wpname),
  procname(that.procname),
  proctype(that.proctype),
  recco_navaid(that.recco_navaid),
  theta(that.theta),
  rho(that.rho),
  mag_course(that.mag_course),
  rt_dist(that.rt_dist),
  spdlim_desc(that.spdlim_desc),
  wp_cat(that.wp_cat),
  phase(that.phase),
  type(that.type) {
}

PointWGS84::~PointWGS84() {
}

PointWGS84& PointWGS84::operator=(const PointWGS84& that) {
  if(this == &that) return *this;

  this->latitude = that.latitude;
  this->longitude = that.longitude;
  this->alt = that.alt;
  this->path_n_terminator = that.path_n_terminator;
  this->alt_desc = that.alt_desc;
  this->alt_1 = that.alt_1;
  this->alt_2 = that.alt_2;
  this->speed_lim = that.speed_lim;
  this->wpname = that.wpname;
  this->procname = that.procname;
  this->proctype = that.proctype;
  this->recco_navaid = that.recco_navaid;
  this->theta = that.theta;
  this->rho = that.rho;
  this->mag_course = that.mag_course;
  this->rt_dist = that.rt_dist;
  this->spdlim_desc = that.spdlim_desc;
  this->wp_cat = that.wp_cat;
  this->phase = that.phase;
  this->type = that.type;

  return *this;
}


bool PointWGS84::operator==(const PointWGS84& that) const{

	return  ( (this->wpname == that.wpname)
			|| ( abs(this->latitude) > 1e-4 &&
				(abs(this->latitude-that.latitude) < 1e-4) &&
				abs(this->longitude) > 1e-4 &&
				(abs(this->longitude-that.longitude) < 1e-4) ) );

}


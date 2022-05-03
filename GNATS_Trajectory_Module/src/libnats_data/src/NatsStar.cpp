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

/**
 * NatsStar.cpp
 * 
 * Star data loaded from NATS data files
 * C++ host-side class
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "NatsStar.h"

#include <vector>
#include <string>

using std::string;
using std::vector;

NatsStar::NatsStar() :
  id(""),
  name(""),
  waypoints(vector<string>()),
  latitudes(vector<double>()),
  longitudes(vector<double>()),
  runway_to_fdf_course_alt(map<string, pair<double,double> >()),
  runway(""),
  wp_map(map<string,vector<string> >()),
  route_to_trans_rttype(map< string, pair<string,string> >() ),
  path_term(map< string, vector<pair<string,string> > >()),
  alt_desc(map<string, vector<pair<string,string> > >()),
  alt_1(map<string, vector<pair<string,double> > >()),
  alt_2(map<string, vector<pair<string,double> > > ()),
  spd_limit(map<string, vector<pair<string,double> > >()),
  recco_nav(map<string, vector<pair<string,string> > >()),
  theta(map<string, vector<pair<string,double> > >()),
  rho(map<string, vector<pair<string,double> > >()),
  mag_course(map<string, vector<pair<string,double> > >()),
  rt_dist(map<string, vector<pair<string,double> > >()),
  spdlim_desc(map<string, vector<pair<string,string> > >()){
}

NatsStar::NatsStar(const NatsStar& that) :
  id(that.id),
  name(that.name),
  waypoints(vector<string>(that.waypoints)),
  latitudes(vector<double>(that.latitudes)),
  longitudes(vector<double>(that.longitudes)),
  runway_to_fdf_course_alt(map<string, pair<double,double> >(that.runway_to_fdf_course_alt)),
  runway(that.runway),
  wp_map(map<string,vector<string> >(that.wp_map)),
  route_to_trans_rttype(map< string, pair<string,string> >(that.route_to_trans_rttype) ),
  path_term(map< string, vector<pair<string,string> > >(that.path_term)),
  alt_desc(map<string, vector<pair<string,string> > >(that.alt_desc)),
  alt_1(map<string, vector<pair<string,double> > >(that.alt_1)),
  alt_2(map<string, vector<pair<string,double> > >(that.alt_2)),
  spd_limit(map<string, vector<pair<string,double> > >(that.spd_limit)),
  recco_nav(map<string, vector<pair<string,string> > >(that.recco_nav)),
  theta(map<string, vector<pair<string,double> > >(that.theta)),
  rho(map<string, vector<pair<string,double> > >(that.rho)),
  mag_course(map<string, vector<pair<string,double> > >(that.mag_course)),
  rt_dist(map<string, vector<pair<string,double> > >(that.rt_dist)),
  spdlim_desc(map<string, vector<pair<string,string> > >(that.spdlim_desc)){
}

NatsStar::~NatsStar() {
	if (!waypoints.empty()) {
		waypoints.clear();
	}
	if (!latitudes.empty()) {
		latitudes.clear();
	}
	if (!longitudes.empty()) {
		longitudes.clear();
	}
	if (!runway_to_fdf_course_alt.empty()) {
		runway_to_fdf_course_alt.clear();
	}

	if (!wp_map.empty()) {
		wp_map.clear();
	}
	if (!route_to_trans_rttype.empty()) {
		route_to_trans_rttype.clear();
	}
	if (!path_term.empty()) {
		map<string, vector<pair<string,string> > >::iterator ite;
		for (ite = path_term.begin(); ite != path_term.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		path_term.clear();
	}
	if (!alt_desc.empty()) {
		map<string, vector<pair<string,string> > >::iterator ite;
		for (ite = alt_desc.begin(); ite != alt_desc.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		alt_desc.clear();
	}
	if (!alt_1.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = alt_1.begin(); ite != alt_1.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		alt_1.clear();
	}
	if (!alt_2.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = alt_2.begin(); ite != alt_2.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		alt_2.clear();
	}
	if (!spd_limit.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = spd_limit.begin(); ite != spd_limit.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		spd_limit.clear();
	}
	if (!recco_nav.empty()) {
		map<string, vector<pair<string,string> > >::iterator ite;
		for (ite = recco_nav.begin(); ite != recco_nav.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		recco_nav.clear();
	}
	if (!theta.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = theta.begin(); ite != theta.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		theta.clear();
	}
	if (!rho.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = rho.begin(); ite != rho.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		rho.clear();
	}
	if (!mag_course.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = mag_course.begin(); ite != mag_course.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		mag_course.clear();
	}
	if (!rt_dist.empty()) {
		map<string, vector<pair<string,double> > >::iterator ite;
		for (ite = rt_dist.begin(); ite != rt_dist.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		rt_dist.clear();
	}
	if (!spdlim_desc.empty()) {
		map<string, vector<pair<string,string> > >::iterator ite;
		for (ite = spdlim_desc.begin(); ite != spdlim_desc.end(); ite++) {
			if (!ite->second.empty()) {
				ite->second.clear();
			}
		}

		spdlim_desc.clear();
	}
}

bool NatsStar::operator<(const NatsStar& that) const {
  return this->name < that.name;
}

bool NatsStar::operator==(const NatsStar& that) const{
	return ( (this->id == that.id
			|| this->id == "K" + that.id
			|| this->id == "P"+that.id
			|| this->id == "C"+that.id) && (this->name == that.name) );
}

size_t NatsStar::size() const {
	return sizeof(NatsStar) + waypoints.size()*sizeof(string) +
			latitudes.size()*sizeof(double) +
			longitudes.size()*sizeof(double);
}


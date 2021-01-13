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


#include "AirportNode.h"

AirportNode::AirportNode():
id(""),
index(-1),
latitude(-1000.0),
longitude(-1000.0),
domain(""),
refName1(""),
type1(""),
refName2(""),
type2("")
{
}

AirportNode::AirportNode(const string& p_id, const int p_index, const double p_latitude, const double p_longitude, const string& p_domain, const string& p_refName1, const string& p_type1, const string& p_refName2, const string& p_type2) :
	id(p_id),
	index(p_index),
	latitude(p_latitude),
	longitude(p_longitude),
	domain(p_domain),
	refName1(p_refName1),
	type1(p_type1),
	refName2(p_refName2),
	type2(p_type2)
{
}

AirportNode::AirportNode(const AirportNode& that) :
	id(""),
	index(-1),
	latitude(0.0),
	longitude(0.0),
	domain(""),
	refName1(""),
	type1(""),
	refName2(""),
	type2("")
{
	id.assign(that.id);
	index = that.index;
	latitude = that.latitude;
	longitude = that.longitude;
	domain.assign(that.domain);
	refName1.assign(that.refName1);
	type1.assign(that.type1);
	refName2.assign(that.refName2);
	type2.assign(that.type2);
}

AirportNode::~AirportNode()
{
}

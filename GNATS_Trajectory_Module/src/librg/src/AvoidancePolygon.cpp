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
 * AvoidancePolygon.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: jason
 */

#include "AvoidancePolygon.h"
#include "Polygon.h"

#include <cstdlib>
#include <cstring>
#include <vector>

using std::vector;

namespace osi {

AvoidancePolygon::AvoidancePolygon() :
	Polygon(),
	scaling_factors(vector<double>()) {
}

AvoidancePolygon::AvoidancePolygon(const double* const x_data,
		const double* const y_data,
		const int& num_vertices,
		const bool& ccw_flag,
		const vector<double>& scaling_factors) :
	Polygon(x_data, y_data, num_vertices, ccw_flag),
	scaling_factors(scaling_factors) {
}

AvoidancePolygon::AvoidancePolygon(const vector<double>& x_data,
		const vector<double>& y_data,
		const int& num_vertices,
		const bool& ccw_flag,
		const vector<double>& scaling_factors) :
	Polygon(x_data, y_data, num_vertices, ccw_flag),
	scaling_factors(scaling_factors) {
}

AvoidancePolygon::AvoidancePolygon(const AvoidancePolygon& that) :
	Polygon(that.x_data, that.y_data, that.num_vertices, that.ccw_flag),
	scaling_factors(that.scaling_factors) {
}

AvoidancePolygon::~AvoidancePolygon() {
}

AvoidancePolygon& AvoidancePolygon::operator=(const AvoidancePolygon& that) {
	if(this == &that) return *this;

	if(this->x_data) {
		free(this->x_data);
	}
	if(this->y_data) {
		free(this->y_data);
	}

	this->xmin = that.xmin;
	this->xmax = that.xmax;
	this->ymin = that.ymin;
	this->ymax = that.ymax;

	this->num_vertices = that.num_vertices;
	this->ccw_flag = that.ccw_flag;

	this->x_centroid = that.x_centroid;
	this->y_centroid = that.y_centroid;

	this->x_data = (double*)calloc(this->num_vertices, sizeof(double));
	this->y_data = (double*)calloc(this->num_vertices, sizeof(double));

	memcpy(this->x_data, that.x_data, this->num_vertices*sizeof(double));
	memcpy(this->y_data, that.y_data, this->num_vertices*sizeof(double));

	this->scaling_factors.clear();
	this->scaling_factors.insert(this->scaling_factors.begin(),
			                     that.scaling_factors.begin(),
			                     that.scaling_factors.end());

	return *this;
}

} /* namespace osi */

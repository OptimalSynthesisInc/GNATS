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

/*
 * Polygon.cpp
 *
 *  Created on: May 18, 2012
 *      Author: jason
 */

#include "Polygon.h"

#include "geometry_utils.h"

#include <cstdlib>
#include <cstring>
#include <cmath>

#include <iostream>

using std::cout;
using std::endl;

namespace osi {

static void max_value(const double* const array, const int& length, double* const max) {
	if(!max) return;
	*max = -999999999999;
	for(int i=0; i<length; ++i) {
		if(array[i] > *max) {
			*max = array[i];
		}
	}
}

static void min_value(const double* const array, const int& length, double* const min) {
	if(!min) return;
	*min = 999999999999;
	for(int i=0; i<length; ++i) {
		if(array[i] < *min) {
			*min = array[i];
		}
	}
}

static void compute_centroid(const double* const xdata,
		                     const double* const ydata,
		                     const int& num_points,
		                     double* const x_centroid,
		                     double* const y_centroid) {

	if(!x_centroid) return;
	if(!y_centroid) return;

	double x_sum = 0;
	double y_sum = 0;
	for(int i=0; i<num_points; ++i) {
		x_sum += xdata[i];
		y_sum += ydata[i];
	}

	*x_centroid = x_sum / num_points;
	*y_centroid = y_sum / num_points;
}

Polygon::Polygon() :
	x_data(NULL),
	y_data(NULL),
	num_vertices(0),
	ccw_flag(true),
	xmin(0),
	xmax(0),
	ymin(0),
	ymax(0),
	x_centroid(0),
	y_centroid(0),
	poly_type("NOT-ASSIGNED"),
	start_hr(0),
	end_hr(24){
}

Polygon::Polygon(const double* const x_data,
		         const double* const y_data,
		         const int& num_vertices,
		         const bool& ccw_flag) :
    x_data(NULL),
    y_data(NULL),
    num_vertices(num_vertices),
    ccw_flag(ccw_flag),
    xmin(0),
    xmax(0),
    ymin(0),
    ymax(0),
    x_centroid(0),
    y_centroid(0),
    poly_type("NOT-ASSIGNED"),
	start_hr(0),
	end_hr(24){

	// check to see if the polygon needs closing
	int end = num_vertices-1;
	bool needs_closing = false;
	if((x_data[0] != x_data[end]) || y_data[0] != y_data[end]) {
		this->num_vertices = num_vertices + 1;
		needs_closing = true;
	}

	this->x_data = (double*)calloc(this->num_vertices, sizeof(double));
	this->y_data = (double*)calloc(this->num_vertices, sizeof(double));

	memcpy(this->x_data, x_data, num_vertices*sizeof(double));
	memcpy(this->y_data, y_data, num_vertices*sizeof(double));

	// close the polygon if necessary
	if(needs_closing) {
		this->x_data[this->num_vertices-1] = this->x_data[0];
		this->y_data[this->num_vertices-1] = this->y_data[0];
	}

	min_value(this->x_data, num_vertices, &this->xmin);
	max_value(this->x_data, num_vertices, &this->xmax);
	min_value(this->y_data, num_vertices, &this->ymin);
	max_value(this->y_data, num_vertices, &this->ymax);

	compute_centroid(this->x_data, this->y_data, num_vertices, &this->x_centroid, &this->y_centroid);
}

Polygon::Polygon(const vector<double>& x_data,
		         const vector<double>& y_data,
		         const int& num_vertices,
		         const bool& ccw_flag) :
	 x_data(NULL),
	 y_data(NULL),
	 num_vertices(num_vertices),
	 ccw_flag(ccw_flag),
	 xmin(0),
	 xmax(0),
	 ymin(0),
	 ymax(0),
	 x_centroid(0),
	 y_centroid(0),
	 poly_type("NOT-ASSIGNED"),
	 start_hr(0),
	 end_hr(24){
	if (x_data.size() > 0) {
		// check to see if the polygon needs closing
		int end = x_data.size()-1;
		bool needs_closing = false;
		if((x_data.at(0) != x_data.at(end)) || y_data.at(0) != y_data.at(end)) {
			this->num_vertices = x_data.size() + 1;
			needs_closing = true;
		}

		this->x_data = (double*)calloc(this->num_vertices, sizeof(double));
		this->y_data = (double*)calloc(this->num_vertices, sizeof(double));

		for(int i=0; i<num_vertices; ++i) {
			this->x_data[i] = x_data.at(i);
			this->y_data[i] = y_data.at(i);
		}

		// close the polygon if necessary
		if(needs_closing) {
			this->x_data[this->num_vertices-1] = this->x_data[0];
			this->y_data[this->num_vertices-1] = this->y_data[0];
		}

		min_value(this->x_data, num_vertices, &this->xmin);
		max_value(this->x_data, num_vertices, &this->xmax);
		min_value(this->y_data, num_vertices, &this->ymin);
		max_value(this->y_data, num_vertices, &this->ymax);

		compute_centroid(this->x_data, this->y_data, this->num_vertices, &this->x_centroid, &this->y_centroid);
	}
}

Polygon::Polygon(const Polygon& that):
    x_data(NULL),
    y_data(NULL),
    num_vertices(that.num_vertices),
    ccw_flag(that.ccw_flag),
    xmin(that.xmin),
    xmax(that.xmax),
    ymin(that.ymin),
    ymax(that.ymax),
    x_centroid(that.x_centroid),
    y_centroid(that.y_centroid),
    poly_type(that.poly_type),
	start_hr(that.start_hr),
	end_hr(that.end_hr){

	this->x_data = (double*)calloc(num_vertices, sizeof(double));
	this->y_data = (double*)calloc(num_vertices, sizeof(double));

	memcpy(this->x_data, that.x_data, num_vertices*sizeof(double));
	memcpy(this->y_data, that.y_data, num_vertices*sizeof(double));
}

Polygon::~Polygon() {
	if(this->x_data) {
		free(this->x_data);
		this->x_data = NULL;
	}
	if(this->y_data) {
		free(this->y_data);
		this->y_data = NULL;
	}
}

const int& Polygon::getNumVertices() const {
	return num_vertices;
}

const double* Polygon::getXData() const {
	return x_data;
}

const double* Polygon::getYData() const {
	return y_data;
}

const bool& Polygon::getCCWFlag() const {
	return ccw_flag;
}

const double& Polygon::getXMin() const {
	return xmin;
}

const double& Polygon::getYMin() const {
	return ymin;
}

const double& Polygon::getXMax() const {
	return xmax;
}

const double& Polygon::getYMax() const {
	return ymax;
}

void Polygon::getCentroid(double* const x_centroid, double* const y_centroid) const {
	*x_centroid = this->x_centroid;
	*y_centroid = this->y_centroid;
}

/*
 * Return true if the point x,y lies inside this polygon or false otherwise.
 * Points that lie exactly on the polygon boundary are considered
 * inside.
 */
bool Polygon::contains(const double& x, const double& y, const bool& gcFlag) const {

	// first check to see if the point is in the bounding box
	// if not, return false.
	if(x < xmin || x > xmax || y < ymin || y > ymax) return false;

	// if the point is in the bounding box then do ray casting
	// to determine if the point is in the polygon.  the ray casting
	// algorithm is as follows...
	// choose a point that lies outside the bounding box.
	// compute the number of intersections of the segment from
	// (x,y)-(xout,yout) with the polygon edges
	// if the number of intersections is odd then point x,y is inside
	// otherwise its outside.

	// first, choose a point that lies outside the bounding box.
	// we'll choose a horizontal ray with xout=xmin-1;
	double xout = xmin-1.;
	double yout = ymin-1.;
	// iterate over polygon edges and count number of intersections
	int num_intersections = 0;
	for(int i=1; i<num_vertices; ++i) {
		double x0 = x_data[i-1];
		double y0 = y_data[i-1];
		double x1 = x_data[i];
		double y1 = y_data[i];

		// compute segment intersection using great-circle or linear
		// cartesian lines.  gcFlag defaults to true if not
		// supplied.
		if(gcFlag) {
			if( segments_intersect_gc(yout, xout, y, x, y0, x0, y1, x1) ) {
				num_intersections++;
			}
		} else {
			if( segments_intersect(yout, xout, y, x, y0, x0, y1, x1) ) {
				num_intersections++;
			}
		}
	}

	double dist_last = compute_distance_gc(y_data[0],x_data[0],y_data[num_vertices-1],x_data[num_vertices-1],0.0);
	if (dist_last >1e-3){
		double x0 = x_data[num_vertices-1];
		double y0 = y_data[num_vertices-1];
		double x1 = x_data[0];
		double y1 = y_data[0];

		// compute segment intersection using great-circle or linear
		// cartesian lines.  gcFlag defaults to true if not
		// supplied.
		if(gcFlag) {
			if( segments_intersect_gc(yout, xout, y, x, y0, x0, y1, x1) ) {
				num_intersections++;
			}
		} else {
			if( segments_intersect(yout, xout, y, x, y0, x0, y1, x1) ) {
				num_intersections++;
			}
		}

	}



	// if the number of intersections is odd then the point x,y is inside
	// the polygon.  if number of intersections is even then the point
	// is outside the polygon.  note: we counted points that lie exactly
	// on the polygon boundary as inside.
	if( (num_intersections % 2) != 0 ) {
		return true;
	}

	return false;
}

bool Polygon::intersectsSegment(const double& x1, const double& y1,
		                        const double& x2, const double& y2,
		                        double* const xint, double* const yint,
		                        const bool& gcFlag) const {

	// find the first intersection of the specfied segment with
	// an edge of this polygon.
	for(int i=1; i<num_vertices; ++i) {
		double x3 = x_data[i-1];
		double y3 = y_data[i-1];
		double x4 = x_data[i];
		double y4 = y_data[i];

		// compute segment intersection using great-circle or linear
		// cartesian lines.  gcFlag defaults to true if not
		// supplied.  Note, y stores latitude, x stores longitude
		if(gcFlag) {
			if(segments_intersect_gc(y1,x1, y2,x2, y3,x3, y4,x4, yint,xint)) {
				return true;
			}
		} else {
			if(segments_intersect(y1,x1, y2,x2, y3,x3, y4,x4, yint,xint)) {
				return true;
			}
		}
	}
	return false;
}

void Polygon::scale(const double& sx, const double& sy, Polygon* const scaled) {

	if(!scaled) return;

	// form the scaling transformation matrix
	// [sx  0   0]
	// [0   sy  0]
	// [0   0   1]

	// compute the scaling anchor point as centroid
	double cx = x_centroid;//(xmax+xmin)/2.;
	double cy = y_centroid;//(ymax+ymin)/2.;

	double x_scaled[num_vertices];
	double y_scaled[num_vertices];

	// iterate over each vertex in the polygon
	// compute the vector from the bbox center to the vertex
	// multiply the vector by the scaling xform matrix:
	// [vx] * [sx  0 ] = [vx*sx]
	// [vy]   [0   sy]   [vy*sy]
	for(int i=0; i<num_vertices; ++i) {
		double x, y;

		x = x_data[i];
		y = y_data[i];
		double vx = x-cx;
		double vy = y-cy;

		x_scaled[i] = cx + sx*vx;
		y_scaled[i] = cy + sy*vy;
	}

	// build the scaled poly
	Polygon p(x_scaled, y_scaled, num_vertices, ccw_flag);

	// copy the scaled poly to the output
	*scaled = p;
}

void Polygon::convexHull(Polygon* const convex) const {

	if(!convex) return;

	// compute the convex hull
	vector<double> xconvex;
	vector<double> yconvex;
	int n = convex_hull_scan(num_vertices, x_data, y_data, &xconvex, &yconvex);

	// make sure the convex polygon is closed
	if(xconvex.front() != xconvex.back() &&
	   yconvex.front() != yconvex.back()) {
		xconvex.push_back(xconvex.front());
		yconvex.push_back(yconvex.front());
		n++;
	}

	// convex_hull_scan returns vertices in clockwise order
	Polygon pconvex(xconvex, yconvex, n, true);

	// copy convex poly to the output
	*convex = pconvex;
}

Polygon& Polygon::operator=(const Polygon& that) {

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

	return *this;
}

void Polygon::printMaxVals() const{
	cout << "xmax = " << this->xmax << " , ymax = " << this->ymax << ", xmin = " << this->xmin <<
			" , ymin = " << this->ymin << endl;
}

void Polygon::setPolyType(const string& ptype){
	this->poly_type = ptype;
}

const string& Polygon::getPolyType() const{
	return poly_type;
}

void Polygon::setStartHour(const int& start_hr){
	this->start_hr = start_hr;
}

const int& Polygon::getStartHour() const{
	return start_hr;
}

void Polygon::setEndHour(const int& end_hr){
	this->end_hr = end_hr;
}

const int& Polygon::getEndHour() const{
	return end_hr;
}

}

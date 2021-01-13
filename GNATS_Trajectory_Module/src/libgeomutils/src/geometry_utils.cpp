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
 * geometry_utils.cpp
 *
 *  Created on: May 31, 2012
 *      Author: jason
 */

#include "geometry_utils.h"

#include "util_string.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>


namespace osi {

double RADIUS_EARTH_FT = 20925524.9;

/*
 * Conversion factors
 */
static const double         DEG_TO_RAD = M_PI / 180.;
static const double         RAD_TO_DEG = 180. / M_PI;


/*
 * ---------------------------------------------------------------------------
 * Class:
 *   hull_point
 *
 * Description:
 *   Private point class used by the convex hull algorithm for sorting
 *   points by x-coordinate.
 * ---------------------------------------------------------------------------
 */
class hull_point {
public:
	hull_point() : x(0), y(0) {}
	hull_point(const double& x, const double& y) : x(x), y(y) {}
	hull_point(const hull_point& that) : x(that.x), y(that.y) {}
	virtual ~hull_point() {}

	hull_point& operator=(const hull_point& that) {
		if(this == &that) return *this;
		this->x = that.x;
		this->y = that.y;
		return *this;
	}

	double x;
	double y;
};

/*
 * ---------------------------------------------------------------------------
 * Function:
 *   gc_intersection()
 *
 * Description:
 *   Private implementation of the great circle segment intersection.
 *
 * Inputs:
 *   pt1   start point of segment 1 (pt1[0] = latitude, pt1[1] = longitude)
 *   pt2   end point of segment 1 (pt2[0] = latitude, pt2[1] = longitude)
 *   pt3   start point of segment 2 (pt3[0] = latitude, pt3[1] = longitude)
 *   pt4   end point of segment 1 (pt4[0] = latitude, pt4[1] = longitude)
 *
 * In/Out:
 *   flag   set to true if an intersection was found, false otherwise
 *   intLat   set to the latitude of the intersection if one is found
 *   intLon   set to the longitude of the intersection if one is found
 *
 * Returns:
 *   none
 * ---------------------------------------------------------------------------
 */
static void gc_intersection(const double* const pt1,
		                    const double* const pt2,
		                    const double* const pt3,
		                    const double* const pt4,
		                    bool* const flag=NULL,
		                    double* const intLat=NULL,
		                    double* const intLon=NULL) {

    double p1[3], p2[3], ph[3], chi;
    double q1[3], q2[3], qh[3], om;
    double ang1, ang2, c1, c2, s1, s2, a, b, c, d, A, B, D, w, x, s, t, nm;
    int i;

    if(!flag) return;

    (*flag) = false;

    ang1 = pt1[0] * DEG_TO_RAD; ang2 = pt1[1] * DEG_TO_RAD;
    c1 = cos(ang1); s1 = sin(ang1); c2 = cos(ang2); s2 = sin(ang2);
    p1[0] = c1 * c2; p1[1] = c1 * s2; p1[2] = s1;
    nm = 0.0; for (i = 0; i < 3; i++) nm += p1[i] * p1[i];
    for (i = 0; i < 3; ++i) p1[i] = p1[i] / nm;
    ang1 = pt2[0] * DEG_TO_RAD; ang2 = pt2[1] * DEG_TO_RAD;
    c1 = cos(ang1); s1 = sin(ang1); c2 = cos(ang2); s2 = sin(ang2);
    p2[0] = c1 * c2; p2[1] = c1 * s2; p2[2] = s1;
    nm = 0.0; for (i = 0; i < 3; i++) nm += p2[i] * p2[i];
    for (i = 0; i < 3; ++i) p2[i] = p2[i] / nm;

    c1 = p1[0] * p2[0] + p1[1] * p2[1] + p1[2] * p2[2]; //cos(chi)
    if (fabs(c1) > 1) return;
    s1 = sqrt(1 - c1 * c1); // sin(chi);
    chi = acos(c1);
    if (chi < 1e-3) return;
    for (i = 0; i < 3; ++i) ph[i] = (p2[i] - c1 * p1[i]) / s1;

    ang1 = pt3[0] * DEG_TO_RAD; ang2 = pt3[1] * DEG_TO_RAD;
    c1 = cos(ang1); s1 = sin(ang1); c2 = cos(ang2); s2 = sin(ang2);
    q1[0] = c1 * c2; q1[1] = c1 * s2; q1[2] = s1;
    nm = 0.0; for (i = 0; i < 3; i++) nm += q1[i] * q1[i];
    for (i = 0; i < 3; ++i) q1[i] = q1[i] / nm;
    ang1 = pt4[0] * DEG_TO_RAD; ang2 = pt4[1] * DEG_TO_RAD;
    c1 = cos(ang1); s1 = sin(ang1); c2 = cos(ang2); s2 = sin(ang2);
    q2[0] = c1 * c2; q2[1] = c1 * s2; q2[2] = s1;
    nm = 0.0; for (i = 0; i < 3; i++) nm += q2[i] * q2[i];
    for (i = 0; i < 3; ++i) q2[i] = q2[i] / nm;

    c1 = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2];
    if (fabs(c1) > 1) return;
    s1 = sqrt(1 - c1 * c1);
    om = acos(c1);
    if (om < 1e-3) return;
    for (i = 0; i < 3; ++i) qh[i] = (q2[i] - c1 * q1[i]) / s1;

    a = b = c = d = 0;
    for (i = 0; i < 3; ++i) {
        a += p1[i] * q1[i];
        b += p1[i] * qh[i];
        c += ph[i] * q1[i];
        d += ph[i] * qh[i];
    }
    A = a * b + c * d;
    B = a * a + c * c - b * b - d * d;
    D = B * B + 4. * A * A;
    w = (-B + sqrt(D)) / 2. / A;
    x = (c + d * w) / (a + b * w);
    s = atan(w) / om;
    t = atan(x) / chi;

    if ((t < 0) | (t > 1) | (s < 0) | (s > 1)) {
        return;
    }

    (*flag) = true;

    c1 = cos(chi * t); s1 = sin(chi * t);
    for (i = 0; i < 3; ++i)
        q1[i] = c1 * p1[i] + s1 * ph[i];

    if(intLat) {
    	(*intLat) = atan2(q1[2], sqrt(q1[0]*q1[0]+q1[1]*q1[1])) * RAD_TO_DEG;
    }
    if(intLon) {
    	(*intLon) = atan2(q1[1], q1[0]) * RAD_TO_DEG;
    }

    return;
}

double compute_distance_gc_rad(const double& latRad1,
						const double& lonRad1,
                        const double& latRad2,
						const double& lonRad2,
                        const double& alt,
                        const double& r) {
	double dum;

    double dlatrad = fabs(latRad1 - latRad2);
    double dlonrad = fabs(lonRad1 - lonRad2);

    double s_half_dlatrad = sin(0.5 * dlatrad);
    double s_half_dlonrad = sin(0.5 * dlonrad);

    double s2_half_dlatrad = s_half_dlatrad * s_half_dlatrad;
    double s2_half_dlonrad = s_half_dlonrad * s_half_dlonrad;

    dum = s2_half_dlatrad + cos(latRad1) * cos(latRad2) * s2_half_dlonrad;

    return (r + alt) * 2. * asin(sqrt(dum));//TODO:CHECKED CORRECT
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   compute_distance_gc()
 *
 * Description:
 *   Compute great circle distance between lat/lon points
 *
 * Inputs:
 *   lat1   latitude of point1
 *   lon1   longitude of point1
 *   lat2   latitude of point2
 *   lon2   longitude of point2
 *   alt    altitude above the surface of the sphere at which the great
 *          circle arc will be computed (optional, default=0)
 *   r      radius of the sphere (optional, default=RADIUS_EARTH_FT)
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   The great circle distance between lat/lon points.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double compute_distance_gc(const double& lat1, const double& lon1,
                           const double& lat2, const double& lon2,
                           const double& alt,
                           const double& r) {

	// convert lat in decimal deg to radians
    double latRad1, lonRad1, latRad2, lonRad2; //, dum;
    latRad1 = lat1 * DEG_TO_RAD;
    lonRad1 = lon1 * DEG_TO_RAD;
    latRad2 = lat2 * DEG_TO_RAD;
    lonRad2 = lon2 * DEG_TO_RAD;

	return compute_distance_gc_rad(latRad1, lonRad1, latRad2, lonRad2, alt, r);
}

double compute_heading_rad_gc(const double& lat1, const double& lon1,
			              const double& lat2, const double& lon2) {

	double degToRad = M_PI / 180.;

    double latRad1, lonRad1, latRad2, lonRad2, num, den, headingRad;

    latRad1 = lat1 * degToRad;
    lonRad1 = lon1 * degToRad;
    latRad2 = lat2 * degToRad;
    lonRad2 = lon2 * degToRad;

    num = sin(lonRad2 - lonRad1) * cos(latRad2);
    den = sin(latRad2) * cos(latRad1) - sin(latRad1) * cos(latRad2) *
    		cos(lonRad2 - lonRad1);
    headingRad = atan2(num, den);

    return headingRad;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   compute_heading_gc()
 *
 * Description:
 *   Compute great circle heading between lat/lon points
 *
 * Inputs:
 *   lat1   latitude of point1
 *   lon1   longitude of point1
 *   lat2   latitude of point2
 *   lon2   longitude of point2
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   The great circle heading between lat/lon points in degrees, N zero.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double compute_heading_gc(const double& lat1, const double& lon1,
			              const double& lat2, const double& lon2) {
	double headingRad = compute_heading_rad_gc(lat1, lon1, lat2, lon2);

	return headingRad * 180. / M_PI;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   compute_location_gc()
 *
 * Description:
 *   Compute the lat/lon location given a starting lat/lon point, a great
 *   circle distance to travel, and the great circle heading.
 *
 * Inputs:
 *   lat0      latitude of point1 (degrees)
 *   lon0      longitude of point1 (degrees)
 *   alt       altitude above surface of sphere (ft)
 *   r         radius of sphere, default radius of earth (ft)
 *   range     gc distance to travel (ft)
 *   heading   heading (degrees)
 *
 * In/Out:
 *   lat  output latitude (degrees)
 *   lon  output longitude (degrees)
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void compute_location_gc(const double& lat0, const double& lon0,
						   const double& range, const double& heading,
						   double* const lat, double* const lon,
						   const double& alt, const double& r) {
	if(!lat) return;
	if(!lon) return;

	// total radius
	double R = r+alt;

	// compute range angle, sigma [radians]
	double sigma = range / R;

	// heading,lat0,lon0 [radians]
	double radHeading = heading * M_PI / 180.;
	double radLat0 = lat0 * M_PI / 180.;
	double radLon0 = lon0 * M_PI / 180.;
	double sL0 = sin(radLat0);
	double cL0 = cos(radLat0);
	double sH = sin(radHeading);
	double cH = cos(radHeading);
	double sSigma = sin(sigma);
	double cSigma = cos(sigma);

	double radLat = asin(sL0 * cSigma + cL0*sSigma*cH);
	double sL1 = sin(radLat);
	double cL1 = cos(radLat);

	double num = sH*sSigma / cL1;
	double den = (cSigma - sL0*sL1) / (cL0*cL1);

	double radLon = radLon0 + atan2(num, den);

	*lat = radLat * 180. / M_PI;
	*lon = radLon * 180. / M_PI;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   compute_distance()
 *
 * Description:
 *   Compute 2D Euclidean distance between two points.
 *
 * Inputs:
 *   x1   x coordinate of point 1
 *   y1   y coordinate of point 1
 *   x2   x coordinate of point 2
 *   y2   y coordinate of point 2
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   The euclidean distance between x1,y1 and x2,y2
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double compute_distance(const double& x1, const double& y1,
		                const double& x2, const double& y2) {
	double dx = x2-x1;
	double dy = y2-y1;
	return sqrt(dx*dx + dy*dy);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   segments_intersect_gc()
 *
 * Description:
 *   Compute intersection of great circle segments.
 *
 * Inputs:
 *   lat1   latitude of the start point of segment 1
 *   lon1   longitude of start point of segment 1
 *   lat2   latitude of the end point of segment 1
 *   lon2   longitude of the end point of segment 1
 *   lat3   latitude of the start point of segment 2
 *   lon3   longitude of the end point of segment 2
 *   lat4   latitude of the start point of segment 2
 *   lon4   longitude of the end point of segment 2
 *
 * In/Out:
 *   lat_int   pointer to the latitude of the intersection of the great
 *             circle segments (optional, default=NULL)
 *   lon_int   pointer to the longitude of the intersection of the great
 *             circle segments (optional, default=NULL)
 *
 * Returns:
 *   true if an intersection point is found, false if the segments are
 *   parallel or if no intersection is found.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
bool segments_intersect_gc(const double& lat1, const double& lon1,
		                   const double& lat2, const double& lon2,
		                   const double& lat3, const double& lon3,
		                   const double& lat4, const double& lon4,
		                   double* const lat_int,
		                   double* const lon_int) {

	double p1[2] = {lat1, lon1};
	double p2[2] = {lat2, lon2};
	double p3[2] = {lat3, lon3};
	double p4[2] = {lat4, lon4};

	bool flag = false;

	gc_intersection(p1, p2, p3, p4, &flag, lat_int, lon_int);

	return flag;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   segments_intersect()
 *
 * Description:
 *   Compute intersection of cartesian straight line segments.
 *
 * Inputs:
 *   x1        x coordinate of the start point of segment 1
 *   y1        y coordinate of the start point of segment 1
 *   x2        x coordinate of the end point of segment 1
 *   y2        y coordinate of the end point of segment 1
 *   x3        x coordinate of the start point of segment 2
 *   y3        y coordinate of the start point of segment 2
 *   x4        x coordinate of the end point of segment 2
 *   y4        y coordinate of the end point of segment 2
 *   epsilon   tolerance for comparison of doubles
 *             (optional, default=MIN_DOUBLE)
 *
 * In/Out:
 *   xint   pointer to x coordinate of the intersection of line segments
 *          (optional, default=NULL)
 *   yint   pointer to y coordinate of the intersection of line segments
 *          (optional, default=NULL)
 *
 * Returns:
 *   true if the line segments intersect, false otherwise.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
bool segments_intersect(const double& x1, const double& y1,
		                const double& x2, const double& y2,
		                const double& x3, const double& y3,
		                const double& x4, const double& y4,
		                double* const xint,
		                double* const yint,
		                const double& epsilon) {

	// form the equation of the line from x1,y1 to x2,y2:
	// Ax + By = C
	double A1 = y2-y1;
	double B1 = x1-x2;
	double C1 = A1*x1+B1*y1;

	// form the equation of the line from x3,y3 to x4,y4:
	// Ax + By = C
	double A2 = y4-y3;
	double B2 = x3-x4;
	double C2 = A2*x3+B2*y3;

	// we now have two lines defined by the equations:
	//   A1*x + B1*y = C1
	//   A2*x + B2*y = C2
	// we solve these equations for x and y to obtain the
	// point of intersection.
	// compute the matrix determinant.  if determinant is 0 then
	// the lines are parallel.  otherwise:
	//   x = (B2*C1 - B1*C2)/det
	//   y = (A1*C2 - A2*C1)/det
	double det = A1*B2 - A2*B1;
	if(fabs(det) < epsilon) return false;
	double _xint = (B2*C1 - B1*C2)/det;
	double _yint = (A1*C2 - A2*C1)/det;

	// if we found the intersection of the lines, we need
	// to make sure that the intersection lies on both
	// of the segments...
	double xmin1 = (x1 < x2 ? x1 : x2);
	double ymin1 = (y1 < y2 ? y1 : y2);
	double xmax1 = (x1 < x2 ? x2 : x1);
	double ymax1 = (y1 < y2 ? y2 : y1);
	double xmin2 = (x3 < x4 ? x3 : x4);
	double ymin2 = (y3 < y4 ? y3 : y4);
	double xmax2 = (x3 < x4 ? x4 : x3);
	double ymax2 = (y3 < y4 ? y4 : y3);
	if((_xint >= xmin1) &&
	   (_xint <= xmax1) &&
	   (_xint >= xmin2) &&
	   (_xint <= xmax2) &&
	   (_yint >= ymin1) &&
	   (_yint <= ymax1) &&
	   (_yint >= ymin2) &&
	   (_yint <= ymax2)) {

		if(xint) *xint = _xint;
		if(yint) *yint = _yint;
		return true;
	}

	return false;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   latlon_to_xy()
 *
 * Description:
 *   Convert lat/lon to x,y.  This function assumes a spherical earth.
 *
 * Inputs:
 *   lat    latitude of the point
 *   lon    longitude of the point
 *   lat0   reference latitude (optional, default=0)
 *   lon0   reference longitude (optional, default=0)
 *   x0     reference x (optional, default=0)
 *   y0     reference y (optional, default=0)
 *   r      radius of sphere in feet (optional, default=RADIUS_EARTH_FT)
 *
 * In/Out:
 *   x   pointer to x converted from longitude
 *   y   pointer to y converted from latitude
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void latlon_to_xy(const double& lat, const double& lon,
		          double* const x, double* const y,
                  const double& lat0, const double& lon0,
                  const double& x0, const double& y0,
                  const double& r) {

	if(!x) return;
	if(!y) return;

	double deg = 180. / M_PI;
	double rad = M_PI / 180.;

	double dlat = lat - lat0;
	double dlon = lon - lon0;
	double dnorth = dlat / deg * r;
	double deast = dlon / deg * cos(lat0*rad) * r;
	*x = deast + x0;
	*y = dnorth + y0;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   xy_to_latlon()
 *
 * Description:
 *   Convert x,y to lat/lon
 *
 * Inputs:
 *   x      x coordinate of the point
 *   y      y coordinate of the point
 *   lat0   reference latitude (optional, default=0)
 *   lon0   reference longitude (optional, default=0)
 *   x0     reference x (optional, default=0)
 *   y0     reference y (optional, default=0)
 *   r      radius of sphere in feet (optional, default=RADIUS_EARTH_FT)
 *
 * In/Out:
 *   lat   pointer to latitude converted from y
 *   lon   pointer to longitude converted from x
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void xy_to_latlon(const double& x, const double& y,
		          double* const lat, double* const lon,
                  const double& lat0, const double& lon0,
                  const double& x0, const double& y0,
                  const double& r) {

	if(!lat) return;
	if(!lon) return;

	double deg = 180. / M_PI;
	double rad = M_PI / 180.;

	double dnorth = y-y0;
	double deast = x-x0;
	double dlat = dnorth / r * deg;
	double dlon = deast / r / cos(lat0*rad) * deg;
	*lat = lat0 + dlat;
	*lon = lon0 + dlon;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   magnitude()
 *
 * Description:
 *   Compute the vector magnitude.
 *
 * Inputs:
 *   x   x component of the vector
 *   y   y component of the vector
 *   z   z component of the vector
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   The magnitude of the vector
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double magnitude(const double& x, const double& y, const double& z) {
	return sqrt(x*x + y*y + z*z);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   dot_product()
 *
 * Description:
 *   Compute the vector dot product.
 *
 * Inputs:
 *   x1   x component of vector 1
 *   y1   y component of vector 1
 *   z1   z component of vector 1
 *   x2   x component of vector 2
 *   y2   y component of vector 2
 *   z2   z component of vector 2
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   The dot product of vector 1 and vector 2
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double dot_product(const double& x1, const double& y1, const double& z1,
		           const double& x2, const double& y2, const double& z2) {
	return (x1*x2) + (y1*y2) + (z1*z2);
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   cross_product()
 *
 * Description:
 *   Compute the vector cross product.
 *   Note: the output vector is not normalized.
 *
 * Inputs:
 *   x1   x component of vector 1
 *   y1   y component of vector 1
 *   z1   z component of vector 1
 *   x2   x component of vector 2
 *   y2   y component of vector 2
 *   z2   z component of vector 2
 *
 * In/Out:
 *   xcross  pointer to the x component of the result vector
 *   ycross  pointer to the y component of the result vector
 *   zcross  pointer to the z component of the result vector
 *
 * Returns:
 *   none
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void cross_product(const double& x1, const double& y1, const double& z1,
		           const double& x2, const double& y2, const double& z2,
		           double* const xcross, double* const ycross,
		           double* const zcross) {

	if(!xcross) return;
	if(!ycross) return;
	if(!zcross) return;

	*xcross = (y1*z2 - z1*y2);
	*ycross = (z1*x2 - x1*z2);
	*zcross = (x1*y2 - y1*x2);
}

/*
 * ---------------------------------------------------------------------------
 * Function:
 *   ch_are_last_three_left()
 *
 * Description:
 *   Convex hull helper function:
 *
 *   return true if last three points in the partial hull are LEFT
 *   turns.
 *   this function assumes that there are at least three points in
 *   the hull.  we do not check the length.
 *
 *   the turn direction is left if the cross product between the last
 *   three points is positive.  We compute the turn direction using
 *   the determinant of the 3x3 matrix:
 *     [1  L[i-1].x  L[i-1].y]
 *     [1  L[i].x    L[i].y  ]
 *     [1  p.x       p.y     ]
 *   where L[i-1] is the third to last point,
 *   L[i] is the second to last point, and
 *   p is the candidate last point.
 *
 *   So, cp = (L[i].x-L[i-1].x)(p.y-L[i-1].y)-(L[i].y-L[i-1].y)(p.x-L[i-1].x)
 *
 * Inputs:
 *   hull   the vector of points in the partial hull to check
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   true if the last three points in the hull form a left turn,
 *   false otherwise.
 * ---------------------------------------------------------------------------
 */
bool ch_are_last_three_left(const vector<hull_point>& hull) {
	int end = hull.size()-1;
	const hull_point* p = &(hull.at(end));
	const hull_point* L_i = &(hull.at(end-1));
	const hull_point* L_i_1 = &(hull.at(end-2));
	double cp = (L_i->x - L_i_1->x) * (p->y - L_i_1->y) -
			    (L_i->y - L_i_1->y) * (p->x - L_i_1->x);
	if(cp >= 0) return true;
	return false;
}

/*
 * ---------------------------------------------------------------------------
 * Function:
 *   ch_point_comparator()
 *
 * Description:
 *   Comparator function for sorting hull points by x-coordinate.  The
 *   y-coordinate is used to break ties.
 *
 * Inputs:
 *   a   the first hull point
 *   b   the second hull point
 *
 * In/Out:
 *   none
 *
 * Returns:
 *   true if the x-coordinate of a is less than the x-coordinate of b.
 *   true if the y-coordinate of a is less than the y-coordinate of b and
 *     the x-coordinates of a and b are equal.
 *   false otherwise.
 * ---------------------------------------------------------------------------
 */
static bool ch_point_comparator(hull_point a, hull_point b) {
	if(a.x != b.x) {
		return a.x < b.x;
	} else {
		return a.y < b.y;
	}
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   convex_hull_scan()
 *
 * Description:
 *   Compute the convex hull of a set of 2D points using the scan algorithm.
 *
 * Inputs:
 *   n       the number of points in the non-convex set
 *   xdata   the array of x-coordinates in the non-convex set
 *   ydata   the array of y-coordinates in the non-convex set
 *
 * In/Out:
 *   xconvex   pointer to an STL vector for holding the convex hull's
 *             x coordinates
 *   yconvex   pointer to an STL vector for holding the convex hull's
 *             y coordinates
 *
 * Returns:
 *   The number of points in the convex hull
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
int convex_hull_scan(int n,
		             const double* const xdata,
		             const double* const ydata,
		             vector<double>* const xconvex, vector<double>* const yconvex) {

	if(!xconvex) return -1;
	if(!yconvex) return -1;

	// store x,y data as vector of points
	vector<hull_point> points;
	for(int i=0; i<n; ++i) {
		points.push_back(hull_point(xdata[i], ydata[i]));
	}

	// sort points by x coordinate, use y to break ties
	sort(points.begin(), points.end(), ch_point_comparator);

	// if there are 3 or fewer points then the polygon is
	// already convex
	if(n <= 3) {
		for(int i=0; i<n; ++i) {
			xconvex->push_back(points.at(i).x);
			yconvex->push_back(points.at(i).y);
		}
		return n;
	}

	// compute the upper hull by starting with the left-most point
	// and ending with the right-most point.  since the points are
	// sorted in order by ascending x, we iterate forward through
	// the vector of points.
	vector<hull_point> upper;
	upper.push_back(points.at(0));
	upper.push_back(points.at(1));
	for(int i=2; i<n; ++i) {
		upper.push_back(points.at(i));
		while(upper.size() >= 3 && ch_are_last_three_left(upper)) {
			// remove the middle of the last three:
			// perhaps there is a better data structure to use
			// than a vector if we need to perform lots of erases
			// from the middle of the vector.
			upper.erase( upper.end()-2 );
		}
	}

	// compute the lower hull by starting with the right-most point
	// and ending with the left-most point.  since the points
	// are sorted in order of ascending x, we iterate backward through
	// the vector of points.
	//int end = points.size()-1;
	vector<hull_point> lower;
	lower.push_back(points.at(n-1));
	lower.push_back(points.at(n-2));
	for(int i=n-3; i>=0; --i) {
		lower.push_back(points.at(i));
		while(lower.size() >= 3 && ch_are_last_three_left(lower)) {
			// remove the middle of the last three:
			// perhaps there is a better data structure to use
			// than a vector if we need to perform lots of erases
			// from the middle of the vector.
			lower.erase( lower.end()-2 );
		}
	}

	// combine the upper and lower hulls.  remove duplicate endpoints.
	int num_hull_points = upper.size() + lower.size() - 2;
	for(unsigned int i=0; i<upper.size(); ++i) {
		xconvex->push_back(upper.at(i).x);
		yconvex->push_back(upper.at(i).y);
	}
	for(unsigned int i=1; i<lower.size()-1; ++i) {
		xconvex->push_back(lower.at(i).x);
		yconvex->push_back(lower.at(i).y);
	}

	return num_hull_points;
}

string convertLatLonDeg_to_degMinSecString(const double degValue) {
	string retString;

	int intDeg = 0;
	if (0 < degValue) {
		intDeg = (int)floor(degValue);
	} else {
		intDeg = (int)ceil(degValue);
	}

	double fraction = 0;
	if (0 < degValue) {
		fraction = degValue - intDeg;
	} else {
		fraction = intDeg - degValue;
	}

	int intMin = (int)floor(fraction * 60);

	double dblSec = (fraction*60 - intMin) * 60;

	stringstream tmpSS;
	tmpSS.str("");

	if (abs(intDeg) < 10)
		tmpSS << "0";
	tmpSS << intDeg;

	if (intMin < 10)
		tmpSS << "0";
	tmpSS << intMin;

	if (dblSec < 10)
		tmpSS << "0";
	tmpSS << dblSec;

	retString.assign(tmpSS.str());

	return retString;
}

double convertLatLonString_to_deg(const char* entry) {
	string str_Input(entry);

	int sign = 1;

	if ((str_Input.find("-") == 0) || (str_Input.find("S") != string::npos) || (str_Input.find("W") != string::npos)) {
		sign = -1;
	}

	char* decimalstr_ptr = NULL;

	int value_ddmm = 0;
	double value_ddmmss = 0;
	double value_decimal = 0;

	if (str_Input.find(".") == string::npos) {
		value_ddmm = abs(atoi(entry));

		if (value_ddmm < 10000) {
			value_ddmmss = value_ddmm * 100;
		} else {
			value_ddmmss = value_ddmm;
		}
	} else {
		value_ddmmss = abs(strtod(entry, NULL));
		value_decimal = value_ddmmss - floor(value_ddmmss);
	}

	double degVal_dd = floor(value_ddmmss / 10000.);

	double degVal_mm = floor((value_ddmmss - degVal_dd * 10000) / 100) / 60;

	double degVal_ss = (((int)value_ddmmss % 100) + value_decimal) / 3600;

	double retValue = degVal_dd + degVal_mm + degVal_ss;
	retValue = retValue * sign;

	return retValue;
}

} /* namespace osi */

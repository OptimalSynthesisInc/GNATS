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
 * geometry_utils.h
 *
 *  Created on: May 31, 2012
 *      Author: jason
 */

#ifndef GEOMETRY_UTILS_H_
#define GEOMETRY_UTILS_H_

#include <vector>
#include <cstdlib>
#include <limits>
#include <string>

using namespace std;

namespace osi {


/*
 * Constants
 */
extern double RADIUS_EARTH_FT;

double compute_distance_gc_rad(const double& latRad1,
						const double& lonRad1,
                        const double& latRad2,
						const double& lonRad2,
                        const double& alt,
                        const double& r);

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
 *   The great circle distance between lat/lon points in feet.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
double compute_distance_gc(const double& lat1, const double& lon1,
                           const double& lat2, const double& lon2,
                           const double& alt=0.,
                           const double& r=RADIUS_EARTH_FT);

double compute_heading_rad_gc(const double& lat1, const double& lon1,
			              const double& lat2, const double& lon2);

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
			              const double& lat2, const double& lon2);

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
						   double* const lat=NULL, double* const lon=NULL,
						   const double& alt=0., const double& r=RADIUS_EARTH_FT);

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function:
 *   compute_distance()
 *
 * Description:
 *   Compute 2D euclidean distance between two points.
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
		                const double& x2, const double& y2);

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
		                   double* const lat_int=NULL,
		                   double* const lon_int=NULL);

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
		                double* const xint=NULL, double* const yint=NULL,
		                const double& epsilon=numeric_limits<double>::min());

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
                  const double& lat0=0, const double& lon0=0,
                  const double& x0=0, const double& y0=0,
                  const double& r=RADIUS_EARTH_FT);

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
                  const double& lat0=0, const double& lon0=0,
                  const double& x0=0, const double& y0=0,
                  const double& r=RADIUS_EARTH_FT);

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
double magnitude(const double& x, const double& y, const double& z);

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
		           const double& x2, const double& y2, const double& z2);

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
		           double* const xcross=NULL,
		           double* const ycross=NULL,
		           double* const zcross=NULL);

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
	                 vector<double>* const xconvex=NULL,
	                 vector<double>* const yconvex=NULL);

/**
 * Convert latitude/longitude degree value to degree-minute-second string
 */
string convertLatLonDeg_to_degMinSecString(const double degValue);

/**
 * Convert latitude/longitude string to degree value
 */
double convertLatLonString_to_deg(const char* entry);

} /* namespace osi */

#endif /* GEOMETRY_UTILS_H_ */

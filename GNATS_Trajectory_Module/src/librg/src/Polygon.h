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
 * Polygon.h
 *
 *  Created on: May 18, 2012
 *      Author: jason
 */

#ifndef POLYGON_H_
#define POLYGON_H_

#include <vector>
#include <cstdlib>
#include <string>

using std::vector;
using std::string;

namespace osi {

class Polygon {
public:
	Polygon();
	Polygon(const double* const x_data,
			const double* const y_data,
			const int& num_vertices,
			const bool& ccw_flag=false);
	Polygon(const vector<double>& x_data,
			const vector<double>& y_data,
			const int& num_vertices,
			const bool& ccw_flag=false);
	Polygon(const Polygon& that);
	virtual ~Polygon();

	/*
	 * Return the nubmer of vertices of this polygon
	 */
	const int& getNumVertices() const;

	/*
	 * Get this polygon's x data
	 */
	const double* getXData() const;

	/*
	 * Get this polygon's y data
	 */
	const double* getYData() const;

	/*
	 * Get the ccw flag
	 */
	const bool& getCCWFlag() const;

	const double& getXMin() const;
	const double& getYMin() const;
	const double& getXMax() const;
	const double& getYMax() const;

	void getCentroid(double* const x_centroid, double* const y_centroid) const;

	/*
	 * Output the scaled copy of this polygon
	 */
	void scale(const double& sx, const double& sy, Polygon* const scaled);

	/*
	 * Output the convex hull of this polygon
	 */
	void convexHull(Polygon* const convex) const;

	/*
	 * Determine if this polygon contains the given point
	 */
	bool contains(const double& x, const double& y,
			      const bool& gcFlag=true) const;

	/*
	 * Determine if this polygon intersects the given line segment
	 */
	bool intersectsSegment(const double& x1, const double& y1,
			               const double& x2, const double& y2,
			               double* const xint=nullptr, double* const yint=nullptr,
			               const bool& gcFlag=true) const;

	/*
	 * Assignment operator
	 */
	Polygon& operator=(const Polygon& that);

	/*
	 * TODO:PARIKSHIT ADDER TO PRINT THE MAX VALUES
	 */
	void printMaxVals() const;

	/*
	* TODO:PARIKSHIT ADDER SET POLY TYPE
	*/
	void setPolyType(const string& ptype);

	/*
	* TODO:PARIKSHIT ADDER RETURN POLY TYPE
	*/
	const string& getPolyType() const;

	/*
	* TODO:PARIKSHIT ADDER SET START HOUR
	*/
	void setStartHour(const int& start_hr);

	/*
	* TODO:PARIKSHIT ADDER RETURN START HOUR
	*/
	const int& getStartHour() const;

	/*
	* TODO:PARIKSHIT ADDER SET END HOUR
	*/
	void setEndHour(const int& end_hr);

	/*
	* TODO:PARIKSHIT ADDER RETURN END HOUR
	*/
	const int& getEndHour() const;

protected:

	// vertex data
	double* x_data;
	double* y_data;
	int num_vertices;
	bool ccw_flag;

	// bounding box definition
	double xmin;
	double xmax;
	double ymin;
	double ymax;

	// centroid
	double x_centroid;
	double y_centroid;

	/*
	 *TODO:PARIKSHIT ADDER FOR TYPE OF WEATHER POLYGON.
	 */
	string poly_type;
	int start_hr;
	int end_hr;
};

}

#endif /* POLYGON_H_ */

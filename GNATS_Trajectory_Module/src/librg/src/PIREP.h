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
 * PIREP.h
 *
 *  Created on: Apr 21, 2017
 *      Author: pdutta
 */

#ifndef _PIREP_H_
#define _PIREP_H_

#include <string>
#define feetToNauticalMile 0.000164579
using std::string;

namespace osi{

class PIREP{
public:
	PIREP();
	PIREP(const PIREP& that);
	PIREP(const string& wpname, const bool& urgent_flag, const int& obs_time,
			const  double& angle, const double &dist,const int& flt_lev,const string& phenom = "TB",
			const int& severity = 1,const int& t_diff=-100);

	PIREP& operator=(const PIREP& that);
	virtual ~PIREP();

	//PLACE HOLDER CAN BE MADE PRIVATE MEMBERS
	// NO SETTER METHOD AS EVERYTHING IS DONE VIA CONSTRUCTOR.

	const string& getWaypoint() const;

	const bool& getUrgentFlag() const;

	const int& getObsTime() const;

	void setObsTime(const int& o_time);

	const double& getAngle() const;

	const double& getDistance() const;

	const int& getFlightLevel() const;

	void setLatitude(const double& latitude);

	const double& getLatitude() const;

	void setLongitude(const double& longitude);

	const double& getLongitude() const;

	const string& getWeatherPhenom() const;

	const int& getSeverity() const;

	bool nearFlag(const double& lat_p, const double& lon_p,
			const double& rad, const int& curr_tm = -10);


protected:
	string wpname;
	bool urgent_flag;
	int	obs_time;
	double angle;
	double dist;
	int flt_lev;
	double latitude;
	double longitude;
	string w_phenom;
	int severity;
	int t_diff;

private:
	bool isWithinObservationWindow(const int& tm = -10);
	double addTime(const double& time1, const double& diff_tm);
};

}


#endif /* _PIREP_H_ */

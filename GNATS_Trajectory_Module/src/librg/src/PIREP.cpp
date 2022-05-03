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
 * PIREP.cpp
 *
 *  Created on: Apr 21, 2017
 *      Author: pdutta
 */

#include "PIREP.h"
#include "geometry_utils.h"
#include <iostream>


namespace osi{
PIREP::PIREP():
		wpname(""),
		urgent_flag(false),
		obs_time(0000),
		angle(0.0),
		dist(0.0),
		flt_lev(0),
		latitude(-1000.0),
		longitude(-1000.0),
		w_phenom("TB"),
		severity(1),
		t_diff(1){}
PIREP::PIREP(const string& wpname, const bool& urgent_flag, const int& obs_time,
		const  double& angle, const double &dist,const int& flt_lev,const string& phenom, const int& severity,
		const int& t_diff):
				wpname(wpname),
				urgent_flag(urgent_flag),
				obs_time(obs_time),
				angle(angle),
				dist(dist),
				flt_lev(flt_lev),
				latitude(-1000.0),
				longitude(-1000.0),
				w_phenom(phenom),
				severity(severity),
				t_diff(t_diff){
	if (t_diff <= 0){
		if (urgent_flag){
			this->t_diff = 2;}
		else{
			this->t_diff = 1;}
	}
}
PIREP::PIREP(const PIREP& that):
		wpname(that.wpname),
		urgent_flag(that.urgent_flag),
		obs_time(that.obs_time),
		angle(that.angle),
		dist(that.dist),
		flt_lev(that.flt_lev),
		latitude(that.latitude),
		longitude(that.longitude),
		w_phenom(that.w_phenom),
		severity(that.severity),
		t_diff(that.t_diff){}

PIREP& PIREP::operator=(const PIREP& that){
	if(this == &that) return *this;
	this->wpname = that.wpname;
	this->urgent_flag = that.urgent_flag;
	this->obs_time = that.obs_time;
	this->angle = that.angle;
	this->dist = that.dist;
	this->flt_lev = that.flt_lev;
	this->latitude = that.latitude;
	this->longitude = that.longitude;
	this->w_phenom = that.w_phenom;
	this->severity = that.severity;
	this->t_diff = that.t_diff;
	return *this;
}

PIREP::~PIREP(){
}

const string& PIREP::getWaypoint() const{
	return wpname;
}

const bool& PIREP::getUrgentFlag() const{
	return urgent_flag;
}

const int& PIREP::getObsTime() const{
	return obs_time;
}

void PIREP::setObsTime(const int& o_time){
	this->obs_time = o_time;
}

const double& PIREP::getAngle() const{
	return angle;
}

const double& PIREP::getDistance() const{
	return dist;
}

const int& PIREP::getFlightLevel() const{
	return flt_lev;
}

void PIREP::setLatitude(const double& latitude){
	this->latitude = latitude;
}

const double& PIREP::getLatitude() const{
	return latitude;
}

void PIREP::setLongitude(const double& longitude){
	this->longitude = longitude;
}

const double& PIREP::getLongitude() const{
	return longitude;
}

const string& PIREP::getWeatherPhenom() const{
	return w_phenom;
}

const int& PIREP::getSeverity() const{
	return severity;
}

bool PIREP::nearFlag(const double& lat_p, const double& lon_p,
		const double& rad, const int& tm){

	if (!isWithinObservationWindow(tm))
		return false;

	double distance = compute_distance_gc(lat_p,lon_p, latitude,longitude, 0.0)*feetToNauticalMile;
	if (distance >= rad)
		return false;

	return true;



}

bool PIREP::isWithinObservationWindow(const int& tm){

	if(tm <=0.0)
		return true;

	int chktm = this->obs_time + this->t_diff;

	if (tm<obs_time)
		return false;
	else if (tm < chktm && tm >= obs_time)
		return true;
	else
		return false;

}

double PIREP::addTime(const double& time1, const double& diff_tm){

	 int hr_to_add = (int)(diff_tm)/60;
	 int min_to_add = int(diff_tm)%60;
	 double tm_to_add = hr_to_add*100+ min_to_add;
	 double chktm = time1 + tm_to_add;

	 if ((int)(chktm)%100 > 60 ){
	   double mins_over_60 = (int)(chktm)%100-60;
	   chktm = chktm - (int)(chktm)%100 + 100 + mins_over_60;

	 }

	 return chktm;
}

}

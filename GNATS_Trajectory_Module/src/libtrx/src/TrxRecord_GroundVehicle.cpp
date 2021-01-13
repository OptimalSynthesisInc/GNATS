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

#include "TrxRecord_GroundVehicle.h"

#include "TrxUtils.h"

#include <string>

using std::string;


namespace osi {

TrxRecord_GroundVehicle::TrxRecord_GroundVehicle() :
	timestamp(UNSET_LONG),
	vehicle_id(UNSET_STRING),
	aircraft_id(UNSET_STRING),
	airport_id(UNSET_STRING),
	latitude(UNSET_DOUBLE),
	longitude(UNSET_DOUBLE),
	speed(UNSET_DOUBLE),
	altitude(UNSET_DOUBLE),
	course(UNSET_DOUBLE),
	route_str(UNSET_STRING) {
}

TrxRecord_GroundVehicle::TrxRecord_GroundVehicle(const long& timestamp,
		     const string& vehicle_id,
			 const string& aircraft_id,
			 const string& airport_id,
		     const double& latitude,
		     const double& longitude,
		     const double& speed,
		     const double& altitude,
		     const double& course,
		     const string& route_str,
			 const string& trx_str) :
	timestamp(timestamp),
	vehicle_id(vehicle_id),
	aircraft_id(aircraft_id),
	airport_id(airport_id),
	latitude(latitude),
	longitude(longitude),
	speed(speed),
	altitude(altitude),
	course(course),
	route_str(route_str),
	trx_str(UNSET_STRING) {
}

TrxRecord_GroundVehicle::TrxRecord_GroundVehicle(const TrxRecord_GroundVehicle& that) :
	timestamp(that.timestamp),
	vehicle_id(that.vehicle_id),
	aircraft_id(that.aircraft_id),
	airport_id(that.airport_id),
	latitude(that.latitude),
	longitude(that.longitude),
	speed(that.speed),
	altitude(that.altitude),
	course(that.course),
	route_str(that.route_str),
	trx_str(that.trx_str) {
}

TrxRecord_GroundVehicle::~TrxRecord_GroundVehicle() {

}

TrxRecord_GroundVehicle& TrxRecord_GroundVehicle::operator=(const TrxRecord_GroundVehicle& that) {
	if (this == &that) return *this;

	this->timestamp = that.timestamp;
	this->vehicle_id = that.vehicle_id;
	this->aircraft_id = that.aircraft_id;
	this->airport_id = that.airport_id;
	this->latitude = that.latitude;
	this->longitude = that.longitude;
	this->speed = that.speed;
	this->altitude = that.altitude;
	this->course = that.course;
	this->route_str = that.route_str;
	this->trx_str = that.trx_str;

	return *this;
}

bool TrxRecord_GroundVehicle::operator==(const TrxRecord_GroundVehicle& that) const {
	if (this == &that) return true;

	if (this->timestamp != that.timestamp) return false;
	if (this->vehicle_id != that.vehicle_id) return false;
	if (this->aircraft_id != that.aircraft_id) return false;
	if (this->airport_id != that.airport_id) return false;
	if (this->latitude != that.latitude) return false;
	if (this->longitude != that.longitude) return false;
	if (this->speed != that.speed) return false;
	if (this->altitude != that.altitude) return false;
	if (this->course != that.course) return false;
	if (this->route_str != that.route_str) return false;
	if (this->trx_str != that.trx_str) return false;

	return true;
}

int TrxRecord_GroundVehicle::operator<(const TrxRecord_GroundVehicle& that) const {
	return this->timestamp < that.timestamp;
}

const string& TrxRecord_GroundVehicle::getVehicle_id() const {
	return vehicle_id;
}

const string& TrxRecord_GroundVehicle::getAircraft_id() const {
	return aircraft_id;
}

const string& TrxRecord_GroundVehicle::getAirport_id() const {
	return airport_id;
}

const double& TrxRecord_GroundVehicle::getAltitude() const {
	return altitude;
}

const double& TrxRecord_GroundVehicle::getCourse() const {
	return course;
}

const double& TrxRecord_GroundVehicle::getLatitude() const {
	return latitude;
}

const double& TrxRecord_GroundVehicle::getLongitude() const {
	return longitude;
}

const string& TrxRecord_GroundVehicle::getRoute_str() const {
	return route_str;
}

const double& TrxRecord_GroundVehicle::getSpeed() const {
	return speed;
}

const long& TrxRecord_GroundVehicle::getTimestamp() const {
	return timestamp;
}

void TrxRecord_GroundVehicle::setVehicle_id(string vehicle_id)
{
	this->vehicle_id = vehicle_id;
}

void TrxRecord_GroundVehicle::setAircraft_id(string aircraft_id)
{
	this->aircraft_id = aircraft_id;
}

void TrxRecord_GroundVehicle::setAirport_id(string airport_id)
{
	this->airport_id = airport_id;
}

void TrxRecord_GroundVehicle::setAltitude(double altitude)
{
	this->altitude = altitude;
}

void TrxRecord_GroundVehicle::setCourse(double course)
{
	this->course = course;
}

void TrxRecord_GroundVehicle::setLatitude(double latitude)
{
	this->latitude = latitude;
}

void TrxRecord_GroundVehicle::setLongitude(double longitude)
{
	this->longitude = longitude;
}

void TrxRecord_GroundVehicle::setRoute_str(string route_str)
{
	this->route_str = route_str;
}

void TrxRecord_GroundVehicle::setSpeed(double speed)
{
	this->speed = speed;
}

void TrxRecord_GroundVehicle::setTimestamp(long timestamp)
{
	this->timestamp = timestamp;
}

void TrxRecord_GroundVehicle::setTrx_str(string trx_str)
{
    this->trx_str = trx_str;
}

}

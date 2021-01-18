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
 * TrxRecord.cpp
 *
 *  Created on: May 4, 2012
 *      Author: jason
 */

#include "TrxRecord.h"
#include "TrxRoute.h"
#include "TrxUtils.h"


#include <string>

using std::string;


namespace osi {



TrxRecord::TrxRecord() :
	timestamp(UNSET_LONG),
	acid(UNSET_STRING),
	actype(UNSET_STRING),
	latitude(UNSET_DOUBLE),
	longitude(UNSET_DOUBLE),
	tas(UNSET_DOUBLE),
	altitude(UNSET_DOUBLE),
	heading(UNSET_DOUBLE),
	center(UNSET_STRING),
	sector(UNSET_STRING),
	route_str(UNSET_STRING),
	cruiseAltitude(UNSET_DOUBLE) {
}

TrxRecord::TrxRecord(const long& timestamp,
		     const string& acid,
		     const string& actype,
		     const double& latitude,
		     const double& longitude,
		     const double& tas,
		     const double& altitude,
		     const double& heading,
		     const string& center,
		     const string& sector,
		     const string& route_str,
		     const double& cruiseAltitude) :
	timestamp(timestamp),
	acid(acid),
	actype(actype),
	latitude(latitude),
	longitude(longitude),
	tas(tas),
	altitude(altitude),
	heading(heading),
	center(center),
	sector(sector),
	route_str(route_str),
	trx_str(UNSET_STRING),
	cruiseAltitude(cruiseAltitude) {

	parseRouteString();
}

TrxRecord::TrxRecord(const long& timestamp,
             const string& acid,
             const string& actype,
             const double& latitude,
             const double& longitude,
             const double& tas,
             const double& altitude,
             const double& heading,
             const string& center,
             const string& sector,
             const string& route_str,
             const string& trx_str,
             const double& cruiseAltitude) :
    timestamp(timestamp),
    acid(acid),
    actype(actype),
    latitude(latitude),
    longitude(longitude),
    tas(tas),
    altitude(altitude),
    heading(heading),
    center(center),
    sector(sector),
    route_str(route_str),
    trx_str(trx_str),
    cruiseAltitude(cruiseAltitude) {

    parseRouteString();
}

TrxRecord::TrxRecord(const TrxRecord& that) :
	timestamp(that.timestamp),
	acid(that.acid),
	actype(that.actype),
	latitude(that.latitude),
	longitude(that.longitude),
	tas(that.tas),
	altitude(that.altitude),
	heading(that.heading),
	center(that.center),
	sector(that.sector),
	route_str(that.route_str),
	trx_str(that.trx_str),
	cruiseAltitude(that.cruiseAltitude),
	flag_geoStyle(that.flag_geoStyle) {

	parseRouteString();
}

TrxRecord::~TrxRecord() {

}

void TrxRecord::parseRouteString() {
	if(route_str != UNSET_STRING) {
		parse_trx_route(route_str, &route);
	}
}

TrxRecord& TrxRecord::operator=(const TrxRecord& that) {
	if(this == &that) return *this;

	this->timestamp = that.timestamp;
	this->acid = that.acid;
	this->actype = that.actype;
	this->latitude = that.latitude;
	this->longitude = that.longitude;
	this->tas = that.tas;
	this->altitude = that.altitude;
	this->heading = that.heading;
	this->center = that.center;
	this->sector = that.sector;
	this->route_str = that.route_str;
	this->trx_str = that.trx_str;
	this->cruiseAltitude = that.cruiseAltitude;
	this->flag_geoStyle = that.flag_geoStyle;

	return *this;
}

bool TrxRecord::operator==(const TrxRecord& that) const {
	if(this == &that) return true;

	if(this->timestamp != that.timestamp) return false;
	if(this->acid != that.acid) return false;
	if(this->actype != that.actype) return false;
	if(this->latitude != that.latitude) return false;
	if(this->longitude != that.longitude) return false;
	if(this->tas != that.tas) return false;
	if(this->altitude != that.altitude) return false;
	if(this->heading != that.heading) return false;
	if(this->center != that.center) return false;
	if(this->sector != that.sector) return false;
	if(this->route_str != that.route_str) return false;
	if(this->trx_str != that.trx_str) return false;
	if(this->cruiseAltitude != that.cruiseAltitude) return false;
	if(this->flag_geoStyle != that.flag_geoStyle) return false;

	return true;
}

int TrxRecord::operator<(const TrxRecord& that) const {
	return this->timestamp < that.timestamp;
}

const string& TrxRecord::getAcid() const {
	return acid;
}

const string& TrxRecord::getActype() const {
	return actype;
}

const double& TrxRecord::getAltitude() const {
	return altitude;
}

const string& TrxRecord::getCenter() const {
	return center;
}

const double& TrxRecord::getHeading() const {
	return heading;
}

const double& TrxRecord::getLatitude() const {
	return latitude;
}

const double& TrxRecord::getLongitude() const {
	return longitude;
}

const string& TrxRecord::getRoute_str() const {
	return route_str;
}

const string& TrxRecord::getSector() const {
	return sector;
}

const double& TrxRecord::getTas() const {
	return tas;
}

const long& TrxRecord::getTimestamp() const {
	return timestamp;
}

const TrxRoute& TrxRecord::getRoute() const {
	return route;
}

const double& TrxRecord::getCruiseAltitude() const {
    return cruiseAltitude;
}

const string& TrxRecord::getTrx_str() const {
    return trx_str;
}

void TrxRecord::setAcid(string acid)
{
	this->acid = acid;
}

void TrxRecord::setActype(string actype)
{
	this->actype = actype;
}

void TrxRecord::setAltitude(double altitude)
{
	this->altitude = altitude;
}

void TrxRecord::setCenter(string center)
{
	this->center = center;
}

void TrxRecord::setHeading(double heading)
{
	this->heading = heading;
}

void TrxRecord::setLatitude(double latitude)
{
	this->latitude = latitude;
}

void TrxRecord::setLongitude(double longitude)
{
	this->longitude = longitude;
}

void TrxRecord::setRoute_str(string route)
{
	this->route_str = route;
}

void TrxRecord::setSector(string sector)
{
	this->sector = sector;
}

void TrxRecord::setTas(double tas)
{
	this->tas = tas;
}

void TrxRecord::setTimestamp(long  timestamp)
{
	this->timestamp = timestamp;
}

void TrxRecord::setTrx_str(string trx_str)
{
    this->trx_str = trx_str;
}

void TrxRecord::setRoute(TrxRoute& route) {
	this->route = route;
}

void TrxRecord::setCruiseAltitude(double cruiseAltitude) {
  this->cruiseAltitude = cruiseAltitude;
}

}

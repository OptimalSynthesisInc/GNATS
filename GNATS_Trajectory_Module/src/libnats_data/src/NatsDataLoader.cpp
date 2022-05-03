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

/**
 * NatsDataLoader.h
 * 
 * Class to load various encrypted NATS data files.
 *
 * Author: jason
 * Date: January 19, 2013
 */

#include "NatsDataLoader.h"

#include "lektor.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <typeinfo>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <unistd.h>
#include "NatsAirport.h"
#include "NatsAirway.h"
#include "NatsSector.h"
#include "NatsWaypoint.h"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::sort;
using std::pair;
using std::make_pair;

vector<NatsAirport> g_airports;

static void trim(string& str) {
	if(str.length() > 0) {
		str.erase(0, str.find_first_not_of(" "));
	}
	if(str.length() > 0) {
		str.erase(str.find_last_not_of(" ")+1);
	}
}



static void parse_sector_name(char* const line, 
		string* const name) {

	if(!line) return;
	if(!name) return;
	if(strlen(line) < 1) return;

	char cname[16];
	memset(cname, 0, 16*sizeof(char));

	char* lt = strchr(line, '<') + 1;
	char* rt = strchr(line, '>');
	int len = rt - lt;
	strncpy(cname, lt, len);

	*name = string(cname);
}

static void parse_sector_altitudes(char* const line, 
		double* const altitudeMin,
		double* const altitudeMax) {

	if(!line) return;
	if(!altitudeMin) return;
	if(!altitudeMax) return;
	if(strlen(line) < 1) return;

	// altitude range contained in :
	// [xxxxx.x,yyyyy.y]

	char minstr[9];
	char maxstr[9];
	memset(minstr, 0, 9*sizeof(char));
	memset(maxstr, 0, 9*sizeof(char));

	char* lb = strchr(line, '[') + 1;
	char* rb = strchr(line, ']');
	char* comma = strchr(line, ',');
	int lenmin = comma - lb;
	int lenmax = rb - comma+1;
	strncpy(minstr, lb, lenmin);
	strncpy(maxstr, comma+1, lenmax);

	*altitudeMin = atof(minstr);
	*altitudeMax = atof(maxstr);
}

static void parse_sector_vertex(char* const line,
		double* const latitude,
		double* const longitude) {
	if(!line) return;
	if(!latitude) return;
	if(!longitude) return;
	if(strlen(line) < 1) return;

	char lonstr[12];
	char latstr[12];
	memset(lonstr, 0, 12*sizeof(char));
	memset(latstr, 0, 12*sizeof(char));

	char* sp = strchr(line, ' ');
	int lenlon = sp - line;
	int lenlat = strlen(line) - lenlon - 1;
	strncpy(lonstr, line, lenlon);
	strncpy(latstr, sp+1, lenlat);

	*longitude = atof(lonstr) - 360.;
	*latitude = atof(latstr);
}

static void parse_waypoint_data(const char* line, 
		string* const name,
		double* const latitude,
		double* const longitude) {

	if(!line) return;
	if(!name) return;
	if(!latitude) return;
	if(!longitude) return;

	// a data line has the following format:
	// 0	SFO	N37 37 10.135	W122 22 26.008	0	0
	char cname[6];
	char latDir;
	int latDeg;
	int latMin;
	double latSec;
	char lonDir;
	int lonDeg;
	int lonMin;
	double lonSec;

	// parse the line
	sscanf(line, "%*s %s %c%d %d %lf %c%d %d %lf %*s %*s",
			cname, &latDir, &latDeg, &latMin, &latSec,
			&lonDir, &lonDeg, &lonMin, &lonSec);

	// convert degrees, minutes seconds to decimal degrees
	*latitude = (double)latDeg + (double)latMin/60. + (double)latSec/3600.;
	*longitude = (double)lonDeg + (double)lonMin/60. + (double)lonSec/3600.;

	// convert latDeg to degrees, N positive
	if(latDir == 'S') {
		*latitude *= -1.;
	}

	// convert lonDeg to degrees, E positive
	if(lonDir == 'W') {
		*longitude *= -1.;
	}

	*name = string(cname);
}

static void parse_airport_data(const char* line, 
		string* const name,
		string* const code,
		double* const latitude,
		double* const longitude,
		double* const elevation) {

	if(!line) return;
	if(!name) return;
	if(!code) return;
	if(!latitude) return;
	if(!longitude) return;
	if(!elevation) return;

	// a data line has the following format:
	// SAN_FRANCISCO_INTERN_CA	KSFO	N37 37 08.40 W122 22 29.40 13 16 0
	char cname[64];
	char ccode[5];
	char latDir;
	int latDeg;
	int latMin;
	double latSec;
	char lonDir;
	int lonDeg;
	int lonMin;
	double lonSec;
	double elev;

	// parse the line
	sscanf(line, "%s %s %c%d %d %lf %c%d %d %lf %lf %*s %*s",
			cname, ccode, &latDir, &latDeg, &latMin, &latSec,
			&lonDir, &lonDeg, &lonMin, &lonSec, &elev);

	// convert degrees, minutes seconds to decimal degrees
	*latitude = (double)latDeg + (double)latMin/60. + (double)latSec/3600.;
	*longitude = (double)lonDeg + (double)lonMin/60. + (double)lonSec/3600.;

	// convert latDeg to degrees, N positive
	if(latDir == 'S') {
		*latitude *= -1.;
	}

	// convert lonDeg to degrees, E positive
	if(lonDir == 'W') {
		*longitude *= -1.;
	}

	*name = string(cname);
	*code = string(ccode);
	*elevation = elev;
}

static void parse_airway_data(char* const line,
		string* const name,
		vector<string>* const route) {

	if(!line) return;
	if(!name) return;
	if(!route) return;

	// a data line has the following format:
	// name [waypoint_list]
	char* saveptr = NULL;
	char* tok = strtok_r(line, " \n", &saveptr);
	if(tok) {
		*name = string(tok);
	}
	while(tok) {
		tok = strtok_r(NULL, " \n", &saveptr);
		if(tok) {
			route->push_back(string(tok));
		}
	}
}

static void parse_string_fixed_width(char* line, char* const buf, 
		int col, int len) {
	if(!line) return;
	if(!buf) return;
	if(col < 0) return;

	// copy len chars from line to buf, starting at col
	if(col > (int)(strlen(line))) return;
	if((col+len) > (int)strlen(line)) return;
	memcpy(buf, line+col, len*sizeof(char));

	// remove trailing whitespace from buf
	for(int i=strlen(buf)-1; i>=0; --i) {
		if(isspace(buf[i])) {
			buf[i] = '\0';
		} else {
			break;
		}
	}
}

void parse_sid_star_latitude(char* const latstr, double* const latitude) {
	// convert string in form [N/S]DDMMSSs to decimal degrees, N positive
	// zero at the equator.
	char cdeg[3];
	char cmin[3];
	char csec[4];
	memset(cdeg, 0, 3*sizeof(char));
	memset(cmin, 0, 3*sizeof(char));
	memset(csec, 0, 4*sizeof(char));
	strncpy(cdeg, latstr+1, 2*sizeof(char));
	strncpy(cmin, latstr+3, 2*sizeof(char));
	strncpy(csec, latstr+5, 3*sizeof(char));

	double deg = atof(cdeg);
	double min = atof(cmin);
	double sec = atof(csec)/10.;

	*latitude = deg + min/60. + sec/3600.;
	if(latstr[0] == 'S') *latitude *= -1;
}

void parse_sid_star_longitude(char* const lonstr, double* const longitude) {
	char cdeg[4];
	char cmin[3];
	char csec[4];
	memset(cdeg, 0, 4*sizeof(char));
	memset(cmin, 0, 3*sizeof(char));
	memset(csec, 0, 4*sizeof(char));
	strncpy(cdeg, lonstr+1, 3*sizeof(char));
	strncpy(cmin, lonstr+4, 2*sizeof(char));
	strncpy(csec, lonstr+6, 3*sizeof(char));

	double deg = atof(cdeg);
	double min = atof(cmin);
	double sec = atof(csec)/10.;

	*longitude = deg + min/60. + sec/3600.;
	if(lonstr[0] == 'W') *longitude *= -1;
}

void parse_sid_star_data(char* line, string* const name, string* const id,
		string* const facilityCode, string* const wpName,
		double* const latitude, double* const longitude,
		bool* const isSid) {

	// note: this function does not parse all data in the line.
	// we're not sure what some of the data pertains to.  for
	// example it appears that some jetroutes or victor airways
	// may be present for some waypoints.  not sure what to do
	// with these.

	// determine if the line is for sid or star.
	// if the first char is 'D' then departure (sid)
	// else star.
	if(line[0] == 'D') {
		*isSid = true;
	} else {
		*isSid = false;
	}

	// parse the id, starting at col 0, len 10
	char cid[11];
	memset(cid, 0, 11*sizeof(char));
	parse_string_fixed_width(line, cid, 0, 10);

	*id = string(cid);
	trim(*id);

	// parse the name, starting at col 36, len 13
	char cname[14];
	memset(cname, 0, 14*sizeof(char));
	parse_string_fixed_width(line, cname, 36, 13);

	*name = string(cname);
	trim(*name);

	// parse the facility code, starting at col 10, len 2
	char cfacility[3];
	memset(cfacility, 0, 3*sizeof(char));
	parse_string_fixed_width(line, cfacility, 10, 2);
	*facilityCode = string(cfacility);
	trim(*facilityCode);

	// parse the latitude string, starting at col 13, len 8
	char clatstr[9];
	memset(clatstr, 0, 9*sizeof(char));
	parse_string_fixed_width(line, clatstr, 13, 8);
	parse_sid_star_latitude(clatstr, latitude);

	// parse the longitude string, starting at col 21, len 9
	char clonstr[10];
	memset(clonstr, 0, 10*sizeof(char));
	parse_string_fixed_width(line, clonstr, 21, 9);
	parse_sid_star_longitude(clonstr, longitude);

	// parse the waypoint name, starting at col 30, len 6
	char cwpname[7];
	memset(cwpname, 0, 7*sizeof(char));
	parse_string_fixed_width(line, cwpname, 30, 6);

	*wpName = string(cwpname);
	trim(*wpName);
}

/*
 * TODO:PARIKSHIT ADDER FOR CONVERTING LAT LON STRING FROM CIFP INTO DOUBLE VALUE
 */

static void convertLatLon(const string& inpstr, double& val,bool latFlag){

	int multval = 1;
	if (latFlag){
		if ( inpstr.compare(0,1,"S") == 0)
			multval = -1;
		int deg = atoi( inpstr.substr(1,2).c_str());
		int min = atoi( inpstr.substr(3,2).c_str());
		int sec = atoi( inpstr.substr(5,4).c_str());
		val = (double)deg + (double)min/60.0 + (double)sec*0.01/3600.0;
		val = multval* val;
	}
	else{
		if ( inpstr.compare(0,1,"W") == 0)
			multval = -1;
		int deg = atoi( inpstr.substr(1,3).c_str());
		int min = atoi( inpstr.substr(4,2).c_str());
		int sec = atoi( inpstr.substr(6,4).c_str());
		val = (double)deg + (double)min/60.0 + (double)sec*0.01/3600.0;
		val = multval* val;

	}

}

static void convertElev(const string& inpstr, double& val){

	size_t len = inpstr.length();
	int multival = 1;
	if (inpstr.compare(0,1,"-") == 0)
		multival = -1;
	val = 1.00*atoi(inpstr.substr(1,len-1).c_str());
	val = multival*val;
}

static void convertMagVarDec(const string &inp_str,double &val){
	size_t len = inp_str.length();
	double mult = 1.0;
	if ( inp_str.compare(0,1,"E") == 0)
		mult = -1.0;
	string angstr = inp_str.substr(1,len-1);
	val = mult * atof(angstr.c_str())/10.0;
}

static double parse_alt_string(const string& str){
	if (str.length() == 0){
		return -10000;
	}
	else if (str == "UNKNN"){
		return -10000;
	}
	else{
		if (str.substr(0,2) == "FL"){
			string alt = str.substr(2,str.length()-2);
			double alt_d = atof(alt.c_str()) * 100.0;
			return alt_d;
		}
		else{
			double alt_d = atof(str.c_str());
			return alt_d;
		}

	}
}

static double parse_str(const string& str){
	if (str == "")
		return -10000;
	else
		return atof(str.c_str());
}

static inline bool isInteger(const string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * p ;
   strtol(s.c_str(), &p, 10) ;

   return (*p == 0) ;
}

// private class for storing temporary sid/star record data
class NatsSidStarRecord {
public:
	NatsSidStarRecord() :
		isSid(true),
		id(""),
		name(""),
		terminalSegmentNames(vector<string>()),
		terminalSegments(vector< vector<string> >()),
		transitionSegmentNames(vector<string>()),
		transitionSegments(vector< vector<string> >()),
		latitudes(map<string, double>()),
		longitudes(map<string, double>()) {
	}
	NatsSidStarRecord(const NatsSidStarRecord& that) :
		isSid(that.isSid),
		id(that.id),
		name(that.name),
		terminalSegmentNames(vector<string>(that.terminalSegmentNames)),
		terminalSegments(vector< vector<string> >()),
		transitionSegmentNames(vector<string>(that.transitionSegmentNames)),
		transitionSegments(vector< vector<string> >()),
		latitudes(map<string, double>(that.latitudes)),
		longitudes(map<string, double>(that.longitudes)) {

		for(unsigned int i=0; i<that.terminalSegments.size(); ++i) {
			terminalSegments.push_back(vector<string>(that.terminalSegments.at(i)));
		}
		for(unsigned int i=0; i<that.transitionSegments.size(); ++i) {
			transitionSegments.push_back(vector<string>(that.transitionSegments.
					at(i)));
		}
	}
	virtual ~NatsSidStarRecord() {
	}

	bool isSid;
	string id;
	string name;
	vector< string >         terminalSegmentNames;
	vector< vector<string> > terminalSegments;
	vector< string>          transitionSegmentNames;
	vector< vector<string> > transitionSegments;

	map<string, double> latitudes;
	map<string, double> longitudes;

	string terminalPoint;
};

NatsDataLoader::NatsDataLoader() {
}

NatsDataLoader::~NatsDataLoader() {
}

int NatsDataLoader::loadSectors(const string& fname,
		vector<NatsSector>* const sectors) {
	if (!sectors) return 0;

	int lineno = 0;
	int maxlen = 200;
	char line[200];
	int dataStartLine = 10;

	vector<string> vectorDecrypted = getVector_decrypted(fname.c_str());

	NatsSector sector;

	if (0 < vectorDecrypted.size()) {
		for (unsigned int i = 0; i < vectorDecrypted.size(); i++) {
			// read the next line. close file and exit if error
			lineno++;

			memset(line, 0, maxlen*sizeof(char));

			strcpy(line, vectorDecrypted.at(i).c_str());

			// if the line starts with % then its a comment.  skip.
			if (line[0] == '%') continue;

			/// skip blank lines
			if (strlen(line) < 1) continue;

			// data starts on line 10
			if (lineno < dataStartLine) continue;

			// the sector data file has the following format for each record:
			// 1 13 "ATC Sector Boundary" <sector-name>[altMin,altMax]xyzf
			// longitude latitude
			// ...
			// if the line is a new sector boundary line then:
			// if the current sector is non-null then add to vector
			// create a new nats sector instance.
			// otherwise add the lat/lon point to the current sector.
			//if(strncmp(line+7, "ATC Sector Boundary", 19) == 0) {
			//cout << lineno << ": " << line << endl;
			if (strstr(line, "ATC Sector Boundary") != NULL) {

				// add a copy of the sector to the vector
				if (sector.numVertices > 0) {
					sectors->push_back(sector);
				}

				// set the new sector name, and altitude range
				sector.name = "";
				sector.minAltitude = 0;
				sector.maxAltitude = 0;
				parse_sector_name(line, &sector.name);
				parse_sector_altitudes(line, &sector.minAltitude, &sector.maxAltitude);
#ifndef NDEBUG
				cout << "Parse sector: " << sector.name
						<< " [" << sector.minAltitude << "," << sector.maxAltitude << "]"
						<< endl;
#endif
				// reset the sector vertices
				sector.numVertices = 0;
				sector.latitudes.clear();
				sector.longitudes.clear();

				continue;

			} else {

				// add the lat/lon to the current sector
				double latitude, longitude;
				parse_sector_vertex(line, &latitude, &longitude);
				sector.latitudes.push_back(latitude);
				sector.longitudes.push_back(longitude);
				sector.numVertices = sector.latitudes.size();
#ifndef NDEBUG
				cout << std::fixed
						<< std::setw(12) << std::setprecision(6) << latitude << ","
						<< std::setw(12) << std::setprecision(6) << longitude
						<< endl;
#endif
			}
		}
	}

	// add a copy of the last sector to the vector
	if (sector.numVertices > 0) {
		sectors->push_back(sector);
	}

	return sectors->size();
}

int NatsDataLoader::loadWaypoints(const string& fname,
		vector<NatsWaypoint>* const waypoints) {

	if(!waypoints) return 0;

	// If we can't access the directory
	if ((fname.length() == 0) || ( access( fname.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open CIFP directory %s\n", fname.c_str());

		return -1;
	}

	size_t found = fname.find("CIFP");
	if (found != string::npos)
		return this->loadWaypointsCIFP(fname,waypoints);

	printf("      Failed to open CIFP directory %s\n", fname.c_str());

	return -1;
}


int NatsDataLoader::loadAirports(const string& fname,
		vector<NatsAirport>* const airports) {

	if(!airports) return 0;

	// If we can't access the directory
	if ((fname.length() == 0) || ( access( fname.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open CIFP directory %s\n", fname.c_str());

		return -1;
	}

	size_t found = fname.find("CIFP");
	if (found != string::npos)
		return this->loadAirportsCIFP(fname,airports);
}


int NatsDataLoader::loadAirways(const string& fname,
		vector<NatsAirway>* const airways) {

	if(!airways) return 0;

	// If we can't access the directory
	if ((fname.length() == 0) || ( access( fname.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open CIFP directory %s\n", fname.c_str());

		return -1;
	}

	size_t found  = fname.find("CIFP");
	if ( found != string::npos){
		return this->loadAirwaysCIFP(fname,airways);
	}
}


int NatsDataLoader::loadSidStars(const string& fname,
		vector<NatsSid>* const sids,
		vector<NatsStar>* const stars,
		vector<NatsApproach>* const approaches) {
	if (!sids) return 0;
	if (!stars) return 0;
	if (!approaches) return 0;

	// If we can't access the file
	if ((fname.length() == 0) || ( access( fname.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open CIFP file %s\n", fname.c_str());

		return -1;
	}

	size_t found = fname.find("CIFP");
	if (found == string::npos) {
		printf("      Not valid CIFP file: %s\n", fname.c_str());

		return -1;
	} else {
		return loadProcsCIFP<NatsSid>(fname,sids) + loadProcsCIFP<NatsStar>(fname,stars)
				+ loadProcsCIFP<NatsApproach>(fname,approaches);
	}
}

int NatsDataLoader::loadPars(const string& fname,
		vector<NatsPar>* const pars) {
	if (!pars) return 0;

	int lineno = 0;
	int maxlen = 200;
	char line[200];
	int dataStartLine = 0;

	vector<string> vectorDecrypted = getVector_decrypted(fname.c_str());

	NatsPar par;

	if (0 < vectorDecrypted.size()) {
		for (unsigned int i = 0; i < vectorDecrypted.size(); i++) {
			// read the next line. close file and exit if error
			lineno++;

			memset(line, 0, maxlen*sizeof(char));

			strcpy(line, vectorDecrypted.at(i).c_str());

			// if the line starts with # then its a comment.  skip.
			if (line[0] == '#') continue;

			/// skip blank lines
			if (strlen(line) < 1) continue;

			// data starts on line 0
			if (lineno < dataStartLine) continue;

			// assume data lines have no leading whitespace
			if (isspace(line[0])) continue;

			// the data is stored in 2 columns.
			// column1 is a label, column2 is a value
			char* saveptr=NULL;
			string label;
			string value;

			char* tok = strtok_r(line, " \t\n", &saveptr);
			if (!tok) continue;

			label = string(tok);
			tok = strtok_r(NULL, " \t\n", &saveptr);
			if (!tok) continue;

			value = string(tok);

			// if label is par, then add the current par to the vector
			// and start a new one.
			// note: we ignore the AIRPORT and ALT labels.
			if (label == "PAR") {
				if (par.name.length() > 0) {

					// verify that we have an identifier.  if no identifier
					// then use the first waypoint name.
					if (par.identifier.length() < 1) {
						par.identifier = par.waypoints.front();
					}
					// add to output
					pars->push_back(par);

					// reset
					par.name = "";
					par.identifier = "";
					par.waypoints.clear();
				}
				par.name = value;
				continue;
			} else if (label == "FIELD_10") {
				// trim potential leading and trailing '.' chars
				// from the identifier.
				while (value.at(0) == '.') {
					value.erase(0,1);
				}
				while (value.at(value.length()-1) == '.') {
					value.erase(value.length()-1, 1);
				}
				par.identifier = value;
				continue;
			} else if (label == "FIX" || label == "FIX-T") {
				par.waypoints.push_back(value);
				continue;
			}
		}
	}

	// sort the pars by identifier
	sort(pars->begin(), pars->end());

	return pars->size();
}

int NatsDataLoader::loadAirportsCIFP(const string& fname, vector<NatsAirport>* airports){


	string line= "";

	/*
	 * THE FIELD LOCATIONS CAN BE INPUTTED FROM A CONFIG FILE AS
	 * IS DONE IN ROUTE GENERATOR. HERE THEY ARE HARDCODED FOR SIMPLICITY
	 * FIXME: INPUT FROM A CONFIG FILE.
	 */

	int secCodeFld = 5;	string secCode = "P";
	int subSecCodeFld = 13; string subSecCode = "A";
	int aptcodestart = 7; int aptcodeend = 10;
	int latstart=33;int latend=41;
	int lonstart=42;int lonend=51;
	int elevstart=57;int elevend = 61;
	int magvarstart=52;int magvarend=56;
	int aptnamestart = 94; int aptnameend = 123;
	/*
	 * TILL HERE.
	 */
	std::ifstream myfile (fname.c_str());

	if (myfile.is_open()){
		while ( getline (myfile,line) ){
			if ((line.length() > 10)
					){

				if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						(line.compare(subSecCodeFld-1,1,subSecCode ) == 0) ){

					string aptcode = line.substr(aptcodestart-1,aptcodeend-aptcodestart+1);

					double lat, lon, ele, mvar;
					string lati = line.substr(latstart-1,latend-latstart+1);
					convertLatLon(lati,lat,true);
					string longi = line.substr(lonstart-1,lonend-lonstart+1);
					convertLatLon(longi,lon,false);
					string elev = line.substr(elevstart-1,elevend-elevstart+1);
					convertElev(elev,ele);
					string magvar = line.substr(magvarstart-1,magvarend-magvarstart+1);
					convertMagVarDec(magvar,mvar);


					string aptname = line.substr(aptnamestart-1,aptnameend-aptnamestart+1);

					NatsAirport airport;
					airport.name = aptname; airport.code = aptcode;
					airport.latitude = lat; airport.longitude = lon;airport.elevation = ele;
					airport.mag_variation = mvar;
					airports->push_back(airport);

				}
			}
		}
		myfile.close();
	}

	return airports->size();
}

int NatsDataLoader::loadWaypointsCIFP(const string& fname,
		vector<NatsWaypoint>* const waypoints){
	string line = "";

	/*
	 * THE FIELD LOCATIONS CAN BE INPUTTED FROM A CONFIG FILE AS
	 * IS DONE IN ROUTE GENERATOR. HERE THEY ARE HARDCODED FOR SIMPLICITY
	 * FIXME: INPUT FROM A CONFIG FILE.
	 */
	string secCodeWp = "E"; string secCodeNa = "D";string secCodeTerm = "P";
	int secCodeFld = 5;
	string subSecCodeWp = "A";
	string subSecCodeTrWp = "C";string subSecCodeTrNa = "N";string subSecCodeTrRw = "G";
	int subSecCodeFld = 6; int subSecCodeFldTermWp = 13;
	int apnamestart = 7; int apnameend = 10;
	int fixnamestart = 14; int fixnameend = 18;
	int latstart = 33;int latend = 41;
	int lonstart = 42;int lonend = 51;
	int dmelatstart = 56;int dmelatend = 64;
	int dmelonstart = 65;int dmelonend = 74;
	int magvarstart = 75;int magvarend = 79;
	/*
	 * TILL HERE
	 */

	std::ifstream myfile (fname.c_str());
	if (myfile.is_open()){
		while ( getline (myfile,line) ){

			if ( (line.length() > 10)
					   ){

				//ENRT FIX

				string name = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
				trim(name);
				double lat=0, lon=0;
				string lati = line.substr(latstart-1,latend-latstart+1);
				convertLatLon(lati,lat,true);
				string longi = line.substr(lonstart-1,lonend-lonstart+1);
				convertLatLon(longi,lon,false);
				string mag_var_dec_str = line.substr(magvarstart-1,magvarend-magvarstart+1);
				double mag_var_dec=0;
				convertMagVarDec(mag_var_dec_str,mag_var_dec);


				if ( (line.compare(secCodeFld-1,1,secCodeWp) == 0)
						&&	(line.compare(subSecCodeFld-1,1,subSecCodeWp )==0) ){

					NatsWaypoint waypoint; waypoint.name = name;
					waypoint.latitude = lat; waypoint.longitude = lon;
					waypoint.mag_var_dec = mag_var_dec;
					waypoints->push_back(waypoint);
				}
				// NAVAIDS
				else if (line.compare(secCodeFld-1,1,secCodeNa) == 0){

					if (lati.length()==0){
						lati = line.substr(dmelatstart-1,dmelatend-dmelatstart+1);
						convertLatLon(lati,lat,true);
					}
					if (longi.length()==0){
						longi = line.substr(dmelonstart-1,dmelonend-dmelonstart+1);
						convertLatLon(longi,lon,false);
					}

					NatsWaypoint waypoint; waypoint.name = name;
					waypoint.latitude = lat; waypoint.longitude = lon;
					waypoint.mag_var_dec = mag_var_dec;
					waypoints->push_back(waypoint);

				}
				//TERMINAL FIX
				else if (line.compare(secCodeFld-1,1,secCodeTerm) == 0 &&
						( (line.compare(subSecCodeFldTermWp-1,1,subSecCodeTrWp )==0) ||
							(line.compare(subSecCodeFldTermWp-1,1,subSecCodeTrRw) == 0) ||
							(line.compare(subSecCodeFld-1,1,subSecCodeTrNa) == 0) ) ){
					string airport = line.substr(apnamestart-1,apnameend-apnamestart+1);
					if (line.compare(subSecCodeFldTermWp-1,1,subSecCodeTrRw) == 0)
						name = name+"-"+airport;

					NatsWaypoint waypoint; waypoint.name = name;
					waypoint.latitude = lat; waypoint.longitude = lon;
					waypoint.mag_var_dec = mag_var_dec;
					waypoints->push_back(waypoint);
				}
			}//if (line.length() > 10)
		}//while ( getline (myfile,line) )
		myfile.close();
	}//if (myfile.is_open())

	return waypoints->size();
}

int NatsDataLoader::loadAirwaysCIFP(const string& fname, vector<NatsAirway>* const airways){

	string line = "";
	/*
	 * THE FIELD LOCATIONS CAN BE INPUTTED FROM A CONFIG FILE AS
	 * IS DONE IN ROUTE GENERATOR. HERE THEY ARE HARDCODED FOR SIMPLICITY
	 * FIXME: INPUT FROM A CONFIG FILE.
	 */
	string secCode="E"; string subSecCode="R";
	int secCodeFld=5; int subSecCodeFld=6;
	int routenamestart=14; int routenameend=18;
	int fixnamestart=30; int fixnameend=34;
	/*
	 * TILL HERE.
	 */

	vector<string> routename;routename.clear();
	vector<string> wpname; wpname.clear();
	string prevname = "None";

	std::ifstream myfile (fname.c_str());
	if (myfile.is_open()){
		while ( getline (myfile,line) ){
			if ( (line.length() > 10) ){
				if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						(line.compare(subSecCodeFld-1,1,subSecCode )==0) ){

					string name = line.substr(routenamestart-1,routenameend-routenamestart+1);
					trim(name);
					vector<string>::iterator it = find(routename.begin(),routename.end(),name);
					if(it == routename.end() ){
						if (!routename.empty()){
							NatsAirway airway; airway.name = prevname;
							airway.route = wpname;
							airways->push_back(airway);
							wpname.clear();
							prevname = name;
						}
						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						trim(fixname);

						wpname.push_back(fixname);
						routename.push_back(name);
					}
					else{
						string fixname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
						trim(fixname);

						wpname.push_back(fixname);
					}

				}
			}
		}

		myfile.close();
	}

	return airways->size();
}


template<typename T>
int NatsDataLoader::loadProcsCIFP(const string& fname, vector<T>* const procs){
	string line = "";

	string secCode="P"; string subSecCode="";
	int secCodeFld=5; int subSecCodeFld=13;
	int aptcodestart=7; int aptcodeend=10;
	int procnamestart=14; int procnameend=19;
	int rttypefld = 20;
	int transidstart=21;int transidend=25;
	int seqnostart=27; int seqnoend=29;
	int fixnamestart=30; int fixnameend=34;
	int pathtermstart=48; int pathtermend=49;
	int recconavstart=51; int recconavend=54;
	int thetastart=63;int thetaend=66;
	int rhostart=67;int rhoend=71;
	int magcoursestart=71; int magcourseend=74;
	int routediststart=75; int routedistend=78;
	int altdescfld = 83;
	int alt1start = 85; int alt1end = 89;
	int alt2start = 90; int alt2end = 94;
	int spdlimstart = 100; int spdlimend = 102;
	int spdlimdescfld = 118;

	if (typeid(T) == typeid(NatsSid))
		subSecCode = "D";
	else if (typeid(T) == typeid(NatsStar))
		subSecCode = "E";
	else
		subSecCode = "F";


	 vector<string> waypoints;
	 map<string,pair<double,double> > runway_to_fdf_course_alt;
	 map<string, vector<string> > wp_map;
	 map<string, pair<string,string> > route_to_trans_rttype;
	 map<string, vector<pair<string,string> > > path_term;
	 map<string, vector<pair<string,string> > > alt_desc;
	 map<string, vector<pair<string,double> > > alt_1;
	 map<string, vector<pair<string,double> > > alt_2;
	 map<string, vector<pair<string,double> > > spd_limit;
	 map<string, vector<pair<string,string> > > recco_nav;
	 map<string, vector<pair<string,double> > > theta;
	 map<string, vector<pair<string,double> > > rho;
	 map<string, vector<pair<string,double> > > mag_course;
	 map<string, vector<pair<string,double> > > rt_dist;
	 map<string, vector<pair<string,string> > > spdlim_desc;

	 int prevseqno = 10000;
	 string currap = "";
	 string currproc = "";
	 string currroute = "";
	 string prevwpname = "";
	 string prevtrans = "";
	 string prevrwy = "";
	 string runway = "";
	 int rtcount = 0;



	 std::ifstream myfile (fname.c_str());
	 if (!myfile.is_open()) {
		 printf("      Failed to open file %s\n", fname.c_str());

		 return -1;
	 } else {
		 while ( getline (myfile,line) ){
			 if ((line.length() > 10)
					 ){
				 if ( (line.compare(secCodeFld-1,1,secCode) == 0) &&
						 (line.compare(subSecCodeFld-1,1,subSecCode ) == 0) ) {
					 string airport = line.substr(aptcodestart-1,aptcodeend-aptcodestart+1);
					 trim(airport);

					 string procname = line.substr(procnamestart-1,procnameend-procnamestart+1);
					 trim(procname);

					 string rttype = line.substr(rttypefld-1,1);

					 string wpname = line.substr(fixnamestart-1,fixnameend-fixnamestart+1);
					 trim(wpname);

					 string trans = line.substr(transidstart-1,transidend-transidstart+1);
					 trim(trans);

					 string seqnostr = line.substr(seqnostart-1,seqnoend-seqnostart+1);
					 trim(seqnostr);

					 string pterm =line.substr(pathtermstart-1,pathtermend-pathtermstart+1);
					 trim(pterm);

					 string recconavstr = line.substr(recconavstart-1,recconavend-recconavstart+1);
					 trim(recconavstr);

					 string thetastr = line.substr(thetastart-1,thetaend-thetastart+1);
					 trim(thetastr); double theta_val = parse_str(thetastr)/10.0;

					 string rhostr = line.substr(rhostart-1,rhoend-rhostart+1);
					 trim(rhostr); double rho_val = parse_str(rhostr)/10.0;

					 string magcoursestr =line.substr(magcoursestart-1, magcourseend-magcoursestart+1);
					 trim(magcoursestr); double magcourse = parse_str(magcoursestr)/10.0;

					 string routediststr = line.substr(routediststart-1,routedistend-routediststart+1);
					 trim(routediststr); double rtdist_nm = parse_str(routediststr)/10.0;

					 string altdescstr = line.substr(altdescfld-1,1);
					 trim(altdescstr); if (altdescstr == "") altdescstr = "NONE";

					 string alt1str = line.substr(alt1start-1,alt1end-alt1start+1);
					 trim(alt1str);double alt1 = parse_alt_string(alt1str);

					 string alt2str = line.substr(alt2start-1,alt2end-alt2start+1);
					 trim(alt2str);double alt2 = parse_alt_string(alt2str);

					 string speedlimstr = line.substr(spdlimstart-1,spdlimend-spdlimstart+1);
					 trim(speedlimstr); double spdlim = -10000;
					 if (speedlimstr.length() != 0)
						 spdlim = atof(speedlimstr.c_str());

					 string spdlimdescstr = line.substr(spdlimdescfld-1,1);
					 trim(spdlimdescstr); if (spdlimdescstr == "") spdlimdescstr = "NONE";

					 int seqno = atoi(seqnostr.c_str());

					 if (wpname.length() == 0 && trans.length() == 0){
/*				FIXME: WHEN THESE TWO ARE NOT GIVEN CREATES ERROR*/
 						 if (pterm.compare("VI") == 0
							|| pterm.compare("VA") == 0
							|| pterm.compare("VD") == 0
							|| pterm.compare("VR") == 0
							|| pterm.compare("CI") == 0
							|| pterm.compare("CA") == 0
							|| pterm.compare("CD") == 0
							|| pterm.compare("CR") == 0
								 ){
							 trans = prevtrans;
							 wpname = "NO_WP_NO_TRANS_ID";
						 }
						 else{
							 cout << " in " << __LINE__ <<" file " <<__FILE__ <<endl;
							 cout <<line <<endl;

							 continue;
						 }
					 }

					 if (trans.length()){
						 if (trans=="ALL"){
							 trans = "RW" + trans +'-'+airport;
							 if(typeid(T) == typeid(NatsSid)
									 && prevtrans.find("ALL") == string::npos
									 && prevtrans != trans
									 ){
								 wpname = trans;
							 }
						 }
					 }

					if (typeid(T) == typeid(NatsApproach)) {
						string rnum =  procname.substr(2-1,3-2+1);
						string dir = procname.substr(4-1,1);
						runway = rnum;
						if(dir != "-")
							runway += dir;
					}

					 bool sidRWFlag = false;
					 if (rttype == "1" || rttype == "4" || rttype == "F" || rttype == "T")
						 sidRWFlag = true;
					 bool starRWFlag = false;
					 if (rttype == "3" || rttype == "6" || rttype == "9" || rttype == "S")
						 starRWFlag = true;

					 if (wpname.length() == 0 &&
						  ((sidRWFlag && typeid(T) == typeid(NatsSid)) ||
								  (starRWFlag && typeid(T) == typeid(NatsStar)))
								  ){
						 if ( (trans.compare(0,2,"RW") == 0)
								 && isInteger(trans.substr(2,2))){
							 string lastchar = trans.substr(trans.length()-1,1);

	 						 if (pterm.compare("VI") == 0
								|| pterm.compare("VA") == 0
								|| pterm.compare("VM") == 0
								|| pterm.compare("VD") == 0
								|| pterm.compare("VR") == 0
								|| pterm.compare("CI") == 0
								|| pterm.compare("CA") == 0
								|| pterm.compare("CD") == 0
								|| pterm.compare("CR") == 0
									 ){
	 							 if (seqno == 10){
	 								 wpname = trans+'-'+airport;
	 							 }
	 							 else{
	 								 wpname = prevwpname + "-NO_WP_NAME";
	 							 }
	 						 }
							 else{
								 cout << " in " << __LINE__ <<" file " <<__FILE__ <<endl;
							 	 cout <<line <<endl;
							 }
						 }
						 else{
							 wpname = trans;
						 }
					 }

					 if (typeid(T) == typeid(NatsApproach)){
						 if (wpname.length() == 0){
							 if ( (pterm.compare("VI") == 0)
								|| (pterm.compare("CI") == 0)
								||(pterm.compare("PI") == 0)){
								 wpname = "INTERCEPT_POINT";
							 }
							 else{
								 cout << " in " << __LINE__ <<" file " <<__FILE__ <<endl;
								 cout <<line <<endl;
								 continue;
							 }
						 }
						 if (wpname.compare(0,2,"RW") == 0 &&
								 isInteger(wpname.substr(2,2)) )
							 wpname = wpname+'-'+airport;
					 }

					if (wpname.length() == 0 &&
							((!sidRWFlag && typeid(T) == typeid(NatsSid) )
									|| (!starRWFlag && typeid(T) == typeid(NatsStar))) ){

						if (trans.compare(0,2,"RW")
								&& isInteger(trans.substr(2,2) ) ){
							cout << " ERROR in " << __LINE__ <<" file " <<__FILE__ <<endl;
							cout <<line <<endl<<endl;
							continue;
						}

						if (trans.length()){
							string lastchar = trans.substr(trans.length()-1,1);

							if (pterm.compare("VI") == 0
									|| pterm.compare("VA") == 0
									|| pterm.compare("VM") == 0
									|| pterm.compare("VD") == 0
									|| pterm.compare("VR") == 0
									|| pterm.compare("CI") == 0
									|| pterm.compare("CA") == 0
									|| pterm.compare("CD") == 0
									|| pterm.compare("CR") == 0
									){
								wpname = trans+'-'+airport+ "-UNSPECI_TERM";
							}
	 						 else{
	 							 cout << " ERROR in " << __LINE__ <<" file " <<__FILE__ <<endl;
	 							 cout <<line <<endl<<endl;
	 						 }
						}
						else{
							printf(" ERROR: No transition or waypoint present for ap %s and proc %s...continuing\n",
									airport.c_str(),procname.c_str());
							continue;
						}
					}

                    if (trans.length()) {
                        if ((trans.compare(0,2,"RW") == 0)
                                && isInteger(trans.substr(2,2))) {
                            runway = trans;
                        }
                    }

					pair<string,string> trans_rttype;
					if (trans.length())
						trans_rttype = make_pair(trans,rttype);
					else
						trans_rttype = make_pair("BLANK-FIELD",rttype);

					if (currproc != procname){
						if (!waypoints.empty()){
							T proc;
							proc.waypoints = waypoints;
							proc.name = currproc;
							proc.id = currap;
							proc.wp_map = wp_map;
							proc.route_to_trans_rttype = route_to_trans_rttype;
							proc.runway_to_fdf_course_alt = runway_to_fdf_course_alt;
							proc.path_term = path_term;
							proc.alt_desc = alt_desc;
							proc.alt_1 = alt_1;
							proc.alt_2 = alt_2;
							proc.spd_limit = spd_limit;
							proc.recco_nav = recco_nav;
							proc.theta = theta;
							proc.rho = rho;
							proc.mag_course = mag_course;
							proc.rt_dist = rt_dist;
							proc.spdlim_desc = spdlim_desc;
							proc.runway = prevrwy;

							procs->push_back(proc);
						}

						currap = airport;
						currproc = procname;

						if (pterm.compare("VI") == 0
								|| pterm.compare("VA") == 0
								|| pterm.compare("VM") == 0
								|| pterm.compare("VD") == 0
								|| pterm.compare("VR") == 0
								|| pterm.compare("CI") == 0
								|| pterm.compare("CA") == 0
								|| pterm.compare("CD") == 0
								|| pterm.compare("CR") == 0
								){
							if (typeid(T) == typeid(NatsSid)){
								currroute = trans+'-'+airport;
							}
						}
						else{
							if (
								typeid(T) == typeid(NatsSid)
								&& trans.length()
									){
								currroute = trans+'-'+airport;
							}
							else{
								currroute = wpname;
							}
						}

						waypoints.clear();
						waypoints.push_back(wpname);

						wp_map.clear();
						wp_map[currroute].push_back(wpname);

						runway_to_fdf_course_alt.clear();

						if (typeid(T) == typeid(NatsSid)){
							if ( ( pterm.compare(0,2,"VA") == 0
									|| pterm.compare(0,2,"CA") == 0
									|| pterm.compare(0,2,"FA") == 0)
									&& (trans.compare(0,2,"RW") == 0 && isInteger(trans.substr(2,2)) )
									&& wpname.compare(0,trans.length(),trans) == 0
									){

								if (alt1 > 0){
									pair<double,double> course_alt = make_pair(magcourse,alt1);
									runway_to_fdf_course_alt[wpname] = course_alt;
								}
								else if (alt2 > 0){
									pair<double,double> course_alt = make_pair(magcourse,alt2);
									runway_to_fdf_course_alt[wpname] = course_alt;
								}
							}
						}

						route_to_trans_rttype.clear();
						route_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));
						path_term.clear();
						path_term[currroute].push_back(pair<string,string>(wpname,pterm));
						alt_desc.clear();

						alt_desc[currroute].push_back(pair<string,string>(wpname,altdescstr));
						alt_1.clear();

						alt_1[currroute].push_back(pair<string,double>(wpname,alt1));
						alt_2.clear();

						alt_2[currroute].push_back(pair<string,double>(wpname,alt2));
						spd_limit.clear();

						spd_limit[currroute].push_back(pair<string,double>(wpname,spdlim));

						recco_nav.clear();
						recco_nav[currroute].push_back(pair<string,string>(wpname,recconavstr));

						theta.clear();
						theta[currroute].push_back(pair<string,double>(wpname,theta_val));

						rho.clear();
						rho[currroute].push_back(pair<string,double>(wpname,rho_val));

						mag_course.clear();
						mag_course[currroute].push_back(pair<string,double>(wpname,magcourse));

						rt_dist.clear();
						rt_dist[currroute].push_back(pair<string,double>(wpname,rtdist_nm));

						spdlim_desc.clear();
						spdlim_desc[currroute].push_back(pair<string,string>(wpname,spdlimdescstr));

						rtcount = 0;
					}
					else {
						waypoints.push_back(wpname);

						if ((!sidRWFlag && typeid(T) == typeid(NatsSid))
								|| typeid(T) == typeid(NatsStar)
								|| typeid(T) == typeid(NatsApproach)){
							if(prevseqno > seqno){
								if (wp_map[wpname].empty()){
									currroute = wpname;
									rtcount = 0;

								}
								else{
									rtcount++;
									std::stringstream ss;
									ss << rtcount;
									string stradd = ss.str();
									currroute = wpname+stradd;
								}

							}

						wp_map[currroute].push_back(wpname);

						}
						else{
							if (trans.length()){

								string lastchar = trans.substr(trans.length()-1,1);
							}
							else{
								cout<<" transition name  length = 0"<<endl;
							}
							if (typeid(T) == typeid(NatsSid)){
								currroute = trans+'-'+airport;
								vector<string>::iterator itr =
										find(wp_map[currroute].begin(),wp_map[currroute].end(),
												currroute );

								if (itr == wp_map[currroute].end()
										){
									if (!wpname.length()){
										wp_map[currroute].push_back(currroute);
									}
									else if (wpname.compare(0,2,"RW") == 0 &&
											 isInteger(wpname.substr(2,2)) ){
										wp_map[currroute].push_back(currroute);
									}
								}
								if (wpname.length() && currroute != wpname){
									wp_map[currroute].push_back(wpname);
								}
								rtcount = 0;
							}
							else{
								cerr<<"Procedure not found.  Exception occurred"<<endl;
								exit(EXIT_SUCCESS);
							}
						}

						route_to_trans_rttype.insert(pair<string,pair<string,string> >(currroute,trans_rttype));

						if (typeid(T) == typeid(NatsSid)){
							if ( ( pterm.compare(0,2,"VA") == 0 || pterm.compare(0,2,"CA") == 0 || pterm.compare(0,2,"FA") == 0)
									&& (trans.compare(0,2,"RW") == 0 && isInteger(trans.substr(2,2)) )
									&& wpname.compare(0,trans.length(),trans) == 0
									){

								if (alt1 > 0){
									pair<double,double> course_alt = make_pair(magcourse,alt1);
									runway_to_fdf_course_alt[wpname] = course_alt;
								}
								else if (alt2 > 0){
									pair<double,double> course_alt = make_pair(magcourse,alt2);
									runway_to_fdf_course_alt[wpname] = course_alt;
								}
							}
						}

						path_term[currroute].push_back(pair<string,string>(wpname,pterm));

						alt_desc[currroute].push_back(pair<string,string>(wpname,altdescstr));

						alt_1[currroute].push_back(pair<string,double>(wpname,alt1));

						alt_2[currroute].push_back(pair<string,double>(wpname,alt2));

						spd_limit[currroute].push_back(pair<string,double>(wpname,spdlim));

						recco_nav[currroute].push_back(pair<string,string>(wpname,recconavstr));

						theta[currroute].push_back(pair<string,double>(wpname,theta_val));

						rho[currroute].push_back(pair<string,double>(wpname,rho_val));

						mag_course[currroute].push_back(pair<string,double>(wpname,magcourse));

						rt_dist[currroute].push_back(pair<string,double>(wpname,rtdist_nm));

						spdlim_desc[currroute].push_back(pair<string,string>(wpname,spdlimdescstr));
					}//if (currproc != procname){

					prevtrans = trans;
					prevrwy = runway;
					prevseqno = seqno;
					prevwpname = wpname;
				  }//if ( (line.compare(secCodeFld-1,1,secCode) == 0) && ...

			  }// if ((line.length() > 10) && ( line.compare(1,3,"USA") == 0 ))

		  }//while ( getline (myfile,line) )

		  myfile.close();
	  }

	  return procs->size();
 }
